#include "ShaderImporter.hpp"

#include <string>
#include <sstream>

#include <cassert>
#include <comdef.h>
#include <unordered_map>

using namespace std;
using namespace DirectX;

enum PARSEMODE {
	NONE,
	PRAGMA,
	INCLUDE,
	SHADERSTAGE,
	PARAMTYPE,
	PARAMNAME,
	PARAMVALUE
};

const unordered_map<Shader::PARAM_TYPE, unsigned int> paramSizes = {
	{ Shader::PARAM_TYPE_CBUFFER, 0 },
	{ Shader::PARAM_TYPE_SRV, 0 },
	{ Shader::PARAM_TYPE_UAV, 0 },
	{ Shader::PARAM_TYPE_SAMPLER, 0 },
	{ Shader::PARAM_TYPE_TEXTURE, 0 },
	
	{ Shader::PARAM_TYPE_FLOATRANGE, 4 },
	{ Shader::PARAM_TYPE_INTRANGE, 4 },
	{ Shader::PARAM_TYPE_UINTRANGE, 4 },

	{ Shader::PARAM_TYPE_COLOR3, 12 },
	{ Shader::PARAM_TYPE_COLOR4, 16 },

	{ Shader::PARAM_TYPE_FLOAT,  4 },
	{ Shader::PARAM_TYPE_FLOAT2, 8 },
	{ Shader::PARAM_TYPE_FLOAT3, 12 },
	{ Shader::PARAM_TYPE_FLOAT4, 16 },

	{ Shader::PARAM_TYPE_INT,  4 },
	{ Shader::PARAM_TYPE_INT2, 8 },
	{ Shader::PARAM_TYPE_INT3, 12 },
	{ Shader::PARAM_TYPE_INT4, 16 },

	{ Shader::PARAM_TYPE_UINT,  4 },
	{ Shader::PARAM_TYPE_UINT2, 8 },
	{ Shader::PARAM_TYPE_UINT3, 12 },
	{ Shader::PARAM_TYPE_UINT4, 16 },
};

// remove whitespace, return new length
int strTrim(wchar_t* str) {
	int count = 0;
	for (int i = 0; str[i]; i++)
		if (!isspace(str[i]))
			str[count++] = str[i];
	str[count] = L'\0';
	return count;
}

XMFLOAT2 atof2(const wchar_t* str) {
	XMFLOAT2 v;
	swscanf_s(str, L"(%f,%f)", &v.x, &v.y);
	return v;
}
XMFLOAT3 atof3(const wchar_t* str) {
	XMFLOAT3 v;
	swscanf_s(str, L"(%f,%f,%f)", &v.x, &v.y, &v.z);
	return v;
}
XMFLOAT4 atof4(const wchar_t* str) {
	XMFLOAT4 v;
	swscanf_s(str, L"(%f,%f,%f,%f)", &v.x, &v.y, &v.z, &v.w);
	return v;
}
XMINT2 atoi2(const wchar_t* str) {
	XMINT2 v;
	swscanf_s(str, L"(%d,%d)", &v.x, &v.y);
	return v;
}
XMINT3 atoi3(const wchar_t* str) {
	XMINT3 v;
	swscanf_s(str, L"(%d,%d,%d)", &v.x, &v.y, &v.z);
	return v;
}
XMINT4 atoi4(const wchar_t* str) {
	XMINT4 v;
	swscanf_s(str, L"(%d,%d,%d,%d)", &v.x, &v.y, &v.z, &v.w);
	return v;
}
XMUINT2 atou2(const wchar_t* str) {
	XMUINT2 v;
	swscanf_s(str, L"(%u,%u)", &v.x, &v.y);
	return v;
}
XMUINT3 atou3(const wchar_t* str) {
	XMUINT3 v;
	swscanf_s(str, L"(%u,%u,%u)", &v.x, &v.y, &v.z);
	return v;
}
XMUINT4 atou4(const wchar_t* str) {
	XMUINT4 v;
	swscanf_s(str, L"(%u,%u,%u,%u)", &v.x, &v.y, &v.z, &v.w);
	return v;
}

