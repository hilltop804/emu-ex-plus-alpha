/*  This file is part of Imagine.

	Imagine is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Imagine is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Imagine.  If not, see <http://www.gnu.org/licenses/> */

#pragma once
#include <emuframework/config.hh>
#include <emuframework/EmuAppHelper.hh>
#include <imagine/input/Input.hh>
#include <imagine/input/DragTracker.hh>
#include <imagine/gfx/GfxSprite.hh>
#include <imagine/gfx/Texture.hh>
#include <imagine/util/variant.hh>
#include <vector>
#include <span>
#include <optional>

namespace IG
{
class Window;
class ApplicationContext;
class FileIO;
class MapIO;
}

namespace EmuEx
{

using namespace IG;
class VController;
class EmuApp;
class EmuViewController;
struct SystemInputDeviceDesc;
struct InputComponentDesc;
enum class InputComponent : uint8_t;
enum class AltSpeedMode;
struct WindowData;

enum class VControllerState : uint8_t
{
	OFF, SHOWN, HIDDEN
};

struct VControllerLayoutPosition
{
	SP pos{-1, -1};
	_2DOrigin origin{LT2DO};

	constexpr VControllerLayoutPosition() = default;
	constexpr VControllerLayoutPosition(_2DOrigin origin, WP pos):
		pos{pos.as<int16_t>()}, origin{origin} {}

	static VControllerLayoutPosition fromPixelPos(WP pos, WP size, WindowRect viewBounds);
	WP toPixelPos(WindowRect viewBounds) const;
};

constexpr int16_t defaultDPadDeadzoneMM100x = 135;
constexpr float defaultDPadDiagonalSensitivity = .57f;
constexpr int8_t defaultButtonSpacingMM = 2;

class VControllerDPad
{
public:
	struct Config
	{
		std::array<unsigned, 4> keys{};
		float diagonalSensitivity{defaultDPadDiagonalSensitivity};
		int16_t deadzoneMM100x{defaultDPadDeadzoneMM100x};
		bool visualizeBounds{};

		void validate(const EmuApp &);
	};

	constexpr VControllerDPad() = default;
	constexpr VControllerDPad(std::span<const unsigned, 4> keys):
		config{.keys{keys[0], keys[1], keys[2], keys[3]}} {}
	constexpr VControllerDPad(const Config &config): config{config} {}
	void setImage(Gfx::TextureSpan);
	void draw(Gfx::RendererCommands &__restrict__, float alpha) const;
	void setShowBounds(Gfx::Renderer &r, bool on);
	bool showBounds() const { return config.visualizeBounds; }
	std::array<int, 2> getInput(WP c) const;
	WRect bounds() const { return padBaseArea; }
	WRect realBounds() const { return padArea; }
	void setPos(WP pos, WindowRect viewBounds);
	void setSize(Gfx::Renderer &, int sizeInPixels);
	bool setDeadzone(Gfx::Renderer &, int newDeadzone, const Window &);
	auto deadzone() const { return config.deadzoneMM100x; }
	bool setDiagonalSensitivity(Gfx::Renderer &, float newDiagonalSensitivity);
	auto diagonalSensitivity() const { return config.diagonalSensitivity; }
	std::string name(const EmuApp &) const { return "D-Pad"; }
	void updateMeasurements(const Window &win);
	void transposeKeysForPlayer(const EmuApp &, int player);

	static size_t configSize()
	{
		return sizeof(Config::keys) +
			sizeof(Config::diagonalSensitivity) +
			sizeof(Config::deadzoneMM100x) +
			sizeof(Config::visualizeBounds);
	}

protected:
	Gfx::Sprite spr;
	Gfx::Sprite mapSpr;
	Gfx::Texture mapImg;
	WRect padBaseArea, padArea;
	int deadzonePixels{};
	int btnSizePixels{};
public:
	Config config;

	void updateBoundingAreaGfx(Gfx::Renderer &);
};

enum class VControllerKbMode: uint8_t
{
	LAYOUT_1,
	LAYOUT_2
};

class VControllerKeyboard
{
public:
	static constexpr int VKEY_COLS = 20;
	static constexpr int KEY_ROWS = 4;
	static constexpr int KEY_COLS = VKEY_COLS/2;
	using KeyTable = std::array<std::array<unsigned, VKEY_COLS>, KEY_ROWS>;
	using KbMap = std::array<unsigned, KEY_ROWS * KEY_COLS>;

