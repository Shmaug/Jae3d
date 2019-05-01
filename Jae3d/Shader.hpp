#pragma once

#include <unordered_map>
#include <unordered_set>
#include <algorithm>

#include "Common.hpp"
#include "Asset.hpp"
#include "Mesh.hpp"

#ifdef max
#undef max
#endif

class Shader : public Asset {
public:
	JAE_API Shader(const jwstring& name);
	JAE_API Shader(const jwstring& name, MemoryStream &ms);
	JAE_API ~Shader();

	// Serialize to a memorystream
	JAE_API void WriteData(MemoryStream &ms) override;
	JAE_API uint64_t TypeId() override;

	// Creates the root signature
	JAE_API void Upload();

	// Compile a shader stage from a file
	JAE_API HRESULT CompileShaderStage(const jwstring& path, const jstring& entryPoint, SHADER_STAGE stage, const jvector<jstring> &includePaths, const jvector<jstring> &keywords);
	// Compile a shader stage from memory
	JAE_API HRESULT CompileShaderStage(const char* text, const char* entryPoint, SHADER_STAGE stage, const jvector<jstring> &keywords);

	bool HasParameter(const jstring& name) const { return mParams.count(name); }
	const ShaderParameter& GetParameter(const jstring& name) const { return mParams.at(name); }

	unsigned int GetParameterBufferCount() const { return (unsigned int)mCBufferParameters.size(); }
	const ShaderParameterBuffer& GetParameterBuffer(unsigned int index) const { return mCBufferParameters[index]; }

	bool HasParameters() const { return !mParams.empty(); }
	std::unordered_map<jstring, ShaderParameter>::const_iterator ParameterBegin() { return mParams.begin(); }
	std::unordered_map<jstring, ShaderParameter>::const_iterator ParameterEnd() { return mParams.end(); }

	JAE_API jstring KeywordListToString(const jvector<jstring>& keywords, bool sorted = false) const;

	// Shouldn't be called at runtime (used by AssetImporter)
	void AddParameter(const jstring& name, ShaderParameter &param) {
		mParams.emplace(name, param);
		mRootParamCount = std::max(mRootParamCount, param.RootIndex() + 1);
	}
	// Shouldn't be called at runtime (used by AssetImporter)
	void AddParameterBuffer(unsigned int rootIndex, unsigned int size) {
		mCBufferParameters.push_back(ShaderParameterBuffer(rootIndex, size));
		mRootParamCount = std::max(mRootParamCount, (unsigned int)rootIndex + 1);
	}

private:
	friend class CommandList;
	// Sets the root signature of this shader on the GPU (called by CommandList)
	JAE_API bool SetActive(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList);
	// Sets a PSO with this shader and the passed in state (called by CommandList)
	JAE_API _WRL::ComPtr<ID3D12PipelineState> GetOrCreatePSO(const ShaderState &state);
	// Sets the compute root signature and PSO on the GPU
	JAE_API void SetCompute(const _WRL::ComPtr<ID3D12GraphicsCommandList2>& commandList, const ShaderState &state);

	JAE_API _WRL::ComPtr<ID3D12PipelineState> CreatePSO(const ShaderState &state);
	JAE_API void CreateComputePSO(const ShaderState &state);

	std::unordered_set<jstring> mKeywords;
	std::unordered_map<ShaderState, _WRL::ComPtr<ID3D12PipelineState>> mStates;
	std::unordered_map<ShaderState, _WRL::ComPtr<ID3D12PipelineState>> mComputeStates;

	bool mCreated = false;
	_WRL::ComPtr<ID3D12RootSignature> mRootSignature;

	// numeric values specified in the shader by #pragma Parameter <type> <name> <default>
	jvector<ShaderParameterBuffer> mCBufferParameters;

	unsigned int mRootParamCount;
	std::unordered_map<jstring, ShaderParameter> mParams;
	// blobs by keywords
	std::unordered_map<jstring, _WRL::ComPtr<ID3DBlob>*> mBlobs;
};