Shader::PARAM_TYPE ParseType(jwstring &str, jwstring &range) {
	static const std::unordered_map<jwstring, Shader::PARAM_TYPE> paramTypes {
		{ L"cbuf",	Shader::PARAM_TYPE_CBUFFER },
		{ L"srv",	Shader::PARAM_TYPE_SRV },
		{ L"uav",	Shader::PARAM_TYPE_UAV },
		{ L"samp",	Shader::PARAM_TYPE_SAMPLER },
		{ L"tex",	Shader::PARAM_TYPE_TEXTURE },

		{ L"floatrange", Shader::PARAM_TYPE_FLOATRANGE },
		{ L"intrange",	Shader::PARAM_TYPE_INTRANGE },
		{ L"uintrange",	Shader::PARAM_TYPE_UINTRANGE },

		{ L"rgb",	Shader::PARAM_TYPE_COLOR3 },
		{ L"rgba",	Shader::PARAM_TYPE_COLOR4 },
		
		{ L"float",	Shader::PARAM_TYPE_FLOAT },
		{ L"float2", Shader::PARAM_TYPE_FLOAT2 },
		{ L"float3", Shader::PARAM_TYPE_FLOAT3 },
		{ L"float4", Shader::PARAM_TYPE_FLOAT4 },
		
		{ L"int",	Shader::PARAM_TYPE_INT },
		{ L"int2",	Shader::PARAM_TYPE_INT2 },
		{ L"int3",	Shader::PARAM_TYPE_INT3 },
		{ L"int4",	Shader::PARAM_TYPE_INT4 },
		
		{ L"uint",	Shader::PARAM_TYPE_UINT },
		{ L"uint2",	Shader::PARAM_TYPE_UINT2 },
		{ L"uint3",	Shader::PARAM_TYPE_UINT3 },
		{ L"uint4",	Shader::PARAM_TYPE_UINT4 },
	};
	
	range = L"(0,0)";
	if (str.empty()) return Shader::PARAM_TYPE_FLOAT;

	jwstring val = str;

	size_t p = str.find(L'(');
	if (p != jstring::npos) {
		val = str.substr(0, p) + L"range";
		range = str.substr((int)p);

		wchar_t* buf = new wchar_t[range.length() + 1];
		wcscpy_s(buf, range.length() + 1, range.c_str());
		strTrim(buf);
		range = jwstring(buf);
		delete[] buf;
	}

	if (paramTypes.count(val) == 0) {
		cerr << "Invalid parameter type " << str.c_str() << "(" << val.c_str() << ")\n";
		return Shader::PARAM_TYPE_FLOAT;
	}
	return paramTypes.at(val);
}