	constexpr VControllerKeyboard() = default;
	void updateImg(Gfx::Renderer &r);
	void setImg(Gfx::Renderer &r, Gfx::TextureSpan img);
	void place(int btnSize, int yOffset, WRect viewBounds);
	void draw(Gfx::RendererCommands &__restrict__) const;
	int getInput(WP c) const;
	unsigned translateInput(int idx) const;
	bool keyInput(VController &v, Gfx::Renderer &r, const Input::KeyEvent &e);
	[[nodiscard]] WindowRect selectKey(unsigned x, unsigned y);
	void selectKeyRel(int x, int y);
	void unselectKey();
	[[nodiscard]] WindowRect extendKeySelection(WindowRect);
	unsigned currentKey() const;
	unsigned currentKey(int x, int y) const;
	VControllerKbMode mode() const { return mode_; }
	void setMode(EmuSystem &, Gfx::Renderer &, VControllerKbMode mode);
	void cycleMode(EmuSystem &, Gfx::Renderer &);
	void applyMap(KbMap map);
	void updateKeyboardMapping(EmuSystem &);
	void setShiftActive(bool);
	bool toggleShiftActive();
	bool shiftIsActive() const;

protected:
	Gfx::Sprite spr;
	WRect bound;
	int keyXSize{}, keyYSize{};
	WRect selected{{-1, -1}, {-1, -1}};
	WRect shiftRect{{-1, -1}, {-1, -1}};
	float texXEnd{};
	KeyTable table{};
	VControllerKbMode mode_{};
};

class VControllerButton
{
public:
	constexpr VControllerButton(unsigned key): key{key} {}
	void setPos(WP pos, WRect viewBounds, _2DOrigin = C2DO);
	void setSize(WP size, WP extendedSize = {});
	void setImage(Gfx::TextureSpan, int aR = 1);
	WRect bounds() const { return bounds_; }
	WRect realBounds() const { return extendedBounds_; }
	const Gfx::Sprite &sprite() const { return spr; }
	void drawBounds(Gfx::RendererCommands &__restrict__, float alpha) const;
	void drawSprite(Gfx::RendererCommands &__restrict__, float alpha) const;
	std::string name(const EmuApp &) const;
	bool overlaps(WP windowPos) const { return enabled && realBounds().overlaps(windowPos); }

protected:
	Gfx::Sprite spr;
	WRect bounds_{};
	WRect extendedBounds_{};
	int aspectRatio{1};
public:
	Gfx::Color color{};
	unsigned key{};
	bool enabled = true;
	bool skipLayout{};
};

class VControllerButtonGroup
{
public:
	struct LayoutConfig
	{
		int8_t rowItems{1};
		int8_t spacingMM{defaultButtonSpacingMM};
		int8_t xPadding{};
		int8_t yPadding{};
		uint8_t staggerType{2};
		_2DOrigin origin{};
		bool showBoundingArea{};
	};

	struct Config
	{
		std::vector<unsigned> keys{};
		LayoutConfig layout{};

		void validate(const EmuApp &);
	};

	constexpr VControllerButtonGroup() = default;
	VControllerButtonGroup(std::span<const unsigned> buttonCodes, _2DOrigin layoutOrigin, int8_t rowItems);
	VControllerButtonGroup(const Config &);
	Config config() const;
	void setPos(WP pos, WindowRect viewBounds);
	void setButtonSize(int sizePx);
	void setStaggerType(uint8_t);
	auto stagger() const { return layout.staggerType; }
	bool setSpacing(int8_t spacingMM, const Window &);
	auto spacing() const { return layout.spacingMM; }
	WP paddingPixels() const { return {int(spacingPixels + btnSize * (layout.xPadding / 100.f)), int(spacingPixels + btnSize * (layout.yPadding / 100.f))}; }
	bool showsBounds() const { return layout.showBoundingArea; }
	void setShowBounds(bool on) { layout.showBoundingArea = on; }
	auto bounds() const { return bounds_; }
	WRect paddingRect() const { return {{int(-paddingPixels().x), int(-paddingPixels().y)}, {int(paddingPixels().x), int(paddingPixels().y)}}; }
	WRect realBounds() const { return bounds() + paddingRect(); }
	int rows() const;
	std::array<int, 2> findButtonIndices(WP windowPos) const;
	void draw(Gfx::RendererCommands &__restrict__, float alpha) const;
	std::string name(const EmuApp &) const;
	void updateMeasurements(const Window &win);
	void transposeKeysForPlayer(const EmuApp &, int player);

	static size_t layoutConfigSize()
	{
		return sizeof(LayoutConfig::rowItems) +
			sizeof(LayoutConfig::spacingMM) +
			sizeof(LayoutConfig::xPadding) +
			sizeof(LayoutConfig::yPadding) +
			sizeof(LayoutConfig::staggerType) +
			sizeof(_2DOrigin::PackedType) +
			sizeof(LayoutConfig::showBoundingArea);
	}

