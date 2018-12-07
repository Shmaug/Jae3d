#include "TextureImporter.hpp"

#include <comdef.h>
#include <DirectXTex.h>

#include <unordered_map>
#include <string>
#include <fstream>

#include <jvector.hpp>

using namespace std;
using namespace DirectX;

inline bool CanBeSRGB(DXGI_FORMAT format) {
	switch (format) {
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_BC1_UNORM:
	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_B8G8R8A8_UNORM:
	case DXGI_FORMAT_B8G8R8X8_UNORM:
	case DXGI_FORMAT_BC7_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
	case DXGI_FORMAT_BC1_UNORM_SRGB:
	case DXGI_FORMAT_BC2_UNORM_SRGB:
	case DXGI_FORMAT_BC3_UNORM_SRGB:
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
	case DXGI_FORMAT_BC7_UNORM_SRGB:
		return true;
	default:
		return false;
	}
}
inline DXGI_FORMAT MakeLinear(DXGI_FORMAT format) {
	switch (format) {
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		return DXGI_FORMAT_R8G8B8A8_UNORM;

	case DXGI_FORMAT_BC1_UNORM_SRGB:
		return DXGI_FORMAT_BC1_UNORM;

	case DXGI_FORMAT_BC2_UNORM_SRGB:
		return DXGI_FORMAT_BC2_UNORM;

	case DXGI_FORMAT_BC3_UNORM_SRGB:
		return DXGI_FORMAT_BC3_UNORM;

	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		return DXGI_FORMAT_B8G8R8A8_UNORM;

	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		return DXGI_FORMAT_B8G8R8X8_UNORM;

	case DXGI_FORMAT_BC7_UNORM_SRGB:
		return DXGI_FORMAT_BC7_UNORM;

	default:
		return format;
	}
}

const wchar_t* formatToString(DXGI_FORMAT f) {
	static const wchar_t* formats[120] = {
		L"UNKNOWN",
		L"R32G32B32A32_TYPELESS",
		L"R32G32B32A32_FLOAT",
		L"R32G32B32A32_UINT",
		L"R32G32B32A32_SINT",
		L"R32G32B32_TYPELESS",
		L"R32G32B32_FLOAT",
		L"R32G32B32_UINT",
		L"R32G32B32_SINT",
		L"R16G16B16A16_TYPELESS",
		L"R16G16B16A16_FLOAT",
		L"R16G16B16A16_UNORM",
		L"R16G16B16A16_UINT",
		L"R16G16B16A16_SNORM",
		L"R16G16B16A16_SINT",
		L"R32G32_TYPELESS",
		L"R32G32_FLOAT",
		L"R32G32_UINT",
		L"R32G32_SINT",
		L"R32G8X24_TYPELESS",
		L"D32_FLOAT_S8X24_UINT",
		L"R32_FLOAT_X8X24_TYPELESS",
		L"X32_TYPELESS_G8X24_UINT",
		L"R10G10B10A2_TYPELESS",
		L"R10G10B10A2_UNORM",
		L"R10G10B10A2_UINT",
		L"R11G11B10_FLOAT",
		L"R8G8B8A8_TYPELESS",
		L"R8G8B8A8_UNORM",
		L"R8G8B8A8_UNORM_SRGB",
		L"R8G8B8A8_UINT",
		L"R8G8B8A8_SNORM",
		L"R8G8B8A8_SINT",
		L"R16G16_TYPELESS",
		L"R16G16_FLOAT",
		L"R16G16_UNORM",
		L"R16G16_UINT",
		L"R16G16_SNORM",
		L"R16G16_SINT",
		L"R32_TYPELESS",
		L"D32_FLOAT",
		L"R32_FLOAT",
		L"R32_UINT",
		L"R32_SINT",
		L"R24G8_TYPELESS",
		L"D24_UNORM_S8_UINT",
		L"R24_UNORM_X8_TYPELESS",
		L"X24_TYPELESS_G8_UINT",
		L"R8G8_TYPELESS",
		L"R8G8_UNORM",
		L"R8G8_UINT",
		L"R8G8_SNORM",
		L"R8G8_SINT",
		L"R16_TYPELESS",
		L"R16_FLOAT",
		L"D16_UNORM",
		L"R16_UNORM",
		L"R16_UINT",
		L"R16_SNORM",
		L"R16_SINT",
		L"R8_TYPELESS",
		L"R8_UNORM",
		L"R8_UINT",
		L"R8_SNORM",
		L"R8_SINT",
		L"A8_UNORM",
		L"R1_UNORM",
		L"R9G9B9E5_SHAREDEXP",
		L"R8G8_B8G8_UNORM",
		L"G8R8_G8B8_UNORM",
		L"BC1_TYPELESS",
		L"BC1_UNORM",
		L"BC1_UNORM_SRGB",
		L"BC2_TYPELESS",
		L"BC2_UNORM",
		L"BC2_UNORM_SRGB",
		L"BC3_TYPELESS",
		L"BC3_UNORM",
		L"BC3_UNORM_SRGB",
		L"BC4_TYPELESS",
		L"BC4_UNORM",
		L"BC4_SNORM",
		L"BC5_TYPELESS",
		L"BC5_UNORM",
		L"BC5_SNORM",
		L"B5G6R5_UNORM",
		L"B5G5R5A1_UNORM",
		L"B8G8R8A8_UNORM",
		L"B8G8R8X8_UNORM",
		L"R10G10B10_XR_BIAS_A2_UNORM",
		L"B8G8R8A8_TYPELESS",
		L"B8G8R8A8_UNORM_SRGB",
		L"B8G8R8X8_TYPELESS",
		L"B8G8R8X8_UNORM_SRGB",
		L"BC6H_TYPELESS",
		L"BC6H_UF16",
		L"BC6H_SF16",
		L"BC7_TYPELESS",
		L"BC7_UNORM",
		L"BC7_UNORM_SRGB",
		L"AYUV",
		L"Y410",
		L"Y416",
		L"NV12",
		L"P010",
		L"P016",
		L"420_OPAQUE",
		L"YUY2",
		L"Y210",
		L"Y216",
		L"NV11",
		L"AI44",
		L"IA44",
		L"P8",
		L"A8P8",
		L"B4G4R4A4_UNORM",
		L"P208",
		L"V208",
		L"V408",
		L"FORCE_UINT"
	};
	int i = (int)f;
	if (i == 0xffffffff)
		return formats[119];
	if (i >= 130) i -= 14;
	return formats[i];
}

