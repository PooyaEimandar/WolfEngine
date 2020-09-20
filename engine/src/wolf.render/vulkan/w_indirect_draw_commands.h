/*
	Project			 : Wolf Engine. Copyright(c) Pooya Eimandar (https://PooyaEimandar.github.io) . All rights reserved.
	Source			 : Please direct any bug to https://github.com/WolfEngine/Wolf.Engine/issues
	Website			 : https://WolfEngine.App
	Name			 : w_indirect_draw_commands
	Description		 : indirect draw command buffers
	Comment          : 
*/

#pragma once

#include <w_graphics_device_manager.h>
#include "w_buffer.h"

namespace wolf::render::vulkan
{
	struct w_indirect_draws_command_buffer
	{
		wolf::render::vulkan::w_buffer								buffer;
		std::vector<w_draw_indexed_indirect_command>				drawing_commands;

		W_VK_EXP W_RESULT load(
			_In_ const std::shared_ptr<w_graphics_device>& pGDevice,
			_In_ const uint32_t pDrawCount);
	};
}
