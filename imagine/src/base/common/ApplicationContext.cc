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

#define LOGTAG "AppContext"
#include <imagine/base/ApplicationContext.hh>
#include <imagine/base/Application.hh>
#include <imagine/base/VibrationManager.hh>
#include <imagine/base/Sensor.hh>
#include <imagine/input/Input.hh>
#include <imagine/fs/FS.hh>
#include <imagine/fs/FSUtils.hh>
#include <imagine/fs/AssetFS.hh>
#include <imagine/fs/ArchiveFS.hh>
#ifdef __ANDROID__
#include <imagine/fs/AAssetFS.hh>
#endif
#include <imagine/io/FileIO.hh>
#include <imagine/io/IO.hh>
#include <imagine/util/ScopeGuard.hh>
#include <imagine/util/format.hh>
#include <imagine/logger/logger.h>
#include <cstring>

namespace IG
{

void ApplicationContext::dispatchOnInit(ApplicationInitParams initParams)
{
	try
	{
		onInit(initParams);
	}
	catch(std::exception &err)
	{
		exitWithMessage(-1, err.what());
	}
}

Application &ApplicationContext::application() const
{
	return ApplicationContextImpl::application();
}

void ApplicationContext::runOnMainThread(MainThreadMessageDelegate del)
{
	application().runOnMainThread(del);
}

void ApplicationContext::flushMainThreadMessages()
{
	application().flushMainThreadMessages();
}

Window *ApplicationContext::makeWindow(WindowConfig config, WindowInitDelegate onInit)
{
	if(!Config::BASE_MULTI_WINDOW && windows().size())
	{
		bug_unreachable("no multi-window support");
	}
	auto winPtr = std::make_unique<Window>(*this, config, onInit);
	if(!*winPtr)
	{
		return nullptr;
	}
	auto ptr = winPtr.get();
	application().addWindow(std::move(winPtr));
	if(Window::shouldRunOnInitAfterAddingWindow && onInit)
	{
		try
		{
			onInit(*this, *ptr);
		}
		catch(std::exception &err)
		{
			exitWithMessage(-1, err.what());
		}
	}
	return ptr;
}

const WindowContainer &ApplicationContext::windows() const
{
	return application().windows();
}

Window &ApplicationContext::mainWindow()
{
	return application().mainWindow();
}

const ScreenContainer &ApplicationContext::screens() const
{
	return application().screens();
}

Screen &ApplicationContext::mainScreen()
{
	return application().mainScreen();
}

bool ApplicationContext::isRunning() const
{
	return application().isRunning();
}

bool ApplicationContext::isPaused() const
{
	return application().isPaused();
}

bool ApplicationContext::isExiting() const
{
	return application().isExiting();
}

bool ApplicationContext::addOnResume(ResumeDelegate del, int priority)
{
	return application().addOnResume(del, priority);
}

bool ApplicationContext::removeOnResume(ResumeDelegate del)
{
	return application().removeOnResume(del);
}

bool ApplicationContext::containsOnResume(ResumeDelegate del) const
{
	return application().containsOnResume(del);
}

bool ApplicationContext::addOnExit(ExitDelegate del, int priority)
{
	return application().addOnExit(del, priority);
}

bool ApplicationContext::removeOnExit(ExitDelegate del)
{
	return application().removeOnExit(del);
}

bool ApplicationContext::containsOnExit(ExitDelegate del) const
{
	return application().containsOnExit(del);
}

void ApplicationContext::dispatchOnResume(bool focused)
{
	application().dispatchOnResume(*this, focused);
}

void ApplicationContext::dispatchOnExit(bool backgrounded)
{
	application().dispatchOnExit(*this, backgrounded);
}

[[gnu::weak]] FS::PathString ApplicationContext::storagePath(const char *) const
{
	return sharedStoragePath();
}

FS::RootPathInfo ApplicationContext::rootPathInfo(std::string_view path) const
{
	if(path.empty())
		return {};
	if(isUri(path))
	{
		if(auto [treePath, treePos] = FS::uriPathSegment(path, FS::uriPathSegmentTreeName);
			Config::envIsAndroid && treePos != std::string_view::npos)
		{
			auto [docPath, docPos] = FS::uriPathSegment(path, FS::uriPathSegmentDocumentName);
			//logMsg("tree path segment:%s", FS::PathString{treePath}.data());
			//logMsg("document path segment:%s", FS::PathString{docPath}.data());
			if(docPos == std::string_view::npos || docPath.size() < treePath.size())
			{
				logErr("invalid document path in tree URI:%s", path.data());
				return {};
			}
			auto rootLen = docPos + treePath.size();
			FS::PathString rootDocUri{path.data(), rootLen};
			logMsg("found root document URI:%s", rootDocUri.data());
			auto name = fileUriDisplayName(rootDocUri);
			if(rootDocUri.ends_with("%3A"))
				name += ':';
			return {name, rootLen};
		}
		else
		{
			logErr("rootPathInfo() unsupported URI:%s", path.data());
			return {};
		}
	}
	auto location = rootFileLocations();
	const FS::PathLocation *nearestPtr{};
	size_t lastMatchOffset = 0;
	for(const auto &l : location)
	{
		if(!path.starts_with(l.root.path))
			continue;
		auto matchOffset = (size_t)(&path[l.root.info.length] - path.data());
		if(matchOffset > lastMatchOffset)
		{
			nearestPtr = &l;
			lastMatchOffset = matchOffset;
		}
	}
	if(!lastMatchOffset)
		return {};
	logMsg("found root location:%s with length:%d", nearestPtr->root.info.name.data(), (int)nearestPtr->root.info.length);
	return nearestPtr->root.info;
}

AssetIO ApplicationContext::openAsset(CStringView name, IOAccessHint hint, OpenFlagsMask openFlags, const char *appName) const
{
	#ifdef __ANDROID__
	return {*this, name, hint, openFlags};
	#else
	return {FS::pathString(assetPath(appName), name), hint, openFlags};
	#endif
}

FS::AssetDirectoryIterator ApplicationContext::openAssetDirectory(CStringView path, const char *appName)
{
	#ifdef __ANDROID__
	return {aAssetManager(), path};
	#else
	return {FS::pathString(assetPath(appName), path)};
	#endif
}

[[gnu::weak]] bool ApplicationContext::hasSystemPathPicker() const { return false; }

[[gnu::weak]] bool ApplicationContext::showSystemPathPicker(SystemDocumentPickerDelegate) { return false; }

[[gnu::weak]] bool ApplicationContext::hasSystemDocumentPicker() const { return false; }

[[gnu::weak]] bool ApplicationContext::showSystemDocumentPicker(SystemDocumentPickerDelegate) { return false; }

[[gnu::weak]] bool ApplicationContext::showSystemCreateDocumentPicker(SystemDocumentPickerDelegate) { return false; }

[[gnu::weak]] FileIO ApplicationContext::openFileUri(CStringView uri, IOAccessHint access, OpenFlagsMask openFlags) const
{
	return {uri, access, openFlags};
}

FileIO ApplicationContext::openFileUri(CStringView uri, OpenFlagsMask openFlags) const
{
	return openFileUri(uri, IOAccessHint::Normal, openFlags);
}

[[gnu::weak]] UniqueFileDescriptor ApplicationContext::openFileUriFd(CStringView uri, OpenFlagsMask openFlags) const
{
	return PosixIO{uri, openFlags}.releaseFd();
}

[[gnu::weak]] bool ApplicationContext::fileUriExists(CStringView uri) const
{
	return FS::exists(uri);
}

[[gnu::weak]] FS::file_time_type ApplicationContext::fileUriLastWriteTime(CStringView uri) const
{
	return FS::status(uri).lastWriteTime();
}

[[gnu::weak]] std::string ApplicationContext::fileUriFormatLastWriteTimeLocal(CStringView uri) const
{
	return FS::formatLastWriteTimeLocal(*this, uri);
}

[[gnu::weak]] FS::FileString ApplicationContext::fileUriDisplayName(CStringView uri) const
{
	return FS::displayName(uri);
}

[[gnu::weak]] bool ApplicationContext::removeFileUri(CStringView uri) const
{
	return FS::remove(uri);
}

[[gnu::weak]] bool ApplicationContext::renameFileUri(CStringView oldUri, CStringView newUri) const
{
	return FS::rename(oldUri, newUri);
}

[[gnu::weak]] bool ApplicationContext::createDirectoryUri(CStringView uri) const
{
	return FS::create_directory(uri);
}

[[gnu::weak]] bool ApplicationContext::removeDirectoryUri(CStringView uri) const
{
	return FS::remove(uri);
}

[[gnu::weak]] bool ApplicationContext::forEachInDirectoryUri(CStringView uri,
	DirectoryEntryDelegate del, FS::DirOpenFlagsMask flags) const
{
	return forEachInDirectory(uri, del, flags);
}

const InputDeviceContainer &ApplicationContext::inputDevices() const
{
	return application().inputDevices();
}

void ApplicationContext::setHintKeyRepeat(bool on)
{
	application().setAllowKeyRepeatTimer(on);
}

Input::Event ApplicationContext::defaultInputEvent() const
{
	if(keyInputIsPresent())
		return Input::KeyEvent{};
	else
		return Input::MotionEvent{};
}

std::optional<bool> ApplicationContext::swappedConfirmKeysOption() const
{
	return application().swappedConfirmKeysOption();
}

bool ApplicationContext::swappedConfirmKeys() const
{
	return application().swappedConfirmKeys();
}

void ApplicationContext::setSwappedConfirmKeys(std::optional<bool> opt)
{
	application().setSwappedConfirmKeys(opt);
}

[[gnu::weak]] void ApplicationContext::setSysUIStyle(uint32_t flags) {}

[[gnu::weak]] bool ApplicationContext::hasTranslucentSysUI() const { return false; }

[[gnu::weak]] bool ApplicationContext::hasHardwareNavButtons() const { return false; }

[[gnu::weak]] bool ApplicationContext::systemAnimatesWindowRotation() const { return Config::SYSTEM_ROTATES_WINDOWS; }

[[gnu::weak]] void ApplicationContext::setDeviceOrientationChangeSensor(bool) {}

[[gnu::weak]] SensorValues ApplicationContext::remapSensorValuesForDeviceRotation(SensorValues v) const { return v; }

[[gnu::weak]] void ApplicationContext::setOnDeviceOrientationChanged(DeviceOrientationChangedDelegate) {}

[[gnu::weak]] void ApplicationContext::setSystemOrientation(Rotation) {}

[[gnu::weak]] OrientationMask ApplicationContext::defaultSystemOrientations() const { return OrientationMask::ALL; }

[[gnu::weak]] void ApplicationContext::setOnSystemOrientationChanged(SystemOrientationChangedDelegate) {}

[[gnu::weak]] bool ApplicationContext::hasDisplayCutout() const { return false; }

[[gnu::weak]] bool ApplicationContext::usesPermission(Permission) const { return false; }

[[gnu::weak]] bool ApplicationContext::permissionIsRestricted(Permission p) const { return false; }

[[gnu::weak]] bool ApplicationContext::requestPermission(Permission) { return false; }

[[gnu::weak]] void ApplicationContext::addNotification(CStringView onShow, CStringView title, CStringView message) {}

[[gnu::weak]] void ApplicationContext::addLauncherIcon(CStringView name, CStringView path) {}

[[gnu::weak]] bool VibrationManager::hasVibrator() const { return false; }

[[gnu::weak]] void VibrationManager::vibrate(Milliseconds) {}

[[gnu::weak]] NativeDisplayConnection ApplicationContext::nativeDisplayConnection() const { return {}; }

[[gnu::weak]] bool ApplicationContext::packageIsInstalled(CStringView name) const { return false; }

[[gnu::weak]] int32_t ApplicationContext::androidSDK() const
{
	bug_unreachable("Invalid platform-specific function");
}

[[gnu::weak]] std::string ApplicationContext::formatDateAndTime(WallClockTime time)
{
	if(!time.count())
		return {};
	std::tm localTime;
	time_t secs = std::chrono::duration_cast<Seconds>(time).count();
	if(!localtime_r(&secs, &localTime)) [[unlikely]]
	{
		logErr("localtime_r failed");
		return {};
	}
	std::string str;
	str.resize_and_overwrite(64, [&](char *buf, size_t size)
	{
		return std::strftime(buf, size, "%x %r", &localTime);
	});
	return str;
}

std::string ApplicationContext::formatDateAndTimeAsFilename(WallClockTime time)
{
	auto filename = formatDateAndTime(time);
	std::ranges::replace(filename, '/', '-');
	std::ranges::replace(filename, ':', '.');
	return filename;
}

[[gnu::weak]] SensorListener::SensorListener(ApplicationContext, SensorType, SensorChangedDelegate) {}

OnExit::OnExit(ResumeDelegate del, ApplicationContext ctx, int priority): del{del}, ctx{ctx}
{
	ctx.addOnExit(del, priority);
}

OnExit::OnExit(OnExit &&o) noexcept
{
	*this = std::move(o);
}

OnExit &OnExit::operator=(OnExit &&o) noexcept
{
	reset();
	del = std::exchange(o.del, {});
	ctx = o.ctx;
	return *this;
}

OnExit::~OnExit()
{
	reset();
}

void OnExit::reset()
{
	if(!del)
		return;
	ctx.removeOnExit(std::exchange(del, {}));
}

ApplicationContext OnExit::appContext() const
{
	return ctx;
}

}

