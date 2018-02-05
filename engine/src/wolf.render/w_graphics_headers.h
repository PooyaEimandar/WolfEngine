/*
	Project			 : Wolf Engine. Copyright(c) Pooya Eimandar (http://PooyaEimandar.com) . All rights reserved.
	Source			 : Please direct any bug to https://github.com/PooyaEimandar/Wolf.Engine/issues
	Website			 : http://WolfSource.io
	Name			 : w_graphics_header.h
	Description		 : The include header for graphics devices. Wolf.Engine supports two render APIs, the first one is DirectX 12 
						which supports both Windows 10 and Universal Windows Platform(UWP) and the second one is Vulkan which supports
						Windows, Linux, Android and OSX/IOS(with MoltenVK)  
	Comment          :
*/

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef __W_GRAPHICS_HEADERS_H__
#define __W_GRAPHICS_HEADERS_H__

#ifdef __DX12__

#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <DirectXMath.h>

using Microsoft::WRL::ComPtr;

#elif defined (__VULKAN__) 
	#ifdef __WIN32
		#ifndef VK_USE_PLATFORM_WIN32_KHR
			#define VK_USE_PLATFORM_WIN32_KHR
		#endif
	#elif defined(__linux) && !defined(__ANDROID)
		#ifndef VK_USE_PLATFORM_XCB_KHR
			#define VK_USE_PLATFORM_XCB_KHR
		#endif
	#endif

    #if defined(__iOS__) || defined(__APPLE__)

#ifdef __iOS__
        #ifndef VK_USE_PLATFORM_IOS_MVK
            #define VK_USE_PLATFORM_IOS_MVK
        #endif
#else
        #ifndef VK_USE_PLATFORM_MACOS_MVK
            #define VK_USE_PLATFORM_MACOS_MVK
        #endif
#endif

        #include <vulkan/vulkan.h>
        #include <MoltenVK/vk_mvk_moltenvk.h>
        #include <unistd.h>

	#elif defined(__ANDROID)
		#include "vk_android/vulkan_wrapper.h"
	#else
		#include <vulkan/vulkan.hpp>
	#endif
#endif

#if defined(__linux) ||  defined(__APPLE__) || defined(__ANDROID)
#include <w_std.h>
#endif

#ifdef __VULKAN__
#define DEFAULT_FENCE_TIMEOUT 1000000000
#endif

namespace wolf
{
    namespace graphics
    {
        typedef enum w_memory_property_flag_bits
        {
#ifdef __VULKAN__
            W_MEMORY_PROPERTY_DEVICE_LOCAL_BIT = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            W_MEMORY_PROPERTY_HOST_VISIBLE_BIT = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            W_MEMORY_PROPERTY_HOST_COHERENT_BIT = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            W_MEMORY_PROPERTY_HOST_CACHED_BIT = VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
            W_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT = VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT,
            W_MEMORY_PROPERTY_FLAG_BITS_MAX_ENUM = VK_MEMORY_PROPERTY_FLAG_BITS_MAX_ENUM
#endif
        } w_memory_property_flag_bits;
        typedef uint32_t w_memory_property_flags;

        typedef enum w_command_buffer_level
        {
#ifdef __VULKAN__
            W_COMMAND_BUFFER_LEVEL_PRIMARY = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            W_COMMAND_BUFFER_LEVEL_SECONDARY = VK_COMMAND_BUFFER_LEVEL_SECONDARY
#endif
        } w_command_buffer_level;
        
        typedef enum w_command_buffer_usage_flag_bits
        {
#ifdef __VULKAN__
            W_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            W_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
            W_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT
#endif
        } w_command_buffer_usage_flag_bits;
        typedef uint32_t w_command_buffer_usage_flags;
        
