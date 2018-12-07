#include "MemoryStream.hpp"

#include <cassert>
#include <vector>
#include <zlib.h>

MemoryStream::MemoryStream(size_t size, bool expand) : mExpand(expand), mSize(size), mCurrent(0) {
	mBuffer = new char[size];
}
MemoryStream::MemoryStream(const char* buf, size_t size, bool expand) : mExpand(expand), mSize(size), mCurrent(0) {
	mBuffer = new char[size];
	memcpy(mBuffer, buf, size);
}
MemoryStream::~MemoryStream() {
	if (mBuffer) delete[] mBuffer;
}

void MemoryStream::WriteString(jwstring str) {
	Write((uint32_t)str.length());
	Write(reinterpret_cast<const char*>(str.c_str()), (str.length() + 1) * sizeof(wchar_t));
}
jwstring MemoryStream::ReadString() {
	uint32_t sz = Read<uint32_t>();
	wchar_t* s = new wchar_t[sz + 1];
	Read(const_cast<char*>(reinterpret_cast<const char*>(s)), (sz + 1) * sizeof(wchar_t));
	return jwstring(s, sz);
}

void MemoryStream::Write(const char* ptr, size_t sz) {
	if (mCurrent + sz > mSize) {
		if (!mExpand) throw std::out_of_range("Write out of bounds!");
		Fit(mCurrent + sz);
	}
	memcpy(mBuffer + mCurrent, ptr, sz);
	mCurrent += sz;
}
void MemoryStream::Read(char* ptr, size_t sz) {
	if (mCurrent + sz > mSize) throw std::out_of_range("Read out of bounds!");
	memcpy(ptr, mBuffer + mCurrent, sz);
	mCurrent += sz;
}

void MemoryStream::Fit(size_t sz) {
	if (sz > mSize && mExpand) {
		size_t ns = 1;
		while (sz > ns) ns <<= 1;
		Expand(ns);
	}
}
void MemoryStream::Expand(size_t sz) {
	char* newbuf = new char[sz];
	memcpy(newbuf, mBuffer, mSize);
	delete[] mBuffer;
	mBuffer = newbuf;
	mSize = sz;
}

void MemoryStream::Compress(MemoryStream &ms) {
	const size_t BUFSIZE = 128 * 1024;
	uint8_t temp_buffer[BUFSIZE];

	z_stream strm;
	strm.zalloc = 0;
	strm.zfree = 0;
	strm.next_in = reinterpret_cast<uint8_t*>(mBuffer);
	strm.avail_in = (uInt)mCurrent;
	strm.next_out = (Bytef*)temp_buffer;
	strm.avail_out = BUFSIZE;

	deflateInit(&strm, Z_BEST_COMPRESSION);

	while (strm.avail_in != 0) {
		int res = deflate(&strm, Z_NO_FLUSH);
		assert(res == Z_OK);
		if (strm.avail_out == 0) {
			ms.Write(reinterpret_cast<const char*>(temp_buffer), BUFSIZE);
			strm.next_out = temp_buffer;
			strm.avail_out = BUFSIZE;
		}
	}

	int deflate_res = Z_OK;
	while (deflate_res == Z_OK) {
		if (strm.avail_out == 0) {
			ms.Write(reinterpret_cast<const char*>(temp_buffer), BUFSIZE);
			strm.next_out = temp_buffer;
			strm.avail_out = BUFSIZE;
		}
		deflate_res = deflate(&strm, Z_FINISH);
	}

	assert(deflate_res == Z_STREAM_END);
	ms.Write(reinterpret_cast<const char*>(temp_buffer), BUFSIZE - strm.avail_out);
	deflateEnd(&strm);
}
void MemoryStream::Decompress(MemoryStream &ms, size_t len) {
	z_stream zInfo = { 0 };
	zInfo.next_in = (Bytef*)(mBuffer + mCurrent);
	zInfo.total_in = zInfo.avail_in = (uInt)len;
	zInfo.next_out = (Bytef*)(ms.mBuffer + ms.mCurrent);
	zInfo.total_out = zInfo.avail_out = (uInt)(ms.mSize - ms.mCurrent);

	int nErr = -1;
	nErr = inflateInit(&zInfo);
	assert(nErr == Z_OK);

	nErr = inflate(&zInfo, Z_FINISH);
	assert(nErr == Z_STREAM_END);

	inflateEnd(&zInfo);
}