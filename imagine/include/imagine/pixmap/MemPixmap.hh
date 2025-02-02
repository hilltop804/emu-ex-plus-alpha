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

#include <imagine/pixmap/Pixmap.hh>
#include <memory>

namespace IG
{

class MemPixmap
{
public:
	constexpr MemPixmap() = default;

	MemPixmap(PixmapDesc desc):
		buffer{new uint8_t[desc.bytes()]},
		desc_{desc} {}

	explicit operator bool() const { return (bool)buffer; }
	MutablePixmapView view() const { return {desc(), buffer.get()}; }
	MutablePixmapView subView(WP pos, WP size) const { return view().subView(pos, size); }
	PixmapDesc desc() const { return desc_; }

protected:
	std::unique_ptr<uint8_t[]> buffer;
	PixmapDesc desc_;
};

}
