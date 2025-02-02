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

#include <imagine/gfx/GfxText.hh>
#include <imagine/gfx/GfxSprite.hh>
#include <imagine/gfx/PixmapBufferTexture.hh>
#include <imagine/gfx/SyncFence.hh>
#include <imagine/time/Time.hh>
#include <imagine/thread/Semaphore.hh>
#include <imagine/base/ApplicationContext.hh>

namespace IG
{
struct ViewAttachParams;
}

namespace FrameRateTest
{

using namespace IG;

enum TestID
{
	TEST_CLEAR,
	TEST_DRAW,
	TEST_WRITE,
};

struct FramePresentTime
{
	IG::FrameTime timestamp{};
	IG::Time atOnFrame{};
	IG::Time atWinPresent{};

	constexpr FramePresentTime() {}
};

struct TestParams
{
	TestID test{};
	IG::WP pixmapSize{};
	Gfx::TextureBufferMode bufferMode{};

	constexpr TestParams(TestID test)
		: test{test} {}

	constexpr TestParams(TestID test, IG::WP pixmapSize, Gfx::TextureBufferMode bufferMode)
		: test{test}, pixmapSize{pixmapSize}, bufferMode{bufferMode} {}
};

struct TestDesc
{
	TestParams params;
	std::string name;

	TestDesc(TestID test, std::string name, IG::WP pixmapSize = {},
		Gfx::TextureBufferMode bufferMode = {})
		: params{test, pixmapSize, bufferMode}, name{name} {}
};

class TestFramework
{
public:
	using TestFinishedDelegate = IG::DelegateFunc<void (TestFramework &test)>;
	bool started{};
	bool shouldEndTest{};
	unsigned frames{};
	unsigned droppedFrames{};
	unsigned continuousFrames{};
	IG::FrameTime startTime{}, endTime{};
	TestFinishedDelegate onTestFinished;
	FramePresentTime lastFramePresentTime;
	Gfx::SyncFence presentFence{};

	TestFramework() {}
	virtual ~TestFramework() {}
	virtual void initTest(IG::ApplicationContext, Gfx::Renderer &, IG::WP pixmapSize, Gfx::TextureBufferMode) {}
	virtual void placeTest(WRect testRect) {}
	virtual void frameUpdateTest(Gfx::RendererTask &rendererTask, IG::Screen &screen, IG::FrameTime frameTime) = 0;
	virtual void drawTest(Gfx::RendererCommands &cmds, Gfx::ClipRect bounds) = 0;
	virtual void presentedTest(Gfx::RendererCommands &cmds) {}
	void init(IG::ApplicationContext, Gfx::Renderer &, Gfx::GlyphTextureSet &face, IG::WP pixmapSize, Gfx::TextureBufferMode);
	void place(Gfx::Renderer &r, WRect viewBounds, WRect testRect);
	void frameUpdate(Gfx::RendererTask &rTask, IG::Window &win, IG::FrameParams frameParams);
	void prepareDraw(Gfx::Renderer &r);
	void draw(Gfx::RendererCommands &cmds, Gfx::ClipRect bounds, int xIndent);
	void finish(Gfx::RendererTask &task, IG::FrameTime frameTime);
	void setCPUFreqText(std::string_view str);
	void setCPUUseText(std::string_view str);

protected:
	Gfx::Text cpuStatsText;
	Gfx::Text frameStatsText;
	std::string cpuFreqStr{};
	std::string cpuUseStr{};
	std::string skippedFrameStr{};
	std::string statsStr{};
	WRect viewBounds{};
	WRect cpuStatsRect{};
	WRect frameStatsRect{};
	unsigned lostFrameProcessTime = 0;

	void placeCPUStatsText(Gfx::Renderer &r);
	void placeFrameStatsText(Gfx::Renderer &r);
};

class ClearTest : public TestFramework
{
protected:
	bool flash{true};

public:
	void frameUpdateTest(Gfx::RendererTask &rendererTask, IG::Screen &screen, IG::FrameTime frameTime) override;
	void drawTest(Gfx::RendererCommands &cmds, Gfx::ClipRect bounds) override;
};

class DrawTest : public TestFramework
{
protected:
	int flash{true};
	Gfx::PixmapBufferTexture texture;
	Gfx::Sprite sprite;

public:
	void initTest(IG::ApplicationContext, Gfx::Renderer &, IG::WP pixmapSize, Gfx::TextureBufferMode) override;
	void placeTest(WRect testRect) override;
	void frameUpdateTest(Gfx::RendererTask &rendererTask, IG::Screen &screen, IG::FrameTime frameTime) override;
	void drawTest(Gfx::RendererCommands &cmds, Gfx::ClipRect bounds) override;
};

class WriteTest : public DrawTest
{
public:
	void frameUpdateTest(Gfx::RendererTask &rendererTask, IG::Screen &screen, IG::FrameTime frameTime) override;
	void drawTest(Gfx::RendererCommands &cmds, Gfx::ClipRect bounds) override;
};

const char *testIDToStr(TestID id);

}
