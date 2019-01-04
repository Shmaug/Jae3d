#include "ShaderImporter.hpp"

#include <string>
#include <sstream>

#include <shlwapi.h>
#pragma comment (lib, "shlwapi.lib")

#include <cassert>
#include <comdef.h>
#include <unordered_map>
#include <unordered_set>

using namespace std;
using namespace DirectX;

enum PARSEMODE {
	PARSEMODE_NONE,
	PARSEMODE_PRAGMA,
	PARSEMODE_INCLUDE,
	PARSEMODE_SHADERSTAGE,
	PARSEMODE_PARAMTYPE,
	PARSEMODE_PARAMNAME,
	PARSEMODE_PARAMVALUE,
	PARSEMODE_TABLESIZE,
	PARSEMODE_MULTI_COMPILE
};

const unordered_map<SHADER_PARAM_TYPE, unsigned int> paramSizes = {
	{ SHADER_PARAM_TYPE_CBUFFER, 0 },
	{ SHADER_PARAM_TYPE_SRV, 0 },
	{ SHADER_PARAM_TYPE_UAV, 0 },
	{ SHADER_PARAM_TYPE_SAMPLER, 0 },
	{ SHADER_PARAM_TYPE_TEXTURE, 0 },
	{ SHADER_PARAM_TYPE_TABLE, 0 },
	
	{ SHADER_PARAM_TYPE_FLOATRANGE, 4 },
	{ SHADER_PARAM_TYPE_INTRANGE, 4 },
	{ SHADER_PARAM_TYPE_UINTRANGE, 4 },

	{ SHADER_PARAM_TYPE_COLOR3, 12 },
	{ SHADER_PARAM_TYPE_COLOR4, 16 },

	{ SHADER_PARAM_TYPE_FLOAT,  4 },
	{ SHADER_PARAM_TYPE_FLOAT2, 8 },
	{ SHADER_PARAM_TYPE_FLOAT3, 12 },
	{ SHADER_PARAM_TYPE_FLOAT4, 16 },

	{ SHADER_PARAM_TYPE_INT,  4 },
	{ SHADER_PARAM_TYPE_INT2, 8 },
	{ SHADER_PARAM_TYPE_INT3, 12 },
	{ SHADER_PARAM_TYPE_INT4, 16 },

	{ SHADER_PARAM_TYPE_UINT,  4 },
	{ SHADER_PARAM_TYPE_UINT2, 8 },
	{ SHADER_PARAM_TYPE_UINT3, 12 },
	{ SHADER_PARAM_TYPE_UINT4, 16 },
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

SHADER_PARAM_TYPE ParseType(jwstring &str, jwstring &range) {
	static const std::unordered_map<jwstring, SHADER_PARAM_TYPE> paramTypes {
		{ L"cbuf",	SHADER_PARAM_TYPE_CBUFFER },
		{ L"srv",	SHADER_PARAM_TYPE_SRV },
		{ L"uav",	SHADER_PARAM_TYPE_UAV },
		{ L"samp",	SHADER_PARAM_TYPE_SAMPLER },
		{ L"tex",	SHADER_PARAM_TYPE_TEXTURE },
		{ L"tbl",	SHADER_PARAM_TYPE_TABLE },

		{ L"floatrange", SHADER_PARAM_TYPE_FLOATRANGE },
		{ L"intrange",	SHADER_PARAM_TYPE_INTRANGE },
		{ L"uintrange",	SHADER_PARAM_TYPE_UINTRANGE },

		{ L"rgb",	SHADER_PARAM_TYPE_COLOR3 },
		{ L"rgba",	SHADER_PARAM_TYPE_COLOR4 },
		
		{ L"float",	SHADER_PARAM_TYPE_FLOAT },
		{ L"float2", SHADER_PARAM_TYPE_FLOAT2 },
		{ L"float3", SHADER_PARAM_TYPE_FLOAT3 },
		{ L"float4", SHADER_PARAM_TYPE_FLOAT4 },
		
		{ L"int",	SHADER_PARAM_TYPE_INT },
		{ L"int2",	SHADER_PARAM_TYPE_INT2 },
		{ L"int3",	SHADER_PARAM_TYPE_INT3 },
		{ L"int4",	SHADER_PARAM_TYPE_INT4 },
		
		{ L"uint",	SHADER_PARAM_TYPE_UINT },
		{ L"uint2",	SHADER_PARAM_TYPE_UINT2 },
		{ L"uint3",	SHADER_PARAM_TYPE_UINT3 },
		{ L"uint4",	SHADER_PARAM_TYPE_UINT4 },
	};
	
	range = L"(0,0)";
	if (str.empty()) return SHADER_PARAM_TYPE_FLOAT;

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
		return SHADER_PARAM_TYPE_FLOAT;
	}
	return paramTypes.at(val);
}

