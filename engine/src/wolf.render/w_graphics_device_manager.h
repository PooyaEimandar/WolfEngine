/*
	Project			 : Wolf Engine. Copyright(c) Pooya Eimandar (https://PooyaEimandar.github.io) . All rights reserved.
	Source			 : Please direct any bug to https://github.com/WolfEngine/Wolf.Engine/issues
	Website			 : https://WolfEngine.App
	Name			 : w_graphics_device_manager.h
	Description		 : The manager for graphics devices.
	Comment          :
*/

#pragma once

#include "w_graphics_headers.h"
#include "w_render_export.h"
#include <w_window.h>
#include <w_color.h>
#include <w_signal.h>
#include <w_point.h>
#include <map>
#include <mutex>
#include <array>
#include "vulkan/w_queue.h"
#include "vulkan/w_semaphore.h"
#include "vulkan/w_fences.h"
#include "vulkan/w_memory_allocator.h"
#include "vulkan/w_command_buffers.h"

#ifdef __PYTHON__
#include <boost/make_shared.hpp>
#endif

namespace wolf::render::vulkan
{
	//forward declaration
	struct w_graphics_device_manager_configs;
	//struct w_buffer;
	//class  w_pipeline;

	//Output window which handles all 3d resources for output renderer
	struct w_output_presentation_window
	{
	public:
		//Release resources
		ULONG release()
		{
			if (this->_is_released) return 0;
			this->_is_released = true;

			//release semaphores
			this->rendering_done_semaphore.release();
			this->swap_chain_image_is_available_semaphore.release();

#ifdef __WIN32
			this->hwnd = NULL;
			this->hInstance = NULL;
#elif defined(__ANDROID) || defined(__APPLE__)
			this->window = nullptr;
#elif defined(__linux)
			this->xcb_connection = nullptr;
			this->xcb_window = nullptr;
#endif 

			return 1;
		}

		bool											is_full_screen = false;
		float											aspect_ratio = 0;

#if defined(__WIN32) || defined(__linux) || defined(__APPLE__) || defined(__ANDROID)
		uint32_t										width = 0;
		uint32_t										height = 0;

#ifdef  __WIN32
		DWORD											pdwCookie;
		HWND											hwnd = NULL;
		HINSTANCE										hInstance = NULL;
#elif defined(__ANDROID)
		ANativeWindow* window = nullptr;
#elif defined(__linux)
		xcb_connection_t* xcb_connection = nullptr;
		xcb_window_t* xcb_window = nullptr;
#elif defined(__APPLE__)
		void* window = nullptr;
#endif

#elif defined(__UWP)
		Windows::Graphics::Display::DisplayOrientations	window_native_orientation;
		Windows::Graphics::Display::DisplayOrientations	window_current_orientation;
		float											window_dpi;
		bool											support_high_resolutions = true;
		IUnknown* window;
		Windows::Foundation::Rect						window_size;
		DirectX::XMFLOAT4X4								orientation_transform_3D;
#endif

		bool									        v_sync = true;
		bool                                            cpu_access_to_swapchain_buffer = false;
		bool											double_buffering = true;

#ifdef __DX12__		
		DXGI_FORMAT								        dx_swap_chain_selected_format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
		ComPtr<IDXGISwapChain3>					        dx_swap_chain;

		std::vector<ID3D12Resource*>			        dx_swap_chain_image_views;
		uint32_t								        dx_swap_chain_image_index = 0;

		ComPtr<ID3D12DescriptorHeap>			        dx_render_target_view_heap;
		UINT									        dx_render_target_descriptor_size = 0;

		ComPtr<ID3D12CommandAllocator>			        dx_command_allocator_pool;
		ComPtr<ID3D12CommandQueue>				        dx_command_queue;
		ComPtr<ID3D12GraphicsCommandList>		        dx_command_list;
		ComPtr<ID3D12PipelineState>				        dx_pipeline_state;

		//Synchronization objects.
		HANDLE									        dx_fence_event = 0;
		ComPtr<ID3D12Fence>						        dx_fence;
		UINT64									        dx_fence_value = 0;

#elif defined(__VULKAN__)
		VkSurfaceKHR                                    vk_presentation_surface = 0;
		VkSurfaceFormatKHR                              vk_swap_chain_selected_format;
		VkSwapchainKHR                                  vk_swap_chain = 0;
		std::vector<w_image_view>				        swap_chain_image_views;
		uint32_t								        swap_chain_image_index = 0;

		std::vector<VkSurfaceFormatKHR>			        vk_surface_formats;

		w_format								        depth_buffer_format = w_format::UNDEFINED;
		w_image_view							        depth_buffer_image_view;
		VkDeviceMemory							        depth_buffer_memory = 0;

