#ifndef IMPORT_COMMON_HPP
#define IMPORT_COMMON_HPP

#include <Common.hpp>

#include <unordered_map>
#include <jstring.hpp>

#include <fstream>
#include <iostream>
#include <string>
#include <memory>

#include <type_traits>
#include <tuple>
#include <utility>
#include <jstring.hpp>

const std::unordered_map<DXGI_FORMAT, jwstring> FormatToString {
	{ DXGI_FORMAT_UNKNOWN		             , L"unknown"					},
	{ DXGI_FORMAT_R32G32B32A32_TYPELESS      , L"r32g32b32a32_typeless"		},
	{ DXGI_FORMAT_R32G32B32A32_FLOAT         , L"r32g32b32a32_float"		},
	{ DXGI_FORMAT_R32G32B32A32_UINT          , L"r32g32b32a32_uint"			},
	{ DXGI_FORMAT_R32G32B32A32_SINT          , L"r32g32b32a32_sint"			},
	{ DXGI_FORMAT_R32G32B32_TYPELESS         , L"r32g32b32_typeless"		},
	{ DXGI_FORMAT_R32G32B32_FLOAT            , L"r32g32b32_float"			},
	{ DXGI_FORMAT_R32G32B32_UINT             , L"r32g32b32_uint"			},
	{ DXGI_FORMAT_R32G32B32_SINT             , L"r32g32b32_sint"			},
	{ DXGI_FORMAT_R16G16B16A16_TYPELESS      , L"r16g16b16a16_typeless"		},
	{ DXGI_FORMAT_R16G16B16A16_FLOAT         , L"r16g16b16a16_float"		},
	{ DXGI_FORMAT_R16G16B16A16_UNORM         , L"r16g16b16a16_unorm"		},
	{ DXGI_FORMAT_R16G16B16A16_UINT          , L"r16g16b16a16_uint"			},
	{ DXGI_FORMAT_R16G16B16A16_SNORM         , L"r16g16b16a16_snorm"		},
	{ DXGI_FORMAT_R16G16B16A16_SINT          , L"r16g16b16a16_sint"			},
	{ DXGI_FORMAT_R32G32_TYPELESS            , L"r32g32_typeless"			},
	{ DXGI_FORMAT_R32G32_FLOAT               , L"r32g32_float"				},
	{ DXGI_FORMAT_R32G32_UINT                , L"r32g32_uint"				},
	{ DXGI_FORMAT_R32G32_SINT                , L"r32g32_sint"				},
	{ DXGI_FORMAT_R32G8X24_TYPELESS          , L"r32g8x24_typeless"			},
	{ DXGI_FORMAT_D32_FLOAT_S8X24_UINT       , L"d32_float_s8x24_uint"		},
	{ DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS   , L"r32_float_x8x24_typeless"	},
	{ DXGI_FORMAT_X32_TYPELESS_G8X24_UINT    , L"x32_typeless_g8x24_uint"	},
	{ DXGI_FORMAT_R10G10B10A2_TYPELESS       , L"r10g10b10a2_typeless"		},
	{ DXGI_FORMAT_R10G10B10A2_UNORM          , L"r10g10b10a2_unorm"			},
	{ DXGI_FORMAT_R10G10B10A2_UINT           , L"r10g10b10a2_uint"			},
	{ DXGI_FORMAT_R11G11B10_FLOAT            , L"r11g11b10_float"			},
	{ DXGI_FORMAT_R8G8B8A8_TYPELESS          , L"r8g8b8a8_typeless"			},
	{ DXGI_FORMAT_R8G8B8A8_UNORM             , L"r8g8b8a8_unorm"			},
	{ DXGI_FORMAT_R8G8B8A8_UNORM_SRGB        , L"r8g8b8a8_unorm_srgb"		},
	{ DXGI_FORMAT_R8G8B8A8_UINT              , L"r8g8b8a8_uint"				},
	{ DXGI_FORMAT_R8G8B8A8_SNORM             , L"r8g8b8a8_snorm"			},
	{ DXGI_FORMAT_R8G8B8A8_SINT              , L"r8g8b8a8_sint"				},
	{ DXGI_FORMAT_R16G16_TYPELESS            , L"r16g16_typeless"			},
	{ DXGI_FORMAT_R16G16_FLOAT               , L"r16g16_float"				},
	{ DXGI_FORMAT_R16G16_UNORM               , L"r16g16_unorm"				},
	{ DXGI_FORMAT_R16G16_UINT                , L"r16g16_uint"				},
	{ DXGI_FORMAT_R16G16_SNORM               , L"r16g16_snorm"				},
	{ DXGI_FORMAT_R16G16_SINT                , L"r16g16_sint"				},
	{ DXGI_FORMAT_R32_TYPELESS               , L"r32_typeless"				},
	{ DXGI_FORMAT_D32_FLOAT                  , L"d32_float"					},
	{ DXGI_FORMAT_R32_FLOAT                  , L"r32_float"					},
	{ DXGI_FORMAT_R32_UINT                   , L"r32_uint"					},
	{ DXGI_FORMAT_R32_SINT                   , L"r32_sint"					},
	{ DXGI_FORMAT_R24G8_TYPELESS             , L"r24g8_typeless"			},
	{ DXGI_FORMAT_D24_UNORM_S8_UINT          , L"d24_unorm_s8_uint"			},
	{ DXGI_FORMAT_R24_UNORM_X8_TYPELESS      , L"r24_unorm_x8_typeless"		},
	{ DXGI_FORMAT_X24_TYPELESS_G8_UINT       , L"x24_typeless_g8_uint"		},
	{ DXGI_FORMAT_R8G8_TYPELESS              , L"r8g8_typeless"				},
	{ DXGI_FORMAT_R8G8_UNORM                 , L"r8g8_unorm"				},
	{ DXGI_FORMAT_R8G8_UINT                  , L"r8g8_uint"					},
	{ DXGI_FORMAT_R8G8_SNORM                 , L"r8g8_snorm"				},
	{ DXGI_FORMAT_R8G8_SINT                  , L"r8g8_sint"					},
	{ DXGI_FORMAT_R16_TYPELESS               , L"r16_typeless"				},
	{ DXGI_FORMAT_R16_FLOAT                  , L"r16_float"					},
	{ DXGI_FORMAT_D16_UNORM                  , L"d16_unorm"					},
	{ DXGI_FORMAT_R16_UNORM                  , L"r16_unorm"					},
	{ DXGI_FORMAT_R16_UINT                   , L"r16_uint"					},
	{ DXGI_FORMAT_R16_SNORM                  , L"r16_snorm"					},
	{ DXGI_FORMAT_R16_SINT                   , L"r16_sint"					},
	{ DXGI_FORMAT_R8_TYPELESS                , L"r8_typeless"				},
	{ DXGI_FORMAT_R8_UNORM                   , L"r8_unorm"					},
	{ DXGI_FORMAT_R8_UINT                    , L"r8_uint"					},
	{ DXGI_FORMAT_R8_SNORM                   , L"r8_snorm"					},
	{ DXGI_FORMAT_R8_SINT                    , L"r8_sint"					},
	{ DXGI_FORMAT_A8_UNORM                   , L"a8_unorm"					},
	{ DXGI_FORMAT_R1_UNORM                   , L"r1_unorm"					},
	{ DXGI_FORMAT_R9G9B9E5_SHAREDEXP         , L"r9g9b9e5_sharedexp"		},
	{ DXGI_FORMAT_R8G8_B8G8_UNORM            , L"r8g8_b8g8_unorm"			},
	{ DXGI_FORMAT_G8R8_G8B8_UNORM            , L"g8r8_g8b8_unorm"			},
	{ DXGI_FORMAT_BC1_TYPELESS               , L"bc1_typeless"				},
	{ DXGI_FORMAT_BC1_UNORM                  , L"bc1_unorm"					},
	{ DXGI_FORMAT_BC1_UNORM_SRGB             , L"bc1_unorm_srgb"			},
	{ DXGI_FORMAT_BC2_TYPELESS               , L"bc2_typeless"				},
	{ DXGI_FORMAT_BC2_UNORM                  , L"bc2_unorm"					},
	{ DXGI_FORMAT_BC2_UNORM_SRGB             , L"bc2_unorm_srgb"			},
	{ DXGI_FORMAT_BC3_TYPELESS               , L"bc3_typeless"				},
	{ DXGI_FORMAT_BC3_UNORM                  , L"bc3_unorm"					},
	{ DXGI_FORMAT_BC3_UNORM_SRGB             , L"bc3_unorm_srgb"			},
	{ DXGI_FORMAT_BC4_TYPELESS               , L"bc4_typeless"				},
	{ DXGI_FORMAT_BC4_UNORM                  , L"bc4_unorm"					},
	{ DXGI_FORMAT_BC4_SNORM                  , L"bc4_snorm"					},
	{ DXGI_FORMAT_BC5_TYPELESS               , L"bc5_typeless"				},
	{ DXGI_FORMAT_BC5_UNORM                  , L"bc5_unorm"					},
	{ DXGI_FORMAT_BC5_SNORM                  , L"bc5_snorm"					},
	{ DXGI_FORMAT_B5G6R5_UNORM               , L"b5g6r5_unorm"				},
	{ DXGI_FORMAT_B5G5R5A1_UNORM             , L"b5g5r5a1_unorm"			},
	{ DXGI_FORMAT_B8G8R8A8_UNORM             , L"b8g8r8a8_unorm"			},
	{ DXGI_FORMAT_B8G8R8X8_UNORM             , L"b8g8r8x8_unorm"			},
	{ DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM , L"r10g10b10_xr_bias_a2_unorm"},
	{ DXGI_FORMAT_B8G8R8A8_TYPELESS          , L"b8g8r8a8_typeless"			},
	{ DXGI_FORMAT_B8G8R8A8_UNORM_SRGB        , L"b8g8r8a8_unorm_srgb"		},
	{ DXGI_FORMAT_B8G8R8X8_TYPELESS          , L"b8g8r8x8_typeless"			},
	{ DXGI_FORMAT_B8G8R8X8_UNORM_SRGB        , L"b8g8r8x8_unorm_srgb"		},
	{ DXGI_FORMAT_BC6H_TYPELESS              , L"bc6h_typeless"				},
	{ DXGI_FORMAT_BC6H_UF16                  , L"bc6h_uf16"					},
	{ DXGI_FORMAT_BC6H_SF16                  , L"bc6h_sf16"					},
	{ DXGI_FORMAT_BC7_TYPELESS               , L"bc7_typeless"				},
	{ DXGI_FORMAT_BC7_UNORM                  , L"bc7_unorm"					},
	{ DXGI_FORMAT_BC7_UNORM_SRGB             , L"bc7_unorm_srgb"			},
	{ DXGI_FORMAT_AYUV                       , L"ayuv"						},
	{ DXGI_FORMAT_Y410                       , L"y410"						},
	{ DXGI_FORMAT_Y416                       , L"y416"						},
	{ DXGI_FORMAT_NV12                       , L"nv12"						},
	{ DXGI_FORMAT_P010                       , L"p010"						},
	{ DXGI_FORMAT_P016                       , L"p016"						},
	{ DXGI_FORMAT_420_OPAQUE                 , L"420_opaque"				},
	{ DXGI_FORMAT_YUY2                       , L"yuy2"						},
	{ DXGI_FORMAT_Y210                       , L"y210"						},
	{ DXGI_FORMAT_Y216                       , L"y216"						},
	{ DXGI_FORMAT_NV11                       , L"nv11"						},
	{ DXGI_FORMAT_AI44                       , L"ai44"						},
	{ DXGI_FORMAT_IA44                       , L"ia44"						},
	{ DXGI_FORMAT_P8                         , L"p8"						},
	{ DXGI_FORMAT_A8P8                       , L"a8p8"						},
	{ DXGI_FORMAT_B4G4R4A4_UNORM             , L"b4g4r4a4_unorm"			},
	{ DXGI_FORMAT_P208                       , L"p208"						},
	{ DXGI_FORMAT_V208                       , L"v208"						},
	{ DXGI_FORMAT_V408                       , L"v408"						},
	{ DXGI_FORMAT_FORCE_UINT                 , L"force_uint"				}
};
const std::unordered_map<jwstring, DXGI_FORMAT> StringToFormat {
	{ L"UNKNOWN",					DXGI_FORMAT_UNKNOWN		                },
	{ L"R32G32B32A32_TYPELESS",		DXGI_FORMAT_R32G32B32A32_TYPELESS       },
	{ L"R32G32B32A32_FLOAT",		DXGI_FORMAT_R32G32B32A32_FLOAT          },
	{ L"R32G32B32A32_UINT",			DXGI_FORMAT_R32G32B32A32_UINT           },
	{ L"R32G32B32A32_SINT",			DXGI_FORMAT_R32G32B32A32_SINT           },
	{ L"R32G32B32_TYPELESS",		DXGI_FORMAT_R32G32B32_TYPELESS          },
	{ L"R32G32B32_FLOAT",			DXGI_FORMAT_R32G32B32_FLOAT             },
	{ L"R32G32B32_UINT",			DXGI_FORMAT_R32G32B32_UINT              },
	{ L"R32G32B32_SINT",			DXGI_FORMAT_R32G32B32_SINT              },
	{ L"R16G16B16A16_TYPELESS",		DXGI_FORMAT_R16G16B16A16_TYPELESS       },
	{ L"R16G16B16A16_FLOAT",		DXGI_FORMAT_R16G16B16A16_FLOAT          },
	{ L"R16G16B16A16_UNORM",		DXGI_FORMAT_R16G16B16A16_UNORM          },
	{ L"R16G16B16A16_UINT",			DXGI_FORMAT_R16G16B16A16_UINT           },
	{ L"R16G16B16A16_SNORM",		DXGI_FORMAT_R16G16B16A16_SNORM          },
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
	{ L"R8G8B8A8_UNORM",			DXGI_FORMAT_R8G8B8A8_UNORM              },
	{ L"R8G8B8A8_UNORM_SRGB",		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB         },
	{ L"R8G8B8A8_UINT",				DXGI_FORMAT_R8G8B8A8_UINT               },
	{ L"R8G8B8A8_SNORM",			DXGI_FORMAT_R8G8B8A8_SNORM              },
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
	{ L"R24G8_TYPELESS",			DXGI_FORMAT_R24G8_TYPELESS              },
	{ L"D24_UNORM_S8_UINT",			DXGI_FORMAT_D24_UNORM_S8_UINT           },
	{ L"R24_UNORM_X8_TYPELESS",		DXGI_FORMAT_R24_UNORM_X8_TYPELESS       },
	{ L"X24_TYPELESS_G8_UINT",		DXGI_FORMAT_X24_TYPELESS_G8_UINT        },
	{ L"R8G8_TYPELESS",				DXGI_FORMAT_R8G8_TYPELESS               },
	{ L"R8G8_UNORM",				DXGI_FORMAT_R8G8_UNORM                  },
	{ L"R8G8_UINT",					DXGI_FORMAT_R8G8_UINT                   },
	{ L"R8G8_SNORM",				DXGI_FORMAT_R8G8_SNORM                  },
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
	{ L"R9G9B9E5_SHAREDEXP",		DXGI_FORMAT_R9G9B9E5_SHAREDEXP          },
	{ L"R8G8_B8G8_UNORM",			DXGI_FORMAT_R8G8_B8G8_UNORM             },
	{ L"G8R8_G8B8_UNORM",			DXGI_FORMAT_G8R8_G8B8_UNORM             },
	{ L"BC1_TYPELESS",				DXGI_FORMAT_BC1_TYPELESS                },
	{ L"BC1_UNORM",					DXGI_FORMAT_BC1_UNORM                   },
	{ L"BC1_UNORM_SRGB",			DXGI_FORMAT_BC1_UNORM_SRGB              },
	{ L"BC2_TYPELESS",				DXGI_FORMAT_BC2_TYPELESS                },
	{ L"BC2_UNORM",					DXGI_FORMAT_BC2_UNORM                   },
	{ L"BC2_UNORM_SRGB",			DXGI_FORMAT_BC2_UNORM_SRGB              },
	{ L"BC3_TYPELESS",				DXGI_FORMAT_BC3_TYPELESS                },
	{ L"BC3_UNORM",					DXGI_FORMAT_BC3_UNORM                   },
	{ L"BC3_UNORM_SRGB",			DXGI_FORMAT_BC3_UNORM_SRGB              },
	{ L"BC4_TYPELESS",				DXGI_FORMAT_BC4_TYPELESS                },
	{ L"BC4_UNORM",					DXGI_FORMAT_BC4_UNORM                   },
	{ L"BC4_SNORM",					DXGI_FORMAT_BC4_SNORM                   },
	{ L"BC5_TYPELESS",				DXGI_FORMAT_BC5_TYPELESS                },
	{ L"BC5_UNORM",					DXGI_FORMAT_BC5_UNORM                   },
	{ L"BC5_SNORM",					DXGI_FORMAT_BC5_SNORM                   },
	{ L"B5G6R5_UNORM",				DXGI_FORMAT_B5G6R5_UNORM                },
	{ L"B5G5R5A1_UNORM",			DXGI_FORMAT_B5G5R5A1_UNORM              },
	{ L"B8G8R8A8_UNORM",			DXGI_FORMAT_B8G8R8A8_UNORM              },
	{ L"B8G8R8X8_UNORM",			DXGI_FORMAT_B8G8R8X8_UNORM              },
	{ L"R10G10B10_XR_BIAS_A2_UNORM",DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM  },
	{ L"B8G8R8A8_TYPELESS",			DXGI_FORMAT_B8G8R8A8_TYPELESS           },
	{ L"B8G8R8A8_UNORM_SRGB",		DXGI_FORMAT_B8G8R8A8_UNORM_SRGB         },
	{ L"B8G8R8X8_TYPELESS",			DXGI_FORMAT_B8G8R8X8_TYPELESS           },
	{ L"B8G8R8X8_UNORM_SRGB",		DXGI_FORMAT_B8G8R8X8_UNORM_SRGB         },
	{ L"BC6H_TYPELESS",				DXGI_FORMAT_BC6H_TYPELESS               },
	{ L"BC6H_UF16",					DXGI_FORMAT_BC6H_UF16                   },
	{ L"BC6H_SF16",					DXGI_FORMAT_BC6H_SF16                   },
	{ L"BC7_TYPELESS",				DXGI_FORMAT_BC7_TYPELESS                },
	{ L"BC7_UNORM",					DXGI_FORMAT_BC7_UNORM                   },
	{ L"BC7_UNORM_SRGB",			DXGI_FORMAT_BC7_UNORM_SRGB              },
	{ L"AYUV",						DXGI_FORMAT_AYUV                        },
	{ L"Y410",						DXGI_FORMAT_Y410                        },
	{ L"Y416",						DXGI_FORMAT_Y416                        },
	{ L"NV12",						DXGI_FORMAT_NV12                        },
	{ L"P010",						DXGI_FORMAT_P010                        },
	{ L"P016",						DXGI_FORMAT_P016                        },
	{ L"420_OPAQUE",				DXGI_FORMAT_420_OPAQUE                  },
	{ L"YUY2",						DXGI_FORMAT_YUY2                        },
	{ L"Y210",						DXGI_FORMAT_Y210                        },
	{ L"Y216",						DXGI_FORMAT_Y216                        },
	{ L"NV11",						DXGI_FORMAT_NV11                        },
	{ L"AI44",						DXGI_FORMAT_AI44                        },
	{ L"IA44",						DXGI_FORMAT_IA44                        },
	{ L"P8",						DXGI_FORMAT_P8                          },
	{ L"A8P8",						DXGI_FORMAT_A8P8                        },
	{ L"B4G4R4A4_UNORM",			DXGI_FORMAT_B4G4R4A4_UNORM              },
	{ L"P208",						DXGI_FORMAT_P208                        },
	{ L"V208",						DXGI_FORMAT_V208                        },
	{ L"V408",						DXGI_FORMAT_V408                        },
	{ L"FORCE_UINT",				DXGI_FORMAT_FORCE_UINT                  }
};
const std::unordered_map<ALPHA_MODE, jwstring> AlphaModeToString {
	{ ALPHA_MODE_OTHER,			L"other"			},
	{ ALPHA_MODE_PREMULTIPLIED,	L"premultiplied"	},
	{ALPHA_MODE_TRANSPARENCY,	L"transparency"		},
	{ ALPHA_MODE_UNUSED,		L"unused"			}
};
const std::unordered_map<jwstring, ALPHA_MODE> StringToAlphaMode {
	{ L"other",			ALPHA_MODE_OTHER			},
	{ L"premultiplied",	ALPHA_MODE_PREMULTIPLIED	},
	{ L"transparency",	ALPHA_MODE_TRANSPARENCY		},
	{ L"unused",		ALPHA_MODE_UNUSED			}
};

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
};

