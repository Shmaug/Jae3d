#pragma once

#include "Util.hpp"

#include <functional>

class jstring {
private:
	char* mCStr;
	size_t mLength;
	size_t mCapacity;

public:
	static constexpr auto npos{ static_cast<size_t>(-1) };

	JAE_API jstring();
	JAE_API jstring(const jstring &str);
	JAE_API jstring(jstring &&mvstr);
	JAE_API jstring(const size_t length);
	JAE_API jstring(const char* str);
	JAE_API jstring(const char* start, size_t length);
	JAE_API ~jstring();

	JAE_API void reserve(size_t cap);

	bool empty() const { return mLength == 0; }
	const char* c_str() const { return mCStr; }
	size_t length() const { return mLength; }

	JAE_API jstring substr(int pos) const;
	JAE_API jstring substr(size_t pos, size_t length) const;
	JAE_API jstring lower() const;
	JAE_API jstring upper() const;

	JAE_API size_t find(const char c) const;
	JAE_API size_t rfind(const char c) const;

	JAE_API char const& operator [](int i) const;
	JAE_API char& operator [](int i);

	JAE_API jstring& operator =(const jstring& str);
	JAE_API jstring& operator =(jstring&& mvstr);
	JAE_API jstring& operator =(const char* cstr);
	
	JAE_API jstring& operator +=(const char rhs);
	JAE_API jstring& operator +=(const char* rhs);
	JAE_API jstring& operator +=(const jstring &rhs);

	JAE_API jstring operator +(const char rhs);
	JAE_API jstring operator +(const char* rhs);
	JAE_API jstring operator +(const jstring &rhs);

	JAE_API friend jstring operator +(const char lhs, const jstring &rhs);
	JAE_API friend jstring operator +(const char* lhs, const jstring &rhs);

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

	JAE_API jwstring();
	JAE_API jwstring(const jwstring &str);
	JAE_API jwstring(jwstring &&mvstr);
	JAE_API jwstring(const size_t length);
	JAE_API jwstring(const wchar_t* str);
	JAE_API jwstring(const wchar_t* start, size_t length);
	JAE_API ~jwstring();

	JAE_API void reserve(size_t cap);

	bool empty() const { return mLength == 0; }
	const wchar_t* c_str() const { return mCStr; }
	size_t length() const { return mLength; }

	JAE_API jwstring substr(int pos) const;
	JAE_API jwstring substr(size_t pos, size_t length) const;
	JAE_API jwstring lower() const;
	JAE_API jwstring upper() const;
	JAE_API bool startswith(jwstring str) const;
	JAE_API bool startswith(const wchar_t* str) const;

	JAE_API size_t find(const wchar_t c) const;
	JAE_API size_t rfind(const wchar_t c) const;

	JAE_API wchar_t const& operator [](int i) const;
	JAE_API wchar_t& operator [](int i);

	JAE_API jwstring& operator =(const jwstring& str);
	JAE_API jwstring& operator =(jwstring&& mvstr);
	JAE_API jwstring& operator =(const wchar_t* cstr);

	JAE_API jwstring& operator +=(const wchar_t rhs);
	JAE_API jwstring& operator +=(const wchar_t* rhs);
	JAE_API jwstring& operator +=(const jwstring &rhs);

	JAE_API jwstring operator +(const wchar_t rhs);
	JAE_API jwstring operator +(const wchar_t* rhs);
	JAE_API jwstring operator +(const jwstring &rhs);

	JAE_API friend jwstring operator +(const wchar_t lhs, const jwstring &rhs);
	JAE_API friend jwstring operator +(const wchar_t* lhs, const jwstring &rhs);

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
template<typename T>
jwstring to_wstring(T &t) {
	return L"";
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