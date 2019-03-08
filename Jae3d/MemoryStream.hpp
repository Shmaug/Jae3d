#pragma once

#include "Util.hpp"
#include "jstring.hpp"

#include <memory>
#include <exception>

class MemoryStream {
public:
	JAE_API MemoryStream(size_t size = 0, bool mExpand = true);
	JAE_API MemoryStream(const char* buf, size_t size, bool expand = true);
	JAE_API ~MemoryStream();

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

	JAE_API void Write(const char* ptr, size_t sz);
	JAE_API void Read(char* ptr, size_t sz);

	JAE_API void WriteStringA(const jstring& jwstring);
	JAE_API void WriteString(const jwstring& jwstring);
	JAE_API jstring ReadStringA();
	JAE_API jwstring ReadString();

	JAE_API void Fit(size_t s);
	void Seek(size_t p) { if (p >= mSize || p < 0) throw new std::exception("Invalid seek pos."); mCurrent = p; }
	size_t Tell() const { return mCurrent; }
	char* Ptr() const { return mBuffer; }
	
	// Compress from 0 to current
	JAE_API void Compress(MemoryStream &ms);
	// Decompress from current to current + len
	JAE_API void Decompress(MemoryStream &ms, size_t len);

private:
	bool mExpand = false;
	size_t mSize = 0;
	char* mBuffer = nullptr;
	size_t mCurrent = 0;

	void Expand(size_t sz);
};