	size_t configSize() const
	{
		return 1 + buttons.size() * sizeof(unsigned) + layoutConfigSize();
	}

	std::vector<VControllerButton> buttons;
protected:
	WRect bounds_;
	int btnSize{};
	int spacingPixels{};
	int16_t btnStagger{};
	int16_t btnRowShift{};

	void drawButtons(Gfx::RendererCommands &__restrict__, float alpha) const;
public:
	LayoutConfig layout{};
};

class VControllerUIButtonGroup
{
public:
	struct LayoutConfig
	{
		int8_t rowItems{1};
		_2DOrigin origin{};
	};

	struct Config
	{
		std::vector<unsigned> keys{};
		LayoutConfig layout{};

		void validate(const EmuApp &);
	};

	constexpr VControllerUIButtonGroup() = default;
	VControllerUIButtonGroup(std::span<const unsigned> buttonCodes, _2DOrigin layoutOrigin);
	VControllerUIButtonGroup(const Config &);
	Config config() const;
	void setPos(WP pos, WindowRect viewBounds);
	void setButtonSize(int sizePx);
	auto bounds() const { return bounds_; }
	WRect realBounds() const { return bounds(); }
	int rows() const;
	void draw(Gfx::RendererCommands &__restrict__, float alpha) const;
	std::string name(const EmuApp &) const;

	static size_t layoutConfigSize()
	{
		return sizeof(LayoutConfig::rowItems) +
			sizeof(_2DOrigin::PackedType);
	}

	size_t configSize() const
	{
		return 1 + buttons.size() * sizeof(unsigned) + layoutConfigSize();
	}

	std::vector<VControllerButton> buttons;
protected:
	WRect bounds_{};
	int btnSize{};
public:
	LayoutConfig layout{};
};

using VControllerElementVariant = std::variant<VControllerButtonGroup, VControllerUIButtonGroup, VControllerDPad>;

class VControllerElement : public VControllerElementVariant
{
public:
	using VControllerElementVariant::VControllerElementVariant;

	std::array<VControllerLayoutPosition, 2> layoutPos;
	VControllerState state{VControllerState::SHOWN};

	constexpr auto dPad() { return std::get_if<VControllerDPad>(this); }
	constexpr auto dPad() const { return std::get_if<VControllerDPad>(this); }
	constexpr auto buttonGroup() { return std::get_if<VControllerButtonGroup>(this); }
	constexpr auto buttonGroup() const { return std::get_if<VControllerButtonGroup>(this); }
	constexpr auto uiButtonGroup() { return std::get_if<VControllerUIButtonGroup>(this); }
	constexpr auto uiButtonGroup() const { return std::get_if<VControllerUIButtonGroup>(this); }

	size_t configSize() const
	{
		return (sizeof(VControllerLayoutPosition::pos) + sizeof(_2DOrigin::PackedType)) * 2
			+ sizeof(state)
			+ visit([](auto &e){ return e.configSize(); }, *this);
	}

	WRect bounds() const { return visit([](auto &e){ return e.bounds(); }, *this); }
	WRect realBounds() const { return visit([](auto &e){ return e.realBounds(); }, *this); }
	void setPos(WP pos, WRect viewBounds) { visit([&](auto &e){ e.setPos(pos, viewBounds); }, *this); }

	static bool shouldDraw(VControllerState state, bool showHidden)
	{
		return state == VControllerState::SHOWN || (showHidden && state != VControllerState::OFF);
	}

	void draw(Gfx::RendererCommands &__restrict__ cmds, float alpha, bool showHidden) const
	{
		if(!shouldDraw(state, showHidden))
			return;
		visit([&](auto &e){ e.draw(cmds, alpha); }, *this);
	}

	void place(WRect viewBounds, WRect windowBounds, int layoutIdx)
	{
		auto &lPos = layoutPos[layoutIdx];
		setPos(lPos.toPixelPos(windowBounds), viewBounds);
	}

	void setShowBounds(Gfx::Renderer &r, bool on)
	{
		visit(overloaded
		{
			[&](VControllerDPad &e){ e.setShowBounds(r, on); },
			[&](VControllerButtonGroup &e){ e.setShowBounds(on); },
			[](auto &e){}
		}, *this);
	}

	_2DOrigin layoutOrigin() const
	{
		return visit(overloaded
		{
			[&](const VControllerDPad &e){ return LB2DO; },
			[](auto &e){ return e.layout.origin; }
		}, *this);
	}