namespace IG::FileUtils
{

ssize_t writeToUri(ApplicationContext ctx, CStringView uri, std::span<const unsigned char> src)
{
	auto f = ctx.openFileUri(uri, OpenFlagsMask::New | OpenFlagsMask::Test);
	return f.write(src).bytes;
}

ssize_t readFromUri(ApplicationContext ctx, CStringView uri, std::span<unsigned char> dest,
	IOAccessHint accessHint)
{
	auto f = ctx.openFileUri(uri, accessHint, OpenFlagsMask::Test);
	return f.read(dest).bytes;
}

std::pair<ssize_t, FS::PathString> readFromUriWithArchiveScan(ApplicationContext ctx, CStringView uri,
	std::span<unsigned char> dest, bool(*nameMatchFunc)(std::string_view), IOAccessHint accessHint)
{
	auto io = ctx.openFileUri(uri, accessHint);
	if(FS::hasArchiveExtension(uri))
	{
		for(auto &entry : FS::ArchiveIterator{std::move(io)})
		{
			if(entry.type() == FS::file_type::directory)
			{
				continue;
			}
			auto name = entry.name();
			if(nameMatchFunc(name))
			{
				return {entry.releaseIO().read(dest).bytes, FS::PathString{name}};
			}
		}
		logErr("no recognized files in archive:%s", uri.data());
		return {-1, {}};
	}
	else
	{
		return {io.read(dest).bytes, FS::PathString{uri}};
	}
}

IOBuffer bufferFromUri(ApplicationContext ctx, CStringView uri, OpenFlagsMask openFlags, size_t sizeLimit)
{
	if(!sizeLimit) [[unlikely]]
		return {};
	auto file = ctx.openFileUri(uri, IOAccessHint::All, openFlags);
	if(!file)
		return {};
	else if(file.size() > sizeLimit)
	{
		if(to_underlying(openFlags & OpenFlagsMask::Test))
			return {};
		else
			throw std::runtime_error(fmt::format("{} exceeds {} byte limit", uri.data(), sizeLimit));
	}
	return file.buffer(IOBufferMode::Release);
}

IOBuffer rwBufferFromUri(ApplicationContext ctx, CStringView uri, OpenFlagsMask extraOFlags, size_t size, uint8_t initValue)
{
	if(!size) [[unlikely]]
		return {};
	auto file = ctx.openFileUri(uri, IOAccessHint::Random, OpenFlagsMask::CreateRW | extraOFlags);
	if(!file) [[unlikely]]
		return {};
	auto fileSize = file.size();
	if(fileSize != size)
		file.truncate(size);
	auto buff = file.buffer(IOBufferMode::Release);
	if(initValue && fileSize < size)
	{
		std::fill(&buff[fileSize], &buff[size], initValue);
	}
	return buff;
}

FILE *fopenUri(ApplicationContext ctx, CStringView path, CStringView mode)
{
	if(isUri(path))
	{
		assert(!mode.contains('a')); //append mode not supported
		OpenFlagsMask openFlags{OpenFlagsMask::Test};
		if(mode.contains('r'))
			openFlags |= OpenFlagsMask::Read;
		if(mode.contains('w'))
			openFlags |= OpenFlagsMask::New;
		if(mode.contains('+'))
			openFlags |= OpenFlagsMask::Write;
		return ctx.openFileUri(path, openFlags).toFileStream(mode);
	}
	else
	{
		return ::fopen(path, mode);
	}
}

}
