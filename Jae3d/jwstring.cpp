#include "jstring.hpp"

#include <cstring>
#include <stdexcept>
#include <algorithm>

jwstring::jwstring() : mLength(0), mCapacity(1) {
	mCStr = new wchar_t[1];
	mCStr[0] = '\0';
}
jwstring::jwstring(const jwstring &str) : mLength(str.mLength), mCapacity(str.mLength + 1) {
	mCStr = new wchar_t[str.mLength + 1];
	if (str.mLength == 0)
		mCStr[0] = L'\0';
	else
		wcscpy_s(mCStr, str.mLength + 1, str.mCStr);
}
jwstring::jwstring(jwstring &&mvstr) {
	mCStr = mvstr.mCStr;
	mLength = mvstr.mLength;
	mCapacity = mvstr.mCapacity;
	mvstr.mCStr = new wchar_t[1];
	mvstr.mCStr[0] = L'\0';
	mvstr.mLength = 0;
	mvstr.mCapacity = 1;
}
jwstring::jwstring(const size_t length) : mLength(0) {
	mCapacity = length + 1;
	mCStr = new wchar_t[length + 1];
	ZeroMemory(mCStr, (length + 1) * sizeof(wchar_t));
}
jwstring::jwstring(const wchar_t* str) {
	if (str) {
		mLength = wcslen(str);
		mCapacity = mLength + 1;
		mCStr = new wchar_t[mCapacity];
		wcscpy_s(mCStr, mCapacity, str);
	} else {
		mCStr = new wchar_t[1];
		mCStr[0] = L'\0';
		mLength = 0;
		mCapacity = 1;
	}
}
jwstring::jwstring(const wchar_t* start, const size_t length) {
	mLength = length;
	mCapacity = mLength + 1;
	mCStr = new wchar_t[mCapacity];
	wcsncpy_s(mCStr, mCapacity, start, mLength);
	mCStr[mLength] = L'\0';
}
jwstring::~jwstring() {
	if (mCStr) delete[] mCStr;
}

void jwstring::reserve(size_t cap) {
	if (cap < mCapacity) return;
	wchar_t* nstr = new wchar_t[cap];
	if (mCStr) {
		wcscpy_s(nstr, cap, mCStr);
		delete[] mCStr;
	}
	mCStr = nstr;
	mCapacity = cap;
}

jwstring jwstring::substr(int pos) const {
	if (pos >= 0)
		return jwstring(mCStr + pos, mLength - pos);
	else
		return jwstring(mCStr + mLength + pos, -pos);
}
jwstring jwstring::substr(size_t pos, size_t length) const {
	return jwstring(mCStr + pos, length);
}
jwstring jwstring::lower() const {
	jwstring j = *this;
	_wcslwr_s(j.mCStr, j.mCapacity);
	return j;
}
jwstring jwstring::upper() const {
	jwstring j = *this;
	_wcsupr_s(j.mCStr, j.mCapacity);
	return j;
}
bool jwstring::startswith(const wchar_t* str) const {
	for (int i = 0; i < mLength; i++)
		if (str[i] == L'\0')
			return true;
		else if (str[i] != mCStr[i])
			return false;
	return true;
}
bool jwstring::startswith(jwstring str) const {
	return startswith(str.mCStr);
}

size_t jwstring::find(const wchar_t c) const {
	const wchar_t* i = wcschr(mCStr, c);
	if (i == nullptr) return npos;
	return i - mCStr;
}
size_t jwstring::rfind(const wchar_t c) const {
	const wchar_t* i = wcsrchr(mCStr, c);
	if (i == nullptr) return npos;
	return i - mCStr;
}

wchar_t const& jwstring::operator [](int i) const {
	if (i < 0 || i >= mCapacity) throw std::out_of_range("Index out of bounds");
	return mCStr[i];
}
wchar_t& jwstring::operator [](int i) {
	if (i < 0 || i >= mCapacity) throw std::out_of_range("Index out of bounds");
	return mCStr[i];
}

jwstring& jwstring::operator +=(const wchar_t rhs) {
	reserve(mLength + 2);
	mCStr[mLength++] = rhs;
	mCStr[mLength] = '\0';
	return *this;
}
jwstring& jwstring::operator +=(const wchar_t* rhs) {
	size_t l = wcslen(rhs);
	reserve(mLength + l + 1);
	wcscpy_s(mCStr + mLength, mCapacity - mLength, rhs);
	mLength += l;
	return *this;
}
jwstring& jwstring::operator +=(const jwstring &rhs) {
	reserve(mLength + rhs.mLength + 1);
	wcscpy_s(mCStr + mLength, mCapacity - mLength, rhs.mCStr);
	mLength += rhs.mLength;
	return *this;
}