	std::string name(const EmuApp &app) const
	{
		return visit([&](auto &e){ return e.name(app); }, *this);
	}

	void updateMeasurements(const Window &win)
	{
		visit([&](auto &e)
		{
			if constexpr(requires {e.updateMeasurements(win);})
			{
				e.updateMeasurements(win);
			}
		}, *this);
	}

	void transposeKeysForPlayer(const EmuApp &app, int player)
	{
		visit([&](auto &e)
		{
			if constexpr(requires {e.transposeKeysForPlayer(app, player);})
			{
				e.transposeKeysForPlayer(app, player);
			}
		}, *this);
	}

	std::span<VControllerButton> buttons()
	{
		return visit([&](auto &e) -> std::span<VControllerButton>
		{
			if constexpr(requires {e.buttons;})
				return e.buttons;
			else
				return {};
		}, *this);
	}

	void add(unsigned keyCode)
	{
		visit([&](auto &e)
		{
			if constexpr(requires {e.buttons;})
			{
				e.buttons.emplace_back(keyCode);
			}
		}, *this);
	}

	void remove(VControllerButton &btnToErase)
	{
		visit([&](auto &e)
		{
			if constexpr(requires {e.buttons;})
			{
				std::erase_if(e.buttons, [&](auto &b) { return &b == &btnToErase; });
			}
		}, *this);
	}

	void setRowSize(int8_t size)
	{
		visit([&](auto &e)
		{
			if constexpr(requires {e.layout.rowItems;})
				e.layout.rowItems = size;
		}, *this);
	}

	auto rowSize() const
	{
		return visit([&](auto &e) -> int8_t
		{
			if constexpr(requires {e.layout.rowItems;})
				return e.layout.rowItems;
			else
				return 1;
		}, *this);
	}
};

enum class VControllerVisibility : uint8_t
{
	OFF, ON, AUTO
};

class VController : public EmuAppHelper<VController>
{
public:
	static constexpr unsigned TOGGLE_KEYBOARD = 65536;
	static constexpr unsigned CHANGE_KEYBOARD_MODE = 65537;
	using KbMap = VControllerKeyboard::KbMap;
	static constexpr uint8_t GAMEPAD_DPAD_BIT = IG::bit(0);
	static constexpr uint8_t GAMEPAD_BUTTONS_BIT = IG::bit(1);
	static constexpr uint8_t GAMEPAD_BITS = GAMEPAD_DPAD_BIT | GAMEPAD_BUTTONS_BIT;