inline bool skipspacet(jwstring &str, int &i) {
	while (i < str.length() && iswspace(str[i]))
		i++;
	return i >= str.length();
};
inline bool skipalnumt(jwstring &str, int &i) {
	while (i < str.length() && (iswalnum(str[i]) || str[i] == L'-' || str[i] == L'_' || str[i] == L'.'))
		i++;
	return i >= str.length();
};

template<typename T>
struct Field {
	Field(jwstring name, T* ptr) : name(name), ptr(ptr) {}

	jwstring Name() const { return name; }
	T* Value() const { return ptr; }

private:
	jwstring name;
	T* ptr;
};
struct AssetMetadata {
	inline auto GetFields() {
		if (asset) {
			switch (asset->TypeId()) {
			case ASSET_TYPE_FONT:
				return std::make_tuple(
					Field<DXGI_FORMAT>(	L"Pixel Format",		nullptr),
					Field<bool>(		L"Linear",				nullptr),
					Field<bool>(		L"Generate Mip Maps",	nullptr),
					Field<ALPHA_MODE>(	L"Alpha Mode",			nullptr),
					Field<unsigned int>(L"Font Size (pts)",		&fontSize),
					Field<unsigned int>(L"Font Dpi",			&fontDpi));

			case ASSET_TYPE_TEXTURE:
				return std::make_tuple(
					Field<DXGI_FORMAT>(	L"Pixel Format",		&textureFormat),
					Field<bool>(		L"Linear",				&textureLinear),
					Field<bool>(		L"Generate Mip Maps",	&textureMipMaps),
					Field<ALPHA_MODE>(	L"Alpha Mode",			&textureAlphaMode),
					Field<unsigned int>(L"Font Size (pts)",		nullptr),
					Field<unsigned int>(L"Font Dpi",			nullptr));
			}
		}
		return std::make_tuple(
			Field<DXGI_FORMAT>(	L"Pixel Format",		nullptr),
			Field<bool>(		L"Linear",				nullptr),
			Field<bool>(		L"Generate Mip Maps",	nullptr),
			Field<ALPHA_MODE>(	L"Alpha Mode",			nullptr),
			Field<unsigned int>(L"Font Size (pts)",		nullptr),
			Field<unsigned int>(L"Font Dpi",			nullptr));
	}

