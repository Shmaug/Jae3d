#include "IOUtil.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

jstring GetFullPath(jstring str) {
	char buf[256];
	if (GetFullPathNameA(str.c_str(), 256, buf, nullptr) == 0) {
		printf("Failed to get full file path of %s (%d)", str.c_str(), GetLastError());
		return str;
	}
	return jstring(buf);
}
jstring GetExt(jstring path) {
	size_t k = path.rfind('.');
	if (k == jstring::npos) return "";
	return path.substr((int)k + 1);
}
jstring GetName(jstring path) {
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
jstring GetNameExt(jstring path) {
	size_t k = path.rfind('\\');
	if (k == jstring::npos) return "";
	return path.substr((int)k + 1);
}

jwstring utf8toUtf16(const jstring &str) {
	if (str.empty()) return jwstring();
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
	wchar_t* wstrTo = new wchar_t[size_needed];
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wstrTo, size_needed);
	jwstring wstr(wstrTo);
	delete[] wstrTo;
	return wstr;
}
jstring utf16toUtf8(const jwstring &wstr) {
	if (wstr.empty()) return jstring();
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
	char* strTo = new char[size_needed];
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, strTo, size_needed, NULL, NULL);
	jstring str(strTo);
	delete[] strTo;
	return str;
}