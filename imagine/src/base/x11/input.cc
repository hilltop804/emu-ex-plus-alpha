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

#define LOGTAG "Input"
#include <imagine/base/ApplicationContext.hh>
#include <imagine/base/Application.hh>
#include <imagine/base/Window.hh>
#include <imagine/input/Input.hh>
#include <imagine/logger/logger.h>
#include <imagine/util/algorithm.h>
#include <imagine/util/string.h>
#include <imagine/util/ScopeGuard.hh>
#include "xlibutils.h"
#include <X11/XKBlib.h>
#include <X11/cursorfont.h>
#include <memory>

namespace IG
{

struct XIDeviceInfo : public ::XIDeviceInfo {};
struct XkbDescRec : public ::XkbDescRec {};

struct XInputDevice : public Input::Device
{
	bool iCadeMode_ = false;

	XInputDevice() = default;
	XInputDevice(TypeBits, std::string name);
	XInputDevice(XIDeviceInfo, bool isPointingDevice, bool isPowerButton);
	void setICadeMode(bool on) final;
	bool iCadeMode() const final;
};

XInputDevice::XInputDevice(TypeBits typeBits, std::string name):
	Device{0, Input::Map::SYSTEM, typeBits, std::move(name)} {}

XInputDevice::XInputDevice(XIDeviceInfo info, bool isPointingDevice, bool isPowerButton):
	Device{info.deviceid, Input::Map::SYSTEM, 0, info.name}
{
	if(isPointingDevice)
	{
		typeBits_ = Input::Device::TYPE_BIT_MOUSE;
	}
	else
	{
		typeBits_ = Input::Device::TYPE_BIT_KEYBOARD;
		if(isPowerButton)
		{
			typeBits_ |= Input::Device::TYPE_BIT_POWER_BUTTON;
		}
	}
}

void XInputDevice::setICadeMode(bool on)
{
	logMsg("set iCade mode %s for %s", on ? "on" : "off", name().data());
	iCadeMode_ = on;
}

bool XInputDevice::iCadeMode() const
{
	return iCadeMode_;
}

static bool isXInputDevice(Input::Device &d)
{
	return d.map() == Input::Map::SYSTEM && (d.typeBits() & (Input::Device::TYPE_BIT_MOUSE | Input::Device::TYPE_BIT_KEYBOARD));
}

static bool hasXInputDeviceId(Input::Device &d, int id)
{
	return isXInputDevice(d) && d.id() == id;
}

struct XIEventMaskData
{
	XIEventMask eventMask{};
	uint8_t maskBits[XIMaskLen(XI_LASTEVENT)]{};
};

const Input::Device *XApplication::deviceForInputId(int osId) const
{
	for(auto &devPtr : inputDev)
	{
		if(hasXInputDeviceId(*devPtr, osId))
		{
			return devPtr.get();
		}
	}
	if(!vkbDevice)
		logErr("device id %d doesn't exist", osId);
	return vkbDevice;
}

static void setXIEventMaskData(XIEventMaskData &data)
{
	data.eventMask.deviceid = XIAllMasterDevices;
	data.eventMask.mask_len = sizeof(data.maskBits); // always in bytes
	data.eventMask.mask = data.maskBits;
	XISetMask(data.maskBits, XI_ButtonPress);
	XISetMask(data.maskBits, XI_ButtonRelease);
	XISetMask(data.maskBits, XI_Motion);
	XISetMask(data.maskBits, XI_FocusIn);
	XISetMask(data.maskBits, XI_Enter);
	XISetMask(data.maskBits, XI_FocusOut);
	XISetMask(data.maskBits, XI_Leave);
	XISetMask(data.maskBits, XI_KeyPress);
	XISetMask(data.maskBits, XI_KeyRelease);
}

void XApplication::initPerWindowInputData(::Window win)
{
	if(Config::MACHINE_IS_PANDORA)
	{
		XFixesHideCursor(dpy, win);
	}
	else
	{
		if(!blankCursor)
		{
			// make a blank cursor
			char data[1]{};
			auto blank = XCreateBitmapFromData(dpy, win, data, 1, 1);
			if(blank == None)
			{
				logErr("unable to create blank cursor");
			}
			XColor dummy;
			blankCursor = XCreatePixmapCursor(dpy, blank, blank, &dummy, &dummy, 0, 0);
			XFreePixmap(dpy, blank);
			normalCursor = XCreateFontCursor(dpy, XC_left_ptr);
		}
	}
	XIEventMaskData xiMask;
	setXIEventMaskData(xiMask);
	XISelectEvents(dpy, win, &xiMask.eventMask, 1);
}

void XApplication::initXInput2()
{
	int event, error;
	if(!XQueryExtension(dpy, "XInputExtension", &xI2opcode, &event, &error))
	{
		logErr("XInput extension not available");
		::exit(-1);
	}
	int major = 2, minor = 0;
	if(XIQueryVersion(dpy, &major, &minor) == BadRequest)
	{
		logErr("required XInput 2.0 version not available, server supports %d.%d", major, minor);
		::exit(-1);
	}
}

static bool isPowerButtonName(std::string_view name)
{
	return IG::stringContains(name, "Power Button")
		|| (Config::MACHINE_IS_PANDORA && IG::stringContains(name, "power-button"));
}

void XApplication::addXInputDevice(XIDeviceInfo xDevInfo, bool notify, bool isPointingDevice)
{
	for(auto &e : inputDev)
	{
		if(hasXInputDeviceId(*e, xDevInfo.deviceid))
		{
			logMsg("X input device %d (%s) is already present", xDevInfo.deviceid, xDevInfo.name);
			return;
		}
	}
	logMsg("adding X input device %d (%s) to device list", xDevInfo.deviceid, xDevInfo.name);
	std::string_view devName{xDevInfo.name};
	auto devPtr = std::make_unique<XInputDevice>(xDevInfo, isPointingDevice, isPowerButtonName(devName));
	if(Config::MACHINE_IS_PANDORA && (devName == "gpio-keys" || devName == "keypad"))
	{
		devPtr->setSubtype(Input::Device::Subtype::PANDORA_HANDHELD);
	}
	addInputDevice(std::move(devPtr), notify);
}

static const char *xInputDeviceTypeToStr(int type)
{
	switch(type)
	{
		case XIMasterPointer: return "Master Pointer";
		case XISlavePointer: return "Slave Pointer";
		case XIMasterKeyboard: return "Master Keyboard";
		case XISlaveKeyboard: return "Slave Keyboard";
		case XIFloatingSlave: return "Floating Slave";
		default: return "Unknown";
	}
}

static Input::Key keysymToKey(KeySym k)
{
	// if the keysym fits in 2 bytes leave as is,
	// otherwise use only first 15-bits to match
	// definition in Keycode namespace
	return k <= 0xFFFF ? k : k & 0xEFFF;
}

void XApplication::initInputSystem()
{
	initXInput2();

	// request input device changes events
	{
		XIEventMask eventMask;
		uint8_t mask[XIMaskLen(XI_LASTEVENT)] {0};
		XISetMask(mask, XI_HierarchyChanged);
		eventMask.deviceid = XIAllDevices;
		eventMask.mask_len = sizeof(mask);
		eventMask.mask = mask;
		XISelectEvents(dpy, DefaultRootWindow(dpy), &eventMask, 1);
	}

	// setup device list
	vkbDevice = &addInputDevice(std::make_unique<XInputDevice>(Input::Device::TYPE_BIT_VIRTUAL | Input::Device::TYPE_BIT_KEYBOARD | Input::Device::TYPE_BIT_KEY_MISC, "Virtual"));
	int devices;
	::XIDeviceInfo *device = XIQueryDevice(dpy, XIAllDevices, &devices);
	for(auto &d : std::span<::XIDeviceInfo>{device, (size_t)devices})
	{
		if(d.use == XIMasterPointer || d.use == XISlaveKeyboard)
		{
			/*logMsg("Device %s (id: %d) %s paired to id %d",
				d.name, d.deviceid, xInputDeviceTypeToStr(d.use), d.attachment);*/
		}
		switch(d.use)
		{
			bcase XIMasterPointer:
			{
				addXInputDevice({d}, false, true);
			}
			bcase XISlaveKeyboard:
			{
				addXInputDevice({d}, false, false);
			}
		}
	}
	XIFreeDeviceInfo(device);

	coreKeyboardDesc = static_cast<XkbDescRec*>(XkbGetKeyboard(dpy, XkbAllComponentsMask, XkbUseCoreKbd));
}

void XApplication::deinitInputSystem()
{
	//logMsg("deinit input data");
	if(blankCursor)
		XFreeCursor(dpy, blankCursor);
	if(normalCursor)
		XFreeCursor(dpy, normalCursor);
	if(coreKeyboardDesc)
		XkbFreeKeyboard(coreKeyboardDesc, XkbAllComponentsMask, true);
}

static uint32_t makePointerButtonState(XIButtonState state)
{
	uint8_t byte1 = state.mask_len > 0 ? state.mask[0] : 0;
	uint8_t byte2 = state.mask_len > 1 ? state.mask[1] : 0;
	return byte1 | (byte2 << 8);
}

// returns true if event is XI2, false otherwise
bool XApplication::handleXI2GenericEvent(XEvent event)
{
	assert(event.type == GenericEvent);
	if(event.xcookie.extension != xI2opcode)
	{
		return false;
	}
	if(!XGetEventData(dpy, &event.xcookie))
	{
		logMsg("error in XGetEventData for XI2 event");
		return true;
	}
	auto freeEventData = IG::scopeGuard([&]() { XFreeEventData(dpy, &event.xcookie); });
	XGenericEventCookie *cookie = &event.xcookie;
	auto &ievent = *((XIDeviceEvent*)cookie->data);
	// XI_HierarchyChanged isn't window-specific
	if(ievent.evtype == XI_HierarchyChanged)
	{
		//logMsg("input device hierarchy changed");
		auto &ev = *((XIHierarchyEvent*)cookie->data);
		for(auto &info : std::span<XIHierarchyInfo>{ev.info, (size_t)ev.num_info})
		{
			if(info.flags & XISlaveAdded)
			{
				int devices;
				::XIDeviceInfo *device = XIQueryDevice(dpy, info.deviceid, &devices);
				if(devices)
				{
					if(device->use == XISlaveKeyboard)
					{
						XIDeviceInfo d{*device};
						addXInputDevice(d, true, false);
					}
					XIFreeDeviceInfo(device);
				}
			}
			else if(info.flags & XISlaveRemoved)
			{
				removeInputDeviceIf([&](auto &devPtr){ return hasXInputDeviceId(*devPtr, info.deviceid); }, true);
			}
		}
		return true;
	}
	// others events are for specific windows
	auto destWin = windowForXWindow(ievent.event);
	if(!destWin) [[unlikely]]
	{
		//logWarn("ignored event for unknown window");
		return true;
	}
	auto &win = *destWin;
	auto time = IG::Milliseconds(ievent.time); // X11 timestamps are in ms
	auto updatePointer =
		[this](Window &win, auto event, Input::Action action, Input::Time time)
		{
			Input::Key key{};
			bool sendKeyEvent{};
			if(action == Input::Action::PUSHED || action == Input::Action::RELEASED)
			{
				if(event.detail == 4)
					action = Input::Action::SCROLL_UP;
				else if(event.detail == 5)
					action = Input::Action::SCROLL_DOWN;
				else
					key = IG::bit(event.detail - 1);
				if(event.detail > 5)
					sendKeyEvent = true;
			}
			else
			{
				// mask of currently pressed buttons
				key = makePointerButtonState(event.buttons) >> 1;
			}
			auto dev = deviceForInputId(event.sourceid);
			if(sendKeyEvent)
			{
				auto ev = Input::KeyEvent{Input::Map::POINTER, key, key, action, (uint32_t)event.mods.effective,
					0, Input::Source::MOUSE, time, dev};
				dispatchKeyInputEvent(ev, win);
			}
			else
			{
				auto pos = win.transformInputPos({(int)event.event_x, (int)event.event_y});
				Input::PointerId p = event.deviceid;
				win.dispatchInputEvent(Input::MotionEvent{Input::Map::POINTER, (Input::Key)key, (uint32_t)event.mods.effective,
					action, pos.x, pos.y, p, Input::Source::MOUSE, time, dev});
			}
		};
	auto handleKeyEvent =
		[this](Window &win, XIDeviceEvent event, Input::Time time, bool pushed)
		{
			auto action = pushed ? Input::Action::PUSHED : Input::Action::RELEASED;
			if(pushed)
				cancelKeyRepeatTimer();
			auto dev = deviceForInputId(event.sourceid);
			KeySym k = XkbKeycodeToKeysym(dpy, event.detail, 0, 0);
			bool repeated = event.flags & XIKeyRepeat;
			//logMsg("KeySym %d, KeyCode %d, repeat: %d", (int)k, ievent.detail, repeated);
			if(pushed && k == XK_Return && (event.mods.effective & (Mod1Mask | Mod5Mask)) && !repeated)
			{
				win.toggleFullScreen();
			}
			else
			{
				if(!dev->iCadeMode()
					|| (dev->iCadeMode() && !processICadeKey(k, action, time, *dev, win)))
				{
					auto key = keysymToKey(k);
					auto ev = Input::KeyEvent{Input::Map::SYSTEM, key, key, action, (uint32_t)event.mods.effective,
						repeated, Input::Source::KEYBOARD, time, dev};
					ev.setX11RawKey(event.detail);
					dispatchKeyInputEvent(ev, win);
				}
			}
		};
	//logMsg("device %d, event %s", ievent.deviceid, xIEventTypeToStr(ievent.evtype));
	switch(ievent.evtype)
	{
		bcase XI_ButtonPress:
			updatePointer(win, ievent, Input::Action::PUSHED, time);
		bcase XI_ButtonRelease:
			updatePointer(win, ievent, Input::Action::RELEASED, time);
		bcase XI_Motion:
			updatePointer(win, ievent, Input::Action::MOVED, time);
		bcase XI_Enter:
			updatePointer(win, *((XIEnterEvent*)cookie->data), Input::Action::ENTER_VIEW, time);
		bcase XI_Leave:
			updatePointer(win, *((XILeaveEvent*)cookie->data), Input::Action::EXIT_VIEW, time);
		bcase XI_FocusIn:
			win.dispatchFocusChange(true);
		bcase XI_FocusOut:
			win.dispatchFocusChange(false);
			deinitKeyRepeatTimer();
		bcase XI_KeyPress:
			handleKeyEvent(win, ievent, time, true);
		bcase XI_KeyRelease:
			handleKeyEvent(win, ievent, time, false);
	}
	return true;
}

std::string XApplication::inputKeyString(Input::Key rawKey, uint32_t modifiers) const
{
	KeySym k;
	XkbTranslateKeyCode(coreKeyboardDesc, rawKey, modifiers, nullptr, &k);
	std::array<char, 4> str{};
	XkbTranslateKeySym(dpy, &k, 0, str.data(), sizeof(str), nullptr);
	return str.data();
}

void ApplicationContext::flushSystemInputEvents()
{
	application().runX11Events();
}

bool XApplication::hasPendingX11Events() const
{
	return XPending(dpy);
}

}

namespace IG::Input
{

bool Device::anyTypeBitsPresent(ApplicationContext, TypeBits typeBits)
{
	// TODO
	if(typeBits & TYPE_BIT_KEYBOARD)
	{
		return true;
	}
	return false;
}

std::string KeyEvent::keyString(ApplicationContext ctx) const
{
	return ctx.application().inputKeyString(rawKey, metaState ? ShiftMask : 0);
}

}

