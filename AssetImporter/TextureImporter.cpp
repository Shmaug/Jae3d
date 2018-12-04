#include "TextureImporter.hpp"

#include <comdef.h>
#include <DirectXTex.h>

#include <unordered_map>
#include <string>
#include "../Common/jvector.hpp"
#include <fstream>

#include "../Common/TextureAsset.hpp"
#include "AssetImporter.hpp"

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

const char* formatToString(DXGI_FORMAT f) {
	static const char* formats[120] = {
		"UNKNOWN",
		"R32G32B32A32_TYPELESS",
		"R32G32B32A32_FLOAT",
		"R32G32B32A32_UINT",
		"R32G32B32A32_SINT",
		"R32G32B32_TYPELESS",
		"R32G32B32_FLOAT",
		"R32G32B32_UINT",
		"R32G32B32_SINT",
		"R16G16B16A16_TYPELESS",
		"R16G16B16A16_FLOAT",
		"R16G16B16A16_UNORM",
		"R16G16B16A16_UINT",
		"R16G16B16A16_SNORM",
		"R16G16B16A16_SINT",
		"R32G32_TYPELESS",
		"R32G32_FLOAT",
		"R32G32_UINT",
		"R32G32_SINT",
		"R32G8X24_TYPELESS",
		"D32_FLOAT_S8X24_UINT",
		"R32_FLOAT_X8X24_TYPELESS",
		"X32_TYPELESS_G8X24_UINT",
		"R10G10B10A2_TYPELESS",
		"R10G10B10A2_UNORM",
		"R10G10B10A2_UINT",
		"R11G11B10_FLOAT",
		"R8G8B8A8_TYPELESS",
		"R8G8B8A8_UNORM",
		"R8G8B8A8_UNORM_SRGB",
		"R8G8B8A8_UINT",
		"R8G8B8A8_SNORM",
		"R8G8B8A8_SINT",
		"R16G16_TYPELESS",
		"R16G16_FLOAT",
		"R16G16_UNORM",
		"R16G16_UINT",
		"R16G16_SNORM",
		"R16G16_SINT",
		"R32_TYPELESS",
		"D32_FLOAT",
		"R32_FLOAT",
		"R32_UINT",
		"R32_SINT",
		"R24G8_TYPELESS",
		"D24_UNORM_S8_UINT",
		"R24_UNORM_X8_TYPELESS",
		"X24_TYPELESS_G8_UINT",
		"R8G8_TYPELESS",
		"R8G8_UNORM",
		"R8G8_UINT",
		"R8G8_SNORM",
		"R8G8_SINT",
		"R16_TYPELESS",
		"R16_FLOAT",
		"D16_UNORM",
		"R16_UNORM",
		"R16_UINT",
		"R16_SNORM",
		"R16_SINT",
		"R8_TYPELESS",
		"R8_UNORM",
		"R8_UINT",
		"R8_SNORM",
		"R8_SINT",
		"A8_UNORM",
		"R1_UNORM",
		"R9G9B9E5_SHAREDEXP",
		"R8G8_B8G8_UNORM",
		"G8R8_G8B8_UNORM",
		"BC1_TYPELESS",
		"BC1_UNORM",
		"BC1_UNORM_SRGB",
		"BC2_TYPELESS",
		"BC2_UNORM",
		"BC2_UNORM_SRGB",
		"BC3_TYPELESS",
		"BC3_UNORM",
		"BC3_UNORM_SRGB",
		"BC4_TYPELESS",
		"BC4_UNORM",
		"BC4_SNORM",
		"BC5_TYPELESS",
		"BC5_UNORM",
		"BC5_SNORM",
		"B5G6R5_UNORM",
		"B5G5R5A1_UNORM",
		"B8G8R8A8_UNORM",
		"B8G8R8X8_UNORM",
		"R10G10B10_XR_BIAS_A2_UNORM",
		"B8G8R8A8_TYPELESS",
		"B8G8R8A8_UNORM_SRGB",
		"B8G8R8X8_TYPELESS",
		"B8G8R8X8_UNORM_SRGB",
		"BC6H_TYPELESS",
		"BC6H_UF16",
		"BC6H_SF16",
		"BC7_TYPELESS",
		"BC7_UNORM",
		"BC7_UNORM_SRGB",
		"AYUV",
		"Y410",
		"Y416",
		"NV12",
		"P010",
		"P016",
		"420_OPAQUE",
		"YUY2",
		"Y210",
		"Y216",
		"NV11",
		"AI44",
		"IA44",
		"P8",
		"A8P8",
		"B4G4R4A4_UNORM",
		"P208",
		"V208",
		"V408",
		"FORCE_UINT"
	};
	int i = (int)f;
	if (i == 0xffffffff)
		return formats[119];
	if (i >= 130) i -= 14;
	return formats[i];
}

