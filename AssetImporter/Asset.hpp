#pragma once

#include <string>
#include <iostream>
#include <fstream>

#include "Util.hpp"

class Asset {
public:
	struct AssetHeader {
		uint8_t id = 0x01;
		uint64_t dataPos;
		uint64_t headerSize;
		uint64_t dataSize;
		std::string name;

		// Read asset header
		AssetHeader(std::ifstream &fs) {
			id = ReadStream<uint8_t>(fs);
			dataPos = ReadStream<uint64_t>(fs);
			headerSize = ReadStream<uint64_t>(fs);
			dataSize = ReadStream<uint64_t>(fs);
			name = ReadStream<std::string>(fs);
		}
		
		// Write asset header
		AssetHeader(std::ofstream &fs, Asset *asset) {
			asset->m_HeaderPositionPos = (uint64_t)fs.tellp(); // store position of the header
			WriteStream(fs, (uint8_t)0x01); // "asset" identifier
			WriteStream(fs, (uint64_t)0); // reserve space for data offset
			WriteStream(fs, (uint64_t)0); // reserve space for header size
			WriteStream(fs, (uint64_t)0); // reserve space for data size
			WriteStream(fs, asset->m_Name);
		}
	};
	struct AssetData {
		uint8_t id = 0x02;
		uint64_t headerPos;
		uint64_t dataSize;
		std::string name;
		
		// Read asset data
		AssetData(std::ifstream &fs) {
			id = ReadStream<uint8_t>(fs);
			headerPos = ReadStream<uint64_t>(fs);
			dataSize = ReadStream<uint64_t>(fs);
			name = ReadStream<std::string>(fs);
		}

		// Write asset data
		AssetData(std::ofstream &fs, Asset *asset) {
			// seek up and write the position of the data in the header
			uint64_t p = (uint64_t)fs.tellp();
			fs.seekp(asset->m_HeaderPositionPos + sizeof(uint8_t));
			WriteStream(fs, p);
			fs.seekp(p); // seek back to the current position

			WriteStream(fs, (uint8_t)0x02); // "data" identifier
			WriteStream(fs, (uint64_t)asset->m_HeaderPositionPos);
			WriteStream(fs, (uint64_t)0);
			WriteStream(fs, asset->m_Name);
		}
	};

	std::string m_Name;
	std::string m_Group;

	Asset(std::string name);
	~Asset();
	
	virtual void WriteHeader(std::ofstream &stream);
	virtual void WriteData(std::ofstream &stream);

private:
	uint64_t m_HeaderPositionPos;
};

