#pragma once

/*  This file is part of EmuFramework.

	Imagine is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Imagine is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with EmuFramework.  If not, see <http://www.gnu.org/licenses/> */

#include <imagine/util/Interpolator.hh>
#include <imagine/base/Timer.hh>
#include <imagine/gui/View.hh>
#include <imagine/gfx/GfxText.hh>
#include <imagine/input/DragTracker.hh>
#include <emuframework/EmuAppHelper.hh>

namespace EmuEx
{

class VController;
class VControllerElement;

using namespace IG;

class PlaceVControlsView final: public View, public EmuAppHelper<PlaceVControlsView>
{
public:
	PlaceVControlsView(ViewAttachParams attach, VController &vController);
	~PlaceVControlsView() final;
	void place() final;
	bool inputEvent(const Input::Event &e) final;
	void draw(Gfx::RendererCommands &__restrict__ cmds) final;

private:
	struct DragData
	{
		VControllerElement *elem{};
		WP startPos{};
	};
	Gfx::Text text;
	VController &vController;
	InterpolatorValue<float, IG::FrameTime, IG::InterpolatorType::LINEAR> textFade{};
	Timer animationStartTimer{"PlaceVControlsView::animationStartTimer"};
	OnFrameDelegate animate;
	WRect exitBtnRect{};
	Input::DragTracker<DragData> dragTracker;
};

}