const unordered_map<jstring, DXGI_FORMAT> formats{
    { "UNKNOWN",					DXGI_FORMAT_UNKNOWN		                },
	{ "R32G32B32A32_TYPELESS",		DXGI_FORMAT_R32G32B32A32_TYPELESS       },
	{ "R32G32B32A32_FLOAT",			DXGI_FORMAT_R32G32B32A32_FLOAT          },
	{ "R32G32B32A32_UINT",			DXGI_FORMAT_R32G32B32A32_UINT           },
	{ "R32G32B32A32_SINT",			DXGI_FORMAT_R32G32B32A32_SINT           },
	{ "R32G32B32_TYPELESS",			DXGI_FORMAT_R32G32B32_TYPELESS          },
	{ "R32G32B32_FLOAT",			DXGI_FORMAT_R32G32B32_FLOAT             },
	{ "R32G32B32_UINT",				DXGI_FORMAT_R32G32B32_UINT              },
	{ "R32G32B32_SINT",				DXGI_FORMAT_R32G32B32_SINT              },
	{ "R16G16B16A16_TYPELESS",		DXGI_FORMAT_R16G16B16A16_TYPELESS       },
	{ "R16G16B16A16_FLOAT",			DXGI_FORMAT_R16G16B16A16_FLOAT          },
	{ "R16G16B16A16_UNORM",			DXGI_FORMAT_R16G16B16A16_UNORM          },
	{ "R16G16B16A16_UINT",			DXGI_FORMAT_R16G16B16A16_UINT           },
	{ "R16G16B16A16_SNORM",			DXGI_FORMAT_R16G16B16A16_SNORM          },
	{ "R16G16B16A16_SINT",			DXGI_FORMAT_R16G16B16A16_SINT           },
	{ "R32G32_TYPELESS",			DXGI_FORMAT_R32G32_TYPELESS             },
	{ "R32G32_FLOAT",				DXGI_FORMAT_R32G32_FLOAT                },
	{ "R32G32_UINT",				DXGI_FORMAT_R32G32_UINT                 },
	{ "R32G32_SINT",				DXGI_FORMAT_R32G32_SINT                 },
	{ "R32G8X24_TYPELESS",			DXGI_FORMAT_R32G8X24_TYPELESS           },
	{ "D32_FLOAT_S8X24_UINT",		DXGI_FORMAT_D32_FLOAT_S8X24_UINT        },
	{ "R32_FLOAT_X8X24_TYPELESS",	DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS    },
	{ "X32_TYPELESS_G8X24_UINT",	DXGI_FORMAT_X32_TYPELESS_G8X24_UINT     },
	{ "R10G10B10A2_TYPELESS",		DXGI_FORMAT_R10G10B10A2_TYPELESS        },
	{ "R10G10B10A2_UNORM",			DXGI_FORMAT_R10G10B10A2_UNORM           },
	{ "R10G10B10A2_UINT",			DXGI_FORMAT_R10G10B10A2_UINT            },
	{ "R11G11B10_FLOAT",			DXGI_FORMAT_R11G11B10_FLOAT             },
	{ "R8G8B8A8_TYPELESS",			DXGI_FORMAT_R8G8B8A8_TYPELESS           },
	{ "R8G8B8A8_UNORM",				DXGI_FORMAT_R8G8B8A8_UNORM              },
	{ "R8G8B8A8_UNORM_SRGB",		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB         },
	{ "R8G8B8A8_UINT",				DXGI_FORMAT_R8G8B8A8_UINT               },
	{ "R8G8B8A8_SNORM",				DXGI_FORMAT_R8G8B8A8_SNORM              },
	{ "R8G8B8A8_SINT",				DXGI_FORMAT_R8G8B8A8_SINT               },
	{ "R16G16_TYPELESS",			DXGI_FORMAT_R16G16_TYPELESS             },
	{ "R16G16_FLOAT",				DXGI_FORMAT_R16G16_FLOAT                },
	{ "R16G16_UNORM",				DXGI_FORMAT_R16G16_UNORM                },
	{ "R16G16_UINT",				DXGI_FORMAT_R16G16_UINT                 },
	{ "R16G16_SNORM",				DXGI_FORMAT_R16G16_SNORM                },
	{ "R16G16_SINT",				DXGI_FORMAT_R16G16_SINT                 },
	{ "R32_TYPELESS",				DXGI_FORMAT_R32_TYPELESS                },
	{ "D32_FLOAT",					DXGI_FORMAT_D32_FLOAT                   },
	{ "R32_FLOAT",					DXGI_FORMAT_R32_FLOAT                   },
	{ "R32_UINT",					DXGI_FORMAT_R32_UINT                    },
	{ "R32_SINT",					DXGI_FORMAT_R32_SINT                    },
	{ "R24G8_TYPELESS",				DXGI_FORMAT_R24G8_TYPELESS              },
	{ "D24_UNORM_S8_UINT",			DXGI_FORMAT_D24_UNORM_S8_UINT           },
	{ "R24_UNORM_X8_TYPELESS",		DXGI_FORMAT_R24_UNORM_X8_TYPELESS       },
	{ "X24_TYPELESS_G8_UINT",		DXGI_FORMAT_X24_TYPELESS_G8_UINT        },
	{ "R8G8_TYPELESS",				DXGI_FORMAT_R8G8_TYPELESS               },
	{ "R8G8_UNORM",					DXGI_FORMAT_R8G8_UNORM                  },
	{ "R8G8_UINT",					DXGI_FORMAT_R8G8_UINT                   },
	{ "R8G8_SNORM",					DXGI_FORMAT_R8G8_SNORM                  },
	{ "R8G8_SINT",					DXGI_FORMAT_R8G8_SINT                   },
	{ "R16_TYPELESS",				DXGI_FORMAT_R16_TYPELESS                },
	{ "R16_FLOAT",					DXGI_FORMAT_R16_FLOAT                   },
	{ "D16_UNORM",					DXGI_FORMAT_D16_UNORM                   },
	{ "R16_UNORM",					DXGI_FORMAT_R16_UNORM                   },
	{ "R16_UINT",					DXGI_FORMAT_R16_UINT                    },
	{ "R16_SNORM",					DXGI_FORMAT_R16_SNORM                   },
	{ "R16_SINT",					DXGI_FORMAT_R16_SINT                    },
	{ "R8_TYPELESS",				DXGI_FORMAT_R8_TYPELESS                 },
	{ "R8_UNORM",					DXGI_FORMAT_R8_UNORM                    },
	{ "R8_UINT",					DXGI_FORMAT_R8_UINT                     },
	{ "R8_SNORM",					DXGI_FORMAT_R8_SNORM                    },
	{ "R8_SINT",					DXGI_FORMAT_R8_SINT                     },
	{ "A8_UNORM",					DXGI_FORMAT_A8_UNORM                    },
	{ "R1_UNORM",					DXGI_FORMAT_R1_UNORM                    },
	{ "R9G9B9E5_SHAREDEXP",			DXGI_FORMAT_R9G9B9E5_SHAREDEXP          },
	{ "R8G8_B8G8_UNORM",			DXGI_FORMAT_R8G8_B8G8_UNORM             },
	{ "G8R8_G8B8_UNORM",			DXGI_FORMAT_G8R8_G8B8_UNORM             },
	{ "BC1_TYPELESS",				DXGI_FORMAT_BC1_TYPELESS                },
	{ "BC1_UNORM",					DXGI_FORMAT_BC1_UNORM                   },
	{ "BC1_UNORM_SRGB",				DXGI_FORMAT_BC1_UNORM_SRGB              },
	{ "BC2_TYPELESS",				DXGI_FORMAT_BC2_TYPELESS                },
	{ "BC2_UNORM",					DXGI_FORMAT_BC2_UNORM                   },
	{ "BC2_UNORM_SRGB",				DXGI_FORMAT_BC2_UNORM_SRGB              },
	{ "BC3_TYPELESS",				DXGI_FORMAT_BC3_TYPELESS                },
	{ "BC3_UNORM",					DXGI_FORMAT_BC3_UNORM                   },
	{ "BC3_UNORM_SRGB",				DXGI_FORMAT_BC3_UNORM_SRGB              },
	{ "BC4_TYPELESS",				DXGI_FORMAT_BC4_TYPELESS                },
	{ "BC4_UNORM",					DXGI_FORMAT_BC4_UNORM                   },
	{ "BC4_SNORM",					DXGI_FORMAT_BC4_SNORM                   },
	{ "BC5_TYPELESS",				DXGI_FORMAT_BC5_TYPELESS                },
	{ "BC5_UNORM",					DXGI_FORMAT_BC5_UNORM                   },
	{ "BC5_SNORM",					DXGI_FORMAT_BC5_SNORM                   },
	{ "B5G6R5_UNORM",				DXGI_FORMAT_B5G6R5_UNORM                },
	{ "B5G5R5A1_UNORM",				DXGI_FORMAT_B5G5R5A1_UNORM              },
	{ "B8G8R8A8_UNORM",				DXGI_FORMAT_B8G8R8A8_UNORM              },
	{ "B8G8R8X8_UNORM",				DXGI_FORMAT_B8G8R8X8_UNORM              },
	{ "R10G10B10_XR_BIAS_A2_UNORM", DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM  },
	{ "B8G8R8A8_TYPELESS",			DXGI_FORMAT_B8G8R8A8_TYPELESS           },
	{ "B8G8R8A8_UNORM_SRGB",		DXGI_FORMAT_B8G8R8A8_UNORM_SRGB         },
	{ "B8G8R8X8_TYPELESS",			DXGI_FORMAT_B8G8R8X8_TYPELESS           },
	{ "B8G8R8X8_UNORM_SRGB",		DXGI_FORMAT_B8G8R8X8_UNORM_SRGB         },
	{ "BC6H_TYPELESS",				DXGI_FORMAT_BC6H_TYPELESS               },
	{ "BC6H_UF16",					DXGI_FORMAT_BC6H_UF16                   },
	{ "BC6H_SF16",					DXGI_FORMAT_BC6H_SF16                   },
	{ "BC7_TYPELESS",				DXGI_FORMAT_BC7_TYPELESS                },
	{ "BC7_UNORM",					DXGI_FORMAT_BC7_UNORM                   },
	{ "BC7_UNORM_SRGB",				DXGI_FORMAT_BC7_UNORM_SRGB              },
	{ "AYUV",						DXGI_FORMAT_AYUV                        },
	{ "Y410",						DXGI_FORMAT_Y410                        },
	{ "Y416",						DXGI_FORMAT_Y416                        },
	{ "NV12",						DXGI_FORMAT_NV12                        },
	{ "P010",						DXGI_FORMAT_P010                        },
	{ "P016",						DXGI_FORMAT_P016                        },
	{ "420_OPAQUE",					DXGI_FORMAT_420_OPAQUE                  },
	{ "YUY2",						DXGI_FORMAT_YUY2                        },
	{ "Y210",						DXGI_FORMAT_Y210                        },
	{ "Y216",						DXGI_FORMAT_Y216                        },
	{ "NV11",						DXGI_FORMAT_NV11                        },
	{ "AI44",						DXGI_FORMAT_AI44                        },
	{ "IA44",						DXGI_FORMAT_IA44                        },
	{ "P8",							DXGI_FORMAT_P8                          },
	{ "A8P8",						DXGI_FORMAT_A8P8                        },
	{ "B4G4R4A4_UNORM",				DXGI_FORMAT_B4G4R4A4_UNORM              },
	{ "P208",						DXGI_FORMAT_P208                        },
	{ "V208",						DXGI_FORMAT_V208                        },
	{ "V408",						DXGI_FORMAT_V408                        },
	{ "FORCE_UINT",					DXGI_FORMAT_FORCE_UINT                  },
};