void SetParam(Shader* shader, int rootParamIndex, int &cbo, jwstring &paramName, jwstring &tstr, jwstring &vstr){
	jwstring range;
	Shader::ShaderParameter::ShaderValue value;
	Shader::PARAM_TYPE paramType = ParseType(tstr, range);

	wstring valstr = L"0";

	if (!vstr.empty()) {
		wchar_t* buf = new wchar_t[vstr.length() + 1];
		wcscpy_s(buf, vstr.length() + 1, vstr.c_str());
		strTrim(buf);
		valstr = wstring(buf);
		delete[] buf;
	}

	switch (paramType) {
		case Shader::PARAM_TYPE_CBUFFER:
		case Shader::PARAM_TYPE_SRV:
		case Shader::PARAM_TYPE_UAV:
		case Shader::PARAM_TYPE_SAMPLER:
		case Shader::PARAM_TYPE_TEXTURE:
			break;

		case Shader::PARAM_TYPE_FLOATRANGE:
			value.floatRange = atof2(range.c_str());
			value.floatValue = (float)_wtof(valstr.c_str());
			break;
		case Shader::PARAM_TYPE_INTRANGE:
			value.intRange = atoi2(range.c_str());
			value.intValue = _wtoi(valstr.c_str());
			break;
		case Shader::PARAM_TYPE_UINTRANGE:
			value.uintRange = atou2(range.c_str());
			value.uintValue = _wtoi(valstr.c_str());
			break;

		case Shader::PARAM_TYPE_FLOAT:
			value.floatValue = (float)_wtof(valstr.c_str());
			break;
		case Shader::PARAM_TYPE_FLOAT2:
			value.float2Value = atof2(valstr.c_str());
			break;
		case Shader::PARAM_TYPE_COLOR3:
			value.float3Value = atof3(valstr.c_str());
			break;
		case Shader::PARAM_TYPE_FLOAT3:
			value.float3Value = atof3(valstr.c_str());
			break;
		case Shader::PARAM_TYPE_COLOR4:
			value.float4Value = atof4(valstr.c_str());
			break;
		case Shader::PARAM_TYPE_FLOAT4:
			value.float4Value = atof4(valstr.c_str());
			break;

		case Shader::PARAM_TYPE_INT:
			value.intValue = _wtoi(valstr.c_str());
			break;
		case Shader::PARAM_TYPE_INT2:
			value.int2Value = atoi2(valstr.c_str());
			break;
		case Shader::PARAM_TYPE_INT3:
			value.int3Value = atoi3(valstr.c_str());
			break;
		case Shader::PARAM_TYPE_INT4:
			value.int4Value = atoi4(valstr.c_str());
			break;

		case Shader::PARAM_TYPE_UINT:
			value.uintValue = _wtoi(valstr.c_str());
			break;
		case Shader::PARAM_TYPE_UINT2:
			value.uint2Value = atou2(valstr.c_str());
			break;
		case Shader::PARAM_TYPE_UINT3:
			value.uint3Value = atou3(valstr.c_str());
			break;
		case Shader::PARAM_TYPE_UINT4:
			value.uint4Value = atou4(valstr.c_str());
			break;
	}
	
	// HLSL aligns cbuffer members on a 16-byte boundary
	unsigned int size = paramSizes.at(paramType);
	unsigned int nextBoundary = 16 * ((cbo + 16) / 16);
	if (cbo + size > nextBoundary) {
		shader->AddParameter(paramName, Shader::ShaderParameter(paramType, rootParamIndex, nextBoundary, value));
		cbo = nextBoundary + size;
	} else {
		shader->AddParameter(paramName, Shader::ShaderParameter(paramType, rootParamIndex, cbo, value));
		cbo += size;
	}
}

