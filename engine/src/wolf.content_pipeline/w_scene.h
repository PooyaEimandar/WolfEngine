/*
	Project			 : Wolf Engine. Copyright(c) Pooya Eimandar (http://PooyaEimandar.com) . All rights reserved.
	Source			 : Please direct any bug to https://github.com/PooyaEimandar/Wolf.Engine/issues
	Website			 : http://WolfSource.io
	Name			 : w_scene.h
	Description		 : This scene class which contains all assets
	Comment          :
*/

#ifndef __W_SCENE_H__
#define __W_SCENE_H__

#include "w_cpipeline_export.h"
#include <w_object.h>
#include "w_model.h"
#include "w_camera.h"
#include <msgpack.hpp>

namespace wolf
{
	namespace content_pipeline
	{
		class w_scene
		{
		public:
			WCP_EXP w_scene();
			WCP_EXP virtual ~w_scene();
			
			WCP_EXP void add_model(_In_ w_model* pModel);
            WCP_EXP void add_models(_Inout_ std::vector<w_model*>& pModel);
            WCP_EXP void add_camera(_In_ w_camera* pCamera);
			WCP_EXP void add_camera(_In_z_ const std::string& pName, _In_ const glm::vec3 pTransform, _In_ const glm::vec3 pInterest);
            
#pragma region Getters

            WCP_EXP void get_models_by_index(_In_ const size_t pIndex, _Inout_ w_model** pModel);
			WCP_EXP void get_models_by_id(const std::string& pID, _Inout_ std::vector<w_model*>& pModels);
            WCP_EXP void get_all_models(_Inout_ std::vector<w_model*>& pModels);

			//Get first camera if avaible, else create a default one
			WCP_EXP void get_first_or_default_camera(_Inout_ w_camera** pCamera);
			WCP_EXP void get_cameras_by_id(const std::string& pID, _Inout_ std::vector<w_camera*>& pCameras);
            WCP_EXP void get_cameras_by_index(const size_t pIndex, _Inout_ w_camera** pCamera);

            WCP_EXP std::string get_name() const         { return this->_name; }
            WCP_EXP ULONG release();

#pragma endregion

#pragma region Setters

            WCP_EXP void set_name(_In_z_ std::string pValue)   { this->_name = pValue; }

            MSGPACK_DEFINE(_name, _cameras, _models);

#pragma endregion

		private:
            std::string                 _name;
			std::vector<w_camera>		_cameras;
            std::vector<w_model>		_models;
		};
	}
}

#endif