	VController(ApplicationContext);
	int xMMSizeToPixel(const Window &win, float mm) const;
	int yMMSizeToPixel(const Window &win, float mm) const;
	void setInputPlayer(int8_t player);
	auto inputPlayer() const { return inputPlayer_; }
	void setDisabledInputKeys(std::span<const unsigned> keys);
	void updateKeyboardMapping();
	void updateTextures();
	void inputAction(Input::Action action, unsigned vBtn);
	void resetInput();
	void updateAltSpeedModeInput(AltSpeedMode, bool on);
	void place();
	void toggleKeyboard();
	bool pointerInputEvent(const Input::MotionEvent &, WindowRect gameRect);
	bool keyInput(const Input::KeyEvent &);
	void draw(Gfx::RendererCommands &__restrict__, bool showHidden = false);
	void draw(Gfx::RendererCommands &__restrict__, bool showHidden, float alpha);
	bool isInKeyboardMode() const;
	void setKeyboardImage(Gfx::TextureSpan img);
	void setButtonAlpha(std::optional<uint8_t>);
	constexpr uint8_t buttonAlpha() const { return alpha; }
	VControllerKeyboard &keyboard() { return kb; }
	void setRenderer(Gfx::Renderer &renderer) { renderer_ = &renderer; }
	Gfx::Renderer &renderer() { return *renderer_; }
	void setWindow(const Window &win);
	bool hasWindow() const { return win; }
	const Window &window() const { return *win; }
	const WindowData &windowData() const { return *winData; }
	ApplicationContext appContext() const { return appCtx; }
	const Gfx::GlyphTextureSet &face() const { return *facePtr; }
	void setFace(const Gfx::GlyphTextureSet &face) { facePtr = &face; }
	bool setButtonSize(int16_t mm100xOpt, bool placeElements = true);
	auto buttonSize() const { return btnSize; }
	void setShowOnTouchInput(std::optional<bool> opt);
	bool showOnTouchInput() const;
	void setVibrateOnTouchInput(EmuApp &, std::optional<bool> opt);
	bool vibrateOnTouchInput() const;
	bool shouldShowOnTouchInput() const;
	void setGamepadControlsVisibility(std::optional<VControllerVisibility>);
	VControllerVisibility gamepadControlsVisibility() const;
	void setGamepadControlsVisible(bool on) { gamepadIsVisible = on; }
	bool gamepadControlsVisible() const { return gamepadIsVisible; }
	static bool visibilityIsValid(VControllerVisibility);
	void setPhysicalControlsPresent(bool);
	bool updateAutoOnScreenControlVisible();
	bool readConfig(EmuApp &, MapIO &, unsigned key, size_t size);
	void writeConfig(FileIO &) const;
	void configure(Window &, Gfx::Renderer &, const Gfx::GlyphTextureSet &face);
	void resetEmulatedDevicePositions();
	void resetEmulatedDeviceGroups();
	void resetUIPositions();
	void resetUIGroups();
	void setGamepadIsEnabled(bool on) { gamepadDisabledFlags = on ? 0 : GAMEPAD_BITS; }
	void setGamepadDPadIsEnabled(bool on) { gamepadDisabledFlags = IG::setOrClearBits(gamepadDisabledFlags, GAMEPAD_DPAD_BIT, !on); }
	void setGamepadButtonsAreEnabled(bool on) { gamepadDisabledFlags = IG::setOrClearBits(gamepadDisabledFlags, GAMEPAD_BUTTONS_BIT, !on); }
	bool gamepadIsEnabled() const { return gamepadDisabledFlags != GAMEPAD_BITS; }
	bool gamepadDPadIsEnabled() const { return !(gamepadDisabledFlags & GAMEPAD_DPAD_BIT); }
	bool gamepadButtonsAreEnabled() const { return !(gamepadDisabledFlags & GAMEPAD_BUTTONS_BIT); }
	bool gamepadIsActive() const;
	bool allowButtonsPastContentBounds() const { return allowButtonsPastContentBounds_; }
	bool setAllowButtonsPastContentBounds(bool on) { return allowButtonsPastContentBounds_ = on; }
	auto gamepadItems() { return gpElements.size(); }
	VControllerElement &add(InputComponentDesc);
	void update(VControllerElement &) const;
	bool remove(VControllerElement &);
	std::span<VControllerElement> deviceElements() { return gpElements; }
	std::span<VControllerElement> guiElements() { return uiElements; }
	WRect layoutBounds() const;

private:
	ApplicationContext appCtx{};
	Gfx::Renderer *renderer_{};
	const Window *win{};
	const WindowData *winData{};
	const Gfx::GlyphTextureSet *facePtr{};
	VControllerKeyboard kb{};
	std::vector<VControllerElement> gpElements{};
	std::vector<VControllerElement> uiElements{};
	float alphaF{};
	Input::DragTracker<std::array<int, 2>> dragTracker{};
	int16_t defaultButtonSize{};
	int16_t btnSize{};
	bool showOnTouchInput_ = true;
	static constexpr auto DEFAULT_GAMEPAD_CONTROLS_VISIBILITY{Config::envIsLinux ? VControllerVisibility::OFF : VControllerVisibility::AUTO};
	VControllerVisibility gamepadControlsVisibility_{DEFAULT_GAMEPAD_CONTROLS_VISIBILITY};
	int8_t inputPlayer_{};
	bool physicalControlsPresent{};
	bool gamepadIsVisible{gamepadControlsVisibility_ != VControllerVisibility::OFF};
	uint8_t gamepadDisabledFlags{};
	bool kbMode{};
	uint8_t alpha{};
	IG_UseMemberIf(Config::DISPLAY_CUTOUT, bool, allowButtonsPastContentBounds_){};
	IG_UseMemberIf(Config::BASE_SUPPORTS_VIBRATOR, bool, vibrateOnTouchInput_){};

	std::array<int, 2> findGamepadElements(WP pos);
	int keyboardKeyFromPointer(const Input::MotionEvent &);
	void applyButtonSize();
	void resetEmulatedDevicePositions(std::vector<VControllerElement> &) const;
	std::vector<VControllerElement> defaultEmulatedDeviceGroups() const;
	void resetUIPositions(std::vector<VControllerElement> &) const;
	std::vector<VControllerElement> defaultUIGroups() const;
	VControllerElement &add(std::vector<VControllerElement> &, InputComponentDesc) const;
	void setButtonSizes(int gamepadBtnSizeInPixels, int uiBtnSizeInPixels);
	int emulatedDeviceButtonPixelSize() const;
	int uiButtonPixelSize() const;
	void writeDeviceButtonsConfig(FileIO &) const;
	void writeUIButtonsConfig(FileIO &) const;
};

}
