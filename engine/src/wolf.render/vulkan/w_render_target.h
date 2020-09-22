/*
	Project			 : Wolf Engine. Copyright(c) Pooya Eimandar (https://PooyaEimandar.github.io) . All rights reserved.
	Source			 : Please direct any bug to https://github.com/WolfEngine/Wolf.Engine/issues
	Website			 : https://WolfEngine.App
	Name			 : w_render_target.h
	Description		 : render to texture
	Comment          : 
*/

#pragma once

#include "w_graphics_device_manager.h"
#include "w_viewport.h"
#include <functional>

namespace wolf::render::vulkan
{
	class w_render_target_pimp;
	class w_render_target
	{
	public:
		W_VK_EXP w_render_target();
		W_VK_EXP virtual ~w_render_target();

		W_VK_EXP W_RESULT load(
			_In_ const std::shared_ptr<w_graphics_device>& pGDevice,
			_In_ const w_point& pOffset,
			_In_ const w_point_t& pSize,
			_In_ std::vector<w_image_view> pAttachments,
			_In_ const size_t pCount);

		W_VK_EXP W_RESULT record_command_buffer(_In_ w_command_buffers* pCommandBuffer,
			_In_ std::function<W_RESULT(void)> pDrawFunction,
			_In_ w_color pClearColor = w_color::PURPLE(),
			_In_ const float pClearDepth = 1.0f,
			_In_ const uint32_t pClearStencil = 0);

		//save texture as bitmap file
	   // W_VK_EXP W_RESULT save_to_file(_In_z_ const char* pFileName);

		//release all resources
		W_VK_EXP ULONG release();

#pragma region Getters

		//get offset of render target
		W_VK_EXP const w_point get_offset() const;
		//get size of render target
		W_VK_EXP const w_point_t get_size() const;
		//get sampler of render target's buffers
		W_VK_EXP w_sampler get_sampler(_In_ size_t pBufferIndex) const;
		//get image view handles of render target's buffers
		W_VK_EXP w_image_view get_image_view(_In_ size_t pBufferIndex) const;
		//get image format of render target's buffers
		W_VK_EXP w_image_type get_image_type(_In_ size_t pBufferIndex) const;
		//get image view type render target's buffer
		W_VK_EXP w_image_view_type get_image_view_type(_In_ size_t pBufferIndex) const;
		//get format of attached buffer
		W_VK_EXP const w_format get_attachment_format(_In_ size_t pBufferIndex) const;
		//get description info of attached buffer
		W_VK_EXP const w_descriptor_image_info get_attachment_descriptor_info(_In_ const size_t& pBufferIndex) const;

#pragma endregion

#ifdef __PYTHON__

		W_RESULT py_load(
			_In_ boost::shared_ptr<w_graphics_device>& pGDevice,
			_In_ w_viewport pViewPort,
			_In_ w_viewport_scissor pViewportScissor,
			_In_ boost::python::list pAttachments,
			_In_ const size_t& pCount)
		{
			if (!pGDevice.get()) return W_FAILED;
			auto _gDevice = boost_shared_ptr_to_std_shared_ptr<w_graphics_device>(pGDevice);

			std::vector<w_image_view> _attachments;
			boost_list_to_std_vector(pAttachments, _attachments);

			auto _hr = load(
				_gDevice,
				pViewPort,
				pViewportScissor,
				_attachments,
				pCount);

			//reset local shared_ptr
			_gDevice.reset();
			_attachments.clear();

			return _hr;
		}

		static W_RESULT py_record_command_buffer(
			_In_ w_render_target& pSelf,
			_In_ w_command_buffers& pCommandBuffer,
			_In_ boost::python::object pDrawFunction,
			_In_ w_color pClearColor = w_color::PURPLE(),
			_In_ const float& pClearDepth = 1.0f,
			_In_ const uint32_t & pClearStencil = 0)
		{
			std::function<W_RESULT(void)> _draw_func = [&]()->W_RESULT
			{
				if (!pDrawFunction.is_none())
				{
					pDrawFunction();
				}
				return W_PASSED;
			};
			return pSelf.record_command_buffer(
				&pCommandBuffer,
				_draw_func,
				pClearColor,
				pClearDepth,
				pClearStencil);
		}

#endif

	private:
		bool											_is_released;
		w_render_target_pimp* _pimp;
	};
}

#include "python_exporter/py_render_target.h"

