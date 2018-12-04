#include "jstring.hpp"

#include <cstring>
#include <stdexcept>
#include <algorithm>

jwstring::jwstring() : mLength(0), mCapacity(1) {
	mCStr = new wchar_t[sizeof(wchar_t)];
	mCStr[0] = L'\0';
}
jwstring::jwstring(const jwstring &str) : mLength(str.mLength), mCapacity(str.mCapacity) {
	mCStr = new wchar_t[(str.mLength + 1) * sizeof(wchar_t)];
	wcscpy_s(mCStr, str.mLength + 1, str.mCStr);
}
jwstring::jwstring(jwstring &&mvstr) {
	mCStr = mvstr.mCStr;
	mLength = mvstr.mLength;
	mCapacity = mvstr.mCapacity;
	mvstr.mCStr = new wchar_t[sizeof(wchar_t)];
	mvstr.mCStr[0] = L'\0';
	mvstr.mLength = 0;
	mvstr.mCapacity = 1;
}
jwstring::jwstring(const size_t length) : mLength(0), mCapacity(length + 1) {
	mCStr = new wchar_t[(length + 1) * sizeof(wchar_t)];
}
jwstring::jwstring(const wchar_t* str) {
	mLength = wcslen(str);
	mCapacity = mLength + 1;
	mCStr = new wchar_t[mCapacity * sizeof(wchar_t)];
	wcscpy_s(mCStr, mCapacity, str);
}
jwstring::jwstring(const wchar_t* start, const size_t length) {
	mLength = length;
	mCapacity = mLength + 1;
	mCStr = new wchar_t[mCapacity * sizeof(wchar_t)];
	wcsncpy_s(mCStr, mCapacity, start, mLength);
	mCStr[mLength] = L'\0';
}
jwstring::~jwstring() {
	delete[] mCStr;
}

void jwstring::reserve(size_t cap) {
	if (cap < mCapacity) return;
	wchar_t* nstr = new wchar_t[cap * sizeof(wchar_t)];
	wcscpy_s(nstr, cap, mCStr);
	delete[] mCStr;
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


size_t jwstring::find(const wchar_t c) const {
	return wcschr(mCStr, c) - mCStr;
}
size_t jwstring::rfind(const wchar_t c) const {
	return wcsrchr(mCStr, c) - mCStr;
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
	reserve(mLength + 1);
	mCStr[mLength++] = rhs;
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
	reserve(mLength + rhs.mLength + 2);
	wcscpy_s(mCStr + mLength, mCapacity - mLength, rhs.mCStr);
	mLength += rhs.mLength;
	return *this;
}

jwstring& jwstring::operator =(const jwstring& str) {
	if (this == &str) return *this;
	if (str.mLength + 1 > mCapacity)
		reserve(str.mLength + 1);
	else if (str.mLength + 1 < mCapacity) {
		delete[] mCStr;
		mCapacity = str.mLength + 1;
		mCStr = new wchar_t[mCapacity * sizeof(wchar_t)];
	}

	wcscpy_s(mCStr, mCapacity, str.mCStr);
	mLength = str.mLength;
	return *this;
}
jwstring& jwstring::operator =(jwstring&& mvstr) {
	if (this == &mvstr) return *this;
	delete[] mCStr;
	mCStr = mvstr.mCStr;
	mLength = mvstr.mLength;
	mCapacity = mvstr.mCapacity;
	mvstr.mCStr = new wchar_t[sizeof(wchar_t)];
	mvstr.mCStr[0] = L'\0';
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
		mCStr = new wchar_t[(l + 1) * sizeof(wchar_t)];
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
	size_t l = wcslen(rhs);
	jwstring s = jwstring(mLength + l + 1);
	wcscpy_s(s.mCStr, s.mCapacity, mCStr);
	wcscpy_s(s.mCStr + mLength, s.mCapacity - mLength, rhs);
	s.mLength = mLength + l;
	return s;
}
jwstring jwstring::operator +(const jwstring &rhs) {
	jwstring s(mLength + rhs.mLength + 1);
	wcscpy_s(s.mCStr, s.mCapacity, mCStr);
	wcscpy_s(s.mCStr + mLength, s.mCapacity - mLength, rhs.mCStr);
	s.mLength = mLength + rhs.mLength;
	return s;
}