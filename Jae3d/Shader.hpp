#pragma once

#include "Common.hpp"
#include "Asset.hpp"
#include "Mesh.hpp"

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

class Shader : public Asset {
public:
	JAE_API Shader(jwstring name);
	JAE_API Shader(jwstring name, MemoryStream &ms);
	JAE_API ~Shader();

	JAE_API void Upload();

	JAE_API HRESULT ReadShaderStage(jwstring path, SHADERSTAGE stage);
	JAE_API HRESULT CompileShaderStage(jwstring path, jwstring entryPoint, SHADERSTAGE stage);
	JAE_API HRESULT CompileShaderStage(const char* text, const char* entryPoint, SHADERSTAGE stage);

	JAE_API void WriteData(MemoryStream &ms);
	JAE_API uint64_t TypeId();

	ID3D12RootSignature* GetRootSig() const { return mRootSignature.Get(); }
	ID3DBlob* GetBlob(SHADERSTAGE stage) const { return mBlobs[stage]; }
	ShaderParameter* GetParameter(jwstring name) const { return &mParams.at(name); }

	// Not to be used at runtime
	void AddParameter(jwstring name, ShaderParameter &param) { mParams.emplace(name, param); }
	// Not to be used at runtime
	void AddParameterBuffer(int rootIndex, int size) { mCBufferParameters.push_back(ShaderParameterBuffer(rootIndex, size)); }
	int GetParameterBufferCount() const { return (int)mCBufferParameters.size(); }
	ShaderParameterBuffer GetParameterBuffer(int index) const { return mCBufferParameters[index]; }

	bool HasParameters() const { return !mParams.empty(); }
	jmap<jwstring, ShaderParameter>::jmap_iterator ParameterBegin() { return mParams.begin(); }

private:
	friend class CommandList;
	JAE_API bool SetActive(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList);
	JAE_API void SetPSO(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList, ShaderState &state);
	JAE_API void SetCompute(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList);

	jmap<ShaderState, _WRL::ComPtr<ID3D12PipelineState>> mStates;
	_WRL::ComPtr<ID3D12PipelineState> mComputePSO;

	JAE_API _WRL::ComPtr<ID3D12PipelineState> CreatePSO(ShaderState &state);
	void CreateComputePSO();

	bool mCreated = false;
	_WRL::ComPtr<ID3D12RootSignature> mRootSignature;

	jvector<ShaderParameterBuffer> mCBufferParameters;

	jmap<jwstring, ShaderParameter> mParams;
	ID3DBlob* mBlobs[7] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
};