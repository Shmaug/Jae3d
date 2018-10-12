#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h> // For HRESULT
#include <shellapi.h> // For CommandLineToArgvW
#include <algorithm>

// From DXSampleHelper.h 
// Source: https://github.com/Microsoft/DirectX-Graphics-Samples
inline void ThrowIfFailed(HRESULT hr) {
	if (FAILED(hr)) {
		throw std::exception();
	}
}

int PrintFormattedf(char *buf, int size, const char* format, float number);
int PrintFormattedl(char *buf, int size, unsigned long number);