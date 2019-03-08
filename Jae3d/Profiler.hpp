#pragma once

#include "Util.hpp"
#include "jstring.hpp"
#include "jvector.hpp"

namespace Profiler {
	class ProfilerSample {
	public:
		jvector<ProfilerSample> children;
		ProfilerSample *parent;
		jwstring name;
		double time;
		double startTime;

		ProfilerSample() : name(L""), parent(nullptr), time(0), startTime(0) {}
		ProfilerSample(const jwstring& name, double startTime) : name(name), parent(nullptr), time(0), startTime(startTime) {}
	};

	JAE_API void BeginSample(const jwstring& name, bool resume = false);
	JAE_API void EndSample();
	JAE_API void FrameStart();
	JAE_API void FrameEnd();
	JAE_API double LastFrameTime();

	JAE_API void PrintLastFrame(wchar_t *buffer, int size);
};

