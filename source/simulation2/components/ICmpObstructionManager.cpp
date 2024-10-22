/* Copyright (C) 2019 Wildfire Games.
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

#include "ICmpObstructionManager.h"

#include "simulation2/system/InterfaceScripted.h"

BEGIN_INTERFACE_WRAPPER(ObstructionManager)
DEFINE_INTERFACE_METHOD_1("SetPassabilityCircular", void, ICmpObstructionManager, SetPassabilityCircular, bool)
DEFINE_INTERFACE_METHOD_1("SetDebugOverlay", void, ICmpObstructionManager, SetDebugOverlay, bool)
DEFINE_INTERFACE_METHOD_CONST_3("DistanceToPoint", fixed, ICmpObstructionManager, DistanceToPoint, entity_id_t, entity_pos_t, entity_pos_t)
DEFINE_INTERFACE_METHOD_CONST_3("MaxDistanceToPoint", fixed, ICmpObstructionManager, MaxDistanceToPoint, entity_id_t, entity_pos_t, entity_pos_t)
DEFINE_INTERFACE_METHOD_CONST_2("DistanceToTarget", fixed, ICmpObstructionManager, DistanceToTarget, entity_id_t, entity_id_t)
DEFINE_INTERFACE_METHOD_CONST_2("MaxDistanceToTarget", fixed, ICmpObstructionManager, MaxDistanceToTarget, entity_id_t, entity_id_t)
DEFINE_INTERFACE_METHOD_CONST_6("IsInPointRange", bool, ICmpObstructionManager, IsInPointRange, entity_id_t, entity_pos_t, entity_pos_t, entity_pos_t, entity_pos_t, bool)
DEFINE_INTERFACE_METHOD_CONST_5("IsInTargetRange", bool, ICmpObstructionManager, IsInTargetRange, entity_id_t, entity_id_t, entity_pos_t, entity_pos_t, bool)
DEFINE_INTERFACE_METHOD_CONST_6("IsPointInPointRange", bool, ICmpObstructionManager, IsPointInPointRange, entity_pos_t, entity_pos_t, entity_pos_t, entity_pos_t, entity_pos_t, entity_pos_t)
END_INTERFACE_WRAPPER(ObstructionManager)