void ParseShader(Shader* shader, std::wistream &infile, int &rootParamIndex, jwstring path) {
	int cbo = 0;
	int linenum = 0;
	std::wstring line;
	while (std::getline(infile, line)) {
		linenum++;
		PARSEMODE mode = NONE;
		Shader::SHADERSTAGE stage;
		jwstring paramType;
		jwstring paramName;

		// scan for whitespace-delimited words
		for (int i = 0; i < line.length(); i++) {
#pragma region get next word
			int parenthesisCount = 0;
			// scan to the next word
			while (i < line.length() && isspace(line[i]))
				i++;
			if (i >= line.length()) break;

			// scan to the end of the word
			int j = i;
			while (j < line.length() && (!isspace(line[j]) || parenthesisCount > 0)) {
				if (line[j] == '(') parenthesisCount++;
				if (line[j] == ')') parenthesisCount--;
				j++;
			}
			jwstring word(&line[i], j - i);
			i = j;
#pragma endregion

			if (mode == NONE) {
				// first word found on the line
				if (word == L"#pragma")
					mode = PRAGMA;
				else if (word == L"#include")
					mode = INCLUDE;
				else
					break;
				continue;
			}

			switch (mode) {
			case INCLUDE: // parse pragmas from #included file
			{
				jwstring dir = path;
				const size_t last_slash_idx = path.rfind(L'\\');
				if (std::string::npos != last_slash_idx)
					dir = path.substr(0, last_slash_idx + 1);
				jwstring inc = GetFullPathW(dir + word.substr(1, word.length() - 2));
				wifstream incstr(inc.c_str());
				if (incstr.is_open())
					ParseShader(shader, incstr, rootParamIndex, inc);
				mode = PRAGMA;
				break;
			}
			case PRAGMA: // reading pragma type
			{
				if (word == L"rootsig") {
					stage = Shader::SHADERSTAGE_ROOTSIG;
					mode = SHADERSTAGE;
				} else if (word == L"vertex") {
					stage = Shader::SHADERSTAGE_VERTEX;
					mode = SHADERSTAGE;
				} else if (word == L"hull") {
					stage = Shader::SHADERSTAGE_HULL;
					mode = SHADERSTAGE;
				} else if (word == L"domain") {
					stage = Shader::SHADERSTAGE_DOMAIN;
					mode = SHADERSTAGE;
				} else if (word == L"geometry") {
					stage = Shader::SHADERSTAGE_GEOMETRY;
					mode = SHADERSTAGE;
				} else if (word == L"pixel") {
					stage = Shader::SHADERSTAGE_PIXEL;
					mode = SHADERSTAGE;
				} else if (word == L"compute") {
					stage = Shader::SHADERSTAGE_COMPUTE;
					mode = SHADERSTAGE;
				} else if (word == L"Parameter") {
					mode = PARAMTYPE;
				}
				break;
			}
			case SHADERSTAGE: // reading entry point
			{
				mode = PRAGMA;
				HRESULT hr = shader->CompileShaderStage(path, word, stage);
				if (FAILED(hr)) {
					_com_error err(hr);
					wprintf(L"%s\n", err.ErrorMessage());
				}
				break;
			}

			case PARAMTYPE: // reading parameter type
			{
				paramType = word;
				mode = PARAMNAME;
				break;
			}
			case PARAMNAME: // reading parameter name
			{ 
				jwstring range;
				Shader::PARAM_TYPE ptype = ParseType(paramType, range);
				paramName = word;
				if (ptype <= Shader::PARAM_TYPE_TEXTURE) {
					// increase Root Parameter index since we are registering a Root Parameter
					if (cbo > 0) {
						shader->AddParameterBuffer(rootParamIndex - 1, cbo);
						wprintf(L"Root Parameter %d: Integral Constant Buffer (%d)\n", rootParamIndex - 1, cbo);
					}
					cbo = 0;
					shader->AddParameter(paramName, Shader::ShaderParameter(ptype, rootParamIndex++, 0, {}));
					wprintf(L"Root Parameter %d: %s (%s)\n", rootParamIndex - 1, paramName.c_str(), shader->GetParameter(paramName)->ToString().c_str());
					mode = PRAGMA;
				} else
					// integral value (will be placed in a cbuffer)
					mode = PARAMVALUE;
				break;
			}
			case PARAMVALUE:
			{
				// cbv value description
				int cbo1 = cbo;
				SetParam(shader, rootParamIndex - 1, cbo, paramName, paramType, word);
				wprintf(L"Root Parameter %d offset %d: %s (%s)\n", rootParamIndex - 1, cbo1, paramName.c_str(), shader->GetParameter(paramName)->ToString().c_str());
				mode = PRAGMA;
				break;
			}
			}
		}

		// if a parameter had no default value
		if (mode == PARAMVALUE)
			SetParam(shader, rootParamIndex - 1, cbo, paramName, paramType, jwstring(L""));
	}

	if (cbo > 0) {
		wprintf(L"Root Parameter %d: Integral Constant Buffer (%d)\n", rootParamIndex - 1, cbo);
		shader->AddParameterBuffer(rootParamIndex - 1, cbo);
	}
}

Shader* ShaderImporter::CompileShader(jwstring path) {
	Shader* shader = new Shader(GetNameW(path));

	wprintf(L"Compiling %s\n", shader->mName.c_str());

	int rootParamIndex = 0;
	wifstream is(path.c_str());
	ParseShader(shader, is, rootParamIndex, path);

	return shader;
}

Shader* ShaderImporter::ReadShader(jwstring path) {
	Shader* shader = new Shader(GetNameW(path));

	Shader::SHADERSTAGE stage;
	jwstring end = GetNameW(path).substr(-3);
	if (end == L"_rs")
		stage = Shader::SHADERSTAGE_ROOTSIG;
	else if (end == L"_vs")
		stage = Shader::SHADERSTAGE_VERTEX;
	else if (end == L"_hs")
		stage = Shader::SHADERSTAGE_HULL;
	else if (end == L"_ds")
		stage = Shader::SHADERSTAGE_DOMAIN;
	else if (end == L"_gs")
		stage = Shader::SHADERSTAGE_GEOMETRY;
	else if (end == L"_ps")
		stage = Shader::SHADERSTAGE_PIXEL;
	else if (end == L"_cs")
		stage = Shader::SHADERSTAGE_COMPUTE;

	shader->ReadShaderStage(path, stage);
	return shader;
}