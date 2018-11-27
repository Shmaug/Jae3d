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

#include "..\Common\IOUtil.hpp"
#include "..\Common\AssetFile.hpp"
#include "..\Common\Asset.hpp"
#include "..\Common\MeshAsset.hpp"
#include "..\Common\ShaderAsset.hpp"
#include "..\Common\TextureAsset.hpp"
#include "AssetImporter.hpp"

using namespace std;

void LoadFile(string file, vector<Asset*> &assets) {
	if (PathFileExists(file.c_str()) != 1) {
		printf("Could not find %s\n", file.c_str());
		return;
	}

	string ext = GetExt(file);

	if (ext == "obj") {
		int count;
		Asset** arr = AssetImporter::ImportObj(file, count);
		for (int i = 0; i < count; i++)
			assets.push_back(arr[i]);
		delete[] arr;
	} else if (ext == "fbx") {
		int count;
		Asset** arr = AssetImporter::ImportFbx(file, count);
		for (int i = 0; i < count; i++)
			assets.push_back(arr[i]);
		delete[] arr;
	} else if (ext == "blend") {
		int count;
		Asset** arr = AssetImporter::ImportBlend(file, count);
		for (int i = 0; i < count; i++)
			assets.push_back(arr[i]);
		delete[] arr;
	}
	else if (ext == "png")
		assets.push_back((Asset*)AssetImporter::ImportPng(file));
	else if (ext == "gif")
		assets.push_back((Asset*)AssetImporter::ImportGif(file));
	else if (ext == "bmp")
		assets.push_back((Asset*)AssetImporter::ImportBmp(file));
	else if (ext == "tif" || ext == "tiff")
		assets.push_back((Asset*)AssetImporter::ImportTif(file));
	else if (ext == "jpg" || ext == "jpeg")
		assets.push_back((Asset*)AssetImporter::ImportJpg(file));
	else if (ext == "dds")
		assets.push_back((Asset*)AssetImporter::ImportDDS(file));
	else if (ext == "tga")
		assets.push_back((Asset*)AssetImporter::ImportTga(file));

	else if (ext == "cso")
		assets.push_back((Asset*)AssetImporter::ImportShader(file));
	else if (ext == "hlsl")
		assets.push_back((Asset*)AssetImporter::CompileShader(file));
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
	string input;

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
			} else if (str == "-r") {
				mode = 3;
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
			case 3:
				input = GetFullPath(str);
				break;
			}
		}
	}

	if (!input.empty()) {
		printf("Reading %s\n", input.c_str());
		int c;
		AssetFile::AssetData* a = AssetFile::Read(input, c);
		for (int i = 0; i < c; i++) {
			Asset* f = nullptr;
			switch (a[i].type) {
			case AssetFile::TYPEID_MESH:
				f = new MeshAsset(a[i].name, *a[i].buffer);
				if (AssetImporter::verbose)
					printf("%d vertices, %d tris\n", ((MeshAsset*)f)->VertexCount(), ((MeshAsset*)f)->IndexCount() / 3);
				break;
			case AssetFile::TYPEID_SHADER:
				f = new ShaderAsset(a[i].name, *a[i].buffer);
				if (AssetImporter::verbose)
					printf("%s\n", f->m_Name.c_str());
				break;
			case AssetFile::TYPEID_TEXTURE:
				f = new TextureAsset(a[i].name, *a[i].buffer);
				if (AssetImporter::verbose)
					printf("%dx%d\n", ((TextureAsset*)f)->Width(), ((TextureAsset*)f)->Height());
				break;
			}
			if (f) delete f;
		}
		printf("Successfully read %s\n", input.c_str());
		delete[] a;
		return 0;
	}

	if (output.empty()) printf("Please specify an output file [-o]");
	if (files.empty() && directories.empty()) printf("Please specify input files [-f] or input directories [-d]");

	for (int i = 0; i < directories.size(); i++)
		LoadDirectory(directories[i], &files);

	// print file names
	vector<Asset*> assets;
	printf("Loading %d files\n", (int)files.size());
	if (AssetImporter::verbose) {
		for (int i = 0; i < files.size(); i++)
			printf("   %s\n", files[i].c_str());
		printf("\n");
	}
	for (int i = 0; i < files.size(); i++)
		LoadFile(files[i], assets);

	printf("\n");

	AssetFile::Write(output.c_str(), assets);

	for (int i = 0; i < assets.size(); i++)
		delete assets[i];
	assets.clear();

	return 0;
}