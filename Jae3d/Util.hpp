#pragma warning(disable:4251)
#ifndef UTIL_HPP
#define UTIL_HPP

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <exception>
#include <intrin.h>

#define _WRL Microsoft::WRL

#ifdef JAE_EXPORTS
#define JAE_API __declspec(dllexport)
#else
#define JAE_API __declspec(dllimport)
#endif

inline void ThrowIfFailed(HRESULT hr) {
	if (FAILED(hr)) {
		throw std::exception();
	}
}

inline void WINAPIV OutputDebugf(LPCWSTR fmt, ...) {
	TCHAR s[1025];
	va_list args;
	va_start(args, fmt);
	wvsprintf(s, fmt, args);
	va_end(args);
	OutputDebugString(s);
}

template <typename T>
inline T AlignUpWithMask(T value, size_t mask) {
	return (T)(((size_t)value + mask) & ~mask);
}
template <typename T>
inline T AlignDownWithMask(T value, size_t mask) {
	return (T)((size_t)value & ~mask);
}
template <typename T>
inline T AlignUp(T value, size_t alignment) {
	return AlignUpWithMask(value, alignment - 1);
}
template <typename T>
inline T AlignDown(T value, size_t alignment) {
	return AlignDownWithMask(value, alignment - 1);
}

template <typename T>
inline T DivideByMultiple(T value, size_t alignment) {
	return (T)((value + alignment - 1) / alignment);
}

template <typename T>
inline bool IsPowerOfTwo(T value) {
	return 0 == (value & (value - 1));
}

inline unsigned char Log2(size_t value) {
	unsigned long mssb; // most significant set bit
	unsigned long lssb; // least significant set bit

	// If perfect power of two (only one set bit), return index of bit.  Otherwise round up
	// fractional log by adding 1 to most signicant set bit's index.
#ifdef _WIN64
	if (_BitScanReverse64(&mssb, value) > 0 && _BitScanForward64(&lssb, value) > 0)
#else
	if (_BitScanReverse(&mssb, value) > 0 && _BitScanForward(&lssb, value) > 0)
#endif
		return unsigned char(mssb + (mssb == lssb ? 0 : 1));
	else
		return 0;
}

#endif