#pragma once

#include <glm/ext/vector_int4_sized.hpp>

namespace pEngine::girEngine::scene {

    /**
     * Similarly to gir::resource::FormatIR, this maps 1-1 with Vulkan's VkFormat enum for the time being.
     */
    enum class ResourceFormat {
        UNDEFINED = 0,
        /**
         * This option is for when you want the image format to be obtained automatically by the backend via the swapchain
         */
        SWAPCHAIN_DEPENDENT = -1,
        R4G4_UNORM_PACK8 = 1,
        R4G4B4A4_UNORM_PACK16 = 2,
        B4G4R4A4_UNORM_PACK16 = 3,
        R5G6B5_UNORM_PACK16 = 4,
        B5G6R5_UNORM_PACK16 = 5,
        R5G5B5A1_UNORM_PACK16 = 6,
        B5G5R5A1_UNORM_PACK16 = 7,
        A1R5G5B5_UNORM_PACK16 = 8,
        R8_UNORM = 9,
        R8_SNORM = 10,
        R8_USCALED = 11,
        R8_SSCALED = 12,
        R8_UINT = 13,
        R8_SINT = 14,
        R8_SRGB = 15,
        R8G8_UNORM = 16,
        R8G8_SNORM = 17,
        R8G8_USCALED = 18,
        R8G8_SSCALED = 19,
        R8G8_UINT = 20,
        R8G8_SINT = 21,
        R8G8_SRGB = 22,
        R8G8B8_UNORM = 23,
        R8G8B8_SNORM = 24,
        R8G8B8_USCALED = 25,
        R8G8B8_SSCALED = 26,
        R8G8B8_UINT = 27,
        R8G8B8_SINT = 28,
        R8G8B8_SRGB = 29,
        B8G8R8_UNORM = 30,
        B8G8R8_SNORM = 31,
        B8G8R8_USCALED = 32,
        B8G8R8_SSCALED = 33,
        B8G8R8_UINT = 34,
        B8G8R8_SINT = 35,
        B8G8R8_SRGB = 36,
        R8G8B8A8_UNORM = 37,
        R8G8B8A8_SNORM = 38,
        R8G8B8A8_USCALED = 39,
        R8G8B8A8_SSCALED = 40,
        R8G8B8A8_UINT = 41,
        R8G8B8A8_SINT = 42,
        R8G8B8A8_SRGB = 43,
        B8G8R8A8_UNORM = 44,
        B8G8R8A8_SNORM = 45,
        B8G8R8A8_USCALED = 46,
        B8G8R8A8_SSCALED = 47,
        B8G8R8A8_UINT = 48,
        B8G8R8A8_SINT = 49,
        B8G8R8A8_SRGB = 50,
        A8B8G8R8_UNORM_PACK32 = 51,
        A8B8G8R8_SNORM_PACK32 = 52,
        A8B8G8R8_USCALED_PACK32 = 53,
        A8B8G8R8_SSCALED_PACK32 = 54,
        A8B8G8R8_UINT_PACK32 = 55,
        A8B8G8R8_SINT_PACK32 = 56,
        A8B8G8R8_SRGB_PACK32 = 57,
        A2R10G10B10_UNORM_PACK32 = 58,
        A2R10G10B10_SNORM_PACK32 = 59,
        A2R10G10B10_USCALED_PACK32 = 60,
        A2R10G10B10_SSCALED_PACK32 = 61,
        A2R10G10B10_UINT_PACK32 = 62,
        A2R10G10B10_SINT_PACK32 = 63,
        A2B10G10R10_UNORM_PACK32 = 64,
        A2B10G10R10_SNORM_PACK32 = 65,
        A2B10G10R10_USCALED_PACK32 = 66,
        A2B10G10R10_SSCALED_PACK32 = 67,
        A2B10G10R10_UINT_PACK32 = 68,
        A2B10G10R10_SINT_PACK32 = 69,
        R16_UNORM = 70,
        R16_SNORM = 71,
        R16_USCALED = 72,
        R16_SSCALED = 73,
        R16_UINT = 74,
        R16_SINT = 75,
        R16_SFLOAT = 76,
        R16G16_UNORM = 77,
        R16G16_SNORM = 78,
        R16G16_USCALED = 79,
        R16G16_SSCALED = 80,
        R16G16_UINT = 81,
        R16G16_SINT = 82,
        R16G16_SFLOAT = 83,
        R16G16B16_UNORM = 84,
        R16G16B16_SNORM = 85,
        R16G16B16_USCALED = 86,
        R16G16B16_SSCALED = 87,
        R16G16B16_UINT = 88,
        R16G16B16_SINT = 89,
        R16G16B16_SFLOAT = 90,
        R16G16B16A16_UNORM = 91,
        R16G16B16A16_SNORM = 92,
        R16G16B16A16_USCALED = 93,
        R16G16B16A16_SSCALED = 94,
        R16G16B16A16_UINT = 95,
        R16G16B16A16_SINT = 96,
        R16G16B16A16_SFLOAT = 97,
        R32_UINT = 98,
        R32_SINT = 99,
        R32_SFLOAT = 100,
        R32G32_UINT = 101,
        R32G32_SINT = 102,
        R32G32_SFLOAT = 103,
        R32G32B32_UINT = 104,
        R32G32B32_SINT = 105,
        R32G32B32_SFLOAT = 106,
        R32G32B32A32_UINT = 107,
        R32G32B32A32_SINT = 108,
        R32G32B32A32_SFLOAT = 109,
        R64_UINT = 110,
        R64_SINT = 111,
        R64_SFLOAT = 112,
        R64G64_UINT = 113,
        R64G64_SINT = 114,
        R64G64_SFLOAT = 115,
        R64G64B64_UINT = 116,
        R64G64B64_SINT = 117,
        R64G64B64_SFLOAT = 118,
        R64G64B64A64_UINT = 119,
        R64G64B64A64_SINT = 120,
        R64G64B64A64_SFLOAT = 121,
        B10G11R11_UFLOAT_PACK32 = 122,
        E5B9G9R9_UFLOAT_PACK32 = 123,
        D16_UNORM = 124,
        X8_D24_UNORM_PACK32 = 125,
        D32_SFLOAT = 126,
        S8_UINT = 127,
        D16_UNORM_S8_UINT = 128,
        D24_UNORM_S8_UINT = 129,
        D32_SFLOAT_S8_UINT = 130,
        BC1_RGB_UNORM_BLOCK = 131,
        BC1_RGB_SRGB_BLOCK = 132,
        BC1_RGBA_UNORM_BLOCK = 133,
        BC1_RGBA_SRGB_BLOCK = 134,
        BC2_UNORM_BLOCK = 135,
        BC2_SRGB_BLOCK = 136,
        BC3_UNORM_BLOCK = 137,
        BC3_SRGB_BLOCK = 138,
        BC4_UNORM_BLOCK = 139,
        BC4_SNORM_BLOCK = 140,
        BC5_UNORM_BLOCK = 141,
        BC5_SNORM_BLOCK = 142,
        BC6H_UFLOAT_BLOCK = 143,
        BC6H_SFLOAT_BLOCK = 144,
        BC7_UNORM_BLOCK = 145,
        BC7_SRGB_BLOCK = 146,
        ETC2_R8G8B8_UNORM_BLOCK = 147,
        ETC2_R8G8B8_SRGB_BLOCK = 148,
        ETC2_R8G8B8A1_UNORM_BLOCK = 149,
        ETC2_R8G8B8A1_SRGB_BLOCK = 150,
        ETC2_R8G8B8A8_UNORM_BLOCK = 151,
        ETC2_R8G8B8A8_SRGB_BLOCK = 152,
        EAC_R11_UNORM_BLOCK = 153,
        EAC_R11_SNORM_BLOCK = 154,
        EAC_R11G11_UNORM_BLOCK = 155,
        EAC_R11G11_SNORM_BLOCK = 156,
        ASTC_4x4_UNORM_BLOCK = 157,
        ASTC_4x4_SRGB_BLOCK = 158,
        ASTC_5x4_UNORM_BLOCK = 159,
        ASTC_5x4_SRGB_BLOCK = 160,
        ASTC_5x5_UNORM_BLOCK = 161,
        ASTC_5x5_SRGB_BLOCK = 162,
        ASTC_6x5_UNORM_BLOCK = 163,
        ASTC_6x5_SRGB_BLOCK = 164,
        ASTC_6x6_UNORM_BLOCK = 165,
        ASTC_6x6_SRGB_BLOCK = 166,
        ASTC_8x5_UNORM_BLOCK = 167,
        ASTC_8x5_SRGB_BLOCK = 168,
        ASTC_8x6_UNORM_BLOCK = 169,
        ASTC_8x6_SRGB_BLOCK = 170,
        ASTC_8x8_UNORM_BLOCK = 171,
        ASTC_8x8_SRGB_BLOCK = 172,
        ASTC_10x5_UNORM_BLOCK = 173,
        ASTC_10x5_SRGB_BLOCK = 174,
        ASTC_10x6_UNORM_BLOCK = 175,
        ASTC_10x6_SRGB_BLOCK = 176,
        ASTC_10x8_UNORM_BLOCK = 177,
        ASTC_10x8_SRGB_BLOCK = 178,
        ASTC_10x10_UNORM_BLOCK = 179,
        ASTC_10x10_SRGB_BLOCK = 180,
        ASTC_12x10_UNORM_BLOCK = 181,
        ASTC_12x10_SRGB_BLOCK = 182,
        ASTC_12x12_UNORM_BLOCK = 183,
        ASTC_12x12_SRGB_BLOCK = 184,
        // Provided by VK_VERSION_1_1
        G8B8G8R8_422_UNORM = 1000156000,
        // Provided by VK_VERSION_1_1
        B8G8R8G8_422_UNORM = 1000156001,
        // Provided by VK_VERSION_1_1
        G8_B8_R8_3PLANE_420_UNORM = 1000156002,
        // Provided by VK_VERSION_1_1
        G8_B8R8_2PLANE_420_UNORM = 1000156003,
        // Provided by VK_VERSION_1_1
        G8_B8_R8_3PLANE_422_UNORM = 1000156004,
        // Provided by VK_VERSION_1_1
        G8_B8R8_2PLANE_422_UNORM = 1000156005,
        // Provided by VK_VERSION_1_1
        G8_B8_R8_3PLANE_444_UNORM = 1000156006,
        // Provided by VK_VERSION_1_1
        R10X6_UNORM_PACK16 = 1000156007,
        // Provided by VK_VERSION_1_1
        R10X6G10X6_UNORM_2PACK16 = 1000156008,
        // Provided by VK_VERSION_1_1
        R10X6G10X6B10X6A10X6_UNORM_4PACK16 = 1000156009,
        // Provided by VK_VERSION_1_1
        G10X6B10X6G10X6R10X6_422_UNORM_4PACK16 = 1000156010,
        // Provided by VK_VERSION_1_1
        B10X6G10X6R10X6G10X6_422_UNORM_4PACK16 = 1000156011,
        // Provided by VK_VERSION_1_1
        G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16 = 1000156012,
        // Provided by VK_VERSION_1_1
        G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16 = 1000156013,
        // Provided by VK_VERSION_1_1
        G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16 = 1000156014,
        // Provided by VK_VERSION_1_1
        G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16 = 1000156015,
        // Provided by VK_VERSION_1_1
        G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16 = 1000156016,
        // Provided by VK_VERSION_1_1
        R12X4_UNORM_PACK16 = 1000156017,
        // Provided by VK_VERSION_1_1
        R12X4G12X4_UNORM_2PACK16 = 1000156018,
        // Provided by VK_VERSION_1_1
        R12X4G12X4B12X4A12X4_UNORM_4PACK16 = 1000156019,
        // Provided by VK_VERSION_1_1
        G12X4B12X4G12X4R12X4_422_UNORM_4PACK16 = 1000156020,
        // Provided by VK_VERSION_1_1
        B12X4G12X4R12X4G12X4_422_UNORM_4PACK16 = 1000156021,
        // Provided by VK_VERSION_1_1
        G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16 = 1000156022,
        // Provided by VK_VERSION_1_1
        G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16 = 1000156023,
        // Provided by VK_VERSION_1_1
        G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16 = 1000156024,
        // Provided by VK_VERSION_1_1
        G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16 = 1000156025,
        // Provided by VK_VERSION_1_1
        G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16 = 1000156026,
        // Provided by VK_VERSION_1_1
        G16B16G16R16_422_UNORM = 1000156027,
        // Provided by VK_VERSION_1_1
        B16G16R16G16_422_UNORM = 1000156028,
        // Provided by VK_VERSION_1_1
        G16_B16_R16_3PLANE_420_UNORM = 1000156029,
        // Provided by VK_VERSION_1_1
        G16_B16R16_2PLANE_420_UNORM = 1000156030,
        // Provided by VK_VERSION_1_1
        G16_B16_R16_3PLANE_422_UNORM = 1000156031,
        // Provided by VK_VERSION_1_1
        G16_B16R16_2PLANE_422_UNORM = 1000156032,
        // Provided by VK_VERSION_1_1
        G16_B16_R16_3PLANE_444_UNORM = 1000156033,
        // Provided by VK_VERSION_1_3
        G8_B8R8_2PLANE_444_UNORM = 1000330000,
        // Provided by VK_VERSION_1_3
        G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16 = 1000330001,
        // Provided by VK_VERSION_1_3
        G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16 = 1000330002,
        // Provided by VK_VERSION_1_3
        G16_B16R16_2PLANE_444_UNORM = 1000330003,
        // Provided by VK_VERSION_1_3
        A4R4G4B4_UNORM_PACK16 = 1000340000,
        // Provided by VK_VERSION_1_3
        A4B4G4R4_UNORM_PACK16 = 1000340001,
        // Provided by VK_VERSION_1_3
        ASTC_4x4_SFLOAT_BLOCK = 1000066000,
        // Provided by VK_VERSION_1_3
        ASTC_5x4_SFLOAT_BLOCK = 1000066001,
        // Provided by VK_VERSION_1_3
        ASTC_5x5_SFLOAT_BLOCK = 1000066002,
        // Provided by VK_VERSION_1_3
        ASTC_6x5_SFLOAT_BLOCK = 1000066003,
        // Provided by VK_VERSION_1_3
        ASTC_6x6_SFLOAT_BLOCK = 1000066004,
        // Provided by VK_VERSION_1_3
        ASTC_8x5_SFLOAT_BLOCK = 1000066005,
        // Provided by VK_VERSION_1_3
        ASTC_8x6_SFLOAT_BLOCK = 1000066006,
        // Provided by VK_VERSION_1_3
        ASTC_8x8_SFLOAT_BLOCK = 1000066007,
        // Provided by VK_VERSION_1_3
        ASTC_10x5_SFLOAT_BLOCK = 1000066008,
        // Provided by VK_VERSION_1_3
        ASTC_10x6_SFLOAT_BLOCK = 1000066009,
        // Provided by VK_VERSION_1_3
        ASTC_10x8_SFLOAT_BLOCK = 1000066010,
        // Provided by VK_VERSION_1_3
        ASTC_10x10_SFLOAT_BLOCK = 1000066011,
        // Provided by VK_VERSION_1_3
        ASTC_12x10_SFLOAT_BLOCK = 1000066012,
        // Provided by VK_VERSION_1_3
        ASTC_12x12_SFLOAT_BLOCK = 1000066013,
        // Provided by VK_IMG_format_pvrtc
        PVRTC1_2BPP_UNORM_BLOCK_IMG = 1000054000,
        // Provided by VK_IMG_format_pvrtc
        PVRTC1_4BPP_UNORM_BLOCK_IMG = 1000054001,
        // Provided by VK_IMG_format_pvrtc
        PVRTC2_2BPP_UNORM_BLOCK_IMG = 1000054002,
        // Provided by VK_IMG_format_pvrtc
        PVRTC2_4BPP_UNORM_BLOCK_IMG = 1000054003,
        // Provided by VK_IMG_format_pvrtc
        PVRTC1_2BPP_SRGB_BLOCK_IMG = 1000054004,
        // Provided by VK_IMG_format_pvrtc
        PVRTC1_4BPP_SRGB_BLOCK_IMG = 1000054005,
        // Provided by VK_IMG_format_pvrtc
        PVRTC2_2BPP_SRGB_BLOCK_IMG = 1000054006,
        // Provided by VK_IMG_format_pvrtc
        PVRTC2_4BPP_SRGB_BLOCK_IMG = 1000054007,
        // Provided by VK_NV_optical_flow
        R16G16_SFIXED5_NV = 1000464000,
        // Provided by VK_KHR_maintenance5
        A1B5G5R5_UNORM_PACK16_KHR = 1000470000,
        // Provided by VK_KHR_maintenance5
        A8_UNORM_KHR = 1000470001,
        // Provided by VK_EXT_texture_compression_astc_hdr
        ASTC_4x4_SFLOAT_BLOCK_EXT = ASTC_4x4_SFLOAT_BLOCK,
        // Provided by VK_EXT_texture_compression_astc_hdr
        ASTC_5x4_SFLOAT_BLOCK_EXT = ASTC_5x4_SFLOAT_BLOCK,
        // Provided by VK_EXT_texture_compression_astc_hdr
        ASTC_5x5_SFLOAT_BLOCK_EXT = ASTC_5x5_SFLOAT_BLOCK,
        // Provided by VK_EXT_texture_compression_astc_hdr
        ASTC_6x5_SFLOAT_BLOCK_EXT = ASTC_6x5_SFLOAT_BLOCK,
        // Provided by VK_EXT_texture_compression_astc_hdr
        ASTC_6x6_SFLOAT_BLOCK_EXT = ASTC_6x6_SFLOAT_BLOCK,
        // Provided by VK_EXT_texture_compression_astc_hdr
        ASTC_8x5_SFLOAT_BLOCK_EXT = ASTC_8x5_SFLOAT_BLOCK,
        // Provided by VK_EXT_texture_compression_astc_hdr
        ASTC_8x6_SFLOAT_BLOCK_EXT = ASTC_8x6_SFLOAT_BLOCK,
        // Provided by VK_EXT_texture_compression_astc_hdr
        ASTC_8x8_SFLOAT_BLOCK_EXT = ASTC_8x8_SFLOAT_BLOCK,
        // Provided by VK_EXT_texture_compression_astc_hdr
        ASTC_10x5_SFLOAT_BLOCK_EXT = ASTC_10x5_SFLOAT_BLOCK,
        // Provided by VK_EXT_texture_compression_astc_hdr
        ASTC_10x6_SFLOAT_BLOCK_EXT = ASTC_10x6_SFLOAT_BLOCK,
        // Provided by VK_EXT_texture_compression_astc_hdr
        ASTC_10x8_SFLOAT_BLOCK_EXT = ASTC_10x8_SFLOAT_BLOCK,
        // Provided by VK_EXT_texture_compression_astc_hdr
        ASTC_10x10_SFLOAT_BLOCK_EXT = ASTC_10x10_SFLOAT_BLOCK,
        // Provided by VK_EXT_texture_compression_astc_hdr
        ASTC_12x10_SFLOAT_BLOCK_EXT = ASTC_12x10_SFLOAT_BLOCK,
        // Provided by VK_EXT_texture_compression_astc_hdr
        ASTC_12x12_SFLOAT_BLOCK_EXT = ASTC_12x12_SFLOAT_BLOCK,
        // Provided by VK_KHR_sampler_ycbcr_conversion
        G8B8G8R8_422_UNORM_KHR = G8B8G8R8_422_UNORM,
        // Provided by VK_KHR_sampler_ycbcr_conversion
        B8G8R8G8_422_UNORM_KHR = B8G8R8G8_422_UNORM,
        // Provided by VK_KHR_sampler_ycbcr_conversion
        G8_B8_R8_3PLANE_420_UNORM_KHR = G8_B8_R8_3PLANE_420_UNORM,
        // Provided by VK_KHR_sampler_ycbcr_conversion
        G8_B8R8_2PLANE_420_UNORM_KHR = G8_B8R8_2PLANE_420_UNORM,
        // Provided by VK_KHR_sampler_ycbcr_conversion
        G8_B8_R8_3PLANE_422_UNORM_KHR = G8_B8_R8_3PLANE_422_UNORM,
        // Provided by VK_KHR_sampler_ycbcr_conversion
        G8_B8R8_2PLANE_422_UNORM_KHR = G8_B8R8_2PLANE_422_UNORM,
        // Provided by VK_KHR_sampler_ycbcr_conversion
        G8_B8_R8_3PLANE_444_UNORM_KHR = G8_B8_R8_3PLANE_444_UNORM,
        // Provided by VK_KHR_sampler_ycbcr_conversion
        R10X6_UNORM_PACK16_KHR = R10X6_UNORM_PACK16,
        // Provided by VK_KHR_sampler_ycbcr_conversion
        R10X6G10X6_UNORM_2PACK16_KHR = R10X6G10X6_UNORM_2PACK16,
        // Provided by VK_KHR_sampler_ycbcr_conversion
        R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHR = R10X6G10X6B10X6A10X6_UNORM_4PACK16,
        // Provided by VK_KHR_sampler_ycbcr_conversion
        G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR = G10X6B10X6G10X6R10X6_422_UNORM_4PACK16,
        // Provided by VK_KHR_sampler_ycbcr_conversion
        B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR = B10X6G10X6R10X6G10X6_422_UNORM_4PACK16,
        // Provided by VK_KHR_sampler_ycbcr_conversion
        G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_KHR = G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16,
        // Provided by VK_KHR_sampler_ycbcr_conversion
        G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_KHR = G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16,
        // Provided by VK_KHR_sampler_ycbcr_conversion
        G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16_KHR = G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16,
        // Provided by VK_KHR_sampler_ycbcr_conversion
        G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16_KHR = G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16,
        // Provided by VK_KHR_sampler_ycbcr_conversion
        G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16_KHR = G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16,
        // Provided by VK_KHR_sampler_ycbcr_conversion
        R12X4_UNORM_PACK16_KHR = R12X4_UNORM_PACK16,
        // Provided by VK_KHR_sampler_ycbcr_conversion
        R12X4G12X4_UNORM_2PACK16_KHR = R12X4G12X4_UNORM_2PACK16,
        // Provided by VK_KHR_sampler_ycbcr_conversion
        R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHR = R12X4G12X4B12X4A12X4_UNORM_4PACK16,
        // Provided by VK_KHR_sampler_ycbcr_conversion
        G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR = G12X4B12X4G12X4R12X4_422_UNORM_4PACK16,
        // Provided by VK_KHR_sampler_ycbcr_conversion
        B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR = B12X4G12X4R12X4G12X4_422_UNORM_4PACK16,
        // Provided by VK_KHR_sampler_ycbcr_conversion
        G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_KHR = G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16,
        // Provided by VK_KHR_sampler_ycbcr_conversion
        G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_KHR = G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16,
        // Provided by VK_KHR_sampler_ycbcr_conversion
        G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16_KHR = G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16,
        // Provided by VK_KHR_sampler_ycbcr_conversion
        G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16_KHR = G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16,
        // Provided by VK_KHR_sampler_ycbcr_conversion
        G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16_KHR = G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16,
        // Provided by VK_KHR_sampler_ycbcr_conversion
        G16B16G16R16_422_UNORM_KHR = G16B16G16R16_422_UNORM,
        // Provided by VK_KHR_sampler_ycbcr_conversion
        B16G16R16G16_422_UNORM_KHR = B16G16R16G16_422_UNORM,
        // Provided by VK_KHR_sampler_ycbcr_conversion
        G16_B16_R16_3PLANE_420_UNORM_KHR = G16_B16_R16_3PLANE_420_UNORM,
        // Provided by VK_KHR_sampler_ycbcr_conversion
        G16_B16R16_2PLANE_420_UNORM_KHR = G16_B16R16_2PLANE_420_UNORM,
        // Provided by VK_KHR_sampler_ycbcr_conversion
        G16_B16_R16_3PLANE_422_UNORM_KHR = G16_B16_R16_3PLANE_422_UNORM,
        // Provided by VK_KHR_sampler_ycbcr_conversion
        G16_B16R16_2PLANE_422_UNORM_KHR = G16_B16R16_2PLANE_422_UNORM,
        // Provided by VK_KHR_sampler_ycbcr_conversion
        G16_B16_R16_3PLANE_444_UNORM_KHR = G16_B16_R16_3PLANE_444_UNORM,
        // Provided by VK_EXT_ycbcr_2plane_444_formats
        G8_B8R8_2PLANE_444_UNORM_EXT = G8_B8R8_2PLANE_444_UNORM,
        // Provided by VK_EXT_ycbcr_2plane_444_formats
        G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16_EXT = G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16,
        // Provided by VK_EXT_ycbcr_2plane_444_formats
        G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16_EXT = G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16,
        // Provided by VK_EXT_ycbcr_2plane_444_formats
        G16_B16R16_2PLANE_444_UNORM_EXT = G16_B16R16_2PLANE_444_UNORM,
        // Provided by VK_EXT_4444_formats
        A4R4G4B4_UNORM_PACK16_EXT = A4R4G4B4_UNORM_PACK16,
        // Provided by VK_EXT_4444_formats
        A4B4G4R4_UNORM_PACK16_EXT = A4B4G4R4_UNORM_PACK16,
    };

    /**
     * This is just 1-1 copied from the constexpr function used in VulkanFormatSize.hpp in the Backend;
     * at some point they should not be coupled like this.
     *
     *  TODO -> group up common cases to shorten this function (it doesn't need to be organized)
     */
    constexpr unsigned getFormatElementSizeInBytes(const ResourceFormat &resourceFormat) {
        switch (resourceFormat) {
            case ResourceFormat::R4G4_UNORM_PACK8 :
                return 1;
            case ResourceFormat::R4G4B4A4_UNORM_PACK16 :
            case ResourceFormat::B4G4R4A4_UNORM_PACK16 :
            case ResourceFormat::R5G6B5_UNORM_PACK16 :
            case ResourceFormat::B5G6R5_UNORM_PACK16 :
            case ResourceFormat::R5G5B5A1_UNORM_PACK16 :
            case ResourceFormat::B5G5R5A1_UNORM_PACK16 :
            case ResourceFormat::A1R5G5B5_UNORM_PACK16 :
                return 2;
            case ResourceFormat::R8_UNORM :
            case ResourceFormat::R8_SNORM :
            case ResourceFormat::R8_USCALED :
            case ResourceFormat::R8_SSCALED :
            case ResourceFormat::R8_UINT :
            case ResourceFormat::R8_SINT :
            case ResourceFormat::R8_SRGB :
                return 1;
            case ResourceFormat::R8G8_UNORM :
            case ResourceFormat::R8G8_SNORM :
            case ResourceFormat::R8G8_USCALED :
            case ResourceFormat::R8G8_SSCALED :
            case ResourceFormat::R8G8_UINT :
            case ResourceFormat::R8G8_SINT :
            case ResourceFormat::R8G8_SRGB :
                return 2;
            case ResourceFormat::R8G8B8_UNORM :
            case ResourceFormat::R8G8B8_SNORM :
            case ResourceFormat::R8G8B8_USCALED :
            case ResourceFormat::R8G8B8_SSCALED :
            case ResourceFormat::R8G8B8_UINT :
            case ResourceFormat::R8G8B8_SINT :
            case ResourceFormat::R8G8B8_SRGB :
            case ResourceFormat::B8G8R8_UNORM :
            case ResourceFormat::B8G8R8_SNORM :
            case ResourceFormat::B8G8R8_USCALED :
            case ResourceFormat::B8G8R8_SSCALED :
            case ResourceFormat::B8G8R8_UINT :
            case ResourceFormat::B8G8R8_SINT :
            case ResourceFormat::B8G8R8_SRGB :
                return 3;
            case ResourceFormat::R8G8B8A8_UNORM :
            case ResourceFormat::R8G8B8A8_SNORM :
            case ResourceFormat::R8G8B8A8_USCALED :
            case ResourceFormat::R8G8B8A8_SSCALED :
            case ResourceFormat::R8G8B8A8_UINT :
            case ResourceFormat::R8G8B8A8_SINT :
            case ResourceFormat::R8G8B8A8_SRGB :
            case ResourceFormat::B8G8R8A8_UNORM :
            case ResourceFormat::B8G8R8A8_SNORM :
            case ResourceFormat::B8G8R8A8_USCALED :
            case ResourceFormat::B8G8R8A8_SSCALED :
            case ResourceFormat::B8G8R8A8_UINT :
            case ResourceFormat::B8G8R8A8_SINT :
            case ResourceFormat::B8G8R8A8_SRGB :
            case ResourceFormat::A8B8G8R8_UNORM_PACK32 :
            case ResourceFormat::A8B8G8R8_SNORM_PACK32 :
            case ResourceFormat::A8B8G8R8_USCALED_PACK32 :
            case ResourceFormat::A8B8G8R8_SSCALED_PACK32 :
            case ResourceFormat::A8B8G8R8_UINT_PACK32 :
            case ResourceFormat::A8B8G8R8_SINT_PACK32 :
            case ResourceFormat::A8B8G8R8_SRGB_PACK32 :
            case ResourceFormat::A2R10G10B10_UNORM_PACK32 :
            case ResourceFormat::A2R10G10B10_SNORM_PACK32 :
            case ResourceFormat::A2R10G10B10_USCALED_PACK32 :
            case ResourceFormat::A2R10G10B10_SSCALED_PACK32 :
            case ResourceFormat::A2R10G10B10_UINT_PACK32 :
            case ResourceFormat::A2R10G10B10_SINT_PACK32 :
            case ResourceFormat::A2B10G10R10_UNORM_PACK32 :
            case ResourceFormat::A2B10G10R10_SNORM_PACK32 :
            case ResourceFormat::A2B10G10R10_USCALED_PACK32 :
            case ResourceFormat::A2B10G10R10_SSCALED_PACK32 :
            case ResourceFormat::A2B10G10R10_UINT_PACK32 :
            case ResourceFormat::A2B10G10R10_SINT_PACK32 :
                return 4;
            case ResourceFormat::R16_UNORM :
            case ResourceFormat::R16_SNORM :
            case ResourceFormat::R16_USCALED :
            case ResourceFormat::R16_SSCALED :
            case ResourceFormat::R16_UINT :
            case ResourceFormat::R16_SINT :
            case ResourceFormat::R16_SFLOAT :
                return 2;
            case ResourceFormat::R16G16_UNORM :
            case ResourceFormat::R16G16_SNORM :
            case ResourceFormat::R16G16_USCALED :
            case ResourceFormat::R16G16_SSCALED :
            case ResourceFormat::R16G16_UINT :
            case ResourceFormat::R16G16_SINT :
            case ResourceFormat::R16G16_SFLOAT :
                return 4;
            case ResourceFormat::R16G16B16_UNORM :
            case ResourceFormat::R16G16B16_SNORM :
            case ResourceFormat::R16G16B16_USCALED :
            case ResourceFormat::R16G16B16_SSCALED :
            case ResourceFormat::R16G16B16_UINT :
            case ResourceFormat::R16G16B16_SINT :
            case ResourceFormat::R16G16B16_SFLOAT :
                return 6;
            case ResourceFormat::R16G16B16A16_UNORM :
            case ResourceFormat::R16G16B16A16_SNORM :
            case ResourceFormat::R16G16B16A16_USCALED :
            case ResourceFormat::R16G16B16A16_SSCALED :
            case ResourceFormat::R16G16B16A16_UINT :
            case ResourceFormat::R16G16B16A16_SINT :
            case ResourceFormat::R16G16B16A16_SFLOAT :
                return 8;
            case ResourceFormat::R32_UINT :
            case ResourceFormat::R32_SINT :
            case ResourceFormat::R32_SFLOAT :
                return 4;
            case ResourceFormat::R32G32_UINT :
            case ResourceFormat::R32G32_SINT :
            case ResourceFormat::R32G32_SFLOAT :
                return 8;
            case ResourceFormat::R32G32B32_UINT :
            case ResourceFormat::R32G32B32_SINT :
            case ResourceFormat::R32G32B32_SFLOAT :
                return 12;
            case ResourceFormat::R32G32B32A32_UINT :
            case ResourceFormat::R32G32B32A32_SINT :
            case ResourceFormat::R32G32B32A32_SFLOAT :
                return 16;
            case ResourceFormat::R64_UINT :
            case ResourceFormat::R64_SINT :
            case ResourceFormat::R64_SFLOAT :
                return 8;
            case ResourceFormat::R64G64_UINT :
            case ResourceFormat::R64G64_SINT :
            case ResourceFormat::R64G64_SFLOAT :
                return 16;
            case ResourceFormat::R64G64B64_UINT :
            case ResourceFormat::R64G64B64_SINT :
            case ResourceFormat::R64G64B64_SFLOAT :
                return 24;
            case ResourceFormat::R64G64B64A64_UINT :
            case ResourceFormat::R64G64B64A64_SINT :
            case ResourceFormat::R64G64B64A64_SFLOAT :
                return 32;
            case ResourceFormat::B10G11R11_UFLOAT_PACK32 :
            case ResourceFormat::E5B9G9R9_UFLOAT_PACK32 :
                return 4;
            case ResourceFormat::D16_UNORM :
                return 2;
            case ResourceFormat::X8_D24_UNORM_PACK32 :
            case ResourceFormat::D32_SFLOAT :
                return 4;
            case ResourceFormat::S8_UINT :
                return 1;
            case ResourceFormat::D16_UNORM_S8_UINT :
                return 3;
            case ResourceFormat::D24_UNORM_S8_UINT :
                return 4;
            case ResourceFormat::D32_SFLOAT_S8_UINT :
                return 5;

                // For compressed block formats return the block size, in bytes

            case ResourceFormat::BC1_RGB_UNORM_BLOCK :
            case ResourceFormat::BC1_RGB_SRGB_BLOCK :
            case ResourceFormat::BC1_RGBA_UNORM_BLOCK :
            case ResourceFormat::BC1_RGBA_SRGB_BLOCK :
                return 8;
            case ResourceFormat::BC2_UNORM_BLOCK :
            case ResourceFormat::BC2_SRGB_BLOCK :
            case ResourceFormat::BC3_UNORM_BLOCK :
            case ResourceFormat::BC3_SRGB_BLOCK :
                return 16;
            case ResourceFormat::BC4_UNORM_BLOCK :
            case ResourceFormat::BC4_SNORM_BLOCK :
                return 8;
            case ResourceFormat::BC5_UNORM_BLOCK :
            case ResourceFormat::BC5_SNORM_BLOCK :
            case ResourceFormat::BC6H_UFLOAT_BLOCK :
            case ResourceFormat::BC6H_SFLOAT_BLOCK :
            case ResourceFormat::BC7_UNORM_BLOCK :
            case ResourceFormat::BC7_SRGB_BLOCK :
                return 16;
            case ResourceFormat::ETC2_R8G8B8_UNORM_BLOCK :
            case ResourceFormat::ETC2_R8G8B8_SRGB_BLOCK :
            case ResourceFormat::ETC2_R8G8B8A1_UNORM_BLOCK :
            case ResourceFormat::ETC2_R8G8B8A1_SRGB_BLOCK :
                return 8;
            case ResourceFormat::ETC2_R8G8B8A8_UNORM_BLOCK :
            case ResourceFormat::ETC2_R8G8B8A8_SRGB_BLOCK :
                return 16;

            case ResourceFormat::EAC_R11_UNORM_BLOCK :
            case ResourceFormat::EAC_R11_SNORM_BLOCK :
                return 8;
            case ResourceFormat::EAC_R11G11_UNORM_BLOCK :
            case ResourceFormat::EAC_R11G11_SNORM_BLOCK :
            case ResourceFormat::ASTC_4x4_UNORM_BLOCK :
            case ResourceFormat::ASTC_4x4_SRGB_BLOCK :
            case ResourceFormat::ASTC_5x4_UNORM_BLOCK :
            case ResourceFormat::ASTC_5x4_SRGB_BLOCK :
            case ResourceFormat::ASTC_5x5_UNORM_BLOCK :
            case ResourceFormat::ASTC_5x5_SRGB_BLOCK :
            case ResourceFormat::ASTC_6x5_UNORM_BLOCK :
            case ResourceFormat::ASTC_6x5_SRGB_BLOCK :
            case ResourceFormat::ASTC_6x6_UNORM_BLOCK :
            case ResourceFormat::ASTC_6x6_SRGB_BLOCK :
            case ResourceFormat::ASTC_8x5_UNORM_BLOCK :
            case ResourceFormat::ASTC_8x5_SRGB_BLOCK :
            case ResourceFormat::ASTC_8x6_UNORM_BLOCK :
            case ResourceFormat::ASTC_8x6_SRGB_BLOCK :
            case ResourceFormat::ASTC_8x8_UNORM_BLOCK :
            case ResourceFormat::ASTC_8x8_SRGB_BLOCK :
            case ResourceFormat::ASTC_10x5_UNORM_BLOCK :
            case ResourceFormat::ASTC_10x5_SRGB_BLOCK :
            case ResourceFormat::ASTC_10x6_UNORM_BLOCK :
            case ResourceFormat::ASTC_10x6_SRGB_BLOCK :
            case ResourceFormat::ASTC_10x8_UNORM_BLOCK :
            case ResourceFormat::ASTC_10x8_SRGB_BLOCK :
            case ResourceFormat::ASTC_10x10_UNORM_BLOCK :
            case ResourceFormat::ASTC_10x10_SRGB_BLOCK :
            case ResourceFormat::ASTC_12x10_UNORM_BLOCK :
            case ResourceFormat::ASTC_12x10_SRGB_BLOCK :
            case ResourceFormat::ASTC_12x12_UNORM_BLOCK :
            case ResourceFormat::ASTC_12x12_SRGB_BLOCK :
                return 16;

            case ResourceFormat::PVRTC1_2BPP_UNORM_BLOCK_IMG :
            case ResourceFormat::PVRTC1_4BPP_UNORM_BLOCK_IMG :
            case ResourceFormat::PVRTC2_2BPP_UNORM_BLOCK_IMG :
            case ResourceFormat::PVRTC2_4BPP_UNORM_BLOCK_IMG :
            case ResourceFormat::PVRTC1_2BPP_SRGB_BLOCK_IMG :
            case ResourceFormat::PVRTC1_4BPP_SRGB_BLOCK_IMG :
            case ResourceFormat::PVRTC2_2BPP_SRGB_BLOCK_IMG :
            case ResourceFormat::PVRTC2_4BPP_SRGB_BLOCK_IMG :
                return 8;
            default:
                // TODO -> Log!
                return 0;
        }
    }
}
