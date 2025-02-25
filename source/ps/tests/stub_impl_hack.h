/* Copyright (C) 2018 Wildfire Games.
 * This file is part of 0 A.D.
 *
 * 0 A.D. is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * 0 A.D. is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with 0 A.D.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "lib/self_test.h"

// usually defined by main.cpp, used by engine's scripting/ScriptFunctions.cpp,
// must be included here to placate linker.

bool g_GameRestarted;

bool IsQuitRequested()
{
	return false;
}

void QuitEngine()
{
}

void StartAtlas()
{
}

void RestartEngine()
{
}

// just so that cxxtestgen doesn't complain "No tests defined"
class TestDummy : public CxxTest::TestSuite
{
public:
	void test_dummy()
	{
	}
};
