#ifndef UTIL_INC
#define UTIL_INC

#include "jstring.hpp"
#include "Util.hpp"

#include <iostream>
#include <fstream>

JAE_API jstring GetFullPathA(const jstring &str);
JAE_API jstring GetExtA(const jstring &str);
JAE_API jstring GetNameA(const jstring &str);
JAE_API jstring GetNameExtA(const jstring &str);
JAE_API jstring GetDirectoryA(const jstring &str);
JAE_API jwstring GetFullPathW(const jwstring &str);
JAE_API jwstring GetExtW(const jwstring &str);
JAE_API jwstring GetNameW(const jwstring &str);
JAE_API jwstring GetNameExtW(const jwstring &str);
JAE_API jwstring GetDirectoryW(const jwstring &file);

JAE_API jwstring utf8toUtf16(const jstring &str);
JAE_API jstring utf16toUtf8(const jwstring &wstr);

#define WriteStreamFunc(type) \
template<> \
inline void WriteStream<type>(std::ostream &stream, type var) { \
	stream.write(reinterpret_cast<const char*>(&var), sizeof(type)); \
}

template<typename T>
inline void WriteStream(std::ostream &stream, T var) {
	static_assert(false, "Unsupported write type");
}
WriteStreamFunc(uint8_t)
WriteStreamFunc(uint16_t)
WriteStreamFunc(uint32_t)
WriteStreamFunc(uint64_t)
WriteStreamFunc(int8_t)
WriteStreamFunc(int16_t)
WriteStreamFunc(int32_t)
WriteStreamFunc(int64_t)
template<>
inline void WriteStream<jwstring>(std::ostream &stream, jwstring var) {
	WriteStream(stream, (uint32_t)var.length());
	stream.write(reinterpret_cast<const char*>(var.c_str()), (var.length() + 1) * sizeof(wchar_t));
}
template<>
inline void WriteStream<const wchar_t*>(std::ostream &stream, const wchar_t *var) {
	size_t sz = wcslen(var);
	WriteStream(stream, (uint32_t)sz);
	stream.write(reinterpret_cast<const char*>(var), (sz + 1) * sizeof(wchar_t));
}

#undef WriteStreamFunc

#define ReadStreamFunc(type) \
template<> \
inline type ReadStream<type>(std::istream &stream) { \
	type r; \
	stream.read(reinterpret_cast<char*>(&r), sizeof(type)); \
	return r; \
}

template<typename T>
inline T ReadStream(std::istream &stream) {
	static_assert(false, "Unsupported read type ");
}
ReadStreamFunc(uint8_t)
ReadStreamFunc(uint16_t)
ReadStreamFunc(uint32_t)
ReadStreamFunc(uint64_t)
ReadStreamFunc(int8_t)
ReadStreamFunc(int16_t)
ReadStreamFunc(int32_t)
ReadStreamFunc(int64_t)
template<>
inline jwstring ReadStream<jwstring>(std::istream &stream) {
	uint32_t sz = ReadStream<uint32_t>(stream);
	wchar_t* s = new wchar_t[sz + 1];
	stream.read(const_cast<char*>(reinterpret_cast<const char*>(s)), (sz + 1) * sizeof(wchar_t));
	return jwstring(s, sz);
}

#undef ReadStreamFunc

#endif