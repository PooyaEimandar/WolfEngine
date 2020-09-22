#include "w_render_pch.h"
#include "w_mesh.h"
#include "w_buffer.h"
#include "w_command_buffers.h"
#include "w_uniform.h"

namespace wolf
{
	namespace render
	{
		namespace vulkan
		{
			class w_mesh_pimp
			{
			public:
				w_mesh_pimp() :
					_name("w_mesh"),
					_gDevice(nullptr),
					_vertices_count(0),
					_indices_count(0),
					_vertex_binding_attributes(w_vertex_declaration::NOT_DEFINED)
				{
				}

				/*
					static data such as index buffer or vertex buffer should be stored on device memory
					for fastest access by GPU.
					So we are giong to do folloing steps:
						* create buffer in DRAM
						* copy user data to this buffer
						* create buffer in VRAM
						* copy from DRAM to VRAM
						* delete DRAM buffer
						* use VRAM buffer for rendering
				*/
				W_RESULT load(_In_ const std::shared_ptr<w_graphics_device>& pGDevice,
					_In_ const void* const pVerticesData,
					_In_ const uint32_t  pVerticesSizeInBytes,
					_In_ const uint32_t pVerticesCount,
					_In_ const uint32_t* const pIndicesData,
					_In_ const uint32_t pIndicesCount,
					_In_ const bool pUseDynamicBuffer)
				{
					this->_gDevice = pGDevice;
					this->_vertices_count = pVerticesCount;
					this->_indices_count = pIndicesCount;
					this->_dynamic_buffer = pUseDynamicBuffer;

					if (pVerticesCount == 0 || pVerticesData == nullptr)
					{
						return W_FAILED;
					}

					bool _there_is_no_index_buffer = false;
					uint32_t _indices_size = pIndicesCount * sizeof(uint32_t);
					if (pIndicesCount == 0 || pIndicesData == nullptr)
					{
						_there_is_no_index_buffer = true;
					}

					//create a buffers hosted into the DRAM named staging buffers
					if (_create_buffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
						pVerticesData,
						pVerticesSizeInBytes,
						w_memory_usage_flag::MEMORY_USAGE_CPU_ONLY,
						this->_stagings_buffers.vertices) == W_FAILED)
					{
						return W_FAILED;
					}

					if (!_there_is_no_index_buffer)
					{
						if (_create_buffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
							pIndicesData,
							_indices_size,
							w_memory_usage_flag::MEMORY_USAGE_CPU_ONLY,
							this->_stagings_buffers.indices) == W_FAILED)
						{
							return W_FAILED;
						}
					}

					// create VRAM buffers
					if (_create_buffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
						nullptr,
						pVerticesSizeInBytes,
						w_memory_usage_flag::MEMORY_USAGE_GPU_ONLY,
						this->_vertex_buffer) == W_FAILED)
					{
						return W_FAILED;
					}

					if (!_there_is_no_index_buffer)
					{
						if (_create_buffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
							nullptr,
							_indices_size,
							w_memory_usage_flag::MEMORY_USAGE_GPU_ONLY,
							this->_index_buffer) == W_FAILED)
						{
							return W_FAILED;
						}
					}

					if (this->_stagings_buffers.vertices.copy_to(this->_vertex_buffer) == W_FAILED)
					{
						return W_FAILED;
					}
					if (!_there_is_no_index_buffer)
					{
						if (this->_stagings_buffers.indices.copy_to(this->_index_buffer) == W_FAILED)
						{
							return W_FAILED;
						}
					}

					//if (_copy_DRAM_to_VRAM(pVerticesSizeInBytes, _indices_size) == W_FAILED)
					//{
					//    return W_FAILED;
					//}

					if (!pUseDynamicBuffer)
					{
						//release staging buffers
						this->_stagings_buffers.vertices.release();
						if (!_there_is_no_index_buffer)
						{
							this->_stagings_buffers.indices.release();
						}
					}

					if (!this->_texture)
					{
						this->_texture = w_texture::default_texture;
					}