void SetCBParam(Shader* shader, int rootParamIndex, int &cbo, jwstring &paramName, jwstring &tstr, jwstring &vstr){
	jwstring range;
	ShaderParameter::ShaderValue value;
	SHADER_PARAM_TYPE paramType = ParseType(tstr, range);

	wstring valstr = L"0";

	if (!vstr.empty()) {
		wchar_t* buf = new wchar_t[vstr.length() + 1];
		wcscpy_s(buf, vstr.length() + 1, vstr.c_str());
		strTrim(buf);
		valstr = wstring(buf);
		delete[] buf;
	}

	switch (paramType) {
		case SHADER_PARAM_TYPE_CBUFFER:
		case SHADER_PARAM_TYPE_SRV:
		case SHADER_PARAM_TYPE_UAV:
		case SHADER_PARAM_TYPE_SAMPLER:
		case SHADER_PARAM_TYPE_TEXTURE:
			break;

		case SHADER_PARAM_TYPE_FLOATRANGE:
			value.floatRange = atof2(range.c_str());
			value.floatValue = (float)_wtof(valstr.c_str());
			break;
		case SHADER_PARAM_TYPE_INTRANGE:
			value.intRange = atoi2(range.c_str());
			value.intValue = _wtoi(valstr.c_str());
			break;
		case SHADER_PARAM_TYPE_UINTRANGE:
			value.uintRange = atou2(range.c_str());
			value.uintValue = _wtoi(valstr.c_str());
			break;

		case SHADER_PARAM_TYPE_FLOAT:
			value.floatValue = (float)_wtof(valstr.c_str());
			break;
		case SHADER_PARAM_TYPE_FLOAT2:
			value.float2Value = atof2(valstr.c_str());
			break;
		case SHADER_PARAM_TYPE_COLOR3:
			value.float3Value = atof3(valstr.c_str());
			break;
		case SHADER_PARAM_TYPE_FLOAT3:
			value.float3Value = atof3(valstr.c_str());
			break;
		case SHADER_PARAM_TYPE_COLOR4:
			value.float4Value = atof4(valstr.c_str());
			break;
		case SHADER_PARAM_TYPE_FLOAT4:
			value.float4Value = atof4(valstr.c_str());
			break;

		case SHADER_PARAM_TYPE_INT:
			value.intValue = _wtoi(valstr.c_str());
			break;
		case SHADER_PARAM_TYPE_INT2:
			value.int2Value = atoi2(valstr.c_str());
			break;
		case SHADER_PARAM_TYPE_INT3:
			value.int3Value = atoi3(valstr.c_str());
			break;
		case SHADER_PARAM_TYPE_INT4:
			value.int4Value = atoi4(valstr.c_str());
			break;

		case SHADER_PARAM_TYPE_UINT:
			value.uintValue = _wtoi(valstr.c_str());
			break;
		case SHADER_PARAM_TYPE_UINT2:
			value.uint2Value = atou2(valstr.c_str());
			break;
		case SHADER_PARAM_TYPE_UINT3:
			value.uint3Value = atou3(valstr.c_str());
			break;
		case SHADER_PARAM_TYPE_UINT4:
			value.uint4Value = atou4(valstr.c_str());
			break;
	}
	
	// HLSL aligns cbuffer members on a 16-byte boundary
	unsigned int size = paramSizes.at(paramType);
	unsigned int nextBoundary = 16 * ((cbo + 16) / 16);
	if (cbo + size > nextBoundary) {
		shader->AddParameter(paramName, ShaderParameter(paramType, rootParamIndex, nextBoundary, value));
		cbo = nextBoundary + size;
	} else {
		shader->AddParameter(paramName, ShaderParameter(paramType, rootParamIndex, cbo, value));
		cbo += size;
	}
}

