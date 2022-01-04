#pragma once

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

#include <imagine/input/Device.hh>
#include <imagine/util/container/ArrayList.hh>

namespace IG::Input
{

class AndroidInputDevice : public Device
{
public:
	AndroidInputDevice(int osId, TypeBits, std::string name);
	AndroidInputDevice(JNIEnv* env, jobject aDev, int osId, int src,
		std::string name, int kbType, uint32_t axisBits, bool isPowerButton);
	bool operator ==(AndroidInputDevice const& rhs) const;
	void setTypeBits(TypeBits);
	std::span<Axis> motionAxes() final;
	void setICadeMode(bool on) final;
	bool iCadeMode() const final;
	auto &jsAxes() { return axis; }
	void update(AndroidInputDevice);

protected:
	static constexpr uint32_t MAX_AXES = 14;
	StaticArrayList<Axis, MAX_AXES> axis{};
	bool iCadeMode_{};
};

}