jwstring& jwstring::operator =(const jwstring& str) {
	if (this == &str) return *this;

	if (str.mLength + 1 > mCapacity)
		reserve(str.mLength + 1);
	else if (str.mLength + 1 < mCapacity) {
		if (mCStr) delete[] mCStr;
		mCapacity = str.mLength + 1;
		mCStr = new wchar_t[mCapacity];
	}

	wcscpy_s(mCStr, mCapacity, str.mCStr);
	mLength = str.mLength;
	return *this;
}
jwstring& jwstring::operator =(jwstring&& mvstr) {
	if (this == &mvstr) return *this;
	if (mCStr) delete[] mCStr;
	mCStr = mvstr.mCStr;
	mLength = mvstr.mLength;
	mCapacity = mvstr.mCapacity;
	mvstr.mCStr = new wchar_t[1];
	mvstr.mCStr[0] = '\0';
	mvstr.mLength = 0;
	mvstr.mCapacity = 1;
	return *this;
}
jwstring& jwstring::operator =(const wchar_t* cstr) {
	if (mCStr == cstr) return *this;
	size_t l = wcslen(cstr);
	if (l + 1 > mCapacity)
		reserve(l + 1);
	else if (l + 1 < mCapacity) {
		delete[] mCStr;
		mCStr = new wchar_t[l + 1];
		mCapacity = l + 1;
	}
	wcscpy_s(mCStr, mCapacity, cstr);
	mLength = l;
	return *this;
}

jwstring jwstring::operator +(const wchar_t rhs) {
	jwstring s(mLength + 2);
	wcscpy_s(s.mCStr, s.mCapacity, mCStr);
	s.mLength = mLength;
	s.mCStr[s.mLength++] = rhs;
	s.mCStr[s.mLength] = '\0';
	return s;
}
jwstring jwstring::operator +(const wchar_t* rhs) {
	size_t rlen = wcslen(rhs);
	jwstring s = jwstring(mLength + rlen);
	wcscpy_s(s.mCStr, s.mCapacity, mCStr);
	wcscpy_s(s.mCStr + mLength, s.mCapacity - mLength, rhs);
	s.mLength = mLength + rlen;
	return s;
}
jwstring jwstring::operator +(const jwstring &rhs) {
	jwstring s(mLength + rhs.mLength + 1);
	wcscpy_s(s.mCStr, s.mCapacity, mCStr);
	wcscpy_s(s.mCStr + mLength, s.mCapacity - mLength, rhs.mCStr);
	s.mLength = mLength + rhs.mLength;
	return s;
}

jwstring operator +(const wchar_t lhs, const jwstring &rhs) {
	jwstring s(rhs.mLength + 1);
	s.mCStr[0] = lhs;
	wcscpy_s(s.mCStr + 1, s.mCapacity - 1, rhs.mCStr);
	return s;
}
jwstring operator +(const wchar_t* lhs, const jwstring &rhs) {
	size_t l = wcslen(lhs);
	jwstring s(rhs.mLength + l + 1);
	s.mLength = rhs.mLength + l;
	wcscpy_s(s.mCStr, s.mCapacity, lhs);
	wcscpy_s(s.mCStr + l, s.mCapacity - l, rhs.mCStr);
	return s;
}

jwstring operator +(int lhs, const jwstring& rhs) {
	wchar_t str[32];
	swprintf_s(str, 32, L"%d", lhs);
	return str + rhs;
}
jwstring operator +(const jwstring& lhs, int rhs) {
	wchar_t str[32];
	swprintf_s(str, 32, L"%d", rhs);
	return jwstring(lhs) + jwstring(str);
}

jwstring operator +(unsigned int lhs, const jwstring& rhs) {
	wchar_t str[32];
	swprintf_s(str, 32, L"%u", lhs);
	return str + rhs;
}
jwstring operator +(const jwstring& lhs, unsigned int rhs) {
	wchar_t str[32];
	swprintf_s(str, 32, L"%u", rhs);
	return jwstring(lhs) + jwstring(str);
}