#pragma once

#include <jstring.hpp>

class Asset;

class MeshImporter {
public:
	static Asset** Import(jwstring file, int &count);
};

