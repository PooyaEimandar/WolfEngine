/*
	Project			 : Wolf Engine. Copyright(c) Pooya Eimandar (http://PooyaEimandar.com) . All rights reserved.
	Source			 : Please direct any bug to https://github.com/PooyaEimandar/Wolf.Engine/issues
	Website			 : http://WolfSource.io
	Name			 : scene.h
	Description		 : The main scene of Wolf Engine
	Comment          : Read more information about this sample on http://wolfsource.io/gpunotes/
*/

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef __SCENE_H__
#define __SCENE_H__

#include <w_framework/w_game.h>
#include <w_framework/w_first_person_camera.h>
#include <w_framework/w_masked_occlusion_culling.h>
#include <vulkan/w_command_buffers.h>
#include <vulkan/w_render_pass.h>
#include <vulkan/w_semaphore.h>
#include <vulkan/w_pipeline.h>
#include <vulkan/w_shader.h>
#include <vulkan/w_imgui.h>
#include "model.h"
#include <tbb/concurrent_vector.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++
//The following codes have been added for this project
//++++++++++++++++++++++++++++++++++++++++++++++++++++
#include <w_thread_pool.h>
//++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++

class scene : public wolf::framework::w_game
{
public:
	scene(_In_z_ const std::wstring& pContentPath, _In_ const wolf::system::w_logger_config& pLogConfig);
	virtual ~scene();

	/*
		Allows the game to perform any initialization and it needs to before starting to run.
		Calling Game::Initialize() will enumerate through any components and initialize them as well.
		The parameter pOutputWindowsInfo represents the information of output window(s) of this game.
	*/
	void initialize(_In_ std::map<int, w_present_info> pOutputWindowsInfo) override;

	//The function "Load()" will be called once per game and is the place to load all of your game assets.
	void load() override;

	//This is the place where allows the game to run logic such as updating the world, checking camera, collisions, physics, input, playing audio and etc.
	void update(_In_ const wolf::system::w_game_time& pGameTime) override;

	//This is called when the game should draw itself.
	W_RESULT render(_In_ const wolf::system::w_game_time& pGameTime) override;

	//This is called when the window game should resized.
	void on_window_resized(_In_ const uint32_t& pGraphicsDeviceIndex, _In_ const w_point& pNewSizeOfWindow) override;

	//This is called when the we lost graphics device.
	void on_device_lost() override;

	//Release will be called once per game and is the place to unload assets and release all resources
	ULONG release() override;

private:

	struct widget_info
	{
		ImVec2	size;
		ImVec2	pos;
	};

	W_RESULT	_load_render_thread_pool(_In_ const size_t& pSwapChainImageSize);
	W_RESULT	_load_scene();
	W_RESULT	_build_draw_command_buffers();
	
	void		_show_floating_debug_window();
	void		_show_floating_moc_debug_window();
	widget_info	_show_left_widget_controller();
	widget_info	_show_search_widget(_In_ widget_info* pRelatedWidgetInfo);
	widget_info	_show_explorer();
	bool    	_update_gui();

	wolf::render::vulkan::w_viewport                                _viewport;
	wolf::render::vulkan::w_viewport_scissor                        _viewport_scissor;

	wolf::render::vulkan::w_command_buffers                         _draw_command_buffers;
	wolf::render::vulkan::w_render_pass                             _draw_render_pass;

	wolf::render::vulkan::w_fences                                  _draw_fence;
	wolf::render::vulkan::w_semaphore                               _draw_semaphore;

	bool															_rebuild_command_buffer;
	bool															_force_update_camera;
	wolf::framework::w_first_person_camera							_first_camera;
	std::vector<model*>												_scene_models;
	model*															_sky;

	bool															_show_all;
	bool															_show_all_instances_colors;
	bool															_show_all_wireframe;
	bool															_show_all_bounding_box;
	model*															_current_selected_model;
	bool															_show_lods;


	std::vector<wolf::render::vulkan::w_pipeline_stage_flag_bits>	_wait_dst_stage_mask;
	bool															_searching;
	bool															_show_moc_debug;
	wolf::framework::w_masked_occlusion_culling						_masked_occlusion_culling;
	wolf::render::vulkan::w_texture*								_masked_occlusion_culling_debug_frame;
	long															_visible_meshes;
	tbb::concurrent_vector<model*>									_visible_models;
	std::vector<model*>												_drawable_models;
	std::vector<model*>												_searched_models;

	wolf::render::vulkan::w_shapes*									_shape_coordinate_axis;


	//++++++++++++++++++++++++++++++++++++++++++++++++++++
	//The following codes have been added for this project
	//++++++++++++++++++++++++++++++++++++++++++++++++++++
	struct render_thread_context
	{
		wolf::system::w_thread                                      thread;
		wolf::render::vulkan::w_command_buffers                     secondary_command_buffers;
		size_t                                                      batch_size = 0;
		void release()
		{
			this->thread.release();
			this->secondary_command_buffers.release();
		}
	};
	tbb::concurrent_vector<render_thread_context*>                  _render_thread_pool;
	//++++++++++++++++++++++++++++++++++++++++++++++++++++
	//++++++++++++++++++++++++++++++++++++++++++++++++++++
};

#endif
