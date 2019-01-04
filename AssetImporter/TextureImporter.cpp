#include "TextureImporter.hpp"

#include <comdef.h>
#include <DirectXTex.h>

#include <unordered_map>
#include <string>
#include <cwctype>
#include <fstream>

#include <jvector.hpp>

using namespace std;
using namespace DirectX;

Texture* ConvertTexture(unique_ptr<ScratchImage> &image, AssetMetadata &meta, jwstring name) {
	const TexMetadata& info = image->GetMetadata();

	DirectX::Blob blob;
	HRESULT hr = SaveToDDSMemory(image->GetImages(), image->GetImageCount(), image->GetMetadata(), DDS_FLAGS_NONE, blob);
	if (FAILED(hr)) {
		_com_error e(hr);
		cerr << "Error saving DDS: " << e.ErrorMessage() << "\n";
		return nullptr;
	}
	D3D12_RESOURCE_DIMENSION dim;
	switch (info.dimension) {
	case TEX_DIMENSION_TEXTURE1D:
		dim = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
		break;
	case TEX_DIMENSION_TEXTURE2D:
		dim = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		break;
	case TEX_DIMENSION_TEXTURE3D:
		dim = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
		break;
	default:
		dim = D3D12_RESOURCE_DIMENSION_UNKNOWN;
		break;
	}
	return new Texture(
		name, (unsigned int)info.width, (unsigned int)info.height, (unsigned int)info.depth, dim,
		(unsigned int)info.arraySize, info.format, meta.textureAlphaMode,
		(unsigned int)info.mipLevels, blob.GetBufferPointer(), blob.GetBufferSize(), true);
}

Texture* ProcessImage(unique_ptr<ScratchImage> &image, AssetMetadata &meta, jwstring name) {
	wprintf(L"%s:\n", name.c_str());
	TexMetadata info = image->GetMetadata();

	if (meta.textureLinear) {
		image->OverrideFormat(MakeLinear(info.format));
		info = image->GetMetadata();
	} else {
		image->OverrideFormat(MakeSRGB(info.format));
		info = image->GetMetadata();
	}

	// convert to desired format
	if (meta.textureFormat != info.format) {
		wprintf(L"   Converting %s to %s\n", FormatToString.at(info.format).c_str(), FormatToString.at(meta.textureFormat).c_str());
		unique_ptr<ScratchImage> tmp = make_unique<ScratchImage>();
		HRESULT hr = Convert(image->GetImages(), image->GetImageCount(), info, meta.textureFormat, TEX_FILTER_DEFAULT, TEX_THRESHOLD_DEFAULT, *tmp);
		if (FAILED(hr)) {
			cerr << "Failed to convert format: " << utf16toUtf8(_com_error(hr).ErrorMessage()).c_str() << "\n";
		} else {
			image.swap(tmp);
			info = image->GetMetadata();
		 }
	}

	// generate mip maps
	if (meta.textureMipMaps) {
		wprintf(L"   Generating mip maps\n");
		unique_ptr<ScratchImage> tmp = make_unique<ScratchImage>();
		HRESULT hr = GenerateMipMaps(image->GetImages(), image->GetImageCount(), info, TEX_FILTER_DEFAULT, (int)Texture::ComputeNumMips((int)info.width, (int)info.height), *tmp);
		if (FAILED(hr)) {
			cerr << "Failed to generate mip maps: " << utf16toUtf8(_com_error(hr).ErrorMessage()).c_str() << "\n";
		} else {
			image.swap(tmp);
			info = image->GetMetadata();
		}
	}

	wprintf(L"%s: %dD %s %dx%d, %d slice(s) %d mip levels\n", name.c_str(), (int)info.dimension - 1, FormatToString.at(info.format).c_str(), (int)info.width, (int)info.height, (int)info.depth, (int)info.mipLevels);

	return ConvertTexture(image, meta, name);
}

// Imports a texture and metadata and stores in meta
void ImportTexture(jwstring path, jvector<AssetMetadata> &meta) {
	AssetMetadata m(path);
	m.asset = std::shared_ptr<Asset>(ImportTexture(path, m));
	meta.push_back(m);
}
// Imports a texture with settings in a file [path].meta
Texture* ImportTexture(jwstring path) {
	AssetMetadata metadata(path);
	return ImportTexture(path, metadata);
}
// Imports a texture with the settings in metadata
Texture* ImportTexture(jwstring path, AssetMetadata &metadata) {
	jwstring ext = GetExtW(path).lower();
	TexMetadata info;
	auto image = make_unique<ScratchImage>();
	HRESULT hr;
	if (ext == L"tga")
		hr = LoadFromTGAFile(path.c_str(), &info, *image);
	else if (ext == L"dds")
		hr = LoadFromDDSFile(path.c_str(), DDS_FLAGS_NONE, &info, *image);
	else
	hr = LoadFromWICFile(path.c_str(), WIC_FLAGS_ALL_FRAMES, &info, *image);
	if (FAILED(hr)) {
		_com_error err(hr);
		wprintf(L"Failed to read %s: %s\n", path.c_str(), err.ErrorMessage());
		return nullptr;
	}
	return ProcessImage(image, metadata, GetNameW(path));
}