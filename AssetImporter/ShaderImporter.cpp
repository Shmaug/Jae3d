#include "ShaderImporter.hpp"

#include <string>

#include <cassert>
#include <comdef.h>
#include <unordered_map>

#include "..\Common\ShaderAsset.hpp"
#include "AssetImporter.hpp"

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

const unordered_map<ShaderAsset::PARAM_TYPE, unsigned int> paramSizes = {
	{ ShaderAsset::PARAM_TYPE_CBUFFER, 0 },
	{ ShaderAsset::PARAM_TYPE_SRV, 0 },
	{ ShaderAsset::PARAM_TYPE_UAV, 0 },
	{ ShaderAsset::PARAM_TYPE_SAMPLER, 0 },
	{ ShaderAsset::PARAM_TYPE_TEXTURE, 0 },
	
	{ ShaderAsset::PARAM_TYPE_FLOATRANGE, 4 },
	{ ShaderAsset::PARAM_TYPE_INTRANGE, 4 },
	{ ShaderAsset::PARAM_TYPE_UINTRANGE, 4 },

	{ ShaderAsset::PARAM_TYPE_COLOR3, 12 },
	{ ShaderAsset::PARAM_TYPE_COLOR4, 16 },

	{ ShaderAsset::PARAM_TYPE_FLOAT,  4 },
	{ ShaderAsset::PARAM_TYPE_FLOAT2, 8 },
	{ ShaderAsset::PARAM_TYPE_FLOAT3, 12 },
	{ ShaderAsset::PARAM_TYPE_FLOAT4, 16 },

	{ ShaderAsset::PARAM_TYPE_INT,  4 },
	{ ShaderAsset::PARAM_TYPE_INT2, 8 },
	{ ShaderAsset::PARAM_TYPE_INT3, 12 },
	{ ShaderAsset::PARAM_TYPE_INT4, 16 },

	{ ShaderAsset::PARAM_TYPE_UINT,  4 },
	{ ShaderAsset::PARAM_TYPE_UINT2, 8 },
	{ ShaderAsset::PARAM_TYPE_UINT3, 12 },
	{ ShaderAsset::PARAM_TYPE_UINT4, 16 },
};

// remove whitespace, return new length
int strTrim(char* str) {
	int count = 0;
	for (int i = 0; str[i]; i++)
		if (!isspace(str[i]))
			str[count++] = str[i];
	str[count] = '\0';
	return count;
}

XMFLOAT2 atof2(const char* str) {
	XMFLOAT2 v;
	sscanf_s(str, "(%f,%f)", &v.x, &v.y);
	return v;
}
XMFLOAT3 atof3(const char* str) {
	XMFLOAT3 v;
	sscanf_s(str, "(%f,%f,%f)", &v.x, &v.y, &v.z);
	return v;
}
XMFLOAT4 atof4(const char* str) {
	XMFLOAT4 v;
	sscanf_s(str, "(%f,%f,%f,%f)", &v.x, &v.y, &v.z, &v.w);
	return v;
}
XMINT2 atoi2(const char* str) {
	XMINT2 v;
	sscanf_s(str, "(%d,%d)", &v.x, &v.y);
	return v;
}
XMINT3 atoi3(const char* str) {
	XMINT3 v;
	sscanf_s(str, "(%d,%d,%d)", &v.x, &v.y, &v.z);
	return v;
}
XMINT4 atoi4(const char* str) {
	XMINT4 v;
	sscanf_s(str, "(%d,%d,%d,%d)", &v.x, &v.y, &v.z, &v.w);
	return v;
}
XMUINT2 atou2(const char* str) {
	XMUINT2 v;
	sscanf_s(str, "(%u,%u)", &v.x, &v.y);
	return v;
}
XMUINT3 atou3(const char* str) {
	XMUINT3 v;
	sscanf_s(str, "(%u,%u,%u)", &v.x, &v.y, &v.z);
	return v;
}
XMUINT4 atou4(const char* str) {
	XMUINT4 v;
	sscanf_s(str, "(%u,%u,%u,%u)", &v.x, &v.y, &v.z, &v.w);
	return v;
}

