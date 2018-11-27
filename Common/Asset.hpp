#pragma once

#include <string>
#include <iostream>
#include <fstream>

#include "IOUtil.hpp"

class AssetImporter;
class MemoryStream;

class Asset {
public:
	std::string m_Name;
	std::string m_Group;

	Asset(std::string name);
	Asset(std::string name, MemoryStream &ms);
	~Asset();
	
	virtual void WriteData(MemoryStream &ms);
	virtual uint64_t TypeId();
};

