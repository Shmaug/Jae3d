#pragma once

#include "Util.hpp"
#include "jstring.hpp"
#include "jvector.hpp"

#define NOPROFILER

#ifdef NOPROFILER
#define PROFILER_BEGIN(name)
#define PROFILER_BEGIN_RESUME(name)
#define PROFILER_END()
#else
#define PROFILER_BEGIN(name) Profiler::BeginSample(name)
#define PROFILER_BEGIN_RESUME(name) Profiler::BeginSample(name, true)
#define PROFILER_END() Profiler::EndSample()
#endif

namespace Profiler {
	JAE_API void BeginSample(const jwstring& name, bool resume = false);
	JAE_API void EndSample();
	JAE_API void FrameStart();
	JAE_API void FrameEnd();
	JAE_API double LastFrameTime();

	JAE_API void PrintLastFrame(wchar_t *buffer, int size);
};

