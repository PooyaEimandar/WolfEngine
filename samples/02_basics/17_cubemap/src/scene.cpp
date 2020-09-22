#include "pch.h"
#include "scene.h"
#include <glm_extention.h>

using namespace std;
using namespace wolf;
using namespace wolf::system;
using namespace wolf::graphics;

static uint32_t sFPS = 0;
static float sElapsedTimeInSec = 0;
static float sTotalTimeTimeInSec = 0;

scene::scene(_In_z_ const std::wstring& pRunningDirectory, _In_z_ const std::wstring& pAppName) :
    w_game(pRunningDirectory, pAppName)
{
    auto _running_dir = pRunningDirectory;

#if defined(__WIN32) || defined(__UWP)
    content_path = _running_dir + L"../../../../content/";
#elif defined(__APPLE__)
    content_path = _running_dir + L"/../../../../../content/";
#elif defined(__linux)
    error
#elif defined(__ANDROID)
    error
#endif

#ifdef __WIN32
    w_graphics_device_manager_configs _config;
    _config.debug_gpu = false;
    w_game::set_graphics_device_manager_configs(_config);
#endif

    w_game::set_fixed_time_step(false);
}

scene::~scene()
{
	//release all resources
	release();
}

void scene::initialize(_In_ std::map<int, std::vector<w_window_info>> pOutputWindowsInfo)
{
	// TODO: Add your pre-initialization logic here

	w_game::initialize(pOutputWindowsInfo);
}