const unordered_map<jwstring, DXGI_FORMAT> formats{
    { L"UNKNOWN",					DXGI_FORMAT_UNKNOWN		                },
	{ L"R32G32B32A32_TYPELESS",		DXGI_FORMAT_R32G32B32A32_TYPELESS       },
	{ L"R32G32B32A32_FLOAT",			DXGI_FORMAT_R32G32B32A32_FLOAT          },
	{ L"R32G32B32A32_UINT",			DXGI_FORMAT_R32G32B32A32_UINT           },
	{ L"R32G32B32A32_SINT",			DXGI_FORMAT_R32G32B32A32_SINT           },
	{ L"R32G32B32_TYPELESS",			DXGI_FORMAT_R32G32B32_TYPELESS          },
	{ L"R32G32B32_FLOAT",			DXGI_FORMAT_R32G32B32_FLOAT             },
	{ L"R32G32B32_UINT",				DXGI_FORMAT_R32G32B32_UINT              },
	{ L"R32G32B32_SINT",				DXGI_FORMAT_R32G32B32_SINT              },
	{ L"R16G16B16A16_TYPELESS",		DXGI_FORMAT_R16G16B16A16_TYPELESS       },
	{ L"R16G16B16A16_FLOAT",			DXGI_FORMAT_R16G16B16A16_FLOAT          },
	{ L"R16G16B16A16_UNORM",			DXGI_FORMAT_R16G16B16A16_UNORM          },
	{ L"R16G16B16A16_UINT",			DXGI_FORMAT_R16G16B16A16_UINT           },
	{ L"R16G16B16A16_SNORM",			DXGI_FORMAT_R16G16B16A16_SNORM          },
	{ L"R16G16B16A16_SINT",			DXGI_FORMAT_R16G16B16A16_SINT           },
	{ L"R32G32_TYPELESS",			DXGI_FORMAT_R32G32_TYPELESS             },
	{ L"R32G32_FLOAT",				DXGI_FORMAT_R32G32_FLOAT                },
	{ L"R32G32_UINT",				DXGI_FORMAT_R32G32_UINT                 },
	{ L"R32G32_SINT",				DXGI_FORMAT_R32G32_SINT                 },
	{ L"R32G8X24_TYPELESS",			DXGI_FORMAT_R32G8X24_TYPELESS           },
	{ L"D32_FLOAT_S8X24_UINT",		DXGI_FORMAT_D32_FLOAT_S8X24_UINT        },
	{ L"R32_FLOAT_X8X24_TYPELESS",	DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS    },
	{ L"X32_TYPELESS_G8X24_UINT",	DXGI_FORMAT_X32_TYPELESS_G8X24_UINT     },
	{ L"R10G10B10A2_TYPELESS",		DXGI_FORMAT_R10G10B10A2_TYPELESS        },
	{ L"R10G10B10A2_UNORM",			DXGI_FORMAT_R10G10B10A2_UNORM           },
	{ L"R10G10B10A2_UINT",			DXGI_FORMAT_R10G10B10A2_UINT            },
	{ L"R11G11B10_FLOAT",			DXGI_FORMAT_R11G11B10_FLOAT             },
	{ L"R8G8B8A8_TYPELESS",			DXGI_FORMAT_R8G8B8A8_TYPELESS           },
	{ L"R8G8B8A8_UNORM",				DXGI_FORMAT_R8G8B8A8_UNORM              },
	{ L"R8G8B8A8_UNORM_SRGB",		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB         },
	{ L"R8G8B8A8_UINT",				DXGI_FORMAT_R8G8B8A8_UINT               },
	{ L"R8G8B8A8_SNORM",				DXGI_FORMAT_R8G8B8A8_SNORM              },
	{ L"R8G8B8A8_SINT",				DXGI_FORMAT_R8G8B8A8_SINT               },
	{ L"R16G16_TYPELESS",			DXGI_FORMAT_R16G16_TYPELESS             },
	{ L"R16G16_FLOAT",				DXGI_FORMAT_R16G16_FLOAT                },
	{ L"R16G16_UNORM",				DXGI_FORMAT_R16G16_UNORM                },
	{ L"R16G16_UINT",				DXGI_FORMAT_R16G16_UINT                 },
	{ L"R16G16_SNORM",				DXGI_FORMAT_R16G16_SNORM                },
	{ L"R16G16_SINT",				DXGI_FORMAT_R16G16_SINT                 },
	{ L"R32_TYPELESS",				DXGI_FORMAT_R32_TYPELESS                },
	{ L"D32_FLOAT",					DXGI_FORMAT_D32_FLOAT                   },
	{ L"R32_FLOAT",					DXGI_FORMAT_R32_FLOAT                   },
	{ L"R32_UINT",					DXGI_FORMAT_R32_UINT                    },
	{ L"R32_SINT",					DXGI_FORMAT_R32_SINT                    },
	{ L"R24G8_TYPELESS",				DXGI_FORMAT_R24G8_TYPELESS              },
	{ L"D24_UNORM_S8_UINT",			DXGI_FORMAT_D24_UNORM_S8_UINT           },
	{ L"R24_UNORM_X8_TYPELESS",		DXGI_FORMAT_R24_UNORM_X8_TYPELESS       },
	{ L"X24_TYPELESS_G8_UINT",		DXGI_FORMAT_X24_TYPELESS_G8_UINT        },
	{ L"R8G8_TYPELESS",				DXGI_FORMAT_R8G8_TYPELESS               },
	{ L"R8G8_UNORM",					DXGI_FORMAT_R8G8_UNORM                  },
	{ L"R8G8_UINT",					DXGI_FORMAT_R8G8_UINT                   },
	{ L"R8G8_SNORM",					DXGI_FORMAT_R8G8_SNORM                  },
	{ L"R8G8_SINT",					DXGI_FORMAT_R8G8_SINT                   },
	{ L"R16_TYPELESS",				DXGI_FORMAT_R16_TYPELESS                },
	{ L"R16_FLOAT",					DXGI_FORMAT_R16_FLOAT                   },
	{ L"D16_UNORM",					DXGI_FORMAT_D16_UNORM                   },
	{ L"R16_UNORM",					DXGI_FORMAT_R16_UNORM                   },
	{ L"R16_UINT",					DXGI_FORMAT_R16_UINT                    },
	{ L"R16_SNORM",					DXGI_FORMAT_R16_SNORM                   },
	{ L"R16_SINT",					DXGI_FORMAT_R16_SINT                    },
	{ L"R8_TYPELESS",				DXGI_FORMAT_R8_TYPELESS                 },
	{ L"R8_UNORM",					DXGI_FORMAT_R8_UNORM                    },
	{ L"R8_UINT",					DXGI_FORMAT_R8_UINT                     },
	{ L"R8_SNORM",					DXGI_FORMAT_R8_SNORM                    },
	{ L"R8_SINT",					DXGI_FORMAT_R8_SINT                     },
	{ L"A8_UNORM",					DXGI_FORMAT_A8_UNORM                    },
	{ L"R1_UNORM",					DXGI_FORMAT_R1_UNORM                    },
	{ L"R9G9B9E5_SHAREDEXP",			DXGI_FORMAT_R9G9B9E5_SHAREDEXP          },
	{ L"R8G8_B8G8_UNORM",			DXGI_FORMAT_R8G8_B8G8_UNORM             },
	{ L"G8R8_G8B8_UNORM",			DXGI_FORMAT_G8R8_G8B8_UNORM             },
	{ L"BC1_TYPELESS",				DXGI_FORMAT_BC1_TYPELESS                },
	{ L"BC1_UNORM",					DXGI_FORMAT_BC1_UNORM                   },
	{ L"BC1_UNORM_SRGB",				DXGI_FORMAT_BC1_UNORM_SRGB              },
	{ L"BC2_TYPELESS",				DXGI_FORMAT_BC2_TYPELESS                },
	{ L"BC2_UNORM",					DXGI_FORMAT_BC2_UNORM                   },
	{ L"BC2_UNORM_SRGB",				DXGI_FORMAT_BC2_UNORM_SRGB              },
	{ L"BC3_TYPELESS",				DXGI_FORMAT_BC3_TYPELESS                },
	{ L"BC3_UNORM",					DXGI_FORMAT_BC3_UNORM                   },
	{ L"BC3_UNORM_SRGB",				DXGI_FORMAT_BC3_UNORM_SRGB              },
	{ L"BC4_TYPELESS",				DXGI_FORMAT_BC4_TYPELESS                },
	{ L"BC4_UNORM",					DXGI_FORMAT_BC4_UNORM                   },
	{ L"BC4_SNORM",					DXGI_FORMAT_BC4_SNORM                   },
	{ L"BC5_TYPELESS",				DXGI_FORMAT_BC5_TYPELESS                },
	{ L"BC5_UNORM",					DXGI_FORMAT_BC5_UNORM                   },
	{ L"BC5_SNORM",					DXGI_FORMAT_BC5_SNORM                   },
	{ L"B5G6R5_UNORM",				DXGI_FORMAT_B5G6R5_UNORM                },
	{ L"B5G5R5A1_UNORM",				DXGI_FORMAT_B5G5R5A1_UNORM              },
	{ L"B8G8R8A8_UNORM",				DXGI_FORMAT_B8G8R8A8_UNORM              },
	{ L"B8G8R8X8_UNORM",				DXGI_FORMAT_B8G8R8X8_UNORM              },
	{ L"R10G10B10_XR_BIAS_A2_UNORM", DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM  },
	{ L"B8G8R8A8_TYPELESS",			DXGI_FORMAT_B8G8R8A8_TYPELESS           },
	{ L"B8G8R8A8_UNORM_SRGB",		DXGI_FORMAT_B8G8R8A8_UNORM_SRGB         },
	{ L"B8G8R8X8_TYPELESS",			DXGI_FORMAT_B8G8R8X8_TYPELESS           },
	{ L"B8G8R8X8_UNORM_SRGB",		DXGI_FORMAT_B8G8R8X8_UNORM_SRGB         },
	{ L"BC6H_TYPELESS",				DXGI_FORMAT_BC6H_TYPELESS               },
	{ L"BC6H_UF16",					DXGI_FORMAT_BC6H_UF16                   },
	{ L"BC6H_SF16",					DXGI_FORMAT_BC6H_SF16                   },
	{ L"BC7_TYPELESS",				DXGI_FORMAT_BC7_TYPELESS                },
	{ L"BC7_UNORM",					DXGI_FORMAT_BC7_UNORM                   },
	{ L"BC7_UNORM_SRGB",				DXGI_FORMAT_BC7_UNORM_SRGB              },
	{ L"AYUV",						DXGI_FORMAT_AYUV                        },
	{ L"Y410",						DXGI_FORMAT_Y410                        },
	{ L"Y416",						DXGI_FORMAT_Y416                        },
	{ L"NV12",						DXGI_FORMAT_NV12                        },
	{ L"P010",						DXGI_FORMAT_P010                        },
	{ L"P016",						DXGI_FORMAT_P016                        },
	{ L"420_OPAQUE",					DXGI_FORMAT_420_OPAQUE                  },
	{ L"YUY2",						DXGI_FORMAT_YUY2                        },
	{ L"Y210",						DXGI_FORMAT_Y210                        },
	{ L"Y216",						DXGI_FORMAT_Y216                        },
	{ L"NV11",						DXGI_FORMAT_NV11                        },
	{ L"AI44",						DXGI_FORMAT_AI44                        },
	{ L"IA44",						DXGI_FORMAT_IA44                        },
	{ L"P8",							DXGI_FORMAT_P8                          },
	{ L"A8P8",						DXGI_FORMAT_A8P8                        },
	{ L"B4G4R4A4_UNORM",				DXGI_FORMAT_B4G4R4A4_UNORM              },
	{ L"P208",						DXGI_FORMAT_P208                        },
	{ L"V208",						DXGI_FORMAT_V208                        },
	{ L"V408",						DXGI_FORMAT_V408                        },
	{ L"FORCE_UINT",					DXGI_FORMAT_FORCE_UINT                  },
};

