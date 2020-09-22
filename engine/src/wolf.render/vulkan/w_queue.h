/*
	Project			 : Wolf Engine. Copyright(c) Pooya Eimandar (https://PooyaEimandar.github.io) . All rights reserved.
	Source			 : Please direct any bug to https://github.com/WolfEngine/Wolf.Engine/issues
	Website			 : https://WolfEngine.App
	Name			 : w_queue.h
	Description		 : graphice queue and index
	Comment          :
*/

#pragma once

#include <w_graphics_headers.h>
#include <w_render_export.h>

namespace wolf::render::vulkan
{
	struct w_queue
	{
#ifdef __VULKAN__
		VkQueue        queue = 0;
#endif
		uint32_t       index = UINT32_MAX;

		W_VK_EXP ULONG release();
	};
}

#include "python_exporter/py_queue.h"
