/*  This file is part of PCE.emu.

	PCE.emu is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	PCE.emu is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with PCE.emu.  If not, see <http://www.gnu.org/licenses/> */

#include <mednafen/mednafen.h>
#include <mednafen/VirtualFS.h>
#include <imagine/base/ApplicationContext.hh>
#include <imagine/fs/FS.hh>

extern Base::ApplicationContext appCtx;

namespace Mednafen
{

VirtualFS::VirtualFS(char preferred_path_separator_, const std::string& allowed_path_separators_)
	: preferred_path_separator(preferred_path_separator_), allowed_path_separators(allowed_path_separators_) {}

VirtualFS::~VirtualFS() {}

void VirtualFS::get_file_path_components(const std::string &file_path, std::string* dir_path_out, std::string* file_base_out, std::string *file_ext_out)
{
	auto dir = std::string{FS::dirnameUri(file_path)};
	auto fileBase = std::string{appCtx.fileUriDisplayName(file_path)};
	std::string fileExt{};
	auto dotPos = fileBase.rfind('.');
	if(dotPos != std::string::npos)
	{
		fileExt = {fileBase, dotPos};
		fileBase.resize(dotPos);
	}
	if(dir_path_out)
		*dir_path_out = dir;
	if(file_base_out)
		*file_base_out = fileBase;
	if(file_ext_out)
		*file_ext_out = fileExt;
}

void VirtualFS::check_firop_safe(const std::string& path) {}

bool VirtualFS::is_path_separator(const char c)
{
	return allowed_path_separators.find(c) != std::string::npos;
}

bool VirtualFS::is_absolute_path(const std::string& path)
{
	return true;
}

std::string VirtualFS::eval_fip(const std::string& dir_path, const std::string& rel_path, bool skip_safety_check)
{
	if(is_absolute_path(rel_path))
		return rel_path;
	else
		return std::string{FS::uriString(dir_path, rel_path)};
}

}

