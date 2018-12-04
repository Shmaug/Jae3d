#pragma once

#include "../Jae3d/Util.hpp"

#include <functional>

class jstring {
private:
	char* mCStr;
	size_t mLength;
	size_t mCapacity;

public:
	static constexpr auto npos{ static_cast<size_t>(-1) };

	jstring();
	jstring(const jstring &str);
	jstring(jstring &&mvstr);
	jstring(const size_t capacity);
	jstring(const char* str);
	jstring(const char* start, size_t length);
	~jstring();

	void reserve(size_t cap);

	bool empty() const { return mLength == 0; }
	const char* c_str() const { return mCStr; }
	size_t length() const { return mLength; }

	jstring substr(int pos) const;
	jstring substr(size_t pos, size_t length) const;
	jstring lower() const;
	jstring upper() const;

	size_t find(const char c) const;
	size_t rfind(const char c) const;

	char const& operator [](int i) const;
	char& operator [](int i);

	jstring& operator =(const jstring& str);
	jstring& operator =(jstring&& mvstr);
	jstring& operator =(const char* cstr);

	jstring& operator +=(const char rhs);
	jstring& operator +=(const char* rhs);
	jstring& operator +=(const jstring &rhs);

	jstring operator +(const char rhs);
	jstring operator +(const char* rhs);
	jstring operator +(const jstring &rhs);

	friend jstring operator +(const char lhs, const jstring &rhs);
	friend jstring operator +(const char* lhs, const jstring &rhs);

	friend bool operator >(const jstring &lhs, const jstring &rhs) { return strcmp(lhs.c_str(), rhs.c_str()) > 0; }
	friend bool operator <(const jstring &lhs, const char* rhs) { return strcmp(lhs.c_str(), rhs) < 0; }

	friend bool operator >=(const jstring &lhs, const jstring &rhs) { return strcmp(lhs.c_str(), rhs.c_str()) >= 0; }
	friend bool operator <=(const jstring &lhs, const char* rhs) { return strcmp(lhs.c_str(), rhs) <= 0; }

	friend bool operator ==(const jstring &lhs, const jstring &rhs) { return strcmp(lhs.c_str(), rhs.c_str()) == 0; }
	friend bool operator ==(const jstring &lhs, const char* rhs) { return strcmp(lhs.c_str(), rhs) == 0; }
	friend bool operator !=(const jstring &lhs, const jstring &rhs) { return !operator==(lhs, rhs); }
	friend bool operator !=(const jstring &lhs, const char* rhs) { return !operator==(lhs, rhs); }
};

class jwstring {
private:
	wchar_t* mCStr;
	size_t mLength;
	size_t mCapacity;

public:
	static constexpr auto npos{ static_cast<size_t>(-1) };

	jwstring();
	jwstring(const jwstring &str);
	jwstring(jwstring &&mvstr);
	jwstring(const size_t capacity);
	jwstring(const wchar_t* str);
	jwstring(const wchar_t* start, const size_t length);
	~jwstring();

	void reserve(size_t cap);

	bool empty() const { return mLength > 0; }
	const wchar_t* c_str() const { return mCStr; }
	size_t length() const { return mLength; }

	jwstring substr(int pos) const;
	jwstring substr(size_t pos, size_t length) const;

	size_t find(const wchar_t c) const;
	size_t rfind(const wchar_t c) const;

	wchar_t const& operator [](int i) const;
	wchar_t& operator [](int i);

	jwstring& operator =(const jwstring& str);
	jwstring& operator =(jwstring&& mvstr);
	jwstring& operator =(const wchar_t* cstr);

	jwstring& operator +=(const wchar_t rhs);
	jwstring& operator +=(const wchar_t* rhs);
	jwstring& operator +=(const jwstring &rhs);

	jwstring operator +(const wchar_t rhs);
	jwstring operator +(const wchar_t* rhs);
	jwstring operator +(const jwstring &rhs);

	friend bool operator >(const jwstring &lhs, const jwstring &rhs) { return wcscmp(lhs.c_str(), rhs.c_str()) > 0; }
	friend bool operator <(const jwstring &lhs, const wchar_t* rhs) { return wcscmp(lhs.c_str(), rhs) < 0; }

	friend bool operator >=(const jwstring &lhs, const jwstring &rhs) { return wcscmp(lhs.c_str(), rhs.c_str()) >= 0; }
	friend bool operator <=(const jwstring &lhs, const wchar_t* rhs) { return wcscmp(lhs.c_str(), rhs) <= 0; }

	friend bool operator ==(const jwstring &lhs, const jwstring &rhs) { return wcscmp(lhs.c_str(), rhs.c_str()) == 0; }
	friend bool operator ==(const jwstring &lhs, const wchar_t* rhs) { return wcscmp(lhs.c_str(), rhs) == 0; }
	friend bool operator !=(const jwstring &lhs, const jwstring &rhs) { return !operator==(lhs, rhs); }
	friend bool operator !=(const jwstring &lhs, const wchar_t* rhs) { return !operator==(lhs, rhs); }
};

template<typename T>
jstring to_string(T &t) {
	return "";
}

#pragma warning(push)
#pragma warning(disable: 4267) // conversion from size_t to int

namespace std {
	template<>
	struct hash<jstring> {
		size_t operator()(jstring const& jstr) const noexcept {
			size_t h = 0;
			for (size_t i = 0; i < jstr.length(); i++) {
				h = (h << 4) + jstr[i];
				size_t g = h & 0xF0000000L;
				if (g != 0) h ^= g >> 24;
				h &= ~g;
			}
			return h;
		}
	};
	template<>
	struct hash<jwstring> {
		size_t operator()(jwstring const& jstr) const noexcept {
			size_t h = 0;
			for (size_t i = 0; i < jstr.length(); i++) {
				h = (h << 4) + jstr[i];
				size_t g = h & 0xF0000000L;
				if (g != 0) h ^= g >> 24;
				h &= ~g;
			}
			return h;
		}
	};
}

#pragma warning(pop)