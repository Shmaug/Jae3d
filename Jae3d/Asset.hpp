#pragma once

#include "Util.hpp"
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
	jwstring mName;
	jwstring mGroup;

	JAE_API Asset(jwstring name);
	JAE_API Asset(jwstring name, MemoryStream &ms);
	JAE_API ~Asset();
	
	JAE_API virtual void WriteData(MemoryStream &ms);
	JAE_API virtual uint64_t TypeId();
};

#pragma warning(pop)