		//Synchronization objects
		w_semaphore								        swap_chain_image_is_available_semaphore;
		w_semaphore								        rendering_done_semaphore;

		//Required objects for sharing swap chain's buffer with CPU
		struct shared_objs_between_cpu_gpu
		{
			VkImage                                     destination_image = 0;
			VkDeviceMemory                              destination_image_memory = 0;
			VkCommandBuffer                             copy_command_buffer = 0;
			VkFence                                     copy_fence = 0;
			bool                                        command_buffer_began = false;
		}; shared_objs_between_cpu_gpu* objs_between_cpu_gpu = nullptr;
		bool                                            bliting_supported_by_swap_chain = true;
#endif


#ifdef __PYTHON__
		boost::python::list py_get_swap_chain_image_views()
		{
			return boost_wrap_array(swap_chain_image_views.data(), swap_chain_image_views.size());
		}
#endif

	private:
		bool									        _is_released = false;
	};

	struct w_device_info
	{
	public:
		w_device_info()
		{
		}

		const uint32_t              get_device_id() const { return this->device_properties->deviceID; }
		const std::string           get_device_name() const { return this->device_properties->deviceName; }
		const uint32_t				get_device_vendor_id() const { return this->device_properties->vendorID; }

#ifdef __VULKAN__
		VkPhysicalDeviceProperties* device_properties = nullptr;
		VkPhysicalDeviceFeatures* device_features = nullptr;
		std::vector<const char*>    device_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

#endif

		void release()
		{
			this->device_features = nullptr;
			SAFE_DELETE(this->device_properties);
			device_extensions.clear();
		}
	};

	//contains graphics device which performs primitive-based rendering
	class w_graphics_device
	{
		friend class w_graphics_device_manager;
	public:
		W_VK_EXP w_graphics_device();

		//get graphics device information
		W_VK_EXP const std::string get_info();
		//get number of swap chains
		W_VK_EXP const size_t get_number_of_swap_chains();

		//release all resources
		W_VK_EXP ULONG release();

		w_output_presentation_window               output_presentation_window;

		//draw primitive(s) and instances using vertex & index buffer
		W_VK_EXP W_RESULT draw(
			_In_ const w_command_buffer& pCommandBuffer,
			_In_ const uint32_t& pVertexCount,
			_In_ const uint32_t& pInstanceCount,
			_In_ const uint32_t& pFirstVertex,
			_In_ const uint32_t& pFirstInstance);

		//submit command buffer
		W_VK_EXP W_RESULT submit(
			_In_ const std::vector<const w_command_buffer*>& pCommandBuffers,
			_In_ const w_queue& pQueue,
			_In_ const std::vector<w_pipeline_stage_flag_bits>& pWaitDstStageMask,
			_In_ std::vector<w_semaphore> 						pWaitForSemaphores,
			_In_ std::vector<w_semaphore> 						pSignalForSemaphores,
			_In_ w_fences* pFence,
			_In_ const bool& pWaitIdleForDone);

		/*
			capture image buffer's data and save to D-RAM and make it accessable by CPU,
			for capturing last presented swap chain image's buffer, use "w_graphics_device::capture_presented_swap_chain_buffer" function
			The source buffer must be created with  VK_IMAGE_USAGE_TRANSFER_SRC_BIT flag
			@param pSourceImage, inputed source image
			@param pSourceImageLayout, inputed source image layout
			@param pWidth, width of inputed source image
			@param pHeight, height of inputed source image
			@param pOnPixelsDataCaptured, raised when pixels just mapped to RAM and become accessable by CPU. (inputs are: const w_point_t Width_Height, const uint8_t* Pixels and outsput is void)
			@return W_PASSED means function did succesfully and W_FAILED means function failed
		*/
		W_VK_EXP W_RESULT capture(
			_In_ VkImage pSourceImage,
			_In_ VkFormat pSourceImageFormat,
			_In_ VkImageLayout pSourceImageLayout,
			_In_ const uint32_t& pWidth,
			_In_ const uint32_t& pHeight,
			_In_ system::w_signal<void(const w_point_t, uint8_t*)>& pOnPixelsDataCaptured);

		/*
			capture last presented swap chain image buffer's data and save to the D-RAM and make it accessable by CPU,
			make sure set true to w_present_info::cpu_access_swap_chain_buffer flag before creating graphics device
			@return W_PASSED means function did succesfully and W_FAILED means function failed
		*/
		W_VK_EXP W_RESULT capture_presented_swap_chain_buffer(_In_ wolf::system::w_signal<void(const w_point_t, uint8_t*)>& pOnPixelsDataCaptured);

