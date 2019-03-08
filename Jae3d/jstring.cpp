#include "jstring.hpp"

#include <cstring>
#include <stdexcept>
#include <algorithm>

jstring::jstring() : mLength(0), mCapacity(1) {
	mCStr = new char[1];
	mCStr[0] = '\0';
}
jstring::jstring(const jstring &str) : mLength(str.mLength), mCapacity(str.mLength + 1) {
	mCStr = new char[str.mLength + 1];
	if (str.mLength == 0)
		mCStr[0] = '\0';
	else
		strcpy_s(mCStr, str.mLength + 1, str.mCStr);
}
jstring::jstring(jstring &&mvstr) {
	mCStr = mvstr.mCStr;
	mLength = mvstr.mLength;
	mCapacity = mvstr.mCapacity;
	mvstr.mCStr = new char[1];
	mvstr.mCStr[0] = '\0';
	mvstr.mLength = 0;
	mvstr.mCapacity = 1;
}
jstring::jstring(const size_t length) : mLength(0) {
	if (length < 1024) {
		mCapacity = length + 1;
		mCStr = new char[length + 1];
		ZeroMemory(mCStr, length + 1);
	} else {
		mCStr = new char[1];
		mCStr[0] = '\0';
		mLength = 0;
		mCapacity = 1;
	}
}
jstring::jstring(const char* str) {
	if (str) {
		mLength = strlen(str);
		mCapacity = mLength + 1;
		mCStr = new char[mCapacity];
		strcpy_s(mCStr, mCapacity, str);
	} else {
		mCStr = new char[1];
		mCStr[0] = '\0';
		mLength = 0;
		mCapacity = 1;
	}
}
jstring::jstring(const char* start, const size_t length) {
	if (start && length < 1024) {
		mLength = length;
		mCapacity = mLength + 1;
		mCStr = new char[mCapacity];
		strncpy_s(mCStr, mCapacity, start, mLength);
		mCStr[mLength] = '\0';
	} else {
		mCStr = new char[1];
		mCStr[0] = '\0';
		mLength = 0;
		mCapacity = 1;
	}
}
jstring::~jstring() {
	if (mCStr) delete[] mCStr;
}

void jstring::reserve(size_t cap) {
	if (cap < mCapacity) return;
	char* nstr = new char[cap];
	if (mCStr) {
		strcpy_s(nstr, cap, mCStr);
		delete[] mCStr;
	}
	mCStr = nstr;
	mCapacity = cap;
}

jstring jstring::substr(int pos) const {
	if (pos >= 0)
		return jstring(mCStr + pos, mLength - pos);
	else
		return jstring(mCStr + mLength + pos, -pos);
}
jstring jstring::substr(size_t pos, size_t length) const {
	return jstring(mCStr + pos, length);
}
jstring jstring::lower() const {
	jstring j = *this;
	_strlwr_s(j.mCStr, j.mCapacity);
	return j;
}
jstring jstring::upper() const {
	jstring j = *this;
	_strupr_s(j.mCStr, j.mCapacity);
	return j;
}

size_t jstring::find(const char c) const {
	const char* i = strchr(mCStr, c);
	if (i == nullptr) return npos;
	return i - mCStr;
}
size_t jstring::rfind(const char c) const {
	const char* i = strrchr(mCStr, c);
	if (i == nullptr) return npos;
	return i - mCStr;
}

char const& jstring::operator [](int i) const {
	if (i < 0 || i >= mCapacity) throw std::out_of_range("Index out of bounds");
	return mCStr[i];
}
char& jstring::operator [](int i) {
	if (i < 0 || i >= mCapacity) throw std::out_of_range("Index out of bounds");
	return mCStr[i];
}

jstring& jstring::operator +=(const char rhs) {
	reserve(mLength + 2);
	mCStr[mLength++] = rhs;
	mCStr[mLength] = '\0';
	return *this;
}
jstring& jstring::operator +=(const char* rhs) {
	size_t l = strlen(rhs);
	reserve(mLength + l + 1);
	strcpy_s(mCStr + mLength, mCapacity - mLength, rhs);
	mLength += l;
	return *this;
}
jstring& jstring::operator +=(const jstring &rhs) {
	reserve(mLength + rhs.mLength + 1);
	strcpy_s(mCStr + mLength, mCapacity - mLength, rhs.mCStr);
	mLength += rhs.mLength;
	return *this;
}

