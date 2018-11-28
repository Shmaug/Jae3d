#include "MemoryStream.hpp"

#include <cassert>
#include <vector>
#include <zlib.h>

using namespace std;

MemoryStream::MemoryStream(size_t size, bool expand) : m_Expand(expand), m_Size(size), m_Current(0) {
	m_Buffer = new char[size];
}
MemoryStream::MemoryStream(const char* buf, size_t size, bool expand) : m_Expand(expand), m_Size(size), m_Current(0) {
	m_Buffer = new char[size];
	memcpy(m_Buffer, buf, size);
}
MemoryStream::~MemoryStream() {
	if (m_Buffer) delete[] m_Buffer;
}

void MemoryStream::WriteString(string string) {
	Write(string.c_str(), string.length() + 1);
}
string MemoryStream::ReadString() {
	size_t start = m_Current;
	while (m_Current < m_Size) {
		if (m_Buffer[m_Current] == '\0') {
			m_Current++;
			return string(m_Buffer + start, m_Buffer + m_Current - 1);
		}
		m_Current++;
	}
	return string(m_Buffer + start, m_Buffer + m_Current);
}

void MemoryStream::Write(const char* ptr, size_t sz) {
	if (m_Current + sz > m_Size) {
		if (!m_Expand) throw new std::exception();
		Fit(m_Current + sz);
	}
	memcpy(m_Buffer + m_Current, ptr, sz);
	m_Current += sz;
}
void MemoryStream::Read(char* ptr, size_t sz) {
	if (m_Current + sz > m_Size) throw new std::exception();
	memcpy(ptr, m_Buffer + m_Current, sz);
	m_Current += sz;
}

void MemoryStream::Fit(size_t sz) {
	if (sz > m_Size && m_Expand) {
		size_t ns = 1;
		while (sz > ns) ns <<= 1;
		Expand(ns);
	}
}
void MemoryStream::Expand(size_t sz) {
	char* newbuf = new char[sz];
	memcpy(newbuf, m_Buffer, m_Size);
	delete[] m_Buffer;
	m_Buffer = newbuf;
	m_Size = sz;
}

void MemoryStream::Compress(MemoryStream &ms) {
	const size_t BUFSIZE = 128 * 1024;
	uint8_t temp_buffer[BUFSIZE];

	z_stream strm;
	strm.zalloc = 0;
	strm.zfree = 0;
	strm.next_in = reinterpret_cast<uint8_t*>(m_Buffer);
	strm.avail_in = (uInt)m_Current;
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
	zInfo.next_in = (Bytef*)(m_Buffer + m_Current);
	zInfo.total_in = zInfo.avail_in = (uInt)len;
	zInfo.next_out = (Bytef*)(ms.m_Buffer + ms.m_Current);
	zInfo.total_out = zInfo.avail_out = (uInt)(ms.m_Size - ms.m_Current);

	int nErr = -1;
	nErr = inflateInit(&zInfo);
	assert(nErr == Z_OK);

	nErr = inflate(&zInfo, Z_FINISH);
	assert(nErr == Z_STREAM_END);

	inflateEnd(&zInfo);
}