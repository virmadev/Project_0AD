/* Copyright (C) 2015 Wildfire Games.
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

#include "precompiled.h"

#include "ICmpOverlayRenderer.h"

#include "simulation2/system/InterfaceScripted.h"

BEGIN_INTERFACE_WRAPPER(OverlayRenderer)
DEFINE_INTERFACE_METHOD_0("Reset", void, ICmpOverlayRenderer, Reset)
DEFINE_INTERFACE_METHOD_5("AddSprite", void, ICmpOverlayRenderer, AddSprite, VfsPath, CFixedVector2D, CFixedVector2D, CFixedVector3D, std::string)
END_INTERFACE_WRAPPER(OverlayRenderer)

bool ICmpOverlayRenderer::m_OverrideVisible = true;
