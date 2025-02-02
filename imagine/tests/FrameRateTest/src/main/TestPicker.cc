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

#include "TestPicker.hh"
#include "main.hh"
#include <imagine/util/algorithm.h>
#include <imagine/util/format.hh>
#include <imagine/gfx/RendererCommands.hh>
#include <imagine/logger/logger.h>

namespace FrameRateTest
{

TestPicker::TestPicker(IG::ViewAttachParams attach):
	TableView
	{
		attach,
		testEntry
	} {}

void TestPicker::setTests(const TestDesc *testDesc, unsigned tests)
{
	testEntry.clear();
	testEntry.reserve(tests);
	testParam.clear();
	testParam.reserve(tests);
	for(auto i : iotaCount(tests))
	{
		testEntry.emplace_back(testDesc[i].name, u"", &defaultFace(),
			[this, i](IG::DualTextMenuItem &, IG::View &, IG::Input::Event e)
			{
				auto &app = mainApp(appContext());
				auto test = app.startTest(window(), testParam[i]);
				test->onTestFinished =
					[this, i](TestFramework &test)
					{
						IG::FloatSeconds diff = test.endTime - test.startTime;
						logMsg("ran from %f to %f, took %f",
							IG::FloatSeconds(test.startTime).count(),
							IG::FloatSeconds(test.endTime).count(),
							diff.count());
						auto &entry = testEntry[i];
						auto fps = double(test.frames-1) / diff.count();
						entry.set2ndName(fmt::format("{:.2f}", fps).data());
						entry.text2Color = test.droppedFrames ? Gfx::ColorName::RED : Gfx::ColorName::WHITE;
					};
			});
		testParam.emplace_back(testDesc[i].params);
	}
	if(appContext().keyInputIsPresent())
		highlightCell(0);
}

}
