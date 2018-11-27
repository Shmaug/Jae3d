#pragma once

#include <string>

class Asset;

class MeshImporter {
public:
	static Asset** Import(std::string file, int &count);
};