void scene::load()
{
    defer(nullptr, [&](...)
    {
        w_game::load();
    });

    const std::string _trace_info = this->name + "::load";

    auto _gDevice = this->graphics_devices[0];
    auto _output_window = &(_gDevice->output_presentation_windows[0]);

    w_point_t _screen_size;
    _screen_size.x = _output_window->width;
    _screen_size.y = _output_window->height;

    //initialize viewport
    this->_viewport.y = 0;
    this->_viewport.width = static_cast<float>(_screen_size.x);
    this->_viewport.height = static_cast<float>(_screen_size.y);
    this->_viewport.minDepth = 0;
    this->_viewport.maxDepth = 1;

    //initialize scissor of viewport
    this->_viewport_scissor.offset.x = 0;
    this->_viewport_scissor.offset.y = 0;
    this->_viewport_scissor.extent.width = _screen_size.x;
    this->_viewport_scissor.extent.height = _screen_size.y;

    //initialize attachment buffers
    w_attachment_buffer_desc _color(w_texture_buffer_type::W_TEXTURE_COLOR_BUFFER);
    w_attachment_buffer_desc _depth(w_texture_buffer_type::W_TEXTURE_DEPTH_BUFFER);

    //define color and depth buffers for render pass
    std::vector<w_attachment_buffer_desc> _attachment_descriptions = { _color, _depth };

    //create render pass
    auto _hr = this->_draw_render_pass.load(_gDevice,
        _viewport,
        _viewport_scissor,
        _attachment_descriptions);
    if (_hr == S_FALSE)
    {
        release();
        V(S_FALSE, "creating render pass", _trace_info, 3, true);
    }
    
    //create frame buffers
    auto _draw_render_pass_handle = this->_draw_render_pass.get_handle();
    _hr = this->_draw_frame_buffers.load(_gDevice,
        _draw_render_pass_handle,
        _output_window);
    if (_hr == S_FALSE)
    {
        release();
        V(S_FALSE, "creating frame buffers", _trace_info, 3, true);
    }

    //create semaphore
    _hr = this->_draw_semaphore.initialize(_gDevice);
    if (_hr == S_FALSE)
    {
        release();
        V(S_FALSE, "creating draw semaphore", _trace_info, 3, true);
    }

    //Fence for syncing
    _hr = this->_draw_fence.initialize(_gDevice);
    if (_hr == S_FALSE)
    {
        release();
        V(S_FALSE, "creating draw fence", _trace_info, 3, true);
    }

	//load imgui
	w_imgui::load(
		_gDevice,
		_output_window,
		this->_viewport,
		this->_viewport_scissor,
		nullptr);

    //create two primary command buffers for clearing screen
    auto _swap_chain_image_size = _output_window->vk_swap_chain_image_views.size();
    _hr = this->_draw_command_buffers.load(_gDevice, _swap_chain_image_size);
    if (_hr == S_FALSE)
    {
        release();
        V(S_FALSE, "creating draw command buffers", _trace_info, 3, true);
    }
	
	//++++++++++++++++++++++++++++++++++++++++++++++++++++
	//The following codes have been added for this project
	//++++++++++++++++++++++++++++++++++++++++++++++++++++

	//Add Line
	this->_shape_line = new (std::nothrow) w_shapes(
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(3.0f, 3.0f, 3.0f),
		w_color::RED());
	if (!this->_shape_line)
	{
		release();
		V(S_FALSE, "allocating memory for shape(line)", _trace_info, 3, true);
	}
	_hr = this->_shape_line->load(_gDevice, this->_draw_render_pass, this->_viewport, this->_viewport_scissor);
	if (_hr == S_FALSE)
	{
		release();
		V(S_FALSE, "loading shape(line)", _trace_info, 3, true);
	}

	//Add Triangle
	this->_shape_triangle = new (std::nothrow) w_shapes(
		glm::vec3(-1.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 2.0f, 0.0f),
		w_color::GREEN());
	if (!this->_shape_triangle)
	{
		release();
		V(S_FALSE, "allocating memory for shape(triangle)", _trace_info, 3, true);
	}
	_hr = this->_shape_triangle->load(_gDevice, this->_draw_render_pass, this->_viewport, this->_viewport_scissor);
	if (_hr == S_FALSE)
	{
		release();
		V(S_FALSE, "loading shape(triangle)", _trace_info, 3, true);
	}

	//Add Circle
	this->_shape_circle = new (std::nothrow) w_shapes(
		glm::vec3(0.0f, 0.0f, 0.0f),
		2.0f,
		w_color::ORANGE(),
		wolf::graphics::w_plan::XY, 
		30.0f);
	if (!this->_shape_circle)
	{
		release();
		V(S_FALSE, "allocating memory for shape(circle)", _trace_info, 3, true);
	}
	_hr = this->_shape_circle->load(_gDevice, this->_draw_render_pass, this->_viewport, this->_viewport_scissor);
	if (_hr == S_FALSE)
	{
		release();
		V(S_FALSE, "loading shape(circle)", _trace_info, 3, true);
	}

	//Add Bounding Box
	w_bounding_box _bounding_box;

	_bounding_box.min[0] = -3.0f;
	_bounding_box.min[1] = -3.0f;
	_bounding_box.min[2] = -3.0f;

	_bounding_box.max[0] = 3.0f;
	_bounding_box.max[1] = 3.0f;
	_bounding_box.max[2] = 3.0f;

	this->_shape_box = new (std::nothrow) w_shapes(_bounding_box, w_color::YELLOW());
	if (!this->_shape_box)
	{
		release();
		V(S_FALSE, "allocating memory for shape(box)", _trace_info, 3, true);
	}
	_hr = this->_shape_box->load(_gDevice, this->_draw_render_pass, this->_viewport, this->_viewport_scissor);
	if (_hr == S_FALSE)
	{
		release();
		V(S_FALSE, "loading shape(box)", _trace_info, 3, true);
	}

	//Add Bounding Sphere
	w_bounding_sphere _bounding_sphere;

	_bounding_sphere.center[0] = 0.0f;
	_bounding_sphere.center[1] = 0.0f;
	_bounding_sphere.center[2] = 0.0f;

	_bounding_sphere.radius = 3.0f;

	const uint32_t _sphere_resolution = 30;
	this->_shape_sphere = new (std::nothrow) w_shapes(_bounding_sphere, w_color::PURPLE(), _sphere_resolution);
	if (!this->_shape_sphere)
	{
		release();
		V(S_FALSE, "allocating memory for shape(sphere)", _trace_info, 3, true);
	}
	_hr = this->_shape_sphere->load(_gDevice, this->_draw_render_pass, this->_viewport, this->_viewport_scissor);
	if (_hr == S_FALSE)
	{
		release();
		V(S_FALSE, "loading shape(sphere)", _trace_info, 3, true);
	}
	//++++++++++++++++++++++++++++++++++++++++++++++++++++
	//++++++++++++++++++++++++++++++++++++++++++++++++++++

	_build_draw_command_buffers();
}

