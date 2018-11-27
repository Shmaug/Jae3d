#ifndef UTIL_INC
#define UTIL_INC

#include <string>
#include <iostream>
#include <fstream>

std::string GetFullPath(std::string str);
std::string GetExt(std::string path);
std::string GetName(std::string path);
std::string GetNameExt(std::string path);
std::wstring utf8toUtf16(const std::string &str);

#define WriteStreamFunc(type) \
template<> \
inline void WriteStream<type>(std::ostream &stream, type var) { \
	stream.write(reinterpret_cast<const char*>(&var), sizeof(type)); \
}

#define ReadStreamFunc(type) \
template<> \
inline type ReadStream<type>(std::istream &stream) { \
	type r; \
	stream.read(reinterpret_cast<char*>(&r), sizeof(type)); \
	return r; \
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
inline void WriteStream<std::string>(std::ostream &stream, std::string var) {
	stream.write(var.c_str(), var.length() + 1);
}
template<>
inline void WriteStream<const char*>(std::ostream &stream, const char *var) {
	stream.write(var, strlen(var) + 1);
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
inline std::string ReadStream<std::string>(std::istream &stream) {
	std::string r = "";
	char ch;
	while ((ch = stream.get()) != '\0')
		r += ch;
	return r;
}

#undef ReadStreamFunc
#undef WriteStreamFunc

#endif