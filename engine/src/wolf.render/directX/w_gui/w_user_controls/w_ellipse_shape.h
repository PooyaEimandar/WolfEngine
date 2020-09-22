/*
	Project			 : Wolf Engine. Copyright(c) Pooya Eimandar (http://PooyaEimandar.com) . All rights reserved.
	Source			 : Please direct any bug to https://github.com/PooyaEimandar/Wolf.Engine/issues
	Website			 : http://WolfSource.io
	Name			 : w_ellipse_shape.h
	Description		 : The ellipse shape control

	Comment          :
*/

#ifndef __W_ELLIPSE_SHAPE__
#define __W_ELLIPSE_SHAPE__

#include "w_control.h"
#include <w_graphics/w_direct2D/w_ellipse.h>
#include <w_color.h>

namespace wolf
{
	namespace gui
	{
		class w_ellipse_shape : public w_control
		{
		public:
			DX_EXP w_ellipse_shape(_In_opt_ w_widget* pParent);
			DX_EXP ~w_ellipse_shape();

			virtual HRESULT									on_initialize(const std::shared_ptr<wolf::graphics::w_graphics_device>& pGDevice) override;
			virtual bool									contains_point(_In_ const POINT& pt) override;
			virtual bool									can_have_focus() override;
			virtual void									render(_In_ const std::shared_ptr<wolf::graphics::w_graphics_device>& pGDevice, _In_ float pElapsedTime) override;
			virtual ULONG									release() override;

#pragma region Getter
			
			DX_EXP w_color									get_fill_color() const							{ return this->fill_color; }
			DX_EXP w_color									get_border_color() const						{ return this->border_color; }
			DX_EXP w_color									get_mouse_over_color() const					{ return this->mouse_over_color; }
			DX_EXP w_color									get_disabled_color() const						{ return this->disabled_color; }
			DX_EXP float									get_centerX() const;
			DX_EXP float									get_centerY() const;
			DX_EXP float									get_radiusX() const;
			DX_EXP float									get_radiusY() const;
			DX_EXP float									get_stroke_width() const						{ return this->stroke_width; }

#pragma endregion

#pragma region Setter

			DX_EXP void										set_fill_color(_In_ const w_color pValue)		{ this->fill_color = pValue; }
			DX_EXP void										set_border_color(_In_ const w_color pValue)		{ this->border_color = pValue; }
			DX_EXP void										set_mouse_over_color(_In_ const w_color pValue)	{ this->mouse_over_color = pValue; }
			DX_EXP void										set_disabled_color(_In_ const w_color pValue)	{ this->disabled_color = pValue; }
			DX_EXP void										set_geormetry(_In_ const float pCenterX, _In_ const float pCenterY,
				_In_ float const pRadiusX, _In_ const float pRadiusY);
			DX_EXP void										set_stroke_width(_In_ const float pValue);

#pragma endregion

		protected:
			
			w_color											fill_color;
			w_color											border_color;
			w_color											mouse_over_color;
			w_color											disabled_color;
			float											stroke_width;

		private:
			typedef w_control	_super;

			wolf::graphics::direct2D::shapes::w_ellipse*	_ellipse;
			
		};
	}
}

#endif