struct ShaderStageCompile {
	jwstring entryPoint;
	SHADER_STAGE stage;
};
void ParseShader(Shader* shader, std::wistream &infile, int &rootParamIndex, jwstring path, jvector<jwstring> includePaths, jvector<jvector<jstring>> &keywords, jvector<ShaderStageCompile> &stages) {
	int cbo = 0;
	int linenum = 0;
	std::wstring line;
	while (std::getline(infile, line)) {
		linenum++;
		PARSEMODE mode = PARSEMODE_NONE;
		SHADER_STAGE stage;
		jwstring paramType;
		jwstring paramName;

		unsigned int kwc = (unsigned int)keywords.size();

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

			if (mode == PARSEMODE_NONE) {
				// first word found on the line
				if (word == L"#pragma")
					mode = PARSEMODE_PRAGMA;
				else if (word == L"#include")
					mode = PARSEMODE_INCLUDE;
				else
					break;
				continue;
			}

			switch (mode) {
			case PARSEMODE_INCLUDE: // parse pragmas from #included file
			{
				jwstring inc;
				if (word[0] == '"') {
					jwstring dir = GetDirectoryW(path);
					inc = GetFullPathW(dir + L"\\" + word.substr(1, word.length() - 2));
				} else if (word[0] == '<') {
					// System include
					for (unsigned int i = 0; i < includePaths.size(); i++) {
						inc = GetFullPathW(includePaths[i] + L"\\" + word.substr(1, word.length() - 2));
						if (PathFileExistsW(inc.c_str()))
							break;
					}
				}
				wifstream incstr(inc.c_str());
				if (incstr.is_open())
					ParseShader(shader, incstr, rootParamIndex, inc, includePaths, keywords, stages);
				mode = PARSEMODE_PRAGMA;
				break;
			}
			case PARSEMODE_PRAGMA: // reading pragma type
			{
				if (word == L"rootsig") {
					stage = SHADER_STAGE_ROOTSIG;
					mode = PARSEMODE_SHADERSTAGE;
				} else if (word == L"vertex") {
					stage = SHADER_STAGE_VERTEX;
					mode = PARSEMODE_SHADERSTAGE;
				} else if (word == L"hull") {
					stage = SHADER_STAGE_HULL;
					mode = PARSEMODE_SHADERSTAGE;
				} else if (word == L"domain") {
					stage = SHADER_STAGE_DOMAIN;
					mode = PARSEMODE_SHADERSTAGE;
				} else if (word == L"geometry") {
					stage = SHADER_STAGE_GEOMETRY;
					mode = PARSEMODE_SHADERSTAGE;
				} else if (word == L"pixel") {
					stage = SHADER_STAGE_PIXEL;
					mode = PARSEMODE_SHADERSTAGE;
				} else if (word == L"compute") {
					stage = SHADER_STAGE_COMPUTE;
					mode = PARSEMODE_SHADERSTAGE;
				} else if (word == L"Parameter") {
					mode = PARSEMODE_PARAMTYPE;
				} else if (word == L"multi_compile") {
					mode = PARSEMODE_MULTI_COMPILE;
				}
				break;
			}
			case PARSEMODE_MULTI_COMPILE:
			{
				jstring keyword = utf16toUtf8(word);
				for (unsigned int i = 0; i < kwc; i++) {
					jvector<jstring> k = keywords[i];
					k.push_back(keyword);
					keywords.push_back(k);
				}
				break;
			}
			case PARSEMODE_SHADERSTAGE: // reading entry point
			{
				mode = PARSEMODE_PRAGMA;
				stages.push_back({ word, stage });
				break;
			}

			case PARSEMODE_PARAMTYPE: // reading parameter type
			{
				paramType = word;
				if (paramType == L"StaticSampler") rootParamIndex++;
				mode = PARSEMODE_PARAMNAME;
				break;
			}
			case PARSEMODE_PARAMNAME: // reading parameter name
			{ 
				jwstring range;
				SHADER_PARAM_TYPE ptype = ParseType(paramType, range);
				paramName = word;
				if (ptype <= SHADER_PARAM_TYPE_TABLE) {
					// increase Root Parameter index since we are registering a Root Parameter
					if (cbo > 0) {
						shader->AddParameterBuffer(rootParamIndex - 1, cbo);
						wprintf(L"Root Parameter %d: Integral Constant Buffer (%d)\n", rootParamIndex - 1, cbo);
					}
					cbo = 0;
					if (ptype <= SHADER_PARAM_TYPE_TEXTURE) {
						shader->AddParameter(paramName, ShaderParameter(ptype, rootParamIndex++, 0, {}));
						wprintf(L"Root Parameter %d: %s (%s)\n", rootParamIndex - 1, paramName.c_str(), shader->GetParameter(paramName).ToString().c_str());
						mode = PARSEMODE_PRAGMA;
					} else
						mode = PARSEMODE_TABLESIZE;
				} else
					// integral value (will be placed in a cbuffer)
					mode = PARSEMODE_PARAMVALUE;
				break;
			}
			case PARSEMODE_PARAMVALUE:
			{
				// cbv value description
				int cbo1 = cbo;
				SetCBParam(shader, rootParamIndex - 1, cbo, paramName, paramType, word);
				wprintf(L"Root Parameter %d offset %d: %s (%s)\n", rootParamIndex - 1, cbo1, paramName.c_str(), shader->GetParameter(paramName).ToString().c_str());
				mode = PARSEMODE_PRAGMA;
				break;
			}
			case PARSEMODE_TABLESIZE:
			{
				// cbv value description
				int s = _wtoi(word.c_str());
				shader->AddParameter(paramName, ShaderParameter(SHADER_PARAM_TYPE_TABLE, rootParamIndex++, s));
				wprintf(L"Root Parameter %d: Table %s [%d]\n", rootParamIndex - 1, paramName.c_str(), s);
				mode = PARSEMODE_PRAGMA;
				break;
			}
			}
		}

		// if a parameter had no default value
		if (mode == PARSEMODE_PARAMVALUE)
			SetCBParam(shader, rootParamIndex - 1, cbo, paramName, paramType, jwstring(L""));
	}

	if (cbo > 0) {
		wprintf(L"Root Parameter %d: Integral Constant Buffer (%d)\n", rootParamIndex - 1, cbo);
		shader->AddParameterBuffer(rootParamIndex - 1, cbo);
	}
}

