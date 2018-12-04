#pragma once

#include "../Jae3d/Util.hpp"
#include "jstring.hpp"
#include "IOUtil.hpp"

#include <iostream>
#include <fstream>

class AssetImporter;
class MemoryStream;

#pragma warning(push)
#pragma warning(disable: 4251) // needs to have dll-interface

class Asset {
public:
	jstring mName;
	jstring mGroup;

	Asset(jstring name);
	Asset(jstring name, MemoryStream &ms);
	~Asset();
	
	virtual void WriteData(MemoryStream &ms);
	virtual uint64_t TypeId();
};

#pragma warning(pop)