struct meta {
	DXGI_FORMAT format;
	bool linear;
	bool mipMaps;
};

bool skipspace(string &str, int &i) {
	while (i < str.length() && isspace(str[i]))
		i++;
	return i >= str.length();
}
bool skipalnum(string &str, int &i) {
	while (i < str.length() && (isalnum(str[i]) || str[i] == '-' || str[i] == '_' || str[i] == '.'))
		i++;
	return i >= str.length();
}
void ReadMetadata(jstring imagePath, meta &data) {
	jstring metaPath = imagePath + ".meta";
	ifstream infile(metaPath.c_str());
	if (!infile.is_open()) {
		cerr << "Could not open " << metaPath.c_str() << "\n";
		return;
	}
	int linenum = 0;
	int mode = 0;

	struct Tag {
		jstring name;
		jstring value;
	};

	jstring name;
	jstring value;
	jvector<Tag> tags;

	string line;
	while (getline(infile, line)) {
		int i = 0;
		while (i < line.length()) {
			// scan to the start of word
			if (skipspace(line, i)) break;
			int j = i;
			if (skipalnum(line, i)) break;

			if (mode == 0)
				name = jstring(line.c_str() + j, i - j);
			else if (mode == 1)
				value = jstring(line.c_str() + j, i - j);

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
		if (name == "format") {
			jstring c = tags[i].value.upper();
			if (formats.count(c))
				data.format = formats.at(c);
			else
				cerr << "Invalid format " << c.c_str() << "\n";
		} else if (name == "mipmaps")
			data.mipMaps = tags[i].value.lower() == "true";
		else if (name == "linear")
			data.linear = tags[i].value.lower() == "true";
	}
}

TextureAsset* SaveDDS(unique_ptr<ScratchImage> &image, TexMetadata &info, jstring name) {
	DirectX::Blob blob;
	HRESULT hr = SaveToDDSMemory(image->GetImages(), image->GetImageCount(), image->GetMetadata(), DDS_FLAGS_NONE, blob);
	if (FAILED(hr)) {
		_com_error e(hr);
		cerr << "Error saving DDS: " << e.ErrorMessage() << "\n";
		return nullptr;
	}
	return new TextureAsset(name, (int)info.width, (int)info.height, (int)info.dimension - 1, info.format, (int)info.mipLevels, blob.GetBufferPointer(), blob.GetBufferSize());
}

TextureAsset* ProcessImage(unique_ptr<ScratchImage> &image, meta &meta, jstring name) {
	printf("%s:\n", name.c_str());
	TexMetadata info = image->GetMetadata();

	if (CanBeSRGB(info.format)) {
		if (meta.linear) {
			image->OverrideFormat(MakeLinear(info.format));
			printf("   Making linear\n");
		} else {
			image->OverrideFormat(MakeSRGB(info.format));
			printf("   Making sRGB\n");
		}
		info = image->GetMetadata();
	}

	// convert to desired format
	if (meta.format != info.format) {
		printf("   Converting %s to %s\n", formatToString(info.format), formatToString(meta.format));
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
		printf("   Generating mip maps\n");
		unique_ptr<ScratchImage> tmp = make_unique<ScratchImage>();
		HRESULT hr = GenerateMipMaps(image->GetImages(), image->GetImageCount(), info, TEX_FILTER_DEFAULT, (int)TextureAsset::ComputeNumMips((int)info.width, (int)info.height), *tmp);
		if (FAILED(hr)) {
			cerr << "Failed to generate mip maps: " << _com_error(hr).ErrorMessage() << "\n";
		} else {
			image.swap(tmp);
			info = image->GetMetadata();
		}
	}

	if (AssetImporter::verbose)
		printf("%s: %dD %s %dx%d, %d slice(s) %d mip levels\n", name.c_str(), (int)info.dimension - 1, formatToString(info.format), (int)info.width, (int)info.height, (int)info.depth, (int)info.mipLevels);

	return SaveDDS(image, info, name);
}

TextureAsset* TextureImporter::Import(jstring path) {
	TexMetadata info;
	HRESULT hr = GetMetadataFromWICFile(utf8toUtf16(path).c_str(), WIC_FLAGS_ALL_FRAMES, info);
	if (FAILED(hr)) {
		_com_error err(hr);
		printf("Failed to read %s: %s\n", path.c_str(), err.ErrorMessage());
		return nullptr;
	}

	meta metadata;
	metadata.mipMaps = true;
	metadata.format = info.format;
	ReadMetadata(path, metadata);

	auto image = make_unique<ScratchImage>();
	hr = LoadFromWICFile(utf8toUtf16(path).c_str(), WIC_FLAGS_ALL_FRAMES, &info, *image);
	if (FAILED(hr)) {
		_com_error err(hr);
		printf("Failed to read %s: %s\n", path.c_str(), err.ErrorMessage());
		return nullptr;
	}
	return ProcessImage(image, metadata, GetName(path));
}
TextureAsset* TextureImporter::ImportTGA(jstring path) {
	TexMetadata info;
	HRESULT hr = GetMetadataFromTGAFile(utf8toUtf16(path).c_str(), info);
	if (FAILED(hr)) {
		_com_error err(hr);
		printf("Failed to read %s: %s\n", path.c_str(), err.ErrorMessage());
		return nullptr;
	}

	meta metadata;
	metadata.mipMaps = true;
	metadata.format = info.format;
	ReadMetadata(path, metadata);

	auto image = make_unique<ScratchImage>();
	hr = LoadFromTGAFile(utf8toUtf16(path).c_str(), &info, *image);
	if (FAILED(hr)) {
		_com_error err(hr);
		printf("Failed to read %s: %s\n", path.c_str(), err.ErrorMessage());
		return nullptr;
	}
	return ProcessImage(image, metadata, GetName(path));
}
TextureAsset* TextureImporter::ImportDDS(jstring path) {
	auto image = make_unique<ScratchImage>();
	TexMetadata info;
	HRESULT hr = LoadFromDDSFile(utf8toUtf16(path).c_str(), DDS_FLAGS_NONE, &info, *image);
	if (FAILED(hr)) {
		_com_error err(hr);
		printf("Failed to read %s: %s\n", path.c_str(), err.ErrorMessage());
		return nullptr;
	}
	return SaveDDS(image, info, path);
}