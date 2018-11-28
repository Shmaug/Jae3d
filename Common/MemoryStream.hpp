#pragma once

#include <memory>
#include <exception>
#include <string>

class MemoryStream {
public:
	MemoryStream(size_t size = 0, bool m_Expand = true);
	MemoryStream(const char* buf, size_t size, bool expand = true);
	~MemoryStream();

	template<typename T>
	inline void Write(T t) {
		if (m_Current + sizeof(T) > m_Size) {
			if (!m_Expand) throw new std::exception();
			Fit(m_Current + sizeof(T));
		}
		memcpy(m_Buffer + m_Current, reinterpret_cast<char*>(&t), sizeof(T));
		m_Current += sizeof(T);
	}

	template<typename T>
	inline T Read() {
		if (m_Current + sizeof(T) > m_Size) throw new std::exception();
		T v;
		memcpy(reinterpret_cast<char*>(&v), m_Buffer + m_Current, sizeof(T));
		m_Current += sizeof(T);
		return v;
	}


	void Write(const char* ptr, size_t sz);
	void Read(char* ptr, size_t sz);

	void WriteString(std::string string);
	std::string ReadString();

	void Fit(size_t s);
	void Seek(size_t p) { if (p >= m_Size || p < 0) throw new std::exception(); m_Current = p; }
	size_t Tell() const { return m_Current; }
	char* Ptr() const { return m_Buffer; }
	
	// Compress from 0 to current
	void Compress(MemoryStream &ms);
	// Decompress from current to current + len
	void Decompress(MemoryStream &ms, size_t len);

private:
	bool m_Expand = false;
	size_t m_Size = 0;
	char* m_Buffer = nullptr;
	size_t m_Current = 0;

	void Expand(size_t sz);
};