bool skipspace(wstring &str, int &i) {
	while (i < str.length() && isspace(str[i]))
		i++;
	return i >= str.length();
}
bool skipalnum(wstring &str, int &i) {
	while (i < str.length() && (isalnum(str[i]) || str[i] == '-' || str[i] == '_' || str[i] == '.'))
		i++;
	return i >= str.length();
}
void ReadMetadata(jwstring imagePath, TextureImporter::meta &data) {
	jwstring metaPath = imagePath + L".meta";
	wifstream infile(metaPath.c_str());
	if (!infile.is_open()) {
		cerr << "Could not open " << metaPath.c_str() << "\n";
		return;
	}
	int linenum = 0;
	int mode = 0;

	struct Tag {
		jwstring name;
		jwstring value;
	};

	jwstring name;
	jwstring value;
	jvector<Tag> tags;

	wstring line;
	while (getline(infile, line)) {
		int i = 0;
		while (i < line.length()) {
			// scan to the start of word
			if (skipspace(line, i)) break;
			int j = i;
			if (skipalnum(line, i)) break;

			if (mode == 0)
				name = jwstring(line.c_str() + j, i - j);
			else if (mode == 1)
				value = jwstring(line.c_str() + j, i - j);

			if (skipspace(line, i)) break;

			if (line[i] == '=') {
				if (mode == 0)
					mode = 1;
				else {
					cerr << metaPath.c_str() << "[" << linenum << "] unexpected '=' \n";
					mode = 0;
				}
			} else if (line[i] == ';') {
				if (mode == 1) {
					mode = 0;
					tags.push_back({ name, value });
				} else {
					cerr << metaPath.c_str() << "[" << linenum << "] unexpected ';'\n";
					mode = 0;
					break;
				}
			} else {
				cerr << metaPath.c_str() << "[" << linenum << "] unexpected '" << line[i] << "'\n";
				mode = 0;
				break;
			}
			i++;
		}
		linenum++;
	}

	for (int i = 0; i < tags.size(); i++) {
		name = tags[i].name.lower();
		if (name == L"format") {
			jwstring c = tags[i].value.upper();
			if (formats.count(c))
				data.format = formats.at(c);
			else
				cerr << "Invalid format " << c.c_str() << "\n";
		} else if (name == L"mipmaps")
			data.mipMaps = tags[i].value.lower() == L"true";
		else if (name == L"linear")
			data.linear = tags[i].value.lower() == L"true";
	}
}