void CompileShader(jwstring path, jvector<AssetMetadata> &meta, jvector<jwstring> includePaths) {
	AssetMetadata m(path);
	m.asset = std::shared_ptr<Asset>(CompileShader(path, includePaths));
	meta.push_back(m);
}
Shader* CompileShader(jwstring path, jvector<jwstring> includePaths) {
	Shader* shader = new Shader(GetNameW(path));

	wprintf(L"Compiling %s\n", shader->mName.c_str());

	jvector<jvector<jstring>> keywords;
	keywords.push_back(jvector<jstring>());
	keywords[0].push_back("");

	jvector<ShaderStageCompile> stages;

	int rootParamIndex = 0;
	wifstream is(path.c_str());
	ParseShader(shader, is, rootParamIndex, path, includePaths, keywords, stages);

	wprintf(L"  %d variants:\n", (int)keywords.size());

	for (unsigned int i = 0; i < keywords.size(); i++){
		for (unsigned int j = 0; j < stages.size(); j++) {
			HRESULT hr = shader->CompileShaderStage(path, stages[j].entryPoint, stages[j].stage, includePaths, keywords[i]);
			if (FAILED(hr)) {
				_com_error err(hr);
				wprintf(L"%s\n", err.ErrorMessage());
			}
		}
	}

	return shader;
}