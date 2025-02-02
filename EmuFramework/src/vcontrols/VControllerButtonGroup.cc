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

#define LOGTAG "VControllerButton"
#include <emuframework/VController.hh>
#include <emuframework/EmuApp.hh>
#include <imagine/base/Window.hh>
#include <imagine/gfx/RendererCommands.hh>
#include <imagine/gfx/BasicEffect.hh>
#include <imagine/gui/View.hh>
#include <imagine/util/math/int.hh>
#include <imagine/logger/logger.h>

namespace EmuEx
{

VControllerButtonGroup::VControllerButtonGroup(std::span<const unsigned> buttonCodes, _2DOrigin layoutOrigin, int8_t rowItems):
	layout
	{
		.rowItems = rowItems,
		.origin = layoutOrigin,
	}
{
	buttons.reserve(buttonCodes.size());
	for(auto c : buttonCodes)
	{
		buttons.emplace_back(c);
	}
}

VControllerButtonGroup::VControllerButtonGroup(const Config &conf):
	layout{conf.layout}
{
	buttons.reserve(conf.keys.size());
	for(auto c : conf.keys)
	{
		buttons.emplace_back(c);
	}
}

VControllerButtonGroup::Config VControllerButtonGroup::config() const
{
	Config conf{.layout = layout};
	conf.keys.reserve(buttons.size());
	for(const auto &b : buttons) { conf.keys.emplace_back(b.key); }
	return conf;
}

static int buttonsToLayout(const auto &buttons)
{
	int count{};
	for(const auto &b : buttons)
	{
		if(b.skipLayout || !b.enabled)
			continue;
		count++;
	}
	return count;
}

static void layoutButtons(auto &buttons, WRect layoutBounds, WRect viewBounds, int size,
	int spacing, int stagger, int rowShift, int rowItems, _2DOrigin o)
{
	if(!rowItems)
		return;
	int rows = divRoundUp(buttonsToLayout(buttons), rowItems);
	int row{}, btnPos{}, y{o.yScaler() == -1 ? -size : 0};
	if(stagger < 0)
		y += stagger * (rowItems - 1);
	int x = -rowShift * (rows - 1);
	int staggerOffset = 0;
	for(auto &b : buttons)
	{
		if(b.skipLayout || !b.enabled)
			continue;
		WP pos = layoutBounds.pos(o) + WP{x, y + staggerOffset} + (size / 2);
		b.setPos(pos, viewBounds);
		x += size + spacing;
		staggerOffset -= stagger;
		if(++btnPos == rowItems)
		{
			row++;
			y += (size + spacing) * o.yScaler();
			staggerOffset = 0;
			x = -rowShift * ((rows - 1) - row);
			btnPos = 0;
		}
	}
}

void VControllerButtonGroup::setPos(WP pos, WindowRect viewBounds)
{
	bounds_.setPos(pos, C2DO);
	bounds_.fitIn(viewBounds);
	layoutButtons(buttons, bounds_, viewBounds, btnSize,
		spacingPixels, btnStagger, btnRowShift, layout.rowItems, LB2DO);
}

void VControllerButtonGroup::setButtonSize(int sizePx)
{
	btnSize = sizePx;
	setStaggerType(layout.staggerType);
	int btnsPerRow = std::min(buttonsToLayout(buttons), int(layout.rowItems));
	int xSizePixel = sizePx * btnsPerRow + spacingPixels*(btnsPerRow-1) + std::abs(btnRowShift*((int)rows()-1));
	int ySizePixel = sizePx * rows() + spacingPixels*(rows()-1) + std::abs(btnStagger*((int)btnsPerRow-1));
	bounds_ = makeWindowRectRel({0, 0}, {xSizePixel, ySizePixel});
	WP extendedSize = paddingPixels();
	for(auto &b : buttons)
	{
		b.setSize({sizePx, sizePx}, extendedSize);
	}
}

void VControllerButtonGroup::setStaggerType(uint8_t type)
{
	layout.staggerType = type;
	btnRowShift = 0;
	switch(type)
	{
		case 0:
			btnStagger = btnSize * -.75f; break;
		case 1:
			btnStagger = btnSize * -.5f; break;
		case 2:
			btnStagger = 0; break;
		case 3:
			btnStagger = btnSize * .5f; break;
		case 4:
			btnStagger = btnSize * .75f; break;
		default:
			btnStagger = btnSize + spacingPixels;
			btnRowShift = -(btnSize + spacingPixels);
			break;
	}
}

bool VControllerButtonGroup::setSpacing(int8_t space, const Window &win)
{
	if(space < 0 || space > 8)
		return false;
	layout.spacingMM = space;
	spacingPixels = makeEvenRoundedUp(win.widthMMInPixels(space));
	setStaggerType(layout.staggerType);
	return true;
}

void VControllerButtonGroup::updateMeasurements(const Window &win)
{
	setSpacing(layout.spacingMM, win);
	setButtonSize(btnSize);
}

void VControllerButtonGroup::transposeKeysForPlayer(const EmuApp &app, int player)
{
	for(auto &b : buttons)
	{
		b.key = app.transposeKeyForPlayer(b.key, player);
	}
}

int VControllerButtonGroup::rows() const
{
	assert(layout.rowItems);
	return divRoundUp(buttonsToLayout(buttons), layout.rowItems);
}

std::array<int, 2> VControllerButtonGroup::findButtonIndices(WP windowPos) const
{
	std::array<int, 2> btnOut{-1, -1};
	for(size_t count = 0; auto &b : buttons)
	{
		if(b.overlaps(windowPos))
		{
			btnOut[count++] = b.key;
			if(count == btnOut.size())
				break;
		}
	}
	return btnOut;
}

void VControllerButtonGroup::draw(Gfx::RendererCommands &__restrict__ cmds, float alpha) const
{
	auto &basicEffect = cmds.basicEffect();
	if(layout.showBoundingArea)
	{
		basicEffect.disableTexture(cmds);
		for(const auto &b : buttons)
		{
			if(!b.enabled)
				continue;
			b.drawBounds(cmds, alpha);
		}
	}
	drawButtons(cmds, alpha);
}

void VControllerButtonGroup::drawButtons(Gfx::RendererCommands &__restrict__ cmds, float alpha) const
{
	cmds.basicEffect().enableTexture(cmds);
	for(auto &b : buttons)
	{
		if(!b.enabled)
			continue;
		b.drawSprite(cmds, alpha);
	}
}

static std::string namesString(auto &buttons, const EmuApp &app)
{
	if(buttons.empty())
		return "Empty Group";
	std::string s;
	for(const auto &b : buttons | std::ranges::views::take(buttons.size() - 1))
	{
		s += b.name(app);
		s += " | ";
	}
	s += buttons.back().name(app);
	return s;
}

std::string VControllerButtonGroup::name(const EmuApp &app) const
{
	return namesString(buttons, app);
}

static bool isValidRowItemCount(int val) { return val >= 1 && val <= 5; }
static bool isValidSpacing(int val) { return val >= 0 && val <= 8; }
static bool isValidPadding(int val) { return val >= 0 && val <= 30; }
static bool isValidStaggerType(int val) { return val >= 0 && val <= 5; }

void VControllerButtonGroup::Config::validate(const EmuApp &app)
{
	for(auto &k : keys) { k =	app.validateSystemKey(k, false); }
	if(!isValidRowItemCount(layout.rowItems))
		layout.rowItems = 2;
	if(!isValidSpacing(layout.spacingMM))
		layout.spacingMM = defaultButtonSpacingMM;
	if(!isValidPadding(layout.xPadding))
		layout.xPadding = 0;
	if(!isValidPadding(layout.yPadding))
		layout.yPadding = 0;
	if(!isValidStaggerType(layout.staggerType))
		layout.staggerType = 2;
}

VControllerUIButtonGroup::VControllerUIButtonGroup(std::span<const unsigned> buttonCodes, _2DOrigin layoutOrigin):
	layout
	{
		.rowItems = 4,
		.origin = layoutOrigin,
	}
{
	buttons.reserve(buttonCodes.size());
	for(auto c : buttonCodes)
	{
		buttons.emplace_back(c);
	}
}

VControllerUIButtonGroup::VControllerUIButtonGroup(const Config &conf):
	layout{conf.layout}
{
	buttons.reserve(conf.keys.size());
	for(auto c : conf.keys) { buttons.emplace_back(c); }
}

VControllerUIButtonGroup::Config VControllerUIButtonGroup::config() const
{
	Config conf{.layout = layout};
	conf.keys.reserve(buttons.size());
	for(const auto &b : buttons) { conf.keys.emplace_back(b.key); }
	return conf;
}

void VControllerUIButtonGroup::setPos(WP pos, WindowRect viewBounds)
{
	bounds_.setPos(pos, C2DO);
	bounds_.fitIn(viewBounds);
	layoutButtons(buttons, bounds_, viewBounds, btnSize,
		0, 0, 0, layout.rowItems, LT2DO);
}

void VControllerUIButtonGroup::setButtonSize(int sizePx)
{
	btnSize = sizePx;
	int btnsPerRow = std::min(buttonsToLayout(buttons), int(layout.rowItems));
	int xSizePixel = sizePx * btnsPerRow;
	int ySizePixel = sizePx * rows();
	bounds_ = makeWindowRectRel({0, 0}, {xSizePixel, ySizePixel});
	for(auto &b : buttons)
	{
		b.setSize({sizePx, sizePx});
	}
}

int VControllerUIButtonGroup::rows() const
{
	assert(layout.rowItems);
	return divRoundUp(buttonsToLayout(buttons), layout.rowItems);
}

void VControllerUIButtonGroup::draw(Gfx::RendererCommands &__restrict__ cmds, float alpha) const
{
	cmds.basicEffect().enableTexture(cmds);
	for(auto &b : buttons)
	{
		if(!b.enabled)
			continue;
		b.drawSprite(cmds, alpha);
	}
}

std::string VControllerUIButtonGroup::name(const EmuApp &app) const
{
	return namesString(buttons, app);
}

void VControllerUIButtonGroup::Config::validate(const EmuApp &app)
{
	for(auto &k : keys) { k =	app.validateSystemKey(k, true); }
	if(!isValidRowItemCount(layout.rowItems))
		layout.rowItems = 2;
}

}
