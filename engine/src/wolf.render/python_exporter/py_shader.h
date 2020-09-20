/*
	Project          : Wolf Engine. Copyright(c) Pooya Eimandar (http://PooyaEimandar.com) . All rights reserved.
	Source           : Please direct any bug to https://github.com/PooyaEimandar/Wolf.Engine/issues
	Website          : http://WolfSource.io
	Name             : py_shader.h
	Description      : The python exporter for w_shader class
	Comment          :
*/

#ifdef __PYTHON__

#ifndef __PY_SHADER_H__
#define __PY_SHADER_H__

#include <python_exporter/w_boost_python_helper.h>

namespace pyWolf
{
	using namespace boost::python;
	using namespace wolf::graphics;

	BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(w_shader_load_overloads, w_shader::py_load, 3, 4)
	BOOST_PYTHON_FUNCTION_OVERLOADS(w_shader_load_shader_overloads, w_shader::py_load_shader, 9, 10)		

	static void py_shader_export()
	{
		//define w_shader_binding_type enum
		enum_<w_shader_binding_type>("w_shader_binding_type")
			.value("SAMPLER2D", w_shader_binding_type::SAMPLER2D)
			.value("SAMPLER", w_shader_binding_type::SAMPLER)
			.value("UNIFORM", w_shader_binding_type::UNIFORM)
			.value("IMAGE", w_shader_binding_type::IMAGE)
			.value("STORAGE", w_shader_binding_type::STORAGE)
			.export_values()
			;

		//export w_pipeline_shader_stage_create_info class
		class_<w_pipeline_shader_stage_create_info, boost::noncopyable>("w_pipeline_shader_stage_create_info");

		//export w_shader_binding_param class
		class_<w_shader_binding_param, boost::noncopyable>("w_shader_binding_param")
			.def_readwrite("index", &w_shader_binding_param::index, "index of shader variable")
			.def_readwrite("type", &w_shader_binding_param::type, "type of shader variable")
			.def_readwrite("stage", &w_shader_binding_param::stage, "shader stage usages")
			.def_readwrite("buffer_info", &w_shader_binding_param::buffer_info, "descriptor buffer info")
			.def_readwrite("image_info", &w_shader_binding_param::image_info, "descriptor image info")
			;

		//export w_shader class
		class_<w_shader, boost::noncopyable>("w_shader")
			.def("load", &w_shader::py_load, w_shader_load_overloads())
			.def("release", &w_shader::release, "release all resources")
			.def("get_shader_binding_params", &w_shader::py_get_shader_binding_params, "get shader binding params")
			.def("get_shader_stages", &w_shader::py_get_shader_stages, "get shader stages")
			.def("get_compute_shader_stage", &w_shader::get_compute_shader_stage, "get compute shader stage")
			.def("set_shader_binding_params", &w_shader::py_set_shader_binding_params, "set and update shader binding params")
			.def("load_shader", w_shader::py_load_shader, w_shader_load_shader_overloads())
			.staticmethod("load_shader")
			;
	}
}

#endif//__PY_SHADER_H__

#endif//__PYTHON__
