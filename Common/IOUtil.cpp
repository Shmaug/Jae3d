#include "IOUtil.hpp"

#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

using namespace std;

string GetFullPath(string str) {
	char buf[256];
	if (GetFullPathNameA(str.c_str(), 256, buf, nullptr) == 0) {
		printf("Failed to get full file path of %s (%d)", str.c_str(), GetLastError());
		return str;
	}
	return string(buf);
}
string GetExt(string path) {
	int k = -1;
	for (int i = 0; i < path.size(); i++)
		if (path[i] == '.')
			k = i;
	if (k == -1) return "";
	return path.substr(k + 1);
}
string GetName(string path) {
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
string GetNameExt(string path) {
	char const *str = path.c_str();

	int f = 0;
	for (int i = 0; i < path.length(); i++)
		if (str[i] == '\\')
			f = i + 1;

	return path.substr(f, path.length() - f);
}
wstring utf8toUtf16(const string &str) {
	if (str.empty())
		return wstring();

	size_t charsNeeded = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), NULL, 0);
	if (charsNeeded == 0)
		throw runtime_error("Failed converting UTF-8 string to UTF-16");

	vector<wchar_t> buffer(charsNeeded);
	int charsConverted = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), &buffer[0], (int)buffer.size());
	if (charsConverted == 0)
		throw runtime_error("Failed converting UTF-8 string to UTF-16");

	return wstring(&buffer[0], charsConverted);
}