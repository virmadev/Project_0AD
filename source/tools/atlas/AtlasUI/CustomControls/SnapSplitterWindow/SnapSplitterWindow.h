/* Copyright (C) 2009 Wildfire Games.
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

#include "wx/splitter.h"

class SnapSplitterWindow : public wxSplitterWindow
{
public:
	SnapSplitterWindow(wxWindow* parent, long style = wxSP_3D);
	void SetDefaultSashPosition(int sashPosition);
	virtual bool SplitVertically(wxWindow* window1, wxWindow* window2, int sashPosition = 0);
	virtual bool SplitHorizontally(wxWindow* window1, wxWindow* window2, int sashPosition = 0);

private:
	void OnSashPosChanging(wxSplitterEvent& evt);
	void OnDoubleClick(wxSplitterEvent& evt);

	int m_DefaultSashPosition;
	int m_SnapTolerance;

	DECLARE_EVENT_TABLE();
};
