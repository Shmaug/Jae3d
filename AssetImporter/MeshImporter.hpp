#pragma once

#include "../Common/jstring.hpp"

class Asset;

class MeshImporter {
public:
	static Asset** Import(jstring file, int &count);
};

