#include <imagine/util/preprocessor/repeat.h>
#include <emuframework/EmuInput.hh>

namespace EmuEx::Controls
{

extern const int gamepadKeys = 22;
const int systemTotalKeys = gameActionKeys + gamepadKeys*5;

void transposeKeysForPlayer(KeyConfig::KeyArray &key, int player)
{
	genericMultiplayerTranspose(key, player, 1);
}

constexpr std::array<const std::string_view, gamepadKeys> gamepadName
{
	"Up",
	"Right",
	"Down",
	"Left",
	"Left+Up",
	"Right+Up",
	"Right+Down",
	"Left+Down",
	"Select",
	"Start",
	"A",
	"B",
	"X",
	"Y",
	"L",
	"R",
	"Turbo A",
	"Turbo B",
	"Turbo X",
	"Turbo Y",
	"Turbo L",
	"Turbo R",
};

constexpr int gamepadKeyOffset = gameActionKeys;
constexpr int gamepad2KeyOffset = gamepadKeyOffset + gamepadKeys;
constexpr int gamepad3KeyOffset = gamepad2KeyOffset + gamepadKeys;
constexpr int gamepad4KeyOffset = gamepad3KeyOffset + gamepadKeys;
constexpr int gamepad5KeyOffset = gamepad4KeyOffset + gamepadKeys;

constexpr KeyCategory category[]
{
	EMU_CONTROLS_IN_GAME_ACTIONS_CATEGORY_INIT,
	{"Set Gamepad Keys", gamepadName, gamepadKeyOffset},
	{"Set Gamepad 2 Keys", gamepadName, gamepad2KeyOffset, 1},
	{"Set Gamepad 3 Keys", gamepadName, gamepad3KeyOffset, 2},
	{"Set Gamepad 4 Keys", gamepadName, gamepad4KeyOffset, 3},
	{"Set Gamepad 5 Keys", gamepadName, gamepad5KeyOffset, 4}
};

std::span<const KeyCategory> categories() { return category; }

const KeyConfig defaultKeyProfile[] =
{
	{
		Map::SYSTEM,
		{"PC Keyboard"},
		{
			EMU_CONTROLS_IN_GAME_ACTIONS_GENERIC_KB_PROFILE_INIT,

			Keycode::UP,
			Keycode::RIGHT,
			Keycode::DOWN,
			Keycode::LEFT,
			0,
			0,
			0,
			0,
			Keycode::SPACE,
			Keycode::ENTER,
			Keycode::X,
			Keycode::Z,
			Keycode::S,
			Keycode::A,
			Keycode::Q,
			Keycode::W,
			Keycode::V,
			Keycode::C,
			Keycode::F,
			Keycode::D,
			Keycode::E,
			Keycode::R,
		}
	},
	#ifdef CONFIG_INPUT_GAMEPAD_DEVICES
	{
		Map::SYSTEM,
		DeviceSubtype::GENERIC_GAMEPAD,
		{"Generic Gamepad"},
		{
			EMU_CONTROLS_IN_GAME_ACTIONS_ANDROID_GENERIC_GAMEPAD_PROFILE_INIT,

			Keycode::UP,
			Keycode::RIGHT,
			Keycode::DOWN,
			Keycode::LEFT,
			0, 0, 0, 0,
			Keycode::GAME_SELECT,
			Keycode::GAME_START,
			Keycode::GAME_B,
			Keycode::GAME_A,
			Keycode::GAME_Y,
			Keycode::GAME_X,
			Keycode::GAME_L1,
			Keycode::GAME_R1,
		}
	},
	{
		Map::SYSTEM,
		DeviceSubtype::PS3_CONTROLLER,
		{"PS3 Controller"},
		{
			EMU_CONTROLS_IN_GAME_ACTIONS_ANDROID_PS3_GAMEPAD_PROFILE_INIT,

			Keycode::PS3::UP,
			Keycode::PS3::RIGHT,
			Keycode::PS3::DOWN,
			Keycode::PS3::LEFT,
			0, 0, 0, 0,
			Keycode::PS3::SELECT,
			Keycode::PS3::START,
			Keycode::PS3::CIRCLE,
			Keycode::PS3::CROSS,
			Keycode::PS3::TRIANGLE,
			Keycode::PS3::SQUARE,
			Keycode::PS3::L1,
			Keycode::PS3::R1,
		}
	},
	{
		Map::SYSTEM,
		DeviceSubtype::OUYA_CONTROLLER,
		{"OUYA Controller"},
		{
			EMU_CONTROLS_IN_GAME_ACTIONS_OUYA_PROFILE_INIT,

			Keycode::Ouya::UP,
			Keycode::Ouya::RIGHT,
			Keycode::Ouya::DOWN,
			Keycode::Ouya::LEFT,
			0, 0, 0, 0,
			Keycode::Ouya::L3,
			Keycode::Ouya::R3,
			Keycode::Ouya::A,
			Keycode::Ouya::O,
			Keycode::Ouya::Y,
			Keycode::Ouya::U,
			Keycode::Ouya::L1,
			Keycode::Ouya::R1,
		}
	},
	{
		Map::SYSTEM,
		DeviceSubtype::NVIDIA_SHIELD,
		{"NVidia Shield"},
		{
			EMU_CONTROLS_IN_GAME_ACTIONS_NVIDIA_SHIELD_PROFILE_INIT,

			Keycode::UP,
			Keycode::RIGHT,
			Keycode::DOWN,
			Keycode::LEFT,
			0, 0, 0, 0,
			Keycode::GAME_LEFT_THUMB,
			Keycode::GAME_START,
			Keycode::GAME_B,
			Keycode::GAME_A,
			Keycode::GAME_Y,
			Keycode::GAME_X,
			Keycode::GAME_L1,
			Keycode::GAME_R1,
		}
	},
	{
		Map::SYSTEM,
		DeviceSubtype::_8BITDO_SF30_PRO,
		{"8Bitdo SF30 Pro"},
		{
			EMU_CONTROLS_IN_GAME_ACTIONS_8BITDO_SF30_PRO_PROFILE_INIT,

			Keycode::UP,
			Keycode::RIGHT,
			Keycode::DOWN,
			Keycode::LEFT,
			0, 0, 0, 0,
			Keycode::GAME_SELECT,
			Keycode::GAME_START,
			Keycode::GAME_A,
			Keycode::GAME_B,
			Keycode::GAME_X,
			Keycode::GAME_Y,
			Keycode::GAME_L1,
			Keycode::GAME_R1,
		}
	},
	{
		Map::SYSTEM,
		DeviceSubtype::_8BITDO_SN30_PRO_PLUS,
		{"8BitDo SN30 Pro+"},
		{
			EMU_CONTROLS_IN_GAME_ACTIONS_8BITDO_SN30_PRO_PLUS_PROFILE_INIT,

			Keycode::UP,
			Keycode::RIGHT,
			Keycode::DOWN,
			Keycode::LEFT,
			0, 0, 0, 0,
			Keycode::GAME_SELECT,
			Keycode::GAME_START,
			Keycode::GAME_A,
			Keycode::GAME_B,
			Keycode::GAME_X,
			Keycode::GAME_Y,
			Keycode::GAME_L1,
			Keycode::GAME_R1,
		}
	},
	#endif
	#if defined(__ANDROID__) && __ARM_ARCH == 7
	{
		Map::SYSTEM,
		DeviceSubtype::XPERIA_PLAY,
		{"Xperia Play"},
		{
			EMU_CONTROLS_IN_GAME_ACTIONS_ANDROID_NAV_PROFILE_INIT,

			Keycode::XperiaPlay::UP,
			Keycode::XperiaPlay::RIGHT,
			Keycode::XperiaPlay::DOWN,
			Keycode::XperiaPlay::LEFT,
			0, 0, 0, 0,
			Keycode::XperiaPlay::SELECT,
			Keycode::XperiaPlay::START,
			Keycode::XperiaPlay::CIRCLE,
			Keycode::XperiaPlay::CROSS,
			Keycode::XperiaPlay::TRIANGLE,
			Keycode::XperiaPlay::SQUARE,
			Keycode::XperiaPlay::L1,
			Keycode::XperiaPlay::R1,
		}
	},
	{
		Map::SYSTEM,
		DeviceSubtype::MOTO_DROID_KEYBOARD,
		{"Droid/Milestone Keyboard"},
		{
			EMU_CONTROLS_IN_GAME_ACTIONS_ANDROID_NAV_PROFILE_INIT,

			Keycode::UP,
			Keycode::RIGHT,
			Keycode::DOWN,
			Keycode::LEFT,
			0, 0, 0, 0,
			Keycode::SPACE,
			Keycode::ENTER,
			Keycode::C,
			Keycode::X,
			Keycode::D,
			Keycode::S,
			Keycode::E,
			Keycode::W,
			Keycode::B,
			Keycode::V,
			Keycode::G,
			Keycode::F,
		}
	},
	#endif
	#ifdef CONFIG_MACHINE_PANDORA
	{
		Map::SYSTEM,
		DeviceSubtype::PANDORA_HANDHELD,
		{"Default Pandora"},
		{
			EMU_CONTROLS_IN_GAME_ACTIONS_PANDORA_ALT_PROFILE_INIT,

			Keycode::Pandora::UP,
			Keycode::Pandora::RIGHT,
			Keycode::Pandora::DOWN,
			Keycode::Pandora::LEFT,
			0, 0, 0, 0,
			Keycode::Pandora::SELECT,
			Keycode::Pandora::START,
			Keycode::Pandora::B,
			Keycode::Pandora::X,
			Keycode::Pandora::Y,
			Keycode::Pandora::A,
			Keycode::Pandora::L,
			Keycode::Pandora::R,
		}
	},
	#endif
};

const unsigned defaultKeyProfiles = std::size(defaultKeyProfile);

#ifdef CONFIG_INPUT_APPLE_GAME_CONTROLLER

const KeyConfig defaultAppleGCProfile[] =
{
	{
		Map::APPLE_GAME_CONTROLLER,
		{"Default"},
		{
			EMU_CONTROLS_IN_GAME_ACTIONS_APPLEGC_PROFILE_INIT,
			AppleGC::UP,
			AppleGC::RIGHT,
			AppleGC::DOWN,
			AppleGC::LEFT,
			0, 0, 0, 0,
			AppleGC::RSTICK_LEFT,
			AppleGC::RSTICK_RIGHT,
			AppleGC::B,
			AppleGC::A,
			AppleGC::Y,
			AppleGC::X,
			AppleGC::L1,
			AppleGC::R1,
		}
	},
};

const unsigned defaultAppleGCProfiles = std::size(defaultAppleGCProfile);

#endif

// Wiimote

const KeyConfig defaultWiimoteProfile[] =
{
	{
			Map::WIIMOTE,
			{"Default"},
			{
					EMU_CONTROLS_IN_GAME_ACTIONS_WIIMOTE_PROFILE_INIT,

					Wiimote::UP,
					Wiimote::RIGHT,
					Wiimote::DOWN,
					Wiimote::LEFT,
					0,
					0,
					0,
					0,
					Wiimote::MINUS,
					Wiimote::PLUS,
					Wiimote::A,
					Wiimote::_2,
					Wiimote::B,
					Wiimote::_1,
					0,
					0,
					0,
					0,
					0,
					0,
			}
	},
};

const unsigned defaultWiimoteProfiles = std::size(defaultWiimoteProfile);

const KeyConfig defaultWiiCCProfile[] =
{
	{
		Map::WII_CC,
		{"Default"},
		{
			EMU_CONTROLS_IN_GAME_ACTIONS_WII_CC_PROFILE_INIT,

			WiiCC::UP,
			WiiCC::RIGHT,
			WiiCC::DOWN,
			WiiCC::LEFT,
			0,
			0,
			0,
			0,
			WiiCC::MINUS,
			WiiCC::PLUS,
			WiiCC::A,
			WiiCC::B,
			WiiCC::X,
			WiiCC::Y,
			WiiCC::L,
			WiiCC::R,
		}
	}
};

const unsigned defaultWiiCCProfiles = std::size(defaultWiiCCProfile);

// iControlPad

const KeyConfig defaultIControlPadProfile[] =
{
	{
			Map::ICONTROLPAD,
			{"Default"},
			{
					EMU_CONTROLS_IN_GAME_ACTIONS_ICP_NUBS_PROFILE_INIT,

					iControlPad::UP,
					iControlPad::RIGHT,
					iControlPad::DOWN,
					iControlPad::LEFT,
					0,
					0,
					0,
					0,
					iControlPad::SELECT,
					iControlPad::START,
					iControlPad::B,
					iControlPad::X,
					iControlPad::Y,
					iControlPad::A,
					iControlPad::L,
					iControlPad::R,
					0,
					0,
					0,
					0,
			}
	},
};

const unsigned defaultIControlPadProfiles = std::size(defaultIControlPadProfile);

// iCade

const KeyConfig defaultICadeProfile[] =
{
	{
			Map::ICADE,
			{"Default"},
			{
					EMU_CONTROLS_IN_GAME_ACTIONS_UNBINDED_PROFILE_INIT,

					ICade::UP,
					ICade::RIGHT,
					ICade::DOWN,
					ICade::LEFT,
					0,
					0,
					0,
					0,
					ICade::SELECT,
					ICade::START,
					ICade::B,
					ICade::A,
					ICade::Y,
					ICade::X,
					ICade::Z,
					ICade::C,
					0,
					0,
					0,
					0,
			}
	},
};

const unsigned defaultICadeProfiles = std::size(defaultICadeProfile);

// Zeemote

const KeyConfig defaultZeemoteProfile[] =
{
	{
			Map::ZEEMOTE,
			{"Default"},
			{
					EMU_CONTROLS_IN_GAME_ACTIONS_UNBINDED_PROFILE_INIT,

					Zeemote::UP,
					Zeemote::RIGHT,
					Zeemote::DOWN,
					Zeemote::LEFT,
					0,
					0,
					0,
					0,
					0,
					Zeemote::POWER,
					Zeemote::A,
					Zeemote::B,
					Zeemote::C,
					0,
					0,
					0,
					0,
					0,
					0,
					0,
			}
	},
};

const unsigned defaultZeemoteProfiles = std::size(defaultZeemoteProfile);

// PS3

const KeyConfig defaultPS3Profile[] =
{
	{
		Map::PS3PAD,
		{"Default"},
		{
			EMU_CONTROLS_IN_GAME_ACTIONS_GENERIC_PS3PAD_PROFILE_INIT,

			PS3::UP,
			PS3::RIGHT,
			PS3::DOWN,
			PS3::LEFT,
			0, 0, 0, 0,
			PS3::SELECT,
			PS3::START,
			PS3::CIRCLE,
			PS3::CROSS,
			PS3::TRIANGLE,
			PS3::SQUARE,
			PS3::L1,
			PS3::R1,
		}
	},
};

const unsigned defaultPS3Profiles = std::size(defaultPS3Profile);

};