		w_device_info* device_info = nullptr;

#ifdef __DX12__

		static ComPtr<IDXGIFactory4>							dx_dxgi_factory;

		ComPtr<IDXGIOutput>										dx_dxgi_outputs;

		bool													dx_device_removed;
		bool													dx_is_wrap_device;
		D3D_FEATURE_LEVEL										dx_feature_level;
		ComPtr<IDXGIAdapter1>									dx_adaptor;
		ComPtr<ID3D12Device>									dx_device;

#elif defined(__VULKAN__)
		static VkInstance                                               vk_instance;

		VkPhysicalDevice                                                vk_physical_device;
		VkPhysicalDeviceFeatures                                        vk_physical_device_features;
		VkPhysicalDeviceMemoryProperties                                vk_physical_device_memory_properties;

		std::vector<VkQueueFamilyProperties>                            vk_queue_family_properties;
		std::vector<VkBool32>                                           vk_queue_family_supports_present;

		w_queue                                                         vk_graphics_queue;
		w_queue                                                         vk_present_queue;
		w_queue                                                         vk_compute_queue;
		w_queue                                                         vk_transfer_queue;
		w_queue                                                         vk_sparse_queue;

		VkDevice                                                        vk_device;

		VkCommandPool                                                   vk_command_allocator_pool;

		//static pipeline defaults
		struct defaults_states
		{
			W_VK_EXP static std::vector<VkSubpassDependency>                   vk_default_subpass_dependencies;

			struct pipelines
			{
				W_VK_EXP static w_pipeline_layout_create_info                  layout_create_info;
				W_VK_EXP static w_pipeline_vertex_input_state_create_info      vertex_input_create_info;
				W_VK_EXP static w_pipeline_input_assembly_state_create_info    input_assembly_create_info;
				W_VK_EXP static w_pipeline_rasterization_state_create_info     rasterization_create_info;
				W_VK_EXP static w_pipeline_multisample_state_create_info       multisample_create_info;
			};

			struct blend_states
			{
				W_VK_EXP static w_pipeline_color_blend_attachment_state        none;
				W_VK_EXP static w_pipeline_color_blend_attachment_state        premulitplied_alpha;
			};
		};



#endif //__DX12__ __VULKAN__

		w_memory_allocator												memory_allocator;

#ifdef __PYTHON__

		boost::shared_ptr<w_output_presentation_window> py_get_output_presentation_window()
		{
			return boost::make_shared<w_output_presentation_window>(this->output_presentation_window);
		}

		W_RESULT py_draw(
			_In_ const w_command_buffer& pCommandBuffer,
			_In_ const uint32_t& pVertexCount,
			_In_ const uint32_t& pInstanceCount,
			_In_ const uint32_t& pFirstVertex,
			_In_ const uint32_t& pFirstInstance)
		{
			return draw(
				pCommandBuffer,
				pVertexCount,
				pInstanceCount,
				pFirstVertex,
				pFirstInstance);
		}

		W_RESULT py_submit(
			_In_ boost::python::list	pCommandBuffers,
			_In_ const w_queue& pQueue,
			_In_ boost::python::list	pWaitDstStageMask,
			_In_ boost::python::list	pWaitForSemaphores,
			_In_ boost::python::list	pSignalForSemaphores,
			_In_ w_fences& pFence)
		{
			std::vector<const w_command_buffer*> _cmds;
			//get command buffers
			for (size_t i = 0; i < len(pCommandBuffers); ++i)
			{
				boost::python::extract<w_command_buffer> _cmd(pCommandBuffers[i]);
				if (_cmd.check())
				{
					auto _c = &(_cmd());
					_cmds.push_back(_c);
				}
			}

			std::vector<uint32_t> _pipeline_stage_flags;
			//get wait dst stage mask
			for (size_t i = 0; i < len(pWaitDstStageMask); ++i)
			{
				boost::python::extract<uint32_t> _stage(pWaitDstStageMask[i]);
				if (_stage.check())
				{
					_pipeline_stage_flags.push_back(_stage());
				}
			}

			std::vector<w_semaphore> _wait_smaphores;
			//get wait semaphores
			for (size_t i = 0; i < len(pWaitForSemaphores); ++i)
			{
				boost::python::extract<w_semaphore> _wait(pWaitForSemaphores[i]);
				if (_wait.check())
				{
					_wait_smaphores.push_back(_wait());
				}
			}

			std::vector<w_semaphore> _signal_smaphores;
			//get wait semaphores
			for (size_t i = 0; i < len(pSignalForSemaphores); ++i)
			{
				boost::python::extract<w_semaphore> _sig(pSignalForSemaphores[i]);
				if (_sig.check())
				{
					_signal_smaphores.push_back(_sig());
				}
			}

			auto _result = submit(
				_cmds,
				pQueue,
				_pipeline_stage_flags.data(),
				_wait_smaphores,
				_signal_smaphores,
				&pFence);

			_cmds.clear();
			_pipeline_stage_flags.clear();
			_wait_smaphores.clear();
			_signal_smaphores.clear();

			return _result;
		}
#endif