	std::shared_ptr<Asset> asset;
	jwstring assetPath;

	DXGI_FORMAT textureFormat;
	bool textureLinear;
	bool textureMipMaps;
	ALPHA_MODE textureAlphaMode;

	unsigned int fontSize;
	unsigned int fontDpi;

	AssetMetadata() :
		textureFormat(DXGI_FORMAT_R8G8B8A8_UNORM), textureLinear(true), textureMipMaps(true), textureAlphaMode(ALPHA_MODE_OTHER),
		fontSize(24), fontDpi(96), asset(nullptr), assetPath(L"") {}
	AssetMetadata(jwstring assetp) : AssetMetadata() {
		assetPath = assetp;
		struct Tag {
			jwstring name;
			jwstring value;
		};
		jvector<Tag> tags;
#pragma region parse tags
		jwstring metaPath = assetPath + L".meta";
		std::wifstream infile(metaPath.c_str());
		if (!infile.is_open()) {
			std::cerr << "No metadata for " << utf16toUtf8(assetPath.c_str()).c_str() << "\n";
			return;
		}
		int linenum = 0;
		int mode = 0;

		jwstring name;
		jwstring value;

		std::wstring l;
		while (std::getline(infile, l)) {
			jwstring line(l.c_str());
			int i = 0;
			while (i < line.length()) {
				// scan to the start of word
				if (skipspacet(line, i)) break;
				int j = i;
				if (skipalnumt(line, i)) break;

				if (mode == 0)
					name = jwstring(line.c_str() + j, i - j).lower();
				else if (mode == 1)
					value = jwstring(line.c_str() + j, i - j).lower();

				if (skipspacet(line, i)) break;

				if (line[i] == '=') {
					if (mode == 0)
						mode = 1;
					else {
						std::cerr << metaPath.c_str() << "[" << linenum << "] unexpected '=' \n";
						mode = 0;
					}
				} else if (line[i] == ';') {
					if (mode == 1) {
						mode = 0;
						tags.push_back({ name, value });
					} else {
						std::cerr << metaPath.c_str() << "[" << linenum << "] unexpected ';'\n";
						mode = 0;
						break;
					}
				} else {
					std::cerr << metaPath.c_str() << "[" << linenum << "] unexpected '" << line[i] << "'\n";
					mode = 0;
					break;
				}
				i++;
			}
			linenum++;
		}
		infile.close();
#pragma endregion

#pragma region generate fields
		for (int i = 0; i < tags.size(); i++) {
			if (tags[i].name == L"format") {
				if (StringToFormat.count(tags[i].value))
					textureFormat = StringToFormat.at(tags[i].value);
				else
					std::cerr << "Invalid format " << tags[i].value.c_str() << "\n";

			} else if (name == L"mipmaps")
				textureMipMaps = tags[i].value == L"true";

			else if (name == L"linear")
				textureLinear = tags[i].value == L"true";

			else if (name == L"alpha") {
				if (StringToAlphaMode.count(tags[i].value))
					textureAlphaMode = StringToAlphaMode.at(tags[i].value);
				else
					std::cerr << "Invalid alpha mode " << tags[i].value.c_str() << "\n";
			} else if (tags[i].name == L"fontsize")
				fontSize = (unsigned int)_wtoi(tags[i].value.c_str());
			else if (tags[i].name == L"fontdpi")
				fontDpi = (unsigned int)_wtoi(tags[i].value.c_str());
		}
#pragma endregion
	}

	void Write() {
		if (!asset || assetPath.length() == 0) return;

		jwstring metaPath = assetPath + L".meta";
		std::wofstream os(metaPath.c_str());
		if (!os.is_open()) {
			std::cerr << "Could not open asset metadata at: " << metaPath.c_str() << "\n";
			return;
		}
		
		switch (asset->TypeId()) {
		case ASSET_TYPE_TEXTURE:
			WriteField(os, L"Format", textureFormat);
			WriteField(os, L"Linear", textureLinear);
			WriteField(os, L"MipMaps", textureMipMaps);
			WriteField(os, L"AlphaMode", textureAlphaMode);
			break;
		case ASSET_TYPE_FONT:
			WriteField(os, L"FontSize", fontSize);
			WriteField(os, L"FontDpi", fontDpi);
			break;
		}

		os.close();
	}

private:
	void WriteField(std::wofstream &os, jwstring name, unsigned int v) {
		os.write(name.c_str(), name.length());
		os.write(L"=", 1);
		wchar_t* buf = new wchar_t[64];
		_itow_s(v, buf, 64, 10);
		os.write(buf, wcsnlen_s(buf, 64));
		os.write(L";\n", 2);
	}
	void WriteField(std::wofstream &os, jwstring name, int v) {
		os.write(name.c_str(), name.length());
		os.write(L"=", 1);
		wchar_t* buf = new wchar_t[64];
		_itow_s(v, buf, 64, 10);
		os.write(buf, wcsnlen_s(buf, 64));
		os.write(L";\n", 2);
	}
	void WriteField(std::wofstream &os, jwstring name, bool v) {
		os.write(name.c_str(), name.length());
		os.write(L"=", 1);
		if (v) os.write(L"true", 4); else os.write(L"false", 5);
		os.write(L";\n", 2);
	}
	void WriteField(std::wofstream &os, jwstring name, DXGI_FORMAT v) {
		os.write(name.c_str(), name.length());
		os.write(L"=", 1);
		jwstring fmt = FormatToString.at(v);
		os.write(fmt.c_str(), fmt.length());
		os.write(L";\n", 2);
	}
	void WriteField(std::wofstream &os, jwstring name, ALPHA_MODE v) {
		os.write(name.c_str(), name.length());
		os.write(L"=", 1);
		jwstring fmt = AlphaModeToString.at(v);
		os.write(fmt.c_str(), fmt.length());
		os.write(L";\n", 2);
	}
};

#endif