					return W_PASSED;
				}

				W_RESULT update_dynamic_buffer(
					_In_ const std::shared_ptr<w_graphics_device>& pGDevice,
					_In_ const void* const pVerticesData,
					_In_ const uint32_t pVerticesSize,
					_In_ const uint32_t pVerticesCount,
					_In_ const uint32_t* const pIndicesData,
					_In_ const uint32_t pIndicesCount)
				{
					const std::string _trace_info = this->_name + "::update_dynamic_buffer";

					if (!this->_dynamic_buffer)
					{
						V(W_FAILED,
							w_log_type::W_WARNING,
							"could not update none dynamic buffer for graphics device: {}. trace info: {}",
							pGDevice->get_info(),
							_trace_info);
						return W_FAILED;
					}
					if (pVerticesCount != this->_vertices_count ||
						pIndicesCount != this->_indices_count)
					{
						V(W_FAILED,
							w_log_type::W_WARNING,
							"size of vertex or index buffer does not match for graphics device: {}. trace info: {}",
							pGDevice->get_info(),
							_trace_info);
						return W_FAILED;
					}

					auto _hr = this->_stagings_buffers.vertices.set_data(pVerticesData);
					V(_hr,
						w_log_type::W_ERROR,
						"updating staging vertex buffer for graphics device: {}. trace info: {}",
						pGDevice->get_info(),
						_trace_info);

					//_hr = this->_stagings_buffers.vertices.bind();
					//V(_hr, "binding to staging vertex buffer", this->_name, 3);

					if (pIndicesCount && pIndicesData)
					{
						_hr = this->_stagings_buffers.indices.set_data(pIndicesData);
						V(_hr,
							w_log_type::W_ERROR,
							"updating staging index buffer for graphics device: {}. trace info: {}",
							pGDevice->get_info(),
							_trace_info);

						//_hr = this->_stagings_buffers.vertices.bind();
						//V(_hr, "binding staging index buffer", this->_name, 3);
					}

					if (_stagings_buffers.vertices.copy_to(this->_vertex_buffer) == W_FAILED)
					{
						V(_hr,
							w_log_type::W_ERROR,
							"copying staging vertex buffer to gpu vertex buffer for graphics device: {}. trace info: {}",
							pGDevice->get_info(),
							_trace_info);
						return W_FAILED;
					}

					auto _index_count = this->_indices_count * sizeof(uint32_t);
					if (_index_count)
					{
						if (_stagings_buffers.indices.copy_to(this->_index_buffer) == W_FAILED)
						{
							V(_hr,
								w_log_type::W_ERROR,
								"copying staging index buffer to gpu index buffer for graphics device: {}. trace info: {}",
								pGDevice->get_info(),
								_trace_info);
							return W_FAILED;
						}
					}

					return W_PASSED;
				}

				W_RESULT draw(
					_In_ const w_command_buffer& pCommandBuffer,
					_In_ const w_buffer_handle* pInstanceHandle,
					_In_ const uint32_t pInstancesCount,
					_In_ const uint32_t pFirstInstance,
					_In_ const w_indirect_draws_command_buffer* pIndirectDrawCommands,
					_In_ const uint32_t pVertexOffset,
					_In_ const int pIndexCount,
					_In_ const uint32_t pFirstIndex,
					_In_ const int pVertexCount,
					_In_ const uint32_t pFirstVertex)
				{
					if (!pCommandBuffer.handle) return W_FAILED;

					VkDeviceSize _offsets[1] = { 0 };

					auto _vertex_buffer_handle = this->_vertex_buffer.get_buffer_handle().handle;
					if (!_vertex_buffer_handle) return W_FAILED;

					auto _cmd = pCommandBuffer.handle;
					vkCmdBindVertexBuffers(_cmd, 0, 1, &_vertex_buffer_handle, _offsets);

					if (pInstanceHandle && pInstanceHandle->handle)
					{
						vkCmdBindVertexBuffers(_cmd, 1, 1, &pInstanceHandle->handle, _offsets);
					}

					bool _draw_indexed = false;
					auto _index_buffer_handle = this->_index_buffer.get_buffer_handle().handle;
					if (_index_buffer_handle)
					{
						_draw_indexed = true;
						vkCmdBindIndexBuffer(_cmd, _index_buffer_handle, 0, VK_INDEX_TYPE_UINT32);
					}

					if (pIndirectDrawCommands)
					{
						auto _size = static_cast<uint32_t>(sizeof(w_draw_indexed_indirect_command));
						auto _draw_counts = static_cast<uint32_t>(pIndirectDrawCommands->drawing_commands.size());
						auto _buffer_handle = pIndirectDrawCommands->buffer.get_buffer_handle().handle;
						if (this->_gDevice->vk_physical_device_features.multiDrawIndirect)
						{
							vkCmdDrawIndexedIndirect(
								pCommandBuffer.handle,
								_buffer_handle,
								0,
								_draw_counts,
								_size);
						}
						else
						{
							// If multi draw is not available, we must issue separate draw commands
							for (auto i = 0; i < _draw_counts; ++i)
							{
								vkCmdDrawIndexedIndirect(
									pCommandBuffer.handle,
									_buffer_handle,
									i * _size,
									1,
									_size);
							}
						}
					}
					else
					{
						if (_draw_indexed)
						{
							vkCmdDrawIndexed(
								_cmd,
								pIndexCount == -1 ? this->_indices_count : static_cast<uint32_t>(pIndexCount),
								pInstancesCount + 1,
								pFirstIndex,
								pVertexOffset,
								pFirstInstance);
						}
						else
						{
							vkCmdDraw(
								_cmd,
								pVertexCount == -1 ? this->_vertices_count : static_cast<uint32_t>(pVertexCount),
								pInstancesCount + 1,
								pFirstVertex,
								pFirstInstance);
						}
					}

					_cmd = nullptr;
					_vertex_buffer_handle = nullptr;
					_index_buffer_handle = nullptr;

					return W_PASSED;
				}

