#include "w_render_pch.h"
#include "w_render_target.h"
#include "w_render_pass.h"
#include "w_texture.h"

namespace wolf
{
	namespace render
	{
		namespace vulkan
		{
			class w_render_target_pimp
			{
			public:
				w_render_target_pimp() :
					_name("w_render_target"),
					_gDevice(nullptr)
				{
				}

				W_RESULT load(
					_In_ const std::shared_ptr<w_graphics_device>& pGDevice,
					_In_ const w_point& pOffset,
					_In_ const w_point_t& pSize,
					_In_ std::vector<w_image_view> pAttachments,
					_In_ const size_t pCount)
				{
					const std::string _trace_info = this->_name + "::load";

					this->_gDevice = pGDevice;

					W_RESULT _hr = W_PASSED;
					std::vector<std::vector<w_image_view>> _frame_buffers;
					for (size_t i = 0; i < pCount; ++i)
					{
						std::vector<w_image_view> _image_views;
						//check for attachment and create texture buffer for them
						for (size_t i = 0; i < pAttachments.size(); ++i)
						{
							auto _attachment = pAttachments[i];

							auto _texture_buffer = new (std::nothrow) w_texture();
							if (!_texture_buffer)
							{
								_hr = W_FAILED;
								V(_hr,
									w_log_type::W_ERROR,
									"allocating memory for texture for graphics device: {}. trace info: {}",
									pGDevice->get_info(),
									_trace_info);
								break;
							}

							if (_attachment.attachment_desc.ref.layout == VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
							{
								//color
								_attachment.attachment_desc.desc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
								_texture_buffer->set_buffer_type(w_texture_buffer_type::COLOR);
								_texture_buffer->set_usage_flags(w_image_usage_flag_bits::IMAGE_USAGE_COLOR_ATTACHMENT_BIT | w_image_usage_flag_bits::IMAGE_USAGE_SAMPLED_BIT);
							}
							else if (_attachment.attachment_desc.ref.layout == VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
							{
								//depth
								_texture_buffer->set_buffer_type(w_texture_buffer_type::DEPTH);
								_texture_buffer->set_usage_flags(w_image_usage_flag_bits::IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | w_image_usage_flag_bits::IMAGE_USAGE_SAMPLED_BIT);
							}

							_texture_buffer->set_format((w_format)_attachment.attachment_desc.desc.format);
							auto _hr = _texture_buffer->initialize(pGDevice, pSize.x, pSize.y, false, _attachment.attachment_desc.memory_flag);
							if (_hr == W_FAILED)
							{
								V(W_FAILED,
									w_log_type::W_ERROR,
									"loading texture with graphics device: {}. trace info: {}",
									pGDevice->get_info(),
									_trace_info);
								break;
							}
							_hr = _texture_buffer->load();
							if (_hr == W_FAILED)
							{
								V(W_FAILED,
									w_log_type::W_ERROR,
									"initializing texture graphics device: {}. trace info: {}",
									pGDevice->get_info(),
									_trace_info);
								break;
							}

							w_image_view _image_view = _texture_buffer->get_image_view();
							_image_view.attachment_desc = _attachment.attachment_desc;
							_image_views.push_back(_image_view);

							//store in global vector
							this->_attachment_buffers.push_back(_texture_buffer);
						}

						_frame_buffers.push_back(_image_views);
					}

					if (_hr == W_FAILED)
					{
						_frame_buffers.clear();
						release();
						return W_FAILED;
					}

					_hr = this->_render_pass.load(pGDevice,
						pOffset,
						pSize,
						_frame_buffers);
					if (_hr == W_FAILED)
					{
						_frame_buffers.clear();
						release();
						V(W_FAILED,
							w_log_type::W_ERROR,
							"loading render pass with graphics device: {}. trace info: {}",
							pGDevice->get_info(),
							_trace_info);
						return W_FAILED;
					}

					_frame_buffers.clear();

					return W_PASSED;
				}

				W_RESULT record_command_buffer(
					_In_ w_command_buffers* pCommandBuffers,
					_In_ std::function<W_RESULT(void)> pDrawFunction,
					_In_ w_color pClearColor,
					_In_ const float pClearDepth,
					_In_ const uint32_t pClearStencil)
				{
					if (!pCommandBuffers) return W_FAILED;

					const std::string _trace_info = this->_name + "::record_command_buffer";

					if (!pCommandBuffers) return W_FAILED;

					auto _cmd_size = pCommandBuffers->get_commands_size();
					if (_cmd_size != this->_render_pass.get_number_of_frame_buffers())
					{
						V(W_FAILED,
							w_log_type::W_ERROR,
							"parameter count mismatch. Number of command buffers must equal to number of frame buffers. graphics device:{} trace info: {}",
							this->_gDevice->get_info(),
							_trace_info);
						return W_FAILED;
					}

					W_RESULT _hr = W_PASSED;
					for (uint32_t i = 0; i < _cmd_size; ++i)
					{
						pCommandBuffers->begin(i);
						{
							auto _cmd = pCommandBuffers->get_command_at(i);
							this->_render_pass.begin(
								i,
								_cmd,
								pClearColor,
								pClearDepth,
								pClearStencil);
							{
								if (pDrawFunction)
								{
									_hr = pDrawFunction();
								}
							}
							this->_render_pass.end(_cmd);
						}
						pCommandBuffers->end(i);
					}
					return _hr;
				}

				ULONG release()
				{
					for (auto _buffer : this->_attachment_buffers)
					{
						SAFE_RELEASE(_buffer);
					}
					this->_attachment_buffers.clear();
					this->_render_pass.release();
					this->_gDevice = nullptr;

					return 0;
				}

#pragma region Getters

				const w_point get_offset() const
				{
					return this->_render_pass.get_offset();
				}

				const w_point_t get_size() const
				{
					return this->_render_pass.get_size();
				}

				w_sampler get_sampler(_In_ size_t pBufferIndex) const
				{
					if (pBufferIndex >= this->_attachment_buffers.size()) return w_sampler();

					auto _t = this->_attachment_buffers.at(pBufferIndex);
					if (_t) return _t->get_sampler();

					return w_sampler();
				}

				w_image_view get_image_view(_In_ size_t pBufferIndex) const
				{
					if (pBufferIndex >= this->_attachment_buffers.size()) return w_image_view();

					auto _t = this->_attachment_buffers.at(pBufferIndex);
					if (_t) return _t->get_image_view();

					return w_image_view();
				}

				w_image_type get_image_type(_In_ size_t pBufferIndex) const
				{
					if (pBufferIndex >= this->_attachment_buffers.size()) return w_image_type::IMAGE_TYPE_END_RANGE;

					auto _t = this->_attachment_buffers.at(pBufferIndex);
					if (_t) return _t->get_image_type();

					return w_image_type::IMAGE_TYPE_END_RANGE;
				}

				w_image_view_type get_image_view_type(_In_ size_t pBufferIndex) const
				{
					if (pBufferIndex >= this->_attachment_buffers.size()) return w_image_view_type::IMAGE_VIEW_TYPE_END_RANGE;

					auto _t = this->_attachment_buffers.at(pBufferIndex);
					if (_t) return _t->get_image_view_type();

					return w_image_view_type::IMAGE_VIEW_TYPE_END_RANGE;
				}

				const w_format get_attachment_format(_In_ size_t pBufferIndex) const
				{
					if (pBufferIndex >= this->_attachment_buffers.size()) return w_format::UNDEFINED;

					auto _t = this->_attachment_buffers.at(pBufferIndex);
					if (_t) return _t->get_format();

					return w_format::UNDEFINED;
				}

				const w_descriptor_image_info get_attachment_descriptor_info(_In_ size_t pBufferIndex) const
				{
					if (pBufferIndex >= this->_attachment_buffers.size()) return w_descriptor_image_info();

					auto _t = this->_attachment_buffers.at(pBufferIndex);
					if (_t)
					{
						return _t->get_descriptor_info();
					}
					return w_descriptor_image_info();
				}

#pragma endregion

			private:

				std::string													_name;
				std::shared_ptr<w_graphics_device>							_gDevice;
				w_render_pass												_render_pass;
				std::vector<w_texture*>										_attachment_buffers;
			};
		}
	}
}

using namespace wolf::render::vulkan;

w_render_target::w_render_target() :
	_is_released(false),
    _pimp(new w_render_target_pimp())
{
}

w_render_target::~w_render_target()
{
	release();
}

W_RESULT w_render_target::load(
	_In_ const std::shared_ptr<w_graphics_device>& pGDevice,
	_In_ const w_point& pOffset,
	_In_ const w_point_t& pSize,
	_In_ std::vector<w_image_view> pAttachments,
	_In_ const size_t pCount)
{
	if (!this->_pimp) return W_FAILED;
	return this->_pimp->load(
		pGDevice,
		pOffset,
		pSize,
		pAttachments,
		pCount);
}

W_RESULT w_render_target::record_command_buffer(
	_In_ w_command_buffers* pCommandBuffer,
	_In_ std::function<W_RESULT(void)> pDrawFunction,
	_In_ w_color pClearColor,
	_In_ const float pClearDepth,
	_In_ const uint32_t pClearStencil)
{
	if (!this->_pimp) return W_FAILED;
	return this->_pimp->record_command_buffer(
		pCommandBuffer,
		pDrawFunction,
		pClearColor,
		pClearDepth,
		pClearStencil);
}

//W_RESULT w_render_target::save_to_file(_In_z_ const char* pFilename)
//{
////    short header[] = { 0x4D42, 0, 0, 0, 0, 26, 0, 12, 0, (short)pWidth, (short)pHeight, 1, 24 };
////    FILE *f;
////    
////#ifdef __WIN32
////    if (!fopen_s(&f, pFilename, "wb"))
////#else
////    f = fopen(pFilename, "wb");
////    if (!f)
////#endif
////    {
////        fwrite(header, 1, sizeof(header), f);
////        fwrite(pData, 1, pWidth * pHeight * 3, f);
////        fclose(f);
////    }
//
//	return W_PASSED;
//}

ULONG w_render_target::release()
{
	if (this->_is_released) return 1;
    
    SAFE_RELEASE(this->_pimp);
	this->_is_released = true;

	return 0;
}

#pragma region Getters

const w_point w_render_target::get_offset() const
{
	if (!this->_pimp) return w_point();
	return this->_pimp->get_offset();
}

const w_point_t w_render_target::get_size() const
{
	if (!this->_pimp) return w_point_t();
	return this->_pimp->get_size();
}

w_sampler w_render_target::get_sampler(_In_ size_t pBufferIndex) const
{
	if (!this->_pimp) return w_sampler();
	return this->_pimp->get_sampler(pBufferIndex);
}

w_image_view w_render_target::get_image_view(_In_ size_t pBufferIndex) const
{
	if (!this->_pimp) return w_image_view();
	return this->_pimp->get_image_view(pBufferIndex);
}

w_image_type w_render_target::get_image_type(_In_ size_t pBufferIndex) const
{
	if (!this->_pimp) return w_image_type::IMAGE_TYPE_END_RANGE;
	return this->_pimp->get_image_type(pBufferIndex);
}

w_image_view_type w_render_target::get_image_view_type(_In_ size_t pBufferIndex) const
{
	if (!this->_pimp) return w_image_view_type::IMAGE_VIEW_TYPE_END_RANGE;
	return this->_pimp->get_image_view_type(pBufferIndex);
}

const w_format w_render_target::get_attachment_format(_In_ size_t pBufferIndex) const
{
	if (!this->_pimp) return w_format::UNDEFINED;
	return this->_pimp->get_attachment_format(pBufferIndex);
}

const w_descriptor_image_info w_render_target::get_attachment_descriptor_info(_In_ const size_t& pBufferIndex) const
{
	if (!this->_pimp) return w_descriptor_image_info();
	return this->_pimp->get_attachment_descriptor_info(pBufferIndex);
}

#pragma endregion