HRESULT scene::_build_draw_command_buffers()
{
    const std::string _trace_info = this->name + "::build_draw_command_buffers";
    HRESULT _hr = S_OK;

    auto _size = this->_draw_command_buffers.get_commands_size();
    for (uint32_t i = 0; i < _size; ++i)
    {
        this->_draw_command_buffers.begin(i);
        {
            auto _frame_buffer_handle = this->_draw_frame_buffers.get_frame_buffer_at(i);

            auto _cmd = this->_draw_command_buffers.get_command_at(i);
            this->_draw_render_pass.begin(_cmd,
                _frame_buffer_handle,
                w_color::CORNFLOWER_BLUE(),
                1.0f,
                0.0f);
            {
				//++++++++++++++++++++++++++++++++++++++++++++++++++++
				//The following codes have been added for this project
				//++++++++++++++++++++++++++++++++++++++++++++++++++++
				this->_shape_line->draw(_cmd);
				this->_shape_triangle->draw(_cmd);
				this->_shape_circle->draw(_cmd);
                this->_shape_box->draw(_cmd);
				this->_shape_sphere->draw(_cmd);
				//++++++++++++++++++++++++++++++++++++++++++++++++++++
				//++++++++++++++++++++++++++++++++++++++++++++++++++++
            }
            this->_draw_render_pass.end(_cmd);
        }
        this->_draw_command_buffers.end(i);
    }
    return _hr;
}

void scene::update(_In_ const wolf::system::w_game_time& pGameTime)
{
	if (w_game::exiting) return;
	const std::string _trace_info = this->name + "::update";

    sFPS = pGameTime.get_frames_per_second();
    sElapsedTimeInSec = pGameTime.get_elapsed_seconds();
    sTotalTimeTimeInSec = pGameTime.get_total_seconds();

    w_imgui::new_frame(sElapsedTimeInSec, [this]()
    {
        _update_gui();
    });
    
	//++++++++++++++++++++++++++++++++++++++++++++++++++++
	//The following codes have been added for this project
	//++++++++++++++++++++++++++++++++++++++++++++++++++++

	auto _angle = (float)pGameTime.get_total_seconds();
	auto _eye = glm::vec3((float)std::cos(_angle * .5f), 0.5f, (float)std::sin(_angle * 0.5f)) * 12.0f;
	auto _up = glm::vec3(0, -1, 0);
	auto _look_at = glm::vec3(0, 0, 0);

	auto _world = glm::mat4(1);//identity
	auto _view = glm::lookAtRH(_eye, _look_at, _up);
	auto _projection = glm::perspectiveRH(
		45.0f * glm::pi<float>() / 180.0f,
		this->_viewport.width / this->_viewport.height, 
		0.1f, 
		1000.0f);

	auto _wvp = _projection * _view * _world;

	this->_shape_line->update(_wvp);
	this->_shape_triangle->update(_wvp);
	this->_shape_circle->update(_wvp);
	this->_shape_box->update(_wvp);
	this->_shape_sphere->update(_wvp);
	//++++++++++++++++++++++++++++++++++++++++++++++++++++
	//++++++++++++++++++++++++++++++++++++++++++++++++++++

	w_game::update(pGameTime);
}

HRESULT scene::render(_In_ const wolf::system::w_game_time& pGameTime)
{
	if (w_game::exiting) return S_OK;

	const std::string _trace_info = this->name + "::render";

	auto _gDevice = this->graphics_devices[0];
	auto _output_window = &(_gDevice->output_presentation_windows[0]);
	auto _frame_index = _output_window->vk_swap_chain_image_index;

	w_imgui::render();
    
	//add wait semaphores
	std::vector<VkSemaphore> _wait_semaphors = { *(_output_window->vk_swap_chain_image_is_available_semaphore.get()) };
	auto _draw_command_buffer = this->_draw_command_buffers.get_command_at(_frame_index);
    auto _gui_command_buffer = w_imgui::get_command_buffer_at(_frame_index);

	const VkPipelineStageFlags _wait_dst_stage_mask[] =
	{
		VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
	};

	//reset draw fence
	this->_draw_fence.reset();
	if (_gDevice->submit(
		{ _draw_command_buffer, _gui_command_buffer },
		_gDevice->vk_graphics_queue,
		&_wait_dst_stage_mask[0],
		_wait_semaphors,
		{ *_output_window->vk_rendering_done_semaphore.get() },
		&this->_draw_fence) == S_FALSE)
	{
		V(S_FALSE, "submiting queue for drawing", _trace_info, 3, true);
	}
	this->_draw_fence.wait();

	//clear all wait semaphores
	_wait_semaphors.clear();

	return w_game::render(pGameTime);
}

void scene::on_window_resized(_In_ uint32_t pIndex)
{
	w_game::on_window_resized(pIndex);
}

void scene::on_device_lost()
{
	w_game::on_device_lost();
}

ULONG scene::release()
{
    if (this->get_is_released()) return 1;

    //release draw's objects
	this->_draw_fence.release();
	this->_draw_semaphore.release();

	this->_draw_command_buffers.release();
	this->_draw_render_pass.release();
	this->_draw_frame_buffers.release();

    //release gui's objects
    w_imgui::release();

	//++++++++++++++++++++++++++++++++++++++++++++++++++++
	//The following codes have been added for this project
	//++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	SAFE_RELEASE(this->_shape_line);
	SAFE_RELEASE(this->_shape_triangle);
	SAFE_RELEASE(this->_shape_circle);
	SAFE_RELEASE(this->_shape_box);
	SAFE_RELEASE(this->_shape_sphere);

	//++++++++++++++++++++++++++++++++++++++++++++++++++++
	//++++++++++++++++++++++++++++++++++++++++++++++++++++

	return w_game::release();
}

bool scene::_update_gui()
{
    //Setting Style
    ImGuiStyle& _style = ImGui::GetStyle();
    _style.Colors[ImGuiCol_Text].x = 1.0f;
    _style.Colors[ImGuiCol_Text].y = 1.0f;
    _style.Colors[ImGuiCol_Text].z = 1.0f;
    _style.Colors[ImGuiCol_Text].w = 1.0f;

    _style.Colors[ImGuiCol_WindowBg].x = 0.0f;
    _style.Colors[ImGuiCol_WindowBg].y = 0.4f;
    _style.Colors[ImGuiCol_WindowBg].z = 1.0f;
    _style.Colors[ImGuiCol_WindowBg].w = 0.9f;

    ImGuiWindowFlags  _window_flags = 0;;
    ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiSetCond_FirstUseEver);
    bool _is_open = true;
    if (!ImGui::Begin("Wolf.Engine", &_is_open, _window_flags))
    {
        ImGui::End();
        return false;
    }

    ImGui::Text("Press Esc to exit\r\nFPS:%d\r\nFrameTime:%f\r\nTotalTime:%f\r\nMouse Position:%d,%d\r\n",
        sFPS,
        sElapsedTimeInSec,
        sTotalTimeTimeInSec,
        wolf::inputs_manager.mouse.pos_x, wolf::inputs_manager.mouse.pos_y);

    ImGui::End();

    return true;
}