	private:
		//prevent copying
		w_graphics_device(w_graphics_device const&);
		w_graphics_device& operator= (w_graphics_device const&);

		void _clean_swap_chain();

		bool                                                            _is_released;
		std::string                                                     _name;
	};

	//forward declaration
	class w_graphics_device_manager_pimp;
	//handles the configuration and management of the graphics devices.
	class w_graphics_device_manager
	{
	public:
		W_VK_EXP w_graphics_device_manager();
		W_VK_EXP virtual ~w_graphics_device_manager();

		//Initialize graphics devices
		W_VK_EXP virtual void initialize(_In_ std::map<int, w_present_info> pOutputWindowsInfo) = 0;

		//Called when corresponding window resized
		W_VK_EXP virtual void on_window_resized(_In_ const uint32_t& pGraphicsDeviceIndex, _In_ const w_point& pNewSizeOfWindow);
		//Called when any graphics device lost
		W_VK_EXP virtual void on_device_lost();
		//Called when the APP suspends. It provides a hint to the driver that the APP is entering an idle state and that temporary buffers can be reclaimed for use by other apps.
		W_VK_EXP virtual void on_suspend();
		//Prepare frame on all graphics devices
		W_VK_EXP virtual W_RESULT prepare();
		//Present on all graphics devices
		W_VK_EXP virtual W_RESULT present();
		//Release all resources
		W_VK_EXP ULONG release();


		//convert DPIs to pixels
		W_VK_EXP static const float convert_dips_to_pixels(_In_ float pDIPS, _In_ float pDPI);

		system::w_signal<void(w_device_info**)> on_device_info_fetched;

#pragma region Getters
		//Get the graphics device
		W_VK_EXP std::shared_ptr<w_graphics_device> get_graphics_device(_In_ const size_t& pGraphicsDeviceIndex) const;
		//Get number of available graphics devices
		W_VK_EXP const size_t get_number_of_graphics_devices() const;

		//#ifdef __DX11__
		//            //Get DPI
		//            W_VK_EXP const DirectX::XMFLOAT2 get_dpi() const;
		//            //Get pixels to inches
		//            W_VK_EXP const DirectX::XMFLOAT2 get_pixels_to_inches(_In_ float pX, _In_ float pY) const;
		//#endif

#pragma endregion

#pragma region Setters
		W_VK_EXP void set_graphics_device_manager_configs(_In_ const w_graphics_device_manager_configs& pConfig);
#pragma endregion


#ifdef __VULKAN__
		W_VK_EXP static VkResult memory_type_from_properties(VkPhysicalDeviceMemoryProperties pMemoryProperties,
			uint32_t pTypeBits,
			uint32_t pRequirementsFlags,
			uint32_t* pTypeIndex);

		W_VK_EXP static VkFormat find_supported_format(
			_In_ const std::shared_ptr<w_graphics_device>& pGDevice,
			_In_ const std::vector<VkFormat>& pFormatCandidates,
			_In_ VkImageTiling pImageTiling,
			_In_ VkFormatFeatureFlags pFormatFeatureFlags);

		W_VK_EXP static void set_src_dst_masks_of_image_barrier(_Inout_ VkImageMemoryBarrier& pImageMemoryBarrier);
#endif

	protected:
		std::vector<std::shared_ptr<w_graphics_device>>		graphics_devices;

	private:
		//prevent copying
		w_graphics_device_manager(w_graphics_device_manager const&);
		w_graphics_device_manager& operator= (w_graphics_device_manager const&);

		void _load_shared_resources();

		void _wait_for_previous_frame(_In_ const std::shared_ptr<w_graphics_device>& pGDevice);

		bool												_is_released;
		w_graphics_device_manager_pimp* _pimp;
	};

	//the default config for creating graphics devices, you can edit the config before calling w_graphics_device_manager::initialize
	struct w_graphics_device_manager_configs
	{
#ifdef __DX12__
		bool							use_wrap_mode = false;
		D3D_FEATURE_LEVEL				wrap_mode_feature_level = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_12_1;
		std::vector<D3D_FEATURE_LEVEL>	hardware_feature_levels = { D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_12_1 };
#endif
		bool debug_gpu = false;
		//used for compute mode
		bool off_screen_mode = false;
	};
}

#include "python_exporter/py_graphics_device_manager.h"
