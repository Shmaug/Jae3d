#pragma once

#include "Util.hpp"
#include "jstring.hpp"
#include "jvector.hpp"

namespace Profiler {
	class ProfilerSample {
	public:
		jvector<ProfilerSample*> children;
		ProfilerSample *parent;
		jwstring name;
		double startTime;
		double endTime;

		ProfilerSample(jwstring name, double startTime) : name(name), startTime(startTime) {
			parent = nullptr;
		}

		~ProfilerSample();
	};

	struct ProfilerFrame {
		jvector<ProfilerSample*> samples;
		long index;
		double startTime;
		double endTime;
	};

	JAE_API void BeginSample(jwstring name);
	JAE_API void EndSample();
	JAE_API void FrameStart();
	JAE_API void FrameEnd();
	JAE_API double LastFrameTime();

	JAE_API void PrintLastFrame(wchar_t *buffer, int size);
};

