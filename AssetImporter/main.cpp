#include <string>
#include <vector>
#include <iostream>
#include <iostream>
#include <fstream>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <wrl.h>
#include <shlwapi.h>
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "runtimeobject.lib")

#include "AssetImporter.hpp"

using namespace std;

string GetExt(string path) {
	int k = -1;
	for (int i = 0; i < path.size(); i++)
		if (path[i] == '.')
			k = i;
	if (k == -1) return "";
	return path.substr(k + 1);
}
string GetFullPath(string str) {
	char buf[256];
	if (GetFullPathNameA(str.c_str(), 256, buf, nullptr) == 0) {
		printf("Failed to get full file path of %s (%d)", str.c_str(), GetLastError());
		return str;
	}
	return string(buf);
}

void LoadFile(string file) {
	if (PathFileExists(file.c_str()) != 1) {
		printf("Could not find %s\n", file.c_str());
		return;
	}

	int assetc = 0;
	string ext = GetExt(file);

	if (AssetImporter::verbose)
		printf("Loading %s (%s)\n", file.c_str(), ext.c_str());

	if (ext == "obj")
		AssetImporter::ImportObj(file);
	else if (ext == "fbx")
		AssetImporter::ImportFbx(file);
	else if (ext == "blend")
		AssetImporter::ImportBlend(file);

	else if (ext == "png")
		AssetImporter::ImportPng(file);
	else if (ext == "gif")
		AssetImporter::ImportGif(file);
	else if (ext == "bmp")
		AssetImporter::ImportBmp(file);
	else if (ext == "tif" || ext == "tiff")
		AssetImporter::ImportTif(file);
	else if (ext == "jpg" || ext == "jpeg")
		AssetImporter::ImportJpg(file);
	else if (ext == "dds")
		AssetImporter::ImportDds(file);
	else if (ext == "tga")
		AssetImporter::ImportTga(file);

	else if (ext == "cso")
		AssetImporter::ImportShader(file);
	else if (ext == "hlsl")
		AssetImporter::CompileShader(file);

	if (AssetImporter::verbose)
		printf("\n");
}

void LoadDirectory(string dir, vector<string>* files) {
	string d = dir + "\\*";

	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile(d.c_str(), &ffd);
	if (hFind == INVALID_HANDLE_VALUE) {
		printf("Failed to find directory %s\n", dir.c_str());
		return;
	}

	do {
		if (ffd.cFileName[0] == '.') continue;

		string c = dir + "\\" + string(ffd.cFileName);
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			LoadDirectory(c.c_str(), files);
		else
			files->push_back(GetFullPath(c));
	} while (FindNextFile(hFind, &ffd) != 0);

	FindClose(hFind);
}

int main(int argc, char **argv) {
#if (_WIN32_WINNT >= 0x0A00 /*_WIN32_WINNT_WIN10*/)
	Microsoft::WRL::Wrappers::RoInitializeWrapper initialize(RO_INIT_MULTITHREADED);
	if (FAILED(initialize))
#else
	HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
	if (FAILED(hr))
#endif
	{
		printf("Failed to initialize COM!\n");
		return -1;
	}

	vector<string> files;
	vector<string> directories;
	string output;

	int mode = 0;
	for (int i = 1; i < argc; i++) {
		string str = string(argv[i]);
		if (argv[i][0] == '-') {
			if (str == "-f") {
				mode = 0;
			} else if (str == "-d") {
				mode = 1;
			} else if (str == "-o") {
				mode = 2;
			} else if (str == "-v") {
				AssetImporter::verbose = true;
			}
		} else {
			switch (mode) {
			case 0:
				files.push_back(GetFullPath(str));
				break;
			case 1:
				directories.push_back(str);
				break;
			case 2:
				output = str;
				break;
			}
		}
	}

	for (int i = 0; i < directories.size(); i++)
		LoadDirectory(directories[i], &files);

	printf("Loading %d files\n", (int)files.size());
	for (int i = 0; i < files.size(); i++)
		LoadFile(files[i]);

	AssetImporter::Write(output.c_str());

	if (AssetImporter::Validate(output.c_str()))
		printf("Output validated.");
	else
		printf("Output failed to validate!");

	AssetImporter::CleanUp();

	return 0;
}