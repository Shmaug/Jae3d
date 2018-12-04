#pragma once

#include "../Jae3d/Util.hpp"
#include "jstring.hpp"

#include <memory>
#include <exception>

class MemoryStream {
public:
	MemoryStream(size_t size = 0, bool mExpand = true);
	MemoryStream(const char* buf, size_t size, bool expand = true);
	~MemoryStream();

	template<typename T>
	inline void Write(T t) {
		if (mCurrent + sizeof(T) > mSize) {
			if (!mExpand) throw new std::exception("Out of memory!");
			Fit(mCurrent + sizeof(T));
		}
		memcpy(mBuffer + mCurrent, reinterpret_cast<char*>(&t), sizeof(T));
		mCurrent += sizeof(T);
	}

	template<typename T>
	inline T Read() {
		if (mCurrent + sizeof(T) > mSize) throw new std::exception("Not enough room to read T!");
		T v(*reinterpret_cast<T*>(mBuffer + mCurrent));
		mCurrent += sizeof(T);
		return v;
	}

	void Write(const char* ptr, size_t sz);
	void Read(char* ptr, size_t sz);

	void WriteString(jstring jstring);
	jstring ReadString();

	void Fit(size_t s);
	void Seek(size_t p) { if (p >= mSize || p < 0) throw new std::exception("Invalid seek pos."); mCurrent = p; }
	size_t Tell() const { return mCurrent; }
	char* Ptr() const { return mBuffer; }
	
	// Compress from 0 to current
	void Compress(MemoryStream &ms);
	// Decompress from current to current + len
	void Decompress(MemoryStream &ms, size_t len);

private:
	bool mExpand = false;
	size_t mSize = 0;
	char* mBuffer = nullptr;
	size_t mCurrent = 0;

	void Expand(size_t sz);
};
