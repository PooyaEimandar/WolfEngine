/*
	Project			 : Wolf Engine. Copyright(c) Pooya Eimandar (https://PooyaEimandar.github.io) . All rights reserved.
	Source			 : Please direct any bug to https://github.com/WolfEngine/Wolf.Engine/issues
	Website			 : https://WolfEngine.App
	Name			 : w_command_buffers
	Description		 : Command buffers
	Comment          : 
*/

#pragma once

#include <w_graphics_headers.h>
#include <w_render_export.h>

namespace wolf::render::vulkan
{
	struct w_command_buffer
	{
		VkCommandBuffer	handle = 0;

		W_VK_EXP W_RESULT begin(_In_ const uint32_t pFlags);

		W_VK_EXP W_RESULT begin_secondary(
			_In_ const w_render_pass_handle& pRenderPassHandle,
			_In_ const w_frame_buffer_handle& pFrameBufferHandle,
			_In_ const uint32_t pFlags);

		W_VK_EXP W_RESULT end();

		W_VK_EXP W_RESULT flush(_In_ const std::shared_ptr<w_graphics_device>& pGDevice);

		W_VK_EXP W_RESULT execute_secondary_commands(_In_ const std::vector<w_command_buffer*>& pSecondaryCommandBuffers);

	private:
		bool began = false;
	};

	class w_command_buffer_pimp;
	class w_command_buffers
	{
	public:
		W_VK_EXP w_command_buffers();
		W_VK_EXP ~w_command_buffers();

		/*
			pGDevice = graphices device,
			pCount = count of command buffers to be created,
			pLevel = primary or secondary level of thread execution
			pCommandPoolQueue = by default vk_graphics_queue will be used, set another w_queue_index for creating command another pool
		*/
		W_VK_EXP W_RESULT load(
			_In_ const std::shared_ptr<w_graphics_device>& pGDevice,
			_In_ const size_t pCount,
			_In_ const w_command_buffer_level pLevel = w_command_buffer_level::PRIMARY,
			_In_ const bool pCreateCommandPool = false,
			_In_ const w_queue* pCommandPoolQueue = nullptr);

		//begin command buffer
		W_VK_EXP W_RESULT begin(_In_ const size_t& pCommandBufferIndex,
			_In_ const uint32_t pFlags = w_command_buffer_usage_flag_bits::SIMULTANEOUS_USE_BIT);

		//begin secondary command buffer
		W_VK_EXP W_RESULT begin_secondary(
			_In_ const size_t pCommandBufferIndex,
			_In_ const w_render_pass_handle& pRenderPassHandle,
			_In_ const w_frame_buffer_handle& pFrameBufferHandle,
			_In_ const uint32_t pFlags = w_command_buffer_usage_flag_bits::RENDER_PASS_CONTINUE_BIT | w_command_buffer_usage_flag_bits::SIMULTANEOUS_USE_BIT);

		//end command buffer
		W_VK_EXP W_RESULT end(_In_ const size_t pCommandBufferIndex);

		//Flushing the command buffer will also submit it to the queue and uses a fence to ensure that command has been executed before returning
		W_VK_EXP W_RESULT flush(_In_ const size_t pCommandBufferIndex);

		//Flushing all command buffers will also submit these commands buffers to the queue and uses a fence to ensure that all commands have been executed before returning
		W_VK_EXP W_RESULT flush_all();

		W_VK_EXP W_RESULT execute_secondary_commands(
			_In_ const size_t pCommandBufferIndex,
			_In_ const std::vector<w_command_buffer*>& pSecondaryCommandBuffers);

		//release all resources
		W_VK_EXP ULONG release();

#pragma region Getters

		W_VK_EXP const w_command_buffer* get_commands() const;
		W_VK_EXP const w_command_buffer get_command_at(_In_ const size_t pIndex) const;
		W_VK_EXP const size_t get_commands_size() const;

#pragma endregion

#ifdef __PYTHON__
		W_RESULT py_load(
			_In_ boost::shared_ptr<wolf::graphics::w_graphics_device>& pGDevice,
			_In_ const size_t& pCount,
			_In_ const w_command_buffer_level& pLevel)
		{
			if (!pGDevice.get()) return W_FAILED;
			auto _gDevice = boost_shared_ptr_to_std_shared_ptr<w_graphics_device>(pGDevice);

			auto _hr = load(_gDevice, pCount, pLevel);

			_gDevice.reset();
			return _hr;
		}

		boost::python::list py_get_commands()
		{
			boost::python::list _list;

			auto _cmds = get_commands();
			for (size_t i = 0; i < get_commands_size(); ++i)
			{
				_list.append(_cmds[i]);
			}
			_cmds = nullptr;
			return _list;
		}

#endif

	private:
		bool								_is_released;
		w_command_buffer_pimp* _pimp;
	};
}

#include "python_exporter/py_command_buffers.h"