#pragma region Getters

				w_buffer_handle get_vertex_buffer_handle() const
				{
					return this->_vertex_buffer.get_buffer_handle();
				}

				w_buffer_handle get_index_buffer_handle() const
				{
					return this->_index_buffer.get_buffer_handle();
				}

				const uint32_t get_vertices_count() const
				{
					return this->_vertices_count;
				}

				const uint32_t get_indices_count() const
				{
					return this->_indices_count;
				}

				w_texture* get_texture() const
				{
					return this->_texture;
				}

				const w_vertex_binding_attributes get_vertex_binding_attributes() const
				{
					return this->_vertex_binding_attributes;
				}

#pragma endregion

#pragma region Setters

				void set_texture(_In_ w_texture* pTexture)
				{
					this->_texture = pTexture;
				}

				void set_vertex_binding_attributes(_In_ const w_vertex_binding_attributes& pVertexBindingAttributes)
				{
					this->_vertex_binding_attributes = pVertexBindingAttributes;
				}

#pragma endregion

				void release()
				{
					//release vertex and index buffers

					this->_vertex_buffer.release();
					if (this->_indices_count)
					{
						this->_index_buffer.release();
					}
					if (this->_dynamic_buffer)
					{
						this->_stagings_buffers.vertices.release();
						if (this->_indices_count)
						{
							this->_stagings_buffers.indices.release();
						}
					}

					this->_texture = nullptr;
					this->_gDevice = nullptr;
				}

			private:
				//Create buffers for rendering
				W_RESULT _create_buffer(_In_ const VkBufferUsageFlags pBufferUsageFlag,
					_In_ const void* const pBufferData,
					_In_ uint32_t pBufferSizeInBytes,
					_In_ const w_memory_usage_flag pMemoryFlag,
					_Inout_ w_buffer& pBuffer)
				{
					const std::string _trace_info = this->_name + "::_create_buffer";

					if (pBuffer.allocate(this->_gDevice, pBufferSizeInBytes, pBufferUsageFlag, pMemoryFlag))
					{
						V(W_FAILED,
							w_log_type::W_ERROR,
							"loading memory of buffer for graphics device: {}. trace info: {}",
							_gDevice->get_info(),
							_trace_info);

						return W_FAILED;
					}


					W_RESULT _hr;
					//we can not access to VRAM, but we can copy our data to DRAM
					if (pMemoryFlag != w_memory_usage_flag::MEMORY_USAGE_GPU_ONLY)
					{
						_hr = pBuffer.set_data(pBufferData);
						if (_hr == W_FAILED)
						{
							V(W_FAILED,
								w_log_type::W_ERROR,
								"setting data to vertex buffer's memory staging for graphics device: {}. trace info: {}",
								_gDevice->get_info(),
								_trace_info);

							return W_FAILED;
						}
					}

					return W_PASSED;
				}

				std::string                                         _name;
				std::shared_ptr<w_graphics_device>                  _gDevice;
				w_buffer                                            _vertex_buffer;
				w_buffer                                            _index_buffer;
				uint32_t                                            _indices_count;
				uint32_t                                            _vertices_count;
				w_texture*                                          _texture;
				w_vertex_binding_attributes                         _vertex_binding_attributes;
				bool                                                _dynamic_buffer;
				struct
				{
					w_buffer vertices;
					w_buffer indices;
				} _stagings_buffers;
			};
		}
	}
}