jstring& jstring::operator =(const jstring& str) {
	if (this == &str) return *this;

	if (str.mLength + 1 > mCapacity)
		reserve(str.mLength + 1);
	else if (str.mLength + 1 < mCapacity) {
		if (mCStr) delete[] mCStr;
		mCapacity = str.mLength + 1;
		mCStr = new char[mCapacity];
	}

	strcpy_s(mCStr, mCapacity, str.mCStr);
	mLength = str.mLength;
	return *this;
}
jstring& jstring::operator =(jstring&& mvstr) {
	if (this == &mvstr) return *this;
	if (mCStr) delete[] mCStr;
	mCStr = mvstr.mCStr;
	mLength = mvstr.mLength;
	mCapacity = mvstr.mCapacity;
	mvstr.mCStr = new char[1];
	mvstr.mCStr[0] = '\0';
	mvstr.mLength = 0;
	mvstr.mCapacity = 1;
	return *this;
}
jstring& jstring::operator =(const char* cstr) {
	if (mCStr == cstr) return *this;
	size_t l = strlen(cstr);
	if (l + 1 > mCapacity)
		reserve(l + 1);
	else if (l + 1 < mCapacity) {
		delete[] mCStr;
		mCStr = new char[l + 1];
		mCapacity = l + 1;
	}
	strcpy_s(mCStr, mCapacity, cstr);
	mLength = l;
	return *this;
}

jstring jstring::operator +(const char rhs) {
	jstring s(mLength + 2);
	strcpy_s(s.mCStr, s.mCapacity, mCStr);
	s.mLength = mLength;
	s.mCStr[s.mLength++] = rhs;
	s.mCStr[s.mLength] = '\0';
	return s;
}
jstring jstring::operator +(const char* rhs) {
	size_t rlen = strlen(rhs);
	jstring s = jstring(mLength + rlen);
	strcpy_s(s.mCStr, s.mCapacity, mCStr);
	strcpy_s(s.mCStr + mLength, s.mCapacity - mLength, rhs);
	s.mLength = mLength + rlen;
	return s;
}
jstring jstring::operator +(const jstring &rhs) {
	jstring s(mLength + rhs.mLength + 1);
	strcpy_s(s.mCStr, s.mCapacity, mCStr);
	strcpy_s(s.mCStr + mLength, s.mCapacity - mLength, rhs.mCStr);
	s.mLength = mLength + rhs.mLength;
	return s;
}

jstring operator +(const char lhs, const jstring &rhs) {
	jstring s(rhs.mLength + 1);
	s.mCStr[0] = lhs;
	strcpy_s(s.mCStr + 1, s.mCapacity - 1, rhs.mCStr);
	return s;
}
jstring operator +(const char* lhs, const jstring &rhs) {
	size_t l = strlen(lhs);
	jstring s(rhs.mLength + l + 1);
	s.mLength = rhs.mLength + l;
	strcpy_s(s.mCStr, s.mCapacity, lhs);
	strcpy_s(s.mCStr + l, s.mCapacity - l, rhs.mCStr);
	return s;
}

jstring operator +(const jstring &lhs, const char rhs) {
	jstring s(lhs.mLength + 1);
	strcpy_s(s.mCStr, s.mCapacity - 1, lhs.mCStr);
	s.mCStr[lhs.mLength] = rhs;
	return s;
}
jstring operator +(const jstring &lhs, const char* rhs) {
	size_t l = strlen(rhs);
	jstring s(lhs.mLength + l + 1);
	s.mLength = lhs.mLength + l;
	strcpy_s(s.mCStr, s.mCapacity, lhs.mCStr);
	strcpy_s(s.mCStr + lhs.mLength, s.mCapacity - lhs.mLength, rhs);
	return s;
}