ShaderAsset::PARAM_TYPE ParseType(jstring &str, jstring &range) {
	static const std::unordered_map<jstring, ShaderAsset::PARAM_TYPE> paramTypes {
		{ "cbuf", ShaderAsset::PARAM_TYPE_CBUFFER },
		{ "srv", ShaderAsset::PARAM_TYPE_SRV },
		{ "uav", ShaderAsset::PARAM_TYPE_UAV },
		{ "samp", ShaderAsset::PARAM_TYPE_SAMPLER },
		{ "tex", ShaderAsset::PARAM_TYPE_TEXTURE },

		{ "floatrange", ShaderAsset::PARAM_TYPE_FLOATRANGE },
		{ "intrange", ShaderAsset::PARAM_TYPE_INTRANGE },
		{ "uintrange", ShaderAsset::PARAM_TYPE_UINTRANGE },

		{ "rgb", ShaderAsset::PARAM_TYPE_COLOR3 },
		{ "rgba", ShaderAsset::PARAM_TYPE_COLOR4 },

		{ "float", ShaderAsset::PARAM_TYPE_FLOAT },
		{ "float2", ShaderAsset::PARAM_TYPE_FLOAT2 },
		{ "float3", ShaderAsset::PARAM_TYPE_FLOAT3 },
		{ "float4", ShaderAsset::PARAM_TYPE_FLOAT4 },

		{ "int", ShaderAsset::PARAM_TYPE_INT },
		{ "int2", ShaderAsset::PARAM_TYPE_INT2 },
		{ "int3", ShaderAsset::PARAM_TYPE_INT3 },
		{ "int4", ShaderAsset::PARAM_TYPE_INT4 },

		{ "uint", ShaderAsset::PARAM_TYPE_UINT },
		{ "uint2", ShaderAsset::PARAM_TYPE_UINT2 },
		{ "uint3", ShaderAsset::PARAM_TYPE_UINT3 },
		{ "uint4", ShaderAsset::PARAM_TYPE_UINT4 },
	};
	
	range = "(0,0)";
	if (str.empty()) return ShaderAsset::PARAM_TYPE_FLOAT;

	jstring val = str;

	size_t p = str.find('(');
	if (p != jstring::npos) {
		val = str.substr(0, p) + "range";
		range = str.substr((int)p);

		char* buf = new char[range.length() + 1];
		strcpy_s(buf, range.length() + 1, range.c_str());
		strTrim(buf);
		range = jstring(buf);
		delete[] buf;
	}

	if (paramTypes.count(val) == 0) {
		cerr << "Invalid parameter type " << str.c_str() << "(" << val.c_str() << ")\n";
		return ShaderAsset::PARAM_TYPE_FLOAT;
	}
	return paramTypes.at(val);
}

void SetParam(ShaderAsset* shader, int rootParamIndex, int &cbo, jstring &paramName, jstring &tstr, jstring &vstr){
	jstring range;
	ShaderAsset::ShaderParameter::ShaderValue value;
	ShaderAsset::PARAM_TYPE paramType = ParseType(tstr, range);

	string valstr = "0";

	if (!vstr.empty()) {
		char* buf = new char[vstr.length() + 1];
		strcpy_s(buf, vstr.length() + 1, vstr.c_str());
		strTrim(buf);
		valstr = string(buf);
		delete[] buf;
	}

	switch (paramType) {
		case ShaderAsset::PARAM_TYPE_CBUFFER:
		case ShaderAsset::PARAM_TYPE_SRV:
		case ShaderAsset::PARAM_TYPE_UAV:
		case ShaderAsset::PARAM_TYPE_SAMPLER:
		case ShaderAsset::PARAM_TYPE_TEXTURE:
			break;

		case ShaderAsset::PARAM_TYPE_FLOATRANGE:
			value.floatRange = atof2(range.c_str());
			value.floatValue = (float)atof(valstr.c_str());
			break;
		case ShaderAsset::PARAM_TYPE_INTRANGE:
			value.intRange = atoi2(range.c_str());
			value.intValue = atoi(valstr.c_str());
			break;
		case ShaderAsset::PARAM_TYPE_UINTRANGE:
			value.uintRange = atou2(range.c_str());
			value.uintValue = atoi(valstr.c_str());
			break;

		case ShaderAsset::PARAM_TYPE_FLOAT:
			value.floatValue = (float)atof(valstr.c_str());
			break;
		case ShaderAsset::PARAM_TYPE_FLOAT2:
			value.float2Value = atof2(valstr.c_str());
			break;
		case ShaderAsset::PARAM_TYPE_COLOR3:
			value.float3Value = atof3(valstr.c_str());
			break;
		case ShaderAsset::PARAM_TYPE_FLOAT3:
			value.float3Value = atof3(valstr.c_str());
			break;
		case ShaderAsset::PARAM_TYPE_COLOR4:
			value.float4Value = atof4(valstr.c_str());
			break;
		case ShaderAsset::PARAM_TYPE_FLOAT4:
			value.float4Value = atof4(valstr.c_str());
			break;

		case ShaderAsset::PARAM_TYPE_INT:
			value.intValue = atoi(valstr.c_str());
			break;
		case ShaderAsset::PARAM_TYPE_INT2:
			value.int2Value = atoi2(valstr.c_str());
			break;
		case ShaderAsset::PARAM_TYPE_INT3:
			value.int3Value = atoi3(valstr.c_str());
			break;
		case ShaderAsset::PARAM_TYPE_INT4:
			value.int4Value = atoi4(valstr.c_str());
			break;

		case ShaderAsset::PARAM_TYPE_UINT:
			value.uintValue = atoi(valstr.c_str());
			break;
		case ShaderAsset::PARAM_TYPE_UINT2:
			value.uint2Value = atou2(valstr.c_str());
			break;
		case ShaderAsset::PARAM_TYPE_UINT3:
			value.uint3Value = atou3(valstr.c_str());
			break;
		case ShaderAsset::PARAM_TYPE_UINT4:
			value.uint4Value = atou4(valstr.c_str());
			break;
	}
	
	// HLSL aligns cbuffer members on a 16-byte boundary
	unsigned int size = paramSizes.at(paramType);
	unsigned int nextBoundary = 16 * ((cbo + 16) / 16);
	if (cbo + size > nextBoundary) {
		shader->AddParameter(paramName, ShaderAsset::ShaderParameter(paramType, rootParamIndex, nextBoundary, value));
		cbo = nextBoundary + size;
	} else {
		shader->AddParameter(paramName, ShaderAsset::ShaderParameter(paramType, rootParamIndex, cbo, value));
		cbo += size;
	}
}

void ParseShader(ShaderAsset* shader, jstring path, int &rootParamIndex) {
	int cbo = 0;
	int linenum = 0;
	jwstring wpath = utf8toUtf16(path);
	ifstream infile(path.c_str());
	std::string line;
	while (std::getline(infile, line)) {
		linenum++;
		PARSEMODE mode = NONE;
		ShaderAsset::SHADERSTAGE stage;
		jstring paramType;
		jstring paramName;

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
			jstring word(&line[i], j - i);
			i = j;
#pragma endregion

			if (mode == NONE) {
				// first word found on the line
				if (word == "#pragma")
					mode = PRAGMA;
				else if (word == "#include")
					mode = INCLUDE;
				else
					break;
				continue;
			}

			switch (mode) {
			case INCLUDE: // parse pragmas from #included file
			{
				jstring dir = path;
				const size_t last_slash_idx = path.rfind('\\');
				if (std::string::npos != last_slash_idx)
					dir = path.substr(0, last_slash_idx + 1);
				ParseShader(shader, GetFullPath(dir + word.substr(1, word.length() - 2)), rootParamIndex);
				mode = PRAGMA;
				break;
			}
			case PRAGMA: // reading pragma type
			{
				if (word == "rootsig") {
					stage = ShaderAsset::SHADERSTAGE_ROOTSIG;
					mode = SHADERSTAGE;
				} else if (word == "vertex") {
					stage = ShaderAsset::SHADERSTAGE_VERTEX;
					mode = SHADERSTAGE;
				} else if (word == "hull") {
					stage = ShaderAsset::SHADERSTAGE_HULL;
					mode = SHADERSTAGE;
				} else if (word == "domain") {
					stage = ShaderAsset::SHADERSTAGE_DOMAIN;
					mode = SHADERSTAGE;
				} else if (word == "geometry") {
					stage = ShaderAsset::SHADERSTAGE_GEOMETRY;
					mode = SHADERSTAGE;
				} else if (word == "pixel") {
					stage = ShaderAsset::SHADERSTAGE_PIXEL;
					mode = SHADERSTAGE;
				} else if (word == "compute") {
					stage = ShaderAsset::SHADERSTAGE_COMPUTE;
					mode = SHADERSTAGE;
				} else if (word == "Parameter") {
					mode = PARAMTYPE;
				}
				break;
			}
			case SHADERSTAGE: // reading entry point
			{
				mode = PRAGMA;
				HRESULT hr = shader->CompileShaderStage(wpath, word, stage);
				if (FAILED(hr)) {
					_com_error err(hr);
					printf("%s\n", err.ErrorMessage());
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
				jstring range;
				ShaderAsset::PARAM_TYPE ptype = ParseType(paramType, range);
				paramName = word;
				if (ptype <= ShaderAsset::PARAM_TYPE_TEXTURE) {
					// increase Root Parameter index since we are registering a Root Parameter
					if (cbo > 0) {
						shader->AddParameterBuffer(rootParamIndex - 1, cbo);
						if (AssetImporter::verbose)
							printf("Root Parameter %d: Integral Constant Buffer (%d)\n", rootParamIndex - 1, cbo);
					}
					cbo = 0;
					shader->AddParameter(paramName, ShaderAsset::ShaderParameter(ptype, rootParamIndex++, 0, {}));
					if (AssetImporter::verbose)
						printf("Root Parameter %d: %s (%s)\n", rootParamIndex - 1, paramName.c_str(), shader->GetParameter(paramName)->ToString().c_str());
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
				if (AssetImporter::verbose)
					printf("Root Parameter %d offset %d: %s (%s)\n", rootParamIndex - 1, cbo1, paramName.c_str(), shader->GetParameter(paramName)->ToString().c_str());
				mode = PRAGMA;
				break;
			}
			}
		}

		// if a parameter had no default value
		if (mode == PARAMVALUE)
			SetParam(shader, rootParamIndex - 1, cbo, paramName, paramType, jstring(""));
	}

	if (cbo > 0) {
		if (AssetImporter::verbose)
			printf("Root Parameter %d: Integral Constant Buffer (%d)\n", rootParamIndex - 1, cbo);
		shader->AddParameterBuffer(rootParamIndex - 1, cbo);
	}
}

ShaderAsset* ShaderImporter::CompileShader(jstring path) {
	ShaderAsset* shader = new ShaderAsset(GetName(path));

	if (AssetImporter::verbose)
		printf("Compiling %s\n", shader->mName.c_str());

	int rootParamIndex = 0;
	ParseShader(shader, path, rootParamIndex);

	return shader;
}