		typedef enum w_format 
		{
#ifdef __VULKAN__
			W_FORMAT_UNDEFINED = VK_FORMAT_UNDEFINED,
			W_FORMAT_R4G4_UNORM_PACK8 = VK_FORMAT_R4G4_UNORM_PACK8,
			W_FORMAT_R4G4B4A4_UNORM_PACK16 = VK_FORMAT_R4G4B4A4_UNORM_PACK16,
			W_FORMAT_B4G4R4A4_UNORM_PACK16 = VK_FORMAT_B4G4R4A4_UNORM_PACK16,
			W_FORMAT_R5G6B5_UNORM_PACK16 = VK_FORMAT_R5G6B5_UNORM_PACK16,
			W_FORMAT_B5G6R5_UNORM_PACK16 = VK_FORMAT_B5G6R5_UNORM_PACK16,
			W_FORMAT_R5G5B5A1_UNORM_PACK16 = VK_FORMAT_R5G5B5A1_UNORM_PACK16,
			W_FORMAT_B5G5R5A1_UNORM_PACK16 = VK_FORMAT_B5G5R5A1_UNORM_PACK16,
			W_FORMAT_A1R5G5B5_UNORM_PACK16 = VK_FORMAT_A1R5G5B5_UNORM_PACK16,
			W_FORMAT_R8_UNORM = VK_FORMAT_R8_UNORM,
			W_FORMAT_R8_SNORM = VK_FORMAT_R8_SNORM,
			W_FORMAT_R8_USCALED = VK_FORMAT_R8_USCALED,
			W_FORMAT_R8_SSCALED = VK_FORMAT_R8_SSCALED,
			W_FORMAT_R8_UINT = VK_FORMAT_R8_UINT,
			W_FORMAT_R8_SINT = VK_FORMAT_R8_SINT,
			W_FORMAT_R8_SRGB = VK_FORMAT_R8_SRGB,
			W_FORMAT_R8G8_UNORM = VK_FORMAT_R8G8_UNORM,
			W_FORMAT_R8G8_SNORM = VK_FORMAT_R8G8_SNORM,
			W_FORMAT_R8G8_USCALED = VK_FORMAT_R8G8_USCALED,
			W_FORMAT_R8G8_SSCALED = VK_FORMAT_R8G8_SSCALED,
			W_FORMAT_R8G8_UINT = VK_FORMAT_R8G8_UINT,
			W_FORMAT_R8G8_SINT = VK_FORMAT_R8G8_SINT,
			W_FORMAT_R8G8_SRGB = VK_FORMAT_R8G8_SRGB,
			W_FORMAT_R8G8B8_UNORM = VK_FORMAT_R8G8B8_UNORM,
			W_FORMAT_R8G8B8_SNORM = VK_FORMAT_R8G8B8_SNORM,
			W_FORMAT_R8G8B8_USCALED = VK_FORMAT_R8G8B8_USCALED,
			W_FORMAT_R8G8B8_SSCALED = VK_FORMAT_R8G8B8_SSCALED,
			W_FORMAT_R8G8B8_UINT = VK_FORMAT_R8G8B8_UINT,
			W_FORMAT_R8G8B8_SINT = VK_FORMAT_R8G8B8_SINT,
			W_FORMAT_R8G8B8_SRGB = VK_FORMAT_R8G8B8_SRGB,
			W_FORMAT_B8G8R8_UNORM = VK_FORMAT_B8G8R8_UNORM,
			W_FORMAT_B8G8R8_SNORM = VK_FORMAT_B8G8R8_SNORM,
			W_FORMAT_B8G8R8_USCALED = VK_FORMAT_B8G8R8_USCALED,
			W_FORMAT_B8G8R8_SSCALED = VK_FORMAT_B8G8R8_SSCALED,
			W_FORMAT_B8G8R8_UINT = VK_FORMAT_B8G8R8_UINT,
			W_FORMAT_B8G8R8_SINT = VK_FORMAT_B8G8R8_SINT,
			W_FORMAT_B8G8R8_SRGB = VK_FORMAT_B8G8R8_SRGB,
			W_FORMAT_R8G8B8A8_UNORM = VK_FORMAT_R8G8B8A8_UNORM,
			W_FORMAT_R8G8B8A8_SNORM = VK_FORMAT_R8G8B8A8_SNORM,
			W_FORMAT_R8G8B8A8_USCALED = VK_FORMAT_R8G8B8A8_USCALED,
			W_FORMAT_R8G8B8A8_SSCALED = VK_FORMAT_R8G8B8A8_SSCALED,
			W_FORMAT_R8G8B8A8_UINT = VK_FORMAT_R8G8B8A8_UINT,
			W_FORMAT_R8G8B8A8_SINT = VK_FORMAT_R8G8B8A8_SINT,
			W_FORMAT_R8G8B8A8_SRGB = VK_FORMAT_R8G8B8A8_SRGB,
			W_FORMAT_B8G8R8A8_UNORM = VK_FORMAT_B8G8R8A8_UNORM,
			W_FORMAT_B8G8R8A8_SNORM = VK_FORMAT_B8G8R8A8_SNORM,
			W_FORMAT_B8G8R8A8_USCALED = VK_FORMAT_B8G8R8A8_USCALED,
			W_FORMAT_B8G8R8A8_SSCALED = VK_FORMAT_B8G8R8A8_SSCALED,
			W_FORMAT_B8G8R8A8_UINT = VK_FORMAT_B8G8R8A8_UINT,
			W_FORMAT_B8G8R8A8_SINT = VK_FORMAT_B8G8R8A8_SINT,
			W_FORMAT_B8G8R8A8_SRGB = VK_FORMAT_B8G8R8A8_SRGB,
			W_FORMAT_A8B8G8R8_UNORM_PACK32 = VK_FORMAT_A8B8G8R8_UNORM_PACK32,
			W_FORMAT_A8B8G8R8_SNORM_PACK32 = VK_FORMAT_A8B8G8R8_SNORM_PACK32,
			W_FORMAT_A8B8G8R8_USCALED_PACK32 = VK_FORMAT_A8B8G8R8_USCALED_PACK32,
			W_FORMAT_A8B8G8R8_SSCALED_PACK32 = VK_FORMAT_A8B8G8R8_SSCALED_PACK32,
			W_FORMAT_A8B8G8R8_UINT_PACK32 = VK_FORMAT_A8B8G8R8_UINT_PACK32,
			W_FORMAT_A8B8G8R8_SINT_PACK32 = VK_FORMAT_A8B8G8R8_SINT_PACK32,
			W_FORMAT_A8B8G8R8_SRGB_PACK32 = VK_FORMAT_A8B8G8R8_SRGB_PACK32,
			W_FORMAT_A2R10G10B10_UNORM_PACK32 = VK_FORMAT_A2R10G10B10_UNORM_PACK32,
			W_FORMAT_A2R10G10B10_SNORM_PACK32 = VK_FORMAT_A2R10G10B10_SNORM_PACK32,
			W_FORMAT_A2R10G10B10_USCALED_PACK32 = VK_FORMAT_A2R10G10B10_USCALED_PACK32,
			W_FORMAT_A2R10G10B10_SSCALED_PACK32 = VK_FORMAT_A2R10G10B10_SSCALED_PACK32,
			W_FORMAT_A2R10G10B10_UINT_PACK32 = VK_FORMAT_A2R10G10B10_UINT_PACK32,
			W_FORMAT_A2R10G10B10_SINT_PACK32 = VK_FORMAT_A2R10G10B10_SINT_PACK32,
			W_FORMAT_A2B10G10R10_UNORM_PACK32 = VK_FORMAT_A2B10G10R10_UNORM_PACK32,
			W_FORMAT_A2B10G10R10_SNORM_PACK32 = VK_FORMAT_A2B10G10R10_SNORM_PACK32,
			W_FORMAT_A2B10G10R10_USCALED_PACK32 = VK_FORMAT_A2B10G10R10_USCALED_PACK32,
			W_FORMAT_A2B10G10R10_SSCALED_PACK32 = VK_FORMAT_A2B10G10R10_SSCALED_PACK32,
			W_FORMAT_A2B10G10R10_UINT_PACK32 = VK_FORMAT_A2B10G10R10_UINT_PACK32,
			W_FORMAT_A2B10G10R10_SINT_PACK32 = VK_FORMAT_A2B10G10R10_SINT_PACK32,
			W_FORMAT_R16_UNORM = VK_FORMAT_R16_UNORM,
			W_FORMAT_R16_SNORM = VK_FORMAT_R16_SNORM,
			W_FORMAT_R16_USCALED = VK_FORMAT_R16_USCALED,
			W_FORMAT_R16_SSCALED = VK_FORMAT_R16_SSCALED,
			W_FORMAT_R16_UINT = VK_FORMAT_R16_UINT,
			W_FORMAT_R16_SINT = VK_FORMAT_R16_SINT,
			W_FORMAT_R16_SFLOAT = VK_FORMAT_R16_SFLOAT,
			W_FORMAT_R16G16_UNORM = VK_FORMAT_R16G16_UNORM,
			W_FORMAT_R16G16_SNORM = VK_FORMAT_R16G16_SNORM,
			W_FORMAT_R16G16_USCALED = VK_FORMAT_R16G16_USCALED,
			W_FORMAT_R16G16_SSCALED = VK_FORMAT_R16G16_SSCALED,
			W_FORMAT_R16G16_UINT = VK_FORMAT_R16G16_UINT,
			W_FORMAT_R16G16_SINT = VK_FORMAT_R16G16_SINT,
			W_FORMAT_R16G16_SFLOAT = VK_FORMAT_R16G16_SFLOAT,
			W_FORMAT_R16G16B16_UNORM = VK_FORMAT_R16G16B16_UNORM,
			W_FORMAT_R16G16B16_SNORM = VK_FORMAT_R16G16B16_SNORM,
			W_FORMAT_R16G16B16_USCALED = VK_FORMAT_R16G16B16_USCALED,
			W_FORMAT_R16G16B16_SSCALED = VK_FORMAT_R16G16B16_SSCALED,
			W_FORMAT_R16G16B16_UINT = VK_FORMAT_R16G16B16_UINT,
			W_FORMAT_R16G16B16_SINT = VK_FORMAT_R16G16B16_SINT,
			W_FORMAT_R16G16B16_SFLOAT = VK_FORMAT_R16G16B16_SFLOAT,
			W_FORMAT_R16G16B16A16_UNORM = VK_FORMAT_R16G16B16A16_UNORM,
			W_FORMAT_R16G16B16A16_SNORM = VK_FORMAT_R16G16B16A16_SNORM,
			W_FORMAT_R16G16B16A16_USCALED = VK_FORMAT_R16G16B16A16_USCALED,
			W_FORMAT_R16G16B16A16_SSCALED = VK_FORMAT_R16G16B16A16_SSCALED,
			W_FORMAT_R16G16B16A16_UINT = VK_FORMAT_R16G16B16A16_UINT,
			W_FORMAT_R16G16B16A16_SINT = VK_FORMAT_R16G16B16A16_SINT,
			W_FORMAT_R16G16B16A16_SFLOAT = VK_FORMAT_R16G16B16A16_SFLOAT,
			W_FORMAT_R32_UINT = VK_FORMAT_R32_UINT,
			W_FORMAT_R32_SINT = VK_FORMAT_R32_SINT,
			W_FORMAT_R32_SFLOAT = VK_FORMAT_R32_SFLOAT,
			W_FORMAT_R32G32_UINT = VK_FORMAT_R32G32_UINT,
			W_FORMAT_R32G32_SINT = VK_FORMAT_R32G32_SINT,
			W_FORMAT_R32G32_SFLOAT = VK_FORMAT_R32G32_SFLOAT,
			W_FORMAT_R32G32B32_UINT = VK_FORMAT_R32G32B32_UINT,
			W_FORMAT_R32G32B32_SINT = VK_FORMAT_R32G32B32_SINT,
			W_FORMAT_R32G32B32_SFLOAT = VK_FORMAT_R32G32B32_SFLOAT,
			W_FORMAT_R32G32B32A32_UINT = VK_FORMAT_R32G32B32A32_UINT,
			W_FORMAT_R32G32B32A32_SINT = VK_FORMAT_R32G32B32A32_SINT,
			W_FORMAT_R32G32B32A32_SFLOAT = VK_FORMAT_R32G32B32A32_SFLOAT,
			W_FORMAT_R64_UINT = VK_FORMAT_R64_UINT,
			W_FORMAT_R64_SINT = VK_FORMAT_R64_SINT,
			W_FORMAT_R64_SFLOAT = VK_FORMAT_R64_SFLOAT,
			W_FORMAT_R64G64_UINT = VK_FORMAT_R64G64_UINT,
			W_FORMAT_R64G64_SINT = VK_FORMAT_R64G64_SINT,
			W_FORMAT_R64G64_SFLOAT = VK_FORMAT_R64G64_SFLOAT,
			W_FORMAT_R64G64B64_UINT = VK_FORMAT_R64G64B64_UINT,
			W_FORMAT_R64G64B64_SINT = VK_FORMAT_R64G64B64_SINT,
			W_FORMAT_R64G64B64_SFLOAT = VK_FORMAT_R64G64B64_SFLOAT,
			W_FORMAT_R64G64B64A64_UINT = VK_FORMAT_R64G64B64A64_UINT,
			W_FORMAT_R64G64B64A64_SINT = VK_FORMAT_R64G64B64A64_SINT,
			W_FORMAT_R64G64B64A64_SFLOAT = VK_FORMAT_R64G64B64A64_SFLOAT,
			W_FORMAT_B10G11R11_UFLOAT_PACK32 = VK_FORMAT_B10G11R11_UFLOAT_PACK32,
			W_FORMAT_E5B9G9R9_UFLOAT_PACK32 = VK_FORMAT_E5B9G9R9_UFLOAT_PACK32,
			W_FORMAT_D16_UNORM = VK_FORMAT_D16_UNORM,
			W_FORMAT_X8_D24_UNORM_PACK32 = VK_FORMAT_X8_D24_UNORM_PACK32,
			W_FORMAT_D32_SFLOAT = VK_FORMAT_D32_SFLOAT,
			W_FORMAT_S8_UINT = VK_FORMAT_S8_UINT,
			W_FORMAT_D16_UNORM_S8_UINT = VK_FORMAT_D16_UNORM_S8_UINT,
			W_FORMAT_D24_UNORM_S8_UINT = VK_FORMAT_D24_UNORM_S8_UINT,
			W_FORMAT_D32_SFLOAT_S8_UINT = VK_FORMAT_D32_SFLOAT_S8_UINT,
			W_FORMAT_BC1_RGB_UNORM_BLOCK = VK_FORMAT_BC1_RGB_UNORM_BLOCK,
			W_FORMAT_BC1_RGB_SRGB_BLOCK = VK_FORMAT_BC1_RGB_SRGB_BLOCK,
			W_FORMAT_BC1_RGBA_UNORM_BLOCK = VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
			W_FORMAT_BC1_RGBA_SRGB_BLOCK = VK_FORMAT_BC1_RGBA_SRGB_BLOCK,
			W_FORMAT_BC2_UNORM_BLOCK = VK_FORMAT_BC2_UNORM_BLOCK,
			W_FORMAT_BC2_SRGB_BLOCK = VK_FORMAT_BC2_SRGB_BLOCK,
			W_FORMAT_BC3_UNORM_BLOCK = VK_FORMAT_BC3_UNORM_BLOCK,
			W_FORMAT_BC3_SRGB_BLOCK = VK_FORMAT_BC3_SRGB_BLOCK,
			W_FORMAT_BC4_UNORM_BLOCK = VK_FORMAT_BC4_UNORM_BLOCK,
			W_FORMAT_BC4_SNORM_BLOCK = VK_FORMAT_BC4_SNORM_BLOCK,
			W_FORMAT_BC5_UNORM_BLOCK = VK_FORMAT_BC5_UNORM_BLOCK,
			W_FORMAT_BC5_SNORM_BLOCK = VK_FORMAT_BC5_SNORM_BLOCK,
			W_FORMAT_BC6H_UFLOAT_BLOCK = VK_FORMAT_BC6H_UFLOAT_BLOCK,
			W_FORMAT_BC6H_SFLOAT_BLOCK = VK_FORMAT_BC6H_SFLOAT_BLOCK,
			W_FORMAT_BC7_UNORM_BLOCK = VK_FORMAT_BC7_UNORM_BLOCK,
			W_FORMAT_BC7_SRGB_BLOCK = VK_FORMAT_BC7_SRGB_BLOCK,
			W_FORMAT_ETC2_R8G8B8_UNORM_BLOCK = VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK,
			W_FORMAT_ETC2_R8G8B8_SRGB_BLOCK = VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK,
			W_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK = VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK,
			W_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK = VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK,
			W_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK = VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK,
			W_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK = VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK,
			W_FORMAT_EAC_R11_UNORM_BLOCK = VK_FORMAT_EAC_R11_UNORM_BLOCK,
			W_FORMAT_EAC_R11_SNORM_BLOCK = VK_FORMAT_EAC_R11_SNORM_BLOCK,
			W_FORMAT_EAC_R11G11_UNORM_BLOCK = VK_FORMAT_EAC_R11G11_UNORM_BLOCK,
			W_FORMAT_EAC_R11G11_SNORM_BLOCK = VK_FORMAT_EAC_R11G11_SNORM_BLOCK,
			W_FORMAT_ASTC_4x4_UNORM_BLOCK = VK_FORMAT_ASTC_4x4_UNORM_BLOCK,
			W_FORMAT_ASTC_4x4_SRGB_BLOCK = VK_FORMAT_ASTC_4x4_SRGB_BLOCK,
			W_FORMAT_ASTC_5x4_UNORM_BLOCK = VK_FORMAT_ASTC_5x4_UNORM_BLOCK,
			W_FORMAT_ASTC_5x4_SRGB_BLOCK = VK_FORMAT_ASTC_5x4_SRGB_BLOCK,
			W_FORMAT_ASTC_5x5_UNORM_BLOCK = VK_FORMAT_ASTC_5x5_UNORM_BLOCK,
			W_FORMAT_ASTC_5x5_SRGB_BLOCK = VK_FORMAT_ASTC_5x5_SRGB_BLOCK,
			W_FORMAT_ASTC_6x5_UNORM_BLOCK = VK_FORMAT_ASTC_6x5_UNORM_BLOCK,
			W_FORMAT_ASTC_6x5_SRGB_BLOCK = VK_FORMAT_ASTC_6x5_SRGB_BLOCK,
			W_FORMAT_ASTC_6x6_UNORM_BLOCK = VK_FORMAT_ASTC_6x6_UNORM_BLOCK,
			W_FORMAT_ASTC_6x6_SRGB_BLOCK = VK_FORMAT_ASTC_6x6_SRGB_BLOCK,
			W_FORMAT_ASTC_8x5_UNORM_BLOCK = VK_FORMAT_ASTC_8x5_UNORM_BLOCK,
			W_FORMAT_ASTC_8x5_SRGB_BLOCK = VK_FORMAT_ASTC_8x5_SRGB_BLOCK,
			W_FORMAT_ASTC_8x6_UNORM_BLOCK = VK_FORMAT_ASTC_8x6_UNORM_BLOCK,
			W_FORMAT_ASTC_8x6_SRGB_BLOCK = VK_FORMAT_ASTC_8x6_SRGB_BLOCK,
			W_FORMAT_ASTC_8x8_UNORM_BLOCK = VK_FORMAT_ASTC_8x8_UNORM_BLOCK,
			W_FORMAT_ASTC_8x8_SRGB_BLOCK = VK_FORMAT_ASTC_8x8_SRGB_BLOCK,
			W_FORMAT_ASTC_10x5_UNORM_BLOCK = VK_FORMAT_ASTC_10x5_UNORM_BLOCK,
			W_FORMAT_ASTC_10x5_SRGB_BLOCK = VK_FORMAT_ASTC_10x5_SRGB_BLOCK,
			W_FORMAT_ASTC_10x6_UNORM_BLOCK = VK_FORMAT_ASTC_10x6_UNORM_BLOCK,
			W_FORMAT_ASTC_10x6_SRGB_BLOCK = VK_FORMAT_ASTC_10x6_SRGB_BLOCK,
			W_FORMAT_ASTC_10x8_UNORM_BLOCK = VK_FORMAT_ASTC_10x8_UNORM_BLOCK,
			W_FORMAT_ASTC_10x8_SRGB_BLOCK = VK_FORMAT_ASTC_10x8_SRGB_BLOCK,
			W_FORMAT_ASTC_10x10_UNORM_BLOCK = VK_FORMAT_ASTC_10x10_UNORM_BLOCK,
			W_FORMAT_ASTC_10x10_SRGB_BLOCK = VK_FORMAT_ASTC_10x10_SRGB_BLOCK,
			W_FORMAT_ASTC_12x10_UNORM_BLOCK = VK_FORMAT_ASTC_12x10_UNORM_BLOCK,
			W_FORMAT_ASTC_12x10_SRGB_BLOCK = VK_FORMAT_ASTC_12x10_SRGB_BLOCK,
			W_FORMAT_ASTC_12x12_UNORM_BLOCK = VK_FORMAT_ASTC_12x12_UNORM_BLOCK,
			W_FORMAT_ASTC_12x12_SRGB_BLOCK = VK_FORMAT_ASTC_12x12_SRGB_BLOCK,
			W_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG = VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG,
			W_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG = VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG,
			W_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG = VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG,
			W_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG = VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG,
			W_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG = VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG,
			W_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG = VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG,
			W_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG = VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG,
			W_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG = VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG,
			W_FORMAT_G8B8G8R8_422_UNORM_KHR = VK_FORMAT_G8B8G8R8_422_UNORM_KHR,
			W_FORMAT_B8G8R8G8_422_UNORM_KHR = VK_FORMAT_B8G8R8G8_422_UNORM_KHR,
			W_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR,
			W_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR,
			W_FORMAT_G8_B8_R8_3PLANE_422_UNORM_KHR = VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM_KHR,
			W_FORMAT_G8_B8R8_2PLANE_422_UNORM_KHR = VK_FORMAT_G8_B8R8_2PLANE_422_UNORM_KHR,
			W_FORMAT_G8_B8_R8_3PLANE_444_UNORM_KHR = VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM_KHR,
			W_FORMAT_R10X6_UNORM_PACK16_KHR = VK_FORMAT_R10X6_UNORM_PACK16_KHR,
			W_FORMAT_R10X6G10X6_UNORM_2PACK16_KHR = VK_FORMAT_R10X6G10X6_UNORM_2PACK16_KHR,
			W_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHR = VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHR,
			W_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR = VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR,
			W_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR = VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR,
			W_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_KHR = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_KHR,
			W_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_KHR = VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_KHR,
			W_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16_KHR = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16_KHR,
			W_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16_KHR = VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16_KHR,
			W_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16_KHR = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16_KHR,
			W_FORMAT_R12X4_UNORM_PACK16_KHR = VK_FORMAT_R12X4_UNORM_PACK16_KHR,
			W_FORMAT_R12X4G12X4_UNORM_2PACK16_KHR = VK_FORMAT_R12X4G12X4_UNORM_2PACK16_KHR,
			W_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHR = VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHR,
			W_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR = VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR,
			W_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR = VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR,
			W_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_KHR = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_KHR,
			W_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_KHR = VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_KHR,
			W_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16_KHR = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16_KHR,
			W_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16_KHR = VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16_KHR,
			W_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16_KHR = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16_KHR,
			W_FORMAT_G16B16G16R16_422_UNORM_KHR = VK_FORMAT_G16B16G16R16_422_UNORM_KHR,
			W_FORMAT_B16G16R16G16_422_UNORM_KHR = VK_FORMAT_B16G16R16G16_422_UNORM_KHR,
			W_FORMAT_G16_B16_R16_3PLANE_420_UNORM_KHR = VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM_KHR,
			W_FORMAT_G16_B16R16_2PLANE_420_UNORM_KHR = VK_FORMAT_G16_B16R16_2PLANE_420_UNORM_KHR,
			W_FORMAT_G16_B16_R16_3PLANE_422_UNORM_KHR = VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM_KHR,
			W_FORMAT_G16_B16R16_2PLANE_422_UNORM_KHR = VK_FORMAT_G16_B16R16_2PLANE_422_UNORM_KHR,
			W_FORMAT_G16_B16_R16_3PLANE_444_UNORM_KHR = VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM_KHR,
			W_FORMAT_BEGIN_RANGE = VK_FORMAT_UNDEFINED,
			W_FORMAT_END_RANGE = VK_FORMAT_ASTC_12x12_SRGB_BLOCK,
			W_FORMAT_RANGE_SIZE = (VK_FORMAT_ASTC_12x12_SRGB_BLOCK - VK_FORMAT_UNDEFINED + 1),
			W_FORMAT_MAX_ENUM = VK_FORMAT_MAX_ENUM
#endif
		} w_format;
    }
}

#endif
