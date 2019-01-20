#include "IOUtil.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <codecvt>

jstring GetFullPathA(const jstring &str) {
	char buf[MAX_PATH];
	if (GetFullPathNameA(str.c_str(), 256, buf, nullptr) == 0) {
		printf("Failed to get full file path of %s (%d)", str.c_str(), GetLastError());
		return str;
	}
	return jstring(buf);
}
jstring GetExtA(const jstring &path) {
	size_t k = path.rfind('.');
	if (k == jstring::npos) return "";
	return path.substr((int)k + 1);
}
jstring GetNameA(const jstring &path) {
	char const *str = path.c_str();

	int f = 0;
	int l = 0;
	for (int i = 0; i < path.length(); i++) {
		if (str[i] == '\\')
			f = i + 1;
		else if (str[i] == '.')
			l = i;
	}

	return path.substr(f, l - f);
}
jstring GetNameExtA(const jstring &path) {
	size_t k = path.rfind('\\');
	if (k == jstring::npos) return "";
	return path.substr((int)k + 1);
}
jstring GetDirectoryA(const jstring &file) {
	size_t k = file.rfind('\\');
	if (k == jstring::npos) return "";
	return file.substr(0, (int)k);
}

jwstring GetFullPathW(const jwstring &str) {
	wchar_t buf[MAX_PATH];
	if (GetFullPathNameW(str.c_str(), 256, buf, nullptr) == 0) {
		printf("Failed to get full file path of %S (%d)", str.c_str(), GetLastError());
		return str;
	}
	return jwstring(buf);
}
jwstring GetExtW(const jwstring &path) {
	size_t k = path.rfind(L'.');
	if (k == jwstring::npos) return L"";
	return path.substr((int)k + 1);
}
jwstring GetNameW(const jwstring &path) {
	wchar_t const *str = path.c_str();

	int f = 0;
	int l = 0;
	for (int i = 0; i < path.length(); i++) {
		if (str[i] == L'\\')
			f = i + 1;
		else if (str[i] == L'.')
			l = i;
	}

	return path.substr(f, l - f);
}
jwstring GetNameExtW(const jwstring &path) {
	size_t k = path.rfind(L'\\');
	if (k == jwstring::npos) return L"";
	return path.substr((int)k + 1);
}
jwstring GetDirectoryW(const jwstring &file) {
	size_t k = file.rfind(L'\\');
	if (k == jwstring::npos) return L"";
	return file.substr(0, (int)k);
}

jwstring utf8toUtf16(const jstring &str) {
	if (str.empty()) return jwstring();

	jwstring rstr;
	int sizeRequired = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
	if (sizeRequired > 0) {
		std::vector<wchar_t> utf16String(sizeRequired);
		int bytesConverted = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, utf16String.data(), (int)utf16String.size());
		if (bytesConverted != 0)
			rstr = &utf16String[0];
	}
	return rstr;
}
jstring utf16toUtf8(const jwstring &wstr) {
	if (wstr.empty()) return jstring();

	jstring rstr;
	int sizeRequired = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
	if (sizeRequired > 0) {
		std::vector<char> utf8String(sizeRequired);
		int bytesConverted = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, utf8String.data(), (int)utf8String.size(), NULL, NULL);
		if (bytesConverted != 0)
			rstr = &utf8String[0];
	}
	return rstr;
}