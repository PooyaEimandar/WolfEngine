#include "w_cpipeline_pch.h"
#include "c_parser.h"
#include "w_cpipeline_model.h"
#include "c_skin.h"

using namespace std;
using namespace wolf::system;
using namespace wolf::content_pipeline;
using namespace wolf::content_pipeline::collada;

W_RESULT c_parser::parse_collada_from_file(
    _In_z_ const std::wstring& pFilePath, 
    _Inout_ w_cpipeline_scene* pScene,
    _In_ const bool& pAMDTootleOptimizing,
#ifdef __WIN32
	_In_ const bool& pSimplygonOptimizing,
#endif
    _In_ const bool& pInvertNormals,
    _In_ const bool& pFindLODs)
{
	auto _hr = W_PASSED;

#if defined(__WIN32) || defined(__UWP)
    auto _path = pFilePath;
#else
    auto _path = wolf::system::convert::wstring_to_string(pFilePath);
#endif
    
	//open stream of xml 
	std::ifstream _file(_path);
	std::stringstream _string_stream;
	_string_stream << _file.rdbuf();
	_file.close();
	std::string content(_string_stream.str());

	sSkipChildrenOfThisNode = "NULL";

	using namespace rapidxml;
	xml_document<> _doc;
	try
	{
		_doc.parse<0>(&content[0]);
	}
	catch (...)
	{
		_hr = W_FAILED;
        V(_hr, w_log_type::W_ERROR, L"Could not parse collada file on following path : {}. trace info: {}", pFilePath, L"::w_collada_parser");
	}

	_hr = _process_xml_node(_doc.first_node());
	//V(_hr, L"processing xml node : " + pFilePath, _trace_class_name, 3);
	
	//create scene
	_create_scene(
        pScene,
        pAMDTootleOptimizing,
#ifdef __WIN32
		pSimplygonOptimizing,
#endif
        pInvertNormals,
        pFindLODs);

	//clear all
	_doc.clear();
	_clear_all_resources();

	return _hr;
}

W_RESULT c_parser::_process_xml_node(_In_ rapidxml::xml_node<>* pXNode)
{
    if (!pXNode) return W_FAILED;

	//get the name of node
	auto _node_name = _get_node_name(pXNode);

#ifdef DEBUG
	//logger.write(_node_name);
#endif

	std::string _parent_node_name;
	auto _parent = pXNode->parent();
	if (_parent)
	{
		_parent_node_name = _get_node_name(_parent);
		if (_parent_node_name == sSkipChildrenOfThisNode) return W_PASSED;
	}

	if (_node_name == "collada")
	{
#pragma region collada headers
		//check collada version 1.4.1
		auto _attr = pXNode->first_attribute("xmlns", 0, false);
		if (_attr && 0 != std::strcmp(_attr->value(), "http://www.collada.org/2005/11/COLLADASchema"))
		{
			logger.error(L"Collada file does not have standard COLLADA header");
			return W_FAILED;
		}

		_attr = pXNode->first_attribute("version", 0, false);
		if (_attr && 0 != std::strcmp(_attr->value(), "1.4.1"))
		{
			logger.error(L"Collada file does not have standard COLLADA header");
			return W_FAILED;
		}
#pragma endregion
	}
	else if (_node_name == "asset")
	{
        sSkipChildrenOfThisNode = _node_name;

        for (auto _child = pXNode->first_node(); _child != nullptr; _child = _child->next_sibling())
        {
            auto _node_name = _get_node_name(_child);

#ifdef DEBUG
            //logger.write(_node_name);
#endif

            if (_node_name == "up_axis")
            {
                auto _str = std::string(_child->value());
                std::transform(_str.begin(), _str.end(), _str.begin(), ::tolower);
                if (_str == "y_up")
                {
                    sZ_Up = false;
                }
                _str.clear();
            }
        }
        return W_PASSED;
	}
	else if (_node_name == "library_cameras")
	{
		//we don't need basic information of camera, such as near plan, far plan and etc
		sSkipChildrenOfThisNode = _node_name;
        _get_library_cameras(pXNode);
		return W_PASSED;
	}
	else if (_node_name == "library_lights")
	{
		//ToDo: read lights
	}
	else if (_node_name == "library_effects")
	{
        sSkipChildrenOfThisNode = _node_name;
        _get_library_effects(pXNode);
        return W_PASSED;
	}
	else if (_node_name == "library_materials")
	{
        sSkipChildrenOfThisNode = _node_name;
        _get_library_materials(pXNode);
        return W_PASSED;
	}
    else if (_node_name == "library_images")
    {
        sSkipChildrenOfThisNode = _node_name;
        _get_library_images(pXNode);
        return W_PASSED;
    }
	else if (_node_name == "library_geometries")
	{
		SGeometryLibraryNode = pXNode;
	}
	else if (_node_name == "library_visual_scenes")
	{
#pragma region parse visual scenes
		//process all childs
		for (auto _child = pXNode->first_node(); _child != nullptr; _child = _child->next_sibling())
		{
			auto _node_name = _get_node_name(_child);
#ifdef DEBUG
            //logger.write(_node_name);
#endif
			if (_node_name == "visual_scene")
			{
				//read scene id
				_get_node_attribute_value(_child, "id", sSceneID);

				//read visual scene nodes
				_read_visual_scene_nodes(_child, sNodes);
			}
		}
		sSkipChildrenOfThisNode = _node_name;
#pragma endregion
	}
	else if (_node_name == "scene")
	{

	}
	else if (_node_name == "extra" && _parent_node_name == "collada")
	{
#pragma region parse extra
		//process all childs of extra
		for (auto _child = pXNode->first_node(); _child != nullptr; _child = _child->next_sibling())
		{
			auto _node_name = _get_node_name(_child);
#ifdef DEBUG
			//logger.write(_node_name);
#endif

			if (_node_name == "technique")
			{
				for (auto __child = _child->first_node(); __child != nullptr; __child = __child->next_sibling())
				{
					auto __node_name = _get_node_name(__child);
#ifdef DEBUG
					//logger.write(__node_name);
#endif

					if (__node_name == "si_scene")
					{
						std::vector<c_value_obj*> _si_scene;
						_get_si_scene_data(__child, _si_scene);

						for (auto _si : _si_scene)
						{
							if (_si == nullptr) continue;

							if (_si->c_sid == "timing")
							{
								sXSI_Extra.timing = _si->value;
							}
							else if (_si->c_sid == "timing")
							{
								sXSI_Extra.start = std::atoi(_si->value.c_str());
							}
							else if (_si->c_sid == "timing")
							{
								sXSI_Extra.end = std::atoi(_si->value.c_str());
							}
							else if (_si->c_sid == "timing")
							{
								sXSI_Extra.frame_rate = std::atoi(_si->value.c_str());
							}
						}
					}
//					else if (__node_name == "xsi_trianglelist")
//					{
//						for (auto ___child = __child->first_node(); ___child != nullptr; ___child = ___child->next_sibling())
//						{
//							string _sid;
//							_get_node_attribute_value(___child, "sid", _sid);
//							if (_sid == "vertexIndices")
//							{
////								std::string _value = ___child->value();
////								wolf::system::convert::split_string_then_convert_to<unsigned short>(_value, " ", sXSI_Indices);
//							}
//						}
//					}
				}
			}
		}
#pragma endregion
	}

	//process all childs
	for (auto _child = pXNode->first_node(); _child != nullptr; _child = _child->next_sibling())
	{
		_process_xml_node(_child);
	}

	return W_PASSED;
}

void c_parser::_get_library_cameras(_In_ rapidxml::xml_node<>* pXNode)
{
    if (!pXNode) return;

    //get instance cameras
    w_camera* _camera = nullptr;
    for (auto _child_0 = pXNode->first_node(); _child_0 != nullptr; _child_0 = _child_0->next_sibling())
    {
        auto _node_name_0 = _get_node_name(_child_0);

#ifdef DEBUG
        //logger.write(_node_name_0);
#endif

        if (_node_name_0 == "camera")
        {
            std::string _camera_id;
            _get_node_attribute_value(_child_0, "id", _camera_id);
            
            _camera = new w_camera();
            _camera->set_name(_camera_id);
            
            for (auto _child_1 = _child_0->first_node(); _child_1 != nullptr; _child_1 = _child_1->next_sibling())
            {
                auto _node_name_1 = _get_node_name(_child_1);
#ifdef DEBUG
                //logger.write(_node_name_1);
#endif

                if (_node_name_1 == "optics")
                {
                    for (auto _child_2 = _child_1->first_node(); _child_2 != nullptr; _child_2 = _child_2->next_sibling())
                    {
                        auto _node_name_2 = _get_node_name(_child_2);
#ifdef DEBUG
                        //logger.write(_node_name_2);
#endif
                        if (_node_name_2 == "technique_common")
                        {
                            auto _child_3 = _child_2->first_node();
                            if (_child_3)
                            {
                                auto _node_name_3 = _get_node_name(_child_3);
#ifdef DEBUG
                                //logger.write(_node_name_3);
#endif

                                if (_node_name_3 == "perspective")
                                {
                                    if (_camera)
                                    {
                                        for (auto _child_4 = _child_3->first_node(); _child_4 != nullptr; _child_4 = _child_4->next_sibling())
                                        {
                                            auto _node_name_4 = _get_node_name(_child_4);
#ifdef DEBUG
                                            //logger.write(_node_name_4);
#endif

                                            if (_node_name_4 == "xfov")
                                            {
                                                _camera->set_field_of_view(std::atof(_child_4->value()));
                                            }
                                            else if (_node_name_4 == "znear")
                                            {
                                                _camera->set_near_plan(std::atof(_child_4->value()));
                                            }
                                            else if (_node_name_4 == "zfar")
                                            {
                                                _camera->set_far_plan(std::atof(_child_4->value()));
                                            }
                                        }
                                    }
                                    break;
                                }
                                break;
                            }
                            break;
                        }
                    }
                }
                else if (_node_name_1 == "extra")
                {
                    for (auto _child_2 = _child_1->first_node(); _child_2 != nullptr; _child_2 = _child_2->next_sibling())
                    {
                        auto _node_name_2 = _get_node_name(_child_2);
#ifdef DEBUG
                        //logger.write(_node_name_2);
#endif
                        if (_node_name_2 == "technique")
                        {
                            auto _child_3 = _child_2->first_node();
                            if (_child_3)
                            {
                                std::string _sid;
                                _get_node_attribute_value(_child_3, "sid", _sid);
                                if (_sid == "target")
                                {
                                    if (_camera)
                                    {
                                        auto _camera_id = _camera->get_name();
                                        auto _iter = sLibraryCameras.find(_camera_id);
                                        if (_iter == sLibraryCameras.end())
                                        {
                                            std::string _camera_target_name = _child_3->value();
                                            if (_camera_target_name.size() &&
                                                _camera_target_name[0] == '#')
                                            {
                                                _camera_target_name = _camera_target_name.erase(0, 1);
                                            }

                                            _camera->set_camera_target_name(_camera_target_name);
                                            sLibraryCameras[_camera_id] = *_camera;
                                        }
                                    }
                                    break;
                                }
                                break;
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
}

void c_parser::_get_library_effects(_In_ rapidxml::xml_node<>* pXNode)
{
    if (!pXNode) return;

    //get instance effects
    for (auto _child_0 = pXNode->first_node(); _child_0 != nullptr; _child_0 = _child_0->next_sibling())
    {
        auto _node_name_0 = _get_node_name(_child_0);

#ifdef DEBUG
        //logger.write(_node_name_0);
#endif

        if (_node_name_0 == "effect")
        {
            std::string _effect_id;
            _get_node_attribute_value(_child_0, "id", _effect_id);

            for (auto _child_1 = _child_0->first_node(); _child_1 != nullptr; _child_1 = _child_1->next_sibling())
            {
                auto _node_name_1 = _get_node_name(_child_1);
#ifdef DEBUG
                //logger.write(_node_name_1);
#endif
                if (_node_name_1 == "profile_common")
                {
                    for (auto _child_2 = _child_1->first_node(); _child_2 != nullptr; _child_2 = _child_2->next_sibling())
                    {
                        auto _node_name_2 = _get_node_name(_child_2);
#ifdef DEBUG
                        //logger.write(_node_name_2);
#endif
                        if (_node_name_2 == "newparam")
                        {
                            for (auto _child_3 = _child_2->first_node(); _child_3 != nullptr; _child_3 = _child_3->next_sibling())
                            {
                                auto _node_name_3 = _get_node_name(_child_3);
#ifdef DEBUG
                                //logger.write(_node_name_3);
#endif
                                if (_node_name_3 == "surface")
                                {
                                    for (auto _child_4 = _child_3->first_node(); _child_4 != nullptr; _child_4 = _child_4->next_sibling())
                                    {
                                        auto _node_name_4 = _get_node_name(_child_4);
#ifdef DEBUG
                                        //logger.write(_node_name_4);
#endif
                                        if (_node_name_4 == "init_from")
                                        {
                                            auto _iter = sLibraryEffects.find(_effect_id);
                                            //we did not find same name for this effect, so we can add it
                                            if (_iter == sLibraryEffects.end())
                                            {
                                                sLibraryEffects[_effect_id] = _child_4->value();
                                            }
                                        }
                                        break; //init_from
                                    }
                                    break; //surface
                                }
                            }
                            break; //newparam
                        }
                    }

                    break; //profile_common
                }
            }
        }
    }
}

void c_parser::_get_library_materials(_In_ rapidxml::xml_node<>* pXNode)
{
    if (!pXNode) return;

    //get materials
    for (auto _child_0 = pXNode->first_node(); _child_0 != nullptr; _child_0 = _child_0->next_sibling())
    {
        auto _node_name_0 = _get_node_name(_child_0);

#ifdef DEBUG
        //logger.write(_node_name_0);
#endif

        if (_node_name_0 == "material")
        {
            std::string _material_id;
            _get_node_attribute_value(_child_0, "id", _material_id);
            auto _child_1 = _child_0->first_node();
            if (_child_1)
            {
                auto _node_name_1 = _get_node_name(_child_1);
#ifdef DEBUG
                //logger.write(_node_name_1);
#endif

                if (_node_name_1 == "instance_effect")
                {
                    std::string _url;
                    _get_node_attribute_value(_child_1, "url", _url);
                    if (_url.size() &&
                        _url[0] == '#')
                    {
                        _url= _url.erase(0, 1);
                    }

                    auto _iter = sLibraryEffects.find(_material_id);
                    //we did not find same name for this effect, so we can add it
                    if (_iter == sLibraryEffects.end())
                    {
                        sLibraryMaterials[_material_id] = _url;
                    }
                }
            }
        }
    }
}

void c_parser::_get_library_images(_In_ rapidxml::xml_node<>* pXNode)
{
    if (!pXNode) return;

    //get texture init from
    for (auto _child_0 = pXNode->first_node(); _child_0 != nullptr; _child_0 = _child_0->next_sibling())
    {
        auto _node_name_0 = _get_node_name(_child_0);

#ifdef DEBUG
        //logger.write(_node_name_0);
#endif

        if (_node_name_0 == "image")
        {
            std::string _image_id;
            _get_node_attribute_value(_child_0, "id", _image_id);
            auto _child_1 = _child_0->first_node();
            if (_child_1)
            {
                auto _node_name_1 = _get_node_name(_child_1);
#ifdef DEBUG
                //logger.write(_node_name_1);
#endif

                if (_node_name_1 == "init_from")
                {
                    auto _iter = sLibraryImages.find(_image_id);
                    //we did not find same name for this effect, so we can add it
                    if (_iter == sLibraryImages.end())
                    {
                        /*
                            We are just save image name without prepath
                            Wolf will be load all images from one folder from /~/content/images/
                            make sure do not use different images with same name
                         */
                      auto _path = wolf::system::io::get_base_file_name(_child_1->value()) +
                            wolf::system::io::get_file_extention(_child_1->value());

                      //remove _number added by open collada
                      size_t i;
                      for ( i = 0; i < _path.size(); i++)
                      {
                          if (_path[i] >= '0' && _path[i] <= '9') continue;
                          break;
                      }

                      _path.replace(_path.begin(), _path.begin() + i + 1, "");

                      sLibraryImages[_image_id] = _path;
                    }
                }
            }
        }
    }
}

void c_parser::_read_visual_scene_nodes(_In_ rapidxml::xml_node<>* pXNode, _Inout_ std::vector<c_node*>& pNodes)
{
#ifdef DEBUG
	std::string _id;
	_get_node_attribute_value(pXNode, "id", _id);
	//logger.write(_id);
#endif

	//iterate over children of this node
    for (auto _child = pXNode->first_node(); _child != nullptr; _child = _child->next_sibling())
    {
        //get the name of node
        auto _name = _get_node_name(_child);
#ifdef DEBUG
        //logger.write(_name);
#endif

        if (_name == "node")
        {
            //create node
            auto _node = new c_node();

            //get collada attributes
            _get_collada_obj_attribute(_child, _node);

            //if this node has a mesh child, so this node is a group or layer
            _get_node_data(_child, &_node);
            
            //merge transforms of nodes
            pNodes.push_back(_node);
        }
    }

}

void c_parser::_get_node_data(_In_ rapidxml::xml_node<>* pXNode, _Inout_ c_node** pParentNode)
{
    auto _parent_node_ptr = (*pParentNode);

    if (pXNode == nullptr || _parent_node_ptr == nullptr || pParentNode == nullptr) return;

    for (auto _child = pXNode->first_node(); _child != nullptr; _child = _child->next_sibling())
    {
        //get the name of node
        auto _name = _get_node_name(_child);

#ifdef DEBUG
        //logger.write(_name);
#endif
        if (_name == "translate")
        {
            _parent_node_ptr->translate = glm::to_vec3(_child->value());
        }
        else if (_name == "rotate")
        {
            _parent_node_ptr->rotation = glm::to_vec4(_child->value());
        }
        else if (_name == "scale")
        {
            _parent_node_ptr->scale = glm::to_vec3(_child->value());
        }
        else if (_name == "matrix")
        {
            auto _matrix = glm::to_mat4(_child->value());
            _parent_node_ptr->translate.x = _matrix[0][3];
            _parent_node_ptr->translate.y = _matrix[1][3];
            _parent_node_ptr->translate.z = _matrix[2][3];
        }
        else if (_name == "instance_camera")
        {
            _get_node_attribute_value(_child, "url", _parent_node_ptr->instanced_camera_name);
            if (_parent_node_ptr->instanced_camera_name.size() &&
                _parent_node_ptr->instanced_camera_name[0] == '#')
            {
                _parent_node_ptr->type = c_node_type::CAMERA;
                _parent_node_ptr->instanced_camera_name = _parent_node_ptr->instanced_camera_name.erase(0, 1);
            }
        }
        else if (_name == "instance_geometry")
        {
            _get_node_attribute_value(_child, "url", _parent_node_ptr->instanced_geometry_name);
            if (_parent_node_ptr->instanced_geometry_name.size() &&
                _parent_node_ptr->instanced_geometry_name[0] == '#')
            {
                _parent_node_ptr->type = c_node_type::MESH;
                _parent_node_ptr->instanced_geometry_name = _parent_node_ptr->instanced_geometry_name.erase(0, 1);
            }

            auto _instanced_material_name = _get_instance_material_symbol_target_name(_child);
            _parent_node_ptr->instanced_material_symbol_name = std::get<0>(_instanced_material_name);
            _parent_node_ptr->instanced_material_target_name = std::get<1>(_instanced_material_name);
        }
        else if (_name == "node")
        {
            //create node
            auto _node = new c_node();

            //get collada attributes
            _get_collada_obj_attribute(_child, _node);

            _get_node_data(_child, &_node);
            _parent_node_ptr->type |= _node->type;

            //merge with parent node transformation
            _node->translate = _parent_node_ptr->translate;
            _node->rotation = _parent_node_ptr->rotation;
            _node->scale = _parent_node_ptr->scale;

            _parent_node_ptr->child_nodes.push_back(_node);
        }

        //_get_node_data(_child, pParentNode);
    }

    _parent_node_ptr->transform = glm::make_wpv_mat(
            _parent_node_ptr->scale,
            _parent_node_ptr->rotation,
            _parent_node_ptr->translate);

    //#pragma region FOR_ANIMATION
    //        //else if (__child_name == "instance_controller")
    //        //{
    //        //    //iterate over children of this node
    //        //    for (auto ___child = __child->first_node(); ___child != nullptr; ___child = ___child->next_sibling())
    //        //    {
    //        //        auto ___child_name = _get_node_name(___child);
    //        //        if (___child_name == "skeleton")
    //        //        {
    //        //            std::string _value = ___child->value();
    //        //            sSkeletonNames.push_back(_value);
    //        //        }
    //        //    }
    //        //}
    //        //				else if (__child_name == "node")
    //        //				{
    //        //					rapidxml::xml_node<>* _joint_node = nullptr;
    //        //					_find_node(__child, "type", "JOINT", _joint_node);
    //        //					if (_joint_node != nullptr)
    //        //					{
    //        //						c_bone _temp_bone;
    //        //						std::memset(&_temp_bone, 0, sizeof(_temp_bone));
    //        //
    //        //						auto _base_class = (c_obj*) (&_temp_bone);
    //        //						_get_collada_obj_attribute(_joint_node, _base_class);
    //        //						_get_bones(_joint_node, &_temp_bone, _temp_bone.flat_bones);
    //        //
    //        //						sBones.push_back(&_temp_bone);
    //        //					}
    //        //					_get_nodes(__child, pNodes);
    //        //
    //        //#ifdef DEBUG
    //        //					logger.write(std::to_string(pNodes.size()));
    //        //#endif
    //        //				}
    //#pragma endregion
    //    }
}

std::tuple<std::string, std::string> c_parser::_get_instance_material_symbol_target_name(_In_ rapidxml::xml_node<>* pXNode)
{
    std::string _instance_material_symbol_name;
    std::string _instance_material_target_name;

    for (auto _child_0 = pXNode->first_node(); _child_0 != nullptr; _child_0 = _child_0->next_sibling())
    {
        //get the name of node
        auto _node_name_0 = _get_node_name(_child_0);

#ifdef DEBUG
        //logger.write(_node_name_0);
#endif
        if (_node_name_0 == "bind_material")
        {
            for (auto _child_1 = _child_0->first_node(); _child_1 != nullptr; _child_1 = _child_1->next_sibling())
            {
                //get the name of node
                auto _node_name_1 = _get_node_name(_child_1);

#ifdef DEBUG
                //logger.write(_node_name_1);
#endif
                if (_node_name_1 == "technique_common")
                {
                    for (auto _child_2 = _child_1->first_node(); _child_2 != nullptr; _child_2 = _child_2->next_sibling())
                    {
                        //get the name of node
                        auto _node_name_2 = _get_node_name(_child_2);

#ifdef DEBUG
                        //logger.write(_node_name_2);
#endif
                        if (_node_name_2 == "instance_material")
                        {
                            //TODO: currently we are not support multi texturing
                            _get_node_attribute_value(_child_2, "symbol", _instance_material_symbol_name);
                            _get_node_attribute_value(_child_2, "target", _instance_material_target_name);

                            break;
                        }
                    }
                    break;
                }
            }

            break;
        }
    }

    if (_instance_material_target_name.size() &&
        _instance_material_target_name[0] == '#')
    {
        _instance_material_target_name = _instance_material_target_name.erase(0, 1);
    }

    return std::make_tuple(_instance_material_symbol_name, _instance_material_target_name);
}

void c_parser::_get_si_scene_data(_In_ rapidxml::xml_node<>* pXNode, _Inout_ std::vector<c_value_obj*>& pSIs)
{
	for (auto _child = pXNode->first_node(); _child != nullptr; _child = _child->next_sibling())
	{
		auto _node_name = _get_node_name(_child);

#ifdef DEBUG
		//logger.write(_node_name);
#endif

		if (_node_name == "xsi_param")
		{
			string _sid;
			_get_node_attribute_value(_child, "sid", _sid);

			auto _value_obj = new c_value_obj();
			_value_obj->c_sid = _sid;
			_value_obj->value = _child->value();

			pSIs.push_back(_value_obj);
		}
	}
}

void c_parser::_find_node(_In_ rapidxml::xml_node<>* pXNode, const string& pAttributeName, const string& pAttributeValue, _Inout_ rapidxml::xml_node<>* pFoundNode)
{
	pFoundNode = nullptr;

	std::string _attr_value = "";
	auto found = _get_node_attribute_value(pXNode, pAttributeName, _attr_value);
	if (found && _attr_value == pAttributeValue)
	{
		pFoundNode = pXNode;
		return;
	}

	//iterate over children of this node
	for (auto _child = pXNode->first_node(); _child != nullptr; _child = _child->next_sibling())
	{
		_find_node(_child, pAttributeName, pAttributeValue, pFoundNode);
		if (pFoundNode != nullptr)
		{
			break;
		}
	}
}

std::string c_parser::_get_node_name(_In_ rapidxml::xml_node<>* pXNode)
{
	//get the name of node
	std::string _name(pXNode->name());
	//to lower it
	std::transform(_name.begin(), _name.end(), _name.begin(), ::tolower);

	return _name;
}

bool c_parser::_get_node_attribute_value(_In_ rapidxml::xml_node<>* pXNode, _In_ const std::string& pAttributeName, _Inout_ std::string& pAttributeValue )
{
	auto _attr = pXNode->first_attribute(pAttributeName.c_str());
	if (_attr)
	{
		pAttributeValue = _attr->value();
		return true;
	}
	return false;
}

void c_parser::_get_collada_obj_attribute(_In_ rapidxml::xml_node<>* pXNode, _Inout_ c_obj* pCObj)
{
	if (pCObj == nullptr) return;

	std::string _id = "NULL", _name = "NULL", _sid = "NULL";

	_get_node_attribute_value(pXNode, "id", _id);
	_get_node_attribute_value(pXNode, "name", _name);
	_get_node_attribute_value(pXNode, "sid", _sid);//just for softimage xsi

	pCObj->c_id = _id;
	pCObj->c_name = _name;
	pCObj->c_sid = _sid;
}

void c_parser::_get_bones(_In_ rapidxml::xml_node<>* pXNode, _Inout_ c_bone* pBone, _Inout_ std::vector<c_bone*>& pFlatBones)
{
	if (pBone == nullptr) throw std::runtime_error("pbone must not null");

	pBone->index = (int)pFlatBones.size();
	pFlatBones.push_back(pBone);

	auto _no_matrix = false;
	auto _transform = glm::vec3(0);
	float _rotate[4];
	auto _rotate_index = 0;
	glm::vec3 _scale;

	for (auto _child = pXNode->first_node(); _child != nullptr; _child = _child->next_sibling())
	{
		auto _bone = new c_bone();
		_bone->parent = pBone;

		auto _name = _get_node_name(_child);
		if (_name == "matrix")
		{
#ifdef DEBUG
			//logger.write(_child->value());
#endif
			//pBone->initial_matrix = e.Value.Split(' ').ToMatrix().Transpose();

			pBone->position = glm::vec4_from_mat4x4_row(pBone->initial_matrix, 3);
		}
		else if (_name == "translate")
		{
			_no_matrix = true;
			_transform = glm::to_vec3(_child->value());
		}
		else if (_name == "scale")
		{
			//auto _parent_rotation = pBone->parent != nullptr ? pBone->parent->rotation : glm::vec3(0);
			pBone->scale = _scale = glm::to_vec3(_child->value());
			pBone->rotation_matrix = glm::rotate(_rotate[2], glm::vec3(1, 0, 0)) *
				glm::rotate(_rotate[1], glm::vec3(0, 1, 0)) *
				glm::rotate(_rotate[0], glm::vec3(0, 0, 1));

			pBone->initial_matrix = pBone->world_transform = (glm::scale(_scale) *
				pBone->rotation_matrix *
				glm::translate(_transform) *
				(pBone->parent != nullptr ? pBone->parent->initial_matrix : glm::mat4x4())); // if glm::mat4x4() is identity

			pBone->initial_matrix = pBone->world_transform = (glm::scale(_scale) *
				pBone->rotation_matrix *
				glm::translate(_transform));

			pBone->position = glm::vec4_from_mat4x4_row(pBone->initial_matrix, 3);
			pBone->rotation = glm::vec3(glm::degrees(_rotate[2]), glm::degrees(_rotate[1]), glm::degrees(_rotate[0]));
		}
		else if (_name == "rotate")
		{
			if (_rotate_index < W_ARRAY_SIZE(_rotate))
			{
				auto _angle = glm::to_vec3(_child->value());
				_rotate[_rotate_index] = glm::radians(_angle[3]);
				
				_rotate_index++;
			}
		}
		else if (_name == "node")
		{
			std::string _id = "NULL", _name = "NULL", _sid = "NULL";

			_get_node_attribute_value(_child, "id", _id);
			_get_node_attribute_value(_child, "name", _name);
			_get_node_attribute_value(_child, "sid", _sid);

			_bone->c_id = _id;
			_bone->c_name = _name;
			_bone->c_sid = _id;

			if (_no_matrix)
			{
				_rotate_index = 0;
			}

			_get_bones(_child, _bone, pFlatBones);
			pBone->children.push_back(_bone);
		}
	}
}

//void c_parser::_get_nodes()
//{
//	auto _node = new c_node();
//
//	_get_collada_obj_attribute(pXNode, _node);
//
//#ifdef DEBUG
//	auto _name = _get_node_name(pXNode);
//	std::string _id;
//	_get_node_attribute_value(pXNode, "id", _id);
//
//	logger.write(_name + " with id: " + _id);
//#endif
//
//	for (auto _child = pXNode->first_node(); _child != nullptr; _child = _child->next_sibling())
//	{
//		auto _node_name = _get_node_name(_child);
//#ifdef DEBUG
//		logger.write(_node_name);
//#endif
//		if (_node_name == "translate")
//		{
//			_node->translate = glm::to_vec3(_child->value());
//		}
//		else if (_node_name == "rotate")
//		{
//			std::string _rotation_type;
//			_get_node_attribute_value(_child, "sid", _rotation_type);
//			auto _vec4 = glm::to_vec4(_child->value());
//
//			if (_rotation_type == "rotation_z")
//			{
//				_node->rotation.z = _vec4[3];
//			}
//			else if (_rotation_type == "rotation_y")
//			{
//				_node->rotation.y = _vec4[3];
//			}
//			else if (_rotation_type == "rotation_x")
//			{
//				_node->rotation.x = _vec4[3];
//			}
//		}
//		else if (_node_name == "scale")
//		{
//			_node->scale = glm::to_vec3(_child->value());
//		}
//		else if (_node_name == "node")
//		{
//			_get_nodes(_child, _node->nodes);
//		}
//		else if (_node_name == "instance_geometry")
//		{
//			_get_node_attribute_value(_child, "url", _node->instanced_geometry_name);
//			if (_node->instanced_geometry_name[0] == '#')
//			{
//				_node->instanced_geometry_name = _node->instanced_geometry_name.erase(0, 1);
//			}
//
//		}
//		else if (_node_name == "instance_node")
//		{
//			_get_node_attribute_value(_child, "url", _node->instanced_node_name);
//			if (_node->instanced_geometry_name[0] == '#')
//			{
//				_node->instanced_geometry_name = _node->instanced_geometry_name.erase(0, 1);
//			}
//		}
//		else if (_node_name == "instance_camera")
//		{
//			_get_node_attribute_value(_child, "url", _node->instanced_camera_name);
//			if (_node->instanced_camera_name[0] == '#')
//			{
//				_node->instanced_camera_name = _node->instanced_camera_name.erase(0, 1);
//			}
//		}
//		else if (_node_name == "instance_light")
//		{
//			//ToDo
//		}
//	}
//
//	_node->transform = glm::make_wpv_mat(_node->scale, _node->rotation, _node->translate);
//	pNodes.push_back(_node);
//
//#ifdef DEBUG
//	logger.write(std::to_string(pNodes.size()));
//#endif // DEBUG
//}

void c_parser::_get_sources(_In_ rapidxml::xml_node<>* pXNode, std::string pID, std::string pName, _Inout_ c_geometry& pGeometry)
{
	string _float_array_str = "";

	for (auto _child = pXNode->first_node(); _child != nullptr; _child = _child->next_sibling())
	{
		auto _node_name =  _get_node_name(_child);

#ifdef DEBUG
		//logger.write(_node_name);
#endif

		if (_node_name == "float_array")
		{
			_float_array_str = _child->value();
		}
		else if (_node_name == "technique_common")
		{
			auto _stride = 1;
            auto _count = 0 ;
			for (auto __child = _child->first_node(); __child != nullptr; __child = __child->next_sibling())
			{
				std::string _str;
				_get_node_attribute_value(__child, "stride", _str);
				_stride = std::atoi(_str.c_str());
                
                _get_node_attribute_value(__child, "count", _str);
                _count = std::atoi(_str.c_str());
			}

			auto _source = new c_source();
			_source->c_id = pID;
			_source->c_name = pName;
			_source->stride = _stride;
            
			wolf::system::convert::find_all_numbers_then_convert_to<float>(_float_array_str, _source->float_array);
			pGeometry.sources.push_back(_source);
		}
	}
}

void c_parser::_get_vertices(_In_ rapidxml::xml_node<>* pXNode, _Inout_ c_geometry& pGeometry)
{
	for (auto _child = pXNode->first_node(); _child != nullptr; _child = _child->next_sibling())
	{
		std::string _semantic, _source;
		_get_node_attribute_value(_child, "semantic", _semantic);
		_get_node_attribute_value(_child, "source", _source);

		if (_semantic != "" && _source != "")
		{
			if (pGeometry.vertices != nullptr)
			{
				auto _c_semantic = new c_semantic();
				if (_semantic[0] == '#') _semantic = _semantic.erase(0, 1);
				_c_semantic->semantic = _semantic;

				if (_source[0] == '#') _source = _source.erase(0, 1);
				_c_semantic->source = _source;

				_c_semantic->offset = (int)pGeometry.vertices->semantics.size();

				pGeometry.vertices->semantics.push_back(_c_semantic);
			}
		}
	}
}

void c_parser::_get_triangles(_In_ rapidxml::xml_node<>* pXNode, _In_ c_node* pNode, _Inout_ c_geometry& pGeometry)
{
	std::string _material_name;
	_get_node_attribute_value(pXNode, "material", _material_name);
    
    auto _triangles = new c_triangles();

    //for open collada
    if (_material_name == pNode->instanced_material_symbol_name)
    {
        _triangles->material_name =  pNode->instanced_material_target_name;
    }
    else
    {
        //for simple collada
        _triangles->material_name = pNode->instanced_material_symbol_name;
    }

	for (auto _child = pXNode->first_node(); _child != nullptr; _child = _child->next_sibling())
	{
		auto _node_name = _get_node_name(_child);

#ifdef DEBUG
		//logger.write(_node_name);
#endif

		if (_node_name == "input")
		{
			std::string _source_str, _offset_str, _semantic_str;

			_get_node_attribute_value(_child, "source", _source_str);
			_get_node_attribute_value(_child, "offset", _offset_str);
			_get_node_attribute_value(_child, "semantic", _semantic_str);

			if (_source_str != "" && _offset_str != "" && _semantic_str != "")
			{
				if (_source_str[0] == '#') _source_str = _source_str.erase(0, 1);
				int _offset_val = atoi(_offset_str.c_str());

				if (pGeometry.vertices->id != _source_str)
				{
					/*
						If we already added an offset index for any other semantic, then skip this semantic
						DAE without UV which has exported from Maya cause this problem
					*/
					for (size_t i = 0; i < _triangles->semantics.size(); ++i)
					{
						if (_triangles->semantics[i]->offset == _offset_val)
						{
							_offset_val = -1;
							break;
						}
					}

					if (_offset_val != -1)
					{
						auto _c_semantic = new c_semantic();
						if (_semantic_str[0] == '#') _semantic_str = _semantic_str.erase(0, 1);
						_c_semantic->offset = (int)_triangles->semantics.size();
						_c_semantic->source = _source_str;
						_c_semantic->semantic = _semantic_str;

						_triangles->semantics.push_back(_c_semantic);
					}
				}
				else
				{
					_triangles->semantics = pGeometry.vertices->semantics;
				}
			}
		}
		else if (_node_name == "p")
		{
            //wolf::system::convert::find_all_numbers_then_convert_to<float>(_float_array_str, _source->float_array);
			wolf::system::convert::find_all_numbers_then_convert_to<uint32_t>(_child->value(), _triangles->indices);
		}
	}

	pGeometry.triangles.push_back(_triangles);
}

W_RESULT c_parser::_create_scene(
    _Inout_ w_cpipeline_scene* pScene,
    _In_ const bool& pAMDTootleOptimizing,
#ifdef __WIN32
	_In_ const bool& pSimplygonOptimizing,
#endif
    _In_ const bool& pInvertNormals, 
    _In_ const bool& pFind_LODs_ConvexHulls_Boundaries)
{
    std::vector<c_node*> _mesh_with_unknown_instance_ref;
    std::vector<w_cpipeline_model*> _models;

    _iterate_over_nodes(
        pAMDTootleOptimizing,
#ifdef __WIN32
		pSimplygonOptimizing,
#endif
        pInvertNormals,
        sNodes,
        _models,
        _mesh_with_unknown_instance_ref);
    
    //if we have nodes without unknown instance ref
    if (_mesh_with_unknown_instance_ref.size())
    {
        for (auto pNode : _mesh_with_unknown_instance_ref)
        {
            w_cpipeline_model* _model = nullptr;
            _create_model(
                pAMDTootleOptimizing,
#ifdef __WIN32
				pSimplygonOptimizing,
#endif
                pInvertNormals,
                &pNode,
                &_model);
            if (_model)
            {
                _models.push_back(_model);
            }
        }
    }

	//creating animation container
	//auto _animation_container = new c_animation_container();
	//_animation_container->xsi_extra = sXSI_Extra;

	////Check bones
	//if (sBones.size() > 0)
	//{

	//}

	//add camera if avaiable
	//std::for_each(sNodes.begin(), sNodes.end(), [pScene](c_node* pNode)
	//{
	//	//we will find #instance_camera and the parent node contains camera information and the next child node is camera interest
	//	if (pNode)
	//	{
	//		bool _found = false;
 //           
 //           glm::vec3 _camera_transform;
 //           glm::vec3 _camera_interest;
 //           
	//		if (!pNode->instanced_camera_name.empty())
	//		{
 //               //TODO : we need to check instanced camera
	//			pScene->add_camera(pNode->c_name, pNode->translate, _camera_interest);
	//			pNode->proceeded = true;
	//			_found = true;
	//		}
	//		else
	//		{
	//			//Serach childs
	//			auto _size = pNode->child_nodes.size();
	//			for (size_t i = 0; i < _size; ++i)
	//			{
	//				auto pInnerNode = pNode->child_nodes[i];
	//				if (pInnerNode && !pInnerNode->instanced_camera_name.empty())
	//				{
	//					_found = true;
	//					break;
	//				}
	//			}
	//		}

	//		if (_found)
	//		{
	//			pNode->proceeded = true;
 //               
 //               //the first one is camera and the second one is camera interest
 //               if (pNode->child_nodes.size() == 2)
 //               {
 //                   _camera_transform = pNode->child_nodes[0]->translate;
 //                   _camera_interest = pNode->child_nodes[1]->translate;
 //               }
 //               else
 //               {
 //                   _camera_transform = pNode->translate;
 //               }
 //               
	//			pScene->add_camera(pNode->c_name, _camera_transform, _camera_interest);
	//		}
	//	}
	//});

	//add models
	//for (auto _geometry : _base_geometeries)
	//{
	//	c_skin* skin = nullptr;
	//	//if (s.Count != 0)
	//	//{
	//	//	skin = s[i];
	//	//}

	//	auto _model = w_cpipeline_model::create_model(_geometry, skin, sBones, sSkeletonNames.data(), sMaterials, sNodes, pOptimizePoints);
	//	//_model->set_effects(effects);
	//	////_model.Textures = textureInfos;
	//	////_model.Initialize(dir);
	//	////_model.SceneName = sceneId;
	//	////_model.Name = geometry.Name;
	//	////_model.AnimationContainers.Add("Animation 1", animContainer);
	//	////_model.SetAnimation("Animation 1");
	//	
	//	pScene->add_model(_model);
	//}

    //find LODS & convex hulls indices
    std::vector<size_t> _roots;
    if (pFind_LODs_ConvexHulls_Boundaries)
    {
        std::vector<size_t> _lods;
        std::vector<size_t> _chs;
        std::vector<w_bounding_sphere*> _bounds;

        for (size_t i = 0; i < _models.size(); i++)
        {
            auto _model = _models[i];
            if (_model)
            {
                auto _name = _model->get_name();
                //to lower
                std::transform(_name.begin(), _name.end(), _name.begin(), ::tolower);
                if (strstr(_name.c_str(), "-lod") != NULL)
                {
                    _lods.push_back(i);
                }
                else if (strstr(_name.c_str(), "-ch") != NULL)
                {
                    _chs.push_back(i);
                }
                else if (_name == "inner_boundary" || _name == "middle_boundary" || _name == "outer_boundary")
                {
                    //get root bounding box
                    auto _bb = _model->get_bounding_box(0);

                    auto _bs = new w_bounding_sphere();
                    _bs->create_from_bounding_box(*_bb);

                    _bs->center[0] = _bb->position[0];
                    _bs->center[1] = _bb->position[1];
                    _bs->center[2] = _bb->position[2];

                    _bounds.push_back(_bs);
                }
                else
                {
                    _roots.push_back(i);
                }
            }
        }

        std::vector<std::string> _splits;
        for (size_t i = 0; i < _roots.size(); ++i)
        {
            auto _index = _roots[i];
            auto _name = _models[_index]->get_name();

            std::vector<int> _index_lods, _index_chs, _index_bounds;
            wolf::system::convert::split_string(_name, "_", _splits);
            if (_splits.size() > 0)
            {
                auto _design_name = _splits[0];

                _splits.clear();
                wolf::system::convert::split_string(_design_name, "-", _splits);
                if (_splits.size())
                {
                    _design_name = _splits[0];
                }

                int _i = 0;
                _lods.erase(std::remove_if(_lods.begin(), _lods.end(),
                    [_design_name, _models, &_index_lods](_In_ size_t pIter)
                {
                    auto __name = _models[pIter]->get_name();
                    if (wolf::system::convert::has_string_start_with(__name, _design_name))
                    {
                        _index_lods.push_back(pIter);
                        return true;
                    }

                    return false;

                }), _lods.end());

                _chs.erase(std::remove_if(_chs.begin(), _chs.end(),
                    [_design_name, _models, &_index_chs](_In_ size_t pIter)
                {
                    auto __name = _models[pIter]->get_name();
                    if (wolf::system::convert::has_string_start_with(__name, _design_name))
                    {
                        _index_chs.push_back(pIter);
                        return true;
                    }

                    return false;

                }), _chs.end());
            }

            //sort lods by name
            std::sort(_index_lods.begin(), _index_lods.end(), [&_models](_In_ int const& a, _In_ int const& b)
            {
                return (_models[a]->get_name().compare(_models[b]->get_name()) < 0);
            });

            std::vector<w_cpipeline_model*> _lods_models;
            for (auto& _index : _index_lods)
            {
                auto _m = _models[_index];
                if (!_m) continue;
                _lods_models.push_back(_m);
            }
            std::vector<w_cpipeline_model*> _convex_hulls;
            for (auto& _index : _index_chs)
            {
                auto _m = _models[_index];
                if (!_m) continue;
                _convex_hulls.push_back(_m);
            }

            /*if (_lods_models.size())
            {
                _models[_index]->add_lods(_lods_models);
                _lods_models.clear();
            }

            if (_convex_hulls.size())
            {
                _models[_index]->add_convex_hulls(_convex_hulls);
                _convex_hulls.clear();
            }*/

            _splits.clear();
        }

        if (_lods.size()) _lods.clear();
        if (_chs.size()) _chs.clear();

        if (_bounds.size())
        {
            //sort boundaries
            std::sort(_bounds.begin(), _bounds.end(), [](_In_ w_bounding_sphere* a, _In_ w_bounding_sphere* b)
            {
                return a->radius < b->radius;
            });

            pScene->add_boundaries(_bounds);
            _bounds.clear();
        }
    }

    if (_models.size())
    {
        if (pFind_LODs_ConvexHulls_Boundaries)
        {
            std::vector<w_cpipeline_model*> _root_models;
            for (auto& _index : _roots)
            {
                _root_models.push_back(_models[_index]);
            }

            //make sure root is not -ins
            //for (auto& _ref : _root_models)
            //{
            //    if (!_ref->get_instances_count()) continue;

            //    auto _name = _ref->get_name();
            //    if (_name.find("-ins"))
            //    {
            //        std::vector<w_instance_info> _instances;
            //        _ref->get_instances(_instances);
            //        for (auto& _ins : _instances)
            //        {
            //            if (_ins.name.find("-ins") == -1)
            //            {
            //                auto _ref_t = &_ref->get_transform();

            //                auto _ins_p = &_ins.position;
            //                auto _ins_r = &_ins.rotation;

            //                std::swap(_ref_t->position, *_ins_p);
            //                std::swap(_ref_t->rotation, *_ins_r);

            //                break;
            //            }
            //        }
            //    }
            //}

            pScene->add_models(_root_models);
            _root_models.clear();
            _roots.clear();
        }
        else
        {
            pScene->add_models(_models);
        }
        _models.clear();
    }

    if (sLibraryCameras.size())
    {
        for (auto _iter : sLibraryCameras)
        {
//            auto _camera = new c_camera(_iter.second);
//            if (sZ_Up)
//            {
//                //make transform change
//                auto _cam_pos = _camera->get_translate();
//                auto _cam_look_at = _camera->get_interest();
//
//                std::swap(_cam_pos.y, _cam_pos.z);
//                _cam_pos.y *= -1;
//                _cam_pos.z *= -1;
//
//                std::swap(_cam_look_at.y, _cam_look_at.z);
//                _cam_look_at.y *= -1;
//                _cam_look_at.z *= -1;
//
//                _camera->set_translate(_cam_pos);
//                _camera->set_interest(_cam_look_at);
//            }
//            pScene->add_camera(_camera);
        }
    }
    
	return W_PASSED;
}

void c_parser::_iterate_over_nodes(
    _In_ const bool& pAMDTootleOptimizing,
#ifdef __WIN32
	_In_ const bool& pSimplygonOptimizing,
#endif
    _In_ const bool& pInvertNormals, 
    _Inout_ std::vector<c_node*> pNodes, 
    _Inout_ std::vector<w_cpipeline_model*>& pModels,
    _Inout_ std::vector<c_node*>& pMeshWithUnknownInstanceRef)
{
    for (auto _node : pNodes)
    {
        if (!_node || _node->proceeded) continue;

        auto _type = static_cast<c_node_type>(_node->type);
        if (_type == c_node_type::CAMERA)
        {
            //find camera
            auto _iter = sLibraryCameras.find(_node->instanced_camera_name);
            if (_iter != sLibraryCameras.end())
            {
//                _iter->second.set_translate(_node->translate);
//                //find target node
//                for (auto __node : pNodes)
//                {
//                    if (__node->c_name == _iter->second.get_camera_target_name())
//                    {
//                        _iter->second.set_interest(__node->translate);
//                        __node->proceeded = true;
//                    }
//                }

                _node->proceeded = true;
            }
        }
        else if (_type == c_node_type::MESH)
        {
            //find instance if avaiable in models
            auto _iter = std::find_if(pModels.begin(), pModels.end(), [_node](_In_ w_cpipeline_model* pModel)
            {
                return pModel->get_instance_geometry_name() == _node->instanced_geometry_name;
            });

            if (_iter != pModels.end())
            {
                //we find source model
                w_instance_info _instance_info;
                _instance_info.name = _node->c_name;

                auto _rotation = glm::rotation_from_angle_axis(_node->rotation.x, _node->rotation.y, _node->rotation.z, _node->rotation.w);
                
                if (sZ_Up)
                {
					_instance_info.position[0] = _node->translate.x;
					_instance_info.position[1] = _node->translate.z;
					_instance_info.position[2] = _node->translate.y;

					_instance_info.rotation[0] = -_rotation.x;
					_instance_info.rotation[1] = -_rotation.z;
					_instance_info.rotation[2] = -_rotation.y;

					_instance_info.scale[0] = _node->scale.x;
					_instance_info.scale[1] = _node->scale.z;
					_instance_info.scale[2] = _node->scale.y;

                }
                else
                {
                    _instance_info.position[0] = _node->translate.x;
                    _instance_info.position[1] = _node->translate.y;
                    _instance_info.position[2] = _node->translate.z;

                    _instance_info.rotation[0] = _rotation.x - glm::radians(90.0f);
                    _instance_info.rotation[1] = _rotation.y;
                    _instance_info.rotation[2] = _rotation.z;

					_instance_info.scale[0] = _node->scale.x;
					_instance_info.scale[1] = _node->scale.y;
					_instance_info.scale[2] = _node->scale.z;
                }

                (*_iter)->add_instance(_instance_info);
                _node->proceeded = true;
            }
            else
            {
                w_cpipeline_model* _model = nullptr;
                _create_model(
                    pAMDTootleOptimizing,
#ifdef __WIN32
					pSimplygonOptimizing,
#endif
                    pInvertNormals,
                    &_node,
                    &_model);
                //if node procceded and model created
                if (_model)
                {
                    pModels.push_back(_model);
                }
                else
                {
                    pMeshWithUnknownInstanceRef.push_back(_node);
                }
                //iterate over sub models
                if (_node->child_nodes.size())
                {
                    if (_node->type == c_node_type::MESH)
                    {
                        //pass the root name to the childs
                        for (size_t i = 0; i < _node->child_nodes.size(); ++i)
                        {
                            _node->child_nodes[i]->c_name = _node->c_name;
                        }
                    }
                    _iterate_over_nodes(
                        pAMDTootleOptimizing,
#ifdef __WIN32
						pSimplygonOptimizing,
#endif
                        pInvertNormals,
                        _node->child_nodes,
                        pModels,
                        pMeshWithUnknownInstanceRef);
                }
            }
        }
    }
}

void c_parser::_create_model(
    _In_ const bool& pAMDTootleOptimizing,
#ifdef __WIN32
	_In_ const bool& pSimplygonOptimizing,
#endif
    _In_ const bool& pInvertNormals,
    _Inout_ c_node** pNode,
    _Inout_ w_cpipeline_model** pModel)
{
    //Loading geometries
    c_geometry _g;
    bool _found_geometry = false;
    auto _node_ptr = *pNode;
    for (auto _child = SGeometryLibraryNode->first_node();
        _child != nullptr;
        _child = _child->next_sibling())
    {
        string _id;
        _get_node_attribute_value(_child, "id", _id);
        if (_node_ptr->instanced_geometry_name == _id)
        {
            _found_geometry = true;
            _node_ptr->proceeded = true;
            _get_node_attribute_value(_child, "id", _g.id);
            _get_node_attribute_value(_child, "name", _g.name);

            for (auto __child = _child->first_node(); __child != nullptr; __child = __child->next_sibling())
            {
                auto _node_name = _get_node_name(__child);
#ifdef DEBUG
                //logger.write(_node_name);
#endif

                if (_node_name == "mesh")
                {
#pragma region read mesh data

                    for (auto ___child = __child->first_node(); ___child != nullptr; ___child = ___child->next_sibling())
                    {
                        std::string _name = ___child->name();

                        std::string __id, __name;
                        _get_node_attribute_value(___child, "id", __id);
                        _get_node_attribute_value(___child, "name", __name);

#ifdef DEBUG
                        //logger.write(_name);
#endif

                        if (_name == "source")
                        {
                            _get_sources(___child, __id, __name, _g);
                        }
                        else if (_name == "vertices")
                        {
                            _g.vertices = new c_vertices();
                            _get_node_attribute_value(___child, "id", _g.vertices->id);

                            _get_vertices(___child, _g);
                        }
                        else if (_name == "triangles")
                        {
                            _get_triangles(___child, *pNode, _g);
                        }
                    }
#pragma endregion
                }
            }
            break;
        }
    }

    if (_found_geometry)
    {
        //create model from geometry
        c_skin* skin = nullptr;
        //if (s.Count != 0)
        //{
        //	skin = s[i];
        //}

        //set transform
        w_transform_info _transform;
        auto _rotation = glm::rotation_from_angle_axis(_node_ptr->rotation.x, _node_ptr->rotation.y, _node_ptr->rotation.z, _node_ptr->rotation.w);

        if (sZ_Up)
        {
            _transform.position[0] = _node_ptr->translate.x;
            _transform.position[1] = _node_ptr->translate.z;
            _transform.position[2] = _node_ptr->translate.y;

			_transform.rotation[0] = -_rotation.x;
            _transform.rotation[1] = -_rotation.z;
            _transform.rotation[2] = -_rotation.y;

			_transform.scale[0] = _node_ptr->scale.x;
			_transform.scale[1] = _node_ptr->scale.z;
			_transform.scale[2] = _node_ptr->scale.y;
        }
        else
        {
            _transform.position[0] = _node_ptr->translate.x;
            _transform.position[1] = _node_ptr->translate.y;
            _transform.position[2] = _node_ptr->translate.z;

            _transform.rotation[0] = _rotation.x - glm::radians(90.0f);
            _transform.rotation[1] = _rotation.y;
            _transform.rotation[2] = _rotation.z;

			_transform.scale[0] = _node_ptr->scale.x;
			_transform.scale[1] = _node_ptr->scale.z;
			_transform.scale[2] = _node_ptr->scale.y;
        }

        _transform.transform = _node_ptr->transform;

//#ifdef DEBUG
        logger.write("creating model {}", _node_ptr->c_name);
//#endif

//        auto _model = w_cpipeline_model::create_model(
//            _g,
//            skin,
//            sBones,
//            sSkeletonNames.data(),
//            sLibraryMaterials,
//            sLibraryEffects,
//            sLibraryImages,
//            _transform,
//            pAMDTootleOptimizing,
//#ifdef __WIN32
//            pSimplygonOptimizing,
//#endif
//            sZ_Up,
//            pInvertNormals);
//
//        _model->set_name(_node_ptr->c_name);
//        _model->set_instance_geometry_name(_node_ptr->instanced_geometry_name);

        //_model->set_effects(effects);
        ////_model.Textures = textureInfos;
        ////_model.Initialize(dir);
        ////_model.SceneName = sceneId;
        ////_model.Name = geometry.Name;
        ////_model.AnimationContainers.Add("Animation 1", animContainer);
        ////_model.SetAnimation("Animation 1");

       // *pModel = _model;
    }
}

void c_parser::_clear_all_resources()
{
	sSceneID = "";
	sSkipChildrenOfThisNode = "NULL";
	
    sSkeletonNames.clear();
	
    sLibraryCameras.clear();
    sLibraryEffects.clear();
    sLibraryMaterials.clear();
    sLibraryImages.clear();

	if (sBones.size() > 0)
	{
		std::for_each(sBones.begin(), sBones.end(), [](c_bone* pBone)
		{
			pBone->release();
		});
	}
	if (sNodes.size() > 0)
	{
		std::for_each(sNodes.begin(), sNodes.end(), [](c_node* pNode)
		{
			pNode->release();
		});
	}
}