using namespace wolf::render::vulkan;

w_mesh::w_mesh() : 
	_is_released(false),
	_pimp(new w_mesh_pimp())
{
}

w_mesh::~w_mesh()
{
	release();
}

W_RESULT w_mesh::load(
	_In_ const std::shared_ptr<w_graphics_device>& pGDevice,
    _In_ const void* const pVerticesData,
    _In_  const uint32_t  pVerticesSize,
    _In_ const uint32_t pVerticesCount,
    _In_ const uint32_t* const pIndicesData,
    _In_ const uint32_t pIndicesCount,
    _In_ const bool pUseDynamicBuffer)
{
    if (!this->_pimp) return W_FAILED;
    
    return this->_pimp->load(
        pGDevice,
        pVerticesData,
        pVerticesSize,
        pVerticesCount,
        pIndicesData,
        pIndicesCount,
        pUseDynamicBuffer);
}

W_RESULT w_mesh::update_dynamic_buffer(
    _In_ const std::shared_ptr<w_graphics_device>& pGDevice,
    _In_ const void* const pVerticesData,
    _In_ const uint32_t pVerticesSize,
    _In_ const uint32_t pVerticesCount,
    _In_ const uint32_t* const pIndicesData,
    _In_ const uint32_t pIndicesCount)
{
    return this->_pimp ? this->_pimp->update_dynamic_buffer(
        pGDevice,
        pVerticesData,
        pVerticesSize,
        pVerticesCount,
        pIndicesData,
        pIndicesCount) : W_FAILED;
}

W_RESULT w_mesh::draw(
	_In_ const w_command_buffer& pCommandBuffer,
	_In_ const w_buffer_handle* pInstanceHandle,
	_In_ const uint32_t pInstancesCount,
	_In_ const uint32_t pFirstInstance,
	_In_ const w_indirect_draws_command_buffer* pIndirectDrawCommands,
	_In_ const uint32_t pVertexOffset,
	_In_ const int pIndexCount,
	_In_ const uint32_t pFirstIndex,
	_In_ const int pVertexCount,
	_In_ const uint32_t pFirstVertex)
{
	if (!this->_pimp) return W_FAILED;
	return this->_pimp->draw(
		pCommandBuffer,
		pInstanceHandle,
		pInstancesCount,
		pFirstInstance,
		pIndirectDrawCommands,
		pVertexOffset,
		pIndexCount,
		pFirstIndex,
		pVertexCount,
		pFirstVertex);
}

ULONG w_mesh::release()
{
    if (this->_is_released) return 1;
	
    SAFE_RELEASE(this->_pimp);
	this->_is_released = true;

	return 0;
}

#pragma region Getters

w_buffer_handle w_mesh::get_vertex_buffer_handle() const
{
    return this->_pimp ? this->_pimp->get_vertex_buffer_handle() : w_buffer_handle();
}

w_buffer_handle w_mesh::get_index_buffer_handle() const
{
    return this->_pimp ? this->_pimp->get_index_buffer_handle() : w_buffer_handle();
}

const uint32_t w_mesh::get_vertices_count() const
{
    return this->_pimp ? this->_pimp->get_vertices_count() : 0;
}

const uint32_t w_mesh::get_indices_count() const
{
    return this->_pimp ? this->_pimp->get_indices_count() : 0;
}

w_texture* w_mesh::get_texture() const
{
    return this->_pimp ? this->_pimp->get_texture() : nullptr;
}

const w_vertex_binding_attributes w_mesh::get_vertex_binding_attributes() const
{
    if (!this->_pimp) return w_vertex_binding_attributes(w_vertex_declaration::NOT_DEFINED);
    return this->_pimp->get_vertex_binding_attributes();
}

#pragma endregion

#pragma region Setters

void w_mesh::set_texture(_In_ w_texture* pTexture)
{
    if (!this->_pimp) return;
    this->_pimp->set_texture(pTexture);
}

void w_mesh::set_vertex_binding_attributes(_In_ const w_vertex_binding_attributes& pVertexBindingAttributes)
{
    if (!this->_pimp) return;
    this->_pimp->set_vertex_binding_attributes(pVertexBindingAttributes);
}

#pragma endregion