Texture* SaveDDS(unique_ptr<ScratchImage> &image, TexMetadata &info, jwstring name) {
	DirectX::Blob blob;
	HRESULT hr = SaveToDDSMemory(image->GetImages(), image->GetImageCount(), image->GetMetadata(), DDS_FLAGS_NONE, blob);
	if (FAILED(hr)) {
		_com_error e(hr);
		cerr << "Error saving DDS: " << e.ErrorMessage() << "\n";
		return nullptr;
	}
	return new Texture(name, (int)info.width, (int)info.height, (int)info.dimension - 1, info.format, (int)info.mipLevels, blob.GetBufferPointer(), blob.GetBufferSize());
}

Texture* ProcessImage(unique_ptr<ScratchImage> &image, TextureImporter::meta &meta, jwstring name) {
	wprintf(L"%s:\n", name.c_str());
	TexMetadata info = image->GetMetadata();

	if (CanBeSRGB(info.format)) {
		if (meta.linear) {
			image->OverrideFormat(MakeLinear(info.format));
			wprintf(L"   Making linear\n");
		} else {
			image->OverrideFormat(MakeSRGB(info.format));
			wprintf(L"   Making sRGB\n");
		}
		info = image->GetMetadata();
	}

	// convert to desired format
	if (meta.format != info.format) {
		wprintf(L"   Converting %s to %s\n", formatToString(info.format), formatToString(meta.format));
		unique_ptr<ScratchImage> tmp = make_unique<ScratchImage>();
		HRESULT hr = Convert(image->GetImages(), image->GetImageCount(), info, meta.format, TEX_FILTER_DEFAULT, TEX_THRESHOLD_DEFAULT, *tmp);
		if (FAILED(hr)) {
			cerr << "Failed to convert format: " << _com_error(hr).ErrorMessage() << "\n";
		} else {
			image.swap(tmp);
			info = image->GetMetadata();
		 }
	}

	// generate mip maps
	if (meta.mipMaps) {
		wprintf(L"   Generating mip maps\n");
		unique_ptr<ScratchImage> tmp = make_unique<ScratchImage>();
		HRESULT hr = GenerateMipMaps(image->GetImages(), image->GetImageCount(), info, TEX_FILTER_DEFAULT, (int)Texture::ComputeNumMips((int)info.width, (int)info.height), *tmp);
		if (FAILED(hr)) {
			cerr << "Failed to generate mip maps: " << _com_error(hr).ErrorMessage() << "\n";
		} else {
			image.swap(tmp);
			info = image->GetMetadata();
		}
	}

	wprintf(L"%s: %dD %s %dx%d, %d slice(s) %d mip levels\n", name.c_str(), (int)info.dimension - 1, formatToString(info.format), (int)info.width, (int)info.height, (int)info.depth, (int)info.mipLevels);

	return SaveDDS(image, info, name);
}

