#pragma once

#include <iostream>
#include <string>
#include <vector>

class Asset;

class AssetImporter {
public:
	static bool verbose;
	static std::vector<Asset*> assets;

	static void ImportObj(std::string file);
	static void ImportFbx(std::string file);
	static void ImportBlend(std::string file);
	
	static void ImportPng(std::string file);
	static void ImportGif(std::string file);
	static void ImportBmp(std::string file);
	static void ImportTif(std::string file);
	static void ImportJpg(std::string file);
	static void ImportDds(std::string file);
	static void ImportTga(std::string file);
		
	static void ImportShader(std::string file);
	static void CompileShader(std::string file);

	static void Write(const char *file);
	static bool Validate(const char *file);
	static void CleanUp();
};