Texture* TextureImporter::Import(jwstring path) {
	TexMetadata info;
	HRESULT hr = GetMetadataFromWICFile(path.c_str(), WIC_FLAGS_ALL_FRAMES, info);
	if (FAILED(hr)) {
		_com_error err(hr);
		wprintf(L"Failed to read %s: %s\n", path.c_str(), err.ErrorMessage());
		return nullptr;
	}

	meta metadata;
	metadata.mipMaps = true;
	metadata.format = info.format;
	ReadMetadata(path, metadata);

	return Import(path, metadata);
}
Texture* TextureImporter::Import(jwstring path, meta &metadata) {
	TexMetadata info;
	auto image = make_unique<ScratchImage>();
	HRESULT hr;
	if (GetExtW(path) == L"tga")
		hr = LoadFromTGAFile(path.c_str(), &info, *image);
	else
		hr = LoadFromWICFile(path.c_str(), WIC_FLAGS_ALL_FRAMES, &info, *image);
	if (FAILED(hr)) {
		_com_error err(hr);
		wprintf(L"Failed to read %s: %s\n", path.c_str(), err.ErrorMessage());
		return nullptr;
	}
	return ProcessImage(image, metadata, GetNameW(path));
}

Texture* TextureImporter::ImportDDS(jwstring path) {
	auto image = make_unique<ScratchImage>();
	TexMetadata info;
	HRESULT hr = LoadFromDDSFile(path.c_str(), DDS_FLAGS_NONE, &info, *image);
	if (FAILED(hr)) {
		_com_error err(hr);
		wprintf(L"Failed to read %s: %s\n", path.c_str(), err.ErrorMessage());
		return nullptr;
	}
	return SaveDDS(image, info, path);
}