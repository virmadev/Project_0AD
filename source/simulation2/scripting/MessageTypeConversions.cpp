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

#include "ps/CLogger.h"
#include "scriptinterface/ScriptInterface.h"
#include "simulation2/MessageTypes.h"

#define TOJSVAL_SETUP() \
	JSContext* cx = scriptInterface.GetContext(); \
	JSAutoRequest rq(cx); \
	JS::RootedObject obj(cx, JS_NewPlainObject(cx)); \
	if (!obj) \
		return JS::UndefinedValue();

#define SET_MSG_PROPERTY(name) \
	do { \
		JS::RootedValue prop(cx);\
		ScriptInterface::ToJSVal(cx, &prop, this->name); \
		if (! JS_SetProperty(cx, obj, #name, prop)) \
			return JS::UndefinedValue(); \
	} while (0);

#define FROMJSVAL_SETUP() \
	JSContext* cx = scriptInterface.GetContext(); \
	JSAutoRequest rq(cx); \
	if (val.isPrimitive()) \
		return NULL; \
	JS::RootedObject obj(cx, &val.toObject()); \
	JS::RootedValue prop(cx);

#define GET_MSG_PROPERTY(type, name) \
	type name; \
	{ \
	if (! JS_GetProperty(cx, obj, #name, &prop)) \
		return NULL; \
	if (! ScriptInterface::FromJSVal(cx, prop, name)) \
		return NULL; \
	}

JS::Value CMessage::ToJSValCached(const ScriptInterface& scriptInterface) const
{
	if (!m_Cached)
		m_Cached.reset(new JS::PersistentRootedValue(scriptInterface.GetJSRuntime(), ToJSVal(scriptInterface)));

	return m_Cached->get();
}

////////////////////////////////

JS::Value CMessageTurnStart::ToJSVal(const ScriptInterface& scriptInterface) const
{
	TOJSVAL_SETUP();
	return JS::ObjectValue(*obj);
}

CMessage* CMessageTurnStart::FromJSVal(const ScriptInterface& UNUSED(scriptInterface), JS::HandleValue UNUSED(val))
{
	return new CMessageTurnStart();
}

////////////////////////////////

#define MESSAGE_1(name, t0, a0) \
	JS::Value CMessage##name::ToJSVal(const ScriptInterface& scriptInterface) const \
	{ \
		TOJSVAL_SETUP(); \
		SET_MSG_PROPERTY(a0); \
		return JS::ObjectValue(*obj); \
	} \
	CMessage* CMessage##name::FromJSVal(const ScriptInterface& scriptInterface, JS::HandleValue val) \
	{ \
		FROMJSVAL_SETUP(); \
		GET_MSG_PROPERTY(t0, a0); \
		return new CMessage##name(a0); \
	}

MESSAGE_1(Update, fixed, turnLength)
MESSAGE_1(Update_MotionFormation, fixed, turnLength)
MESSAGE_1(Update_MotionUnit, fixed, turnLength)
MESSAGE_1(Update_Final, fixed, turnLength)

////////////////////////////////

JS::Value CMessageInterpolate::ToJSVal(const ScriptInterface& scriptInterface) const
{
	TOJSVAL_SETUP();
	SET_MSG_PROPERTY(deltaSimTime);
	SET_MSG_PROPERTY(offset);
	SET_MSG_PROPERTY(deltaRealTime);
	return JS::ObjectValue(*obj);
}

CMessage* CMessageInterpolate::FromJSVal(const ScriptInterface& scriptInterface, JS::HandleValue val)
{
	FROMJSVAL_SETUP();
	GET_MSG_PROPERTY(float, deltaSimTime);
	GET_MSG_PROPERTY(float, offset);
	GET_MSG_PROPERTY(float, deltaRealTime);
	return new CMessageInterpolate(deltaSimTime, offset, deltaRealTime);
}

////////////////////////////////

JS::Value CMessageRenderSubmit::ToJSVal(const ScriptInterface& UNUSED(scriptInterface)) const
{
	LOGWARNING("CMessageRenderSubmit::ToJSVal not implemented");
	return JS::UndefinedValue();
}

CMessage* CMessageRenderSubmit::FromJSVal(const ScriptInterface& UNUSED(scriptInterface), JS::HandleValue UNUSED(val))
{
	LOGWARNING("CMessageRenderSubmit::FromJSVal not implemented");
	return NULL;
}

////////////////////////////////

JS::Value CMessageProgressiveLoad::ToJSVal(const ScriptInterface& UNUSED(scriptInterface)) const
{
	LOGWARNING("CMessageProgressiveLoad::ToJSVal not implemented");
	return JS::UndefinedValue();
}

CMessage* CMessageProgressiveLoad::FromJSVal(const ScriptInterface& UNUSED(scriptInterface), JS::HandleValue UNUSED(val))
{
	LOGWARNING("CMessageProgressiveLoad::FromJSVal not implemented");
	return NULL;
}

////////////////////////////////

JS::Value CMessageDeserialized::ToJSVal(const ScriptInterface& scriptInterface) const
{
	TOJSVAL_SETUP();
	return JS::ObjectValue(*obj);
}

CMessage* CMessageDeserialized::FromJSVal(const ScriptInterface& scriptInterface, JS::HandleValue val)
{
	FROMJSVAL_SETUP();
	return new CMessageDeserialized();
}

////////////////////////////////

JS::Value CMessageCreate::ToJSVal(const ScriptInterface& scriptInterface) const
{
	TOJSVAL_SETUP();
	SET_MSG_PROPERTY(entity);
	return JS::ObjectValue(*obj);
}

CMessage* CMessageCreate::FromJSVal(const ScriptInterface& scriptInterface, JS::HandleValue val)
{
	FROMJSVAL_SETUP();
	GET_MSG_PROPERTY(entity_id_t, entity);
	return new CMessageCreate(entity);
}

////////////////////////////////

JS::Value CMessageDestroy::ToJSVal(const ScriptInterface& scriptInterface) const
{
	TOJSVAL_SETUP();
	SET_MSG_PROPERTY(entity);
	return JS::ObjectValue(*obj);
}

CMessage* CMessageDestroy::FromJSVal(const ScriptInterface& scriptInterface, JS::HandleValue val)
{
	FROMJSVAL_SETUP();
	GET_MSG_PROPERTY(entity_id_t, entity);
	return new CMessageDestroy(entity);
}

////////////////////////////////

JS::Value CMessageOwnershipChanged::ToJSVal(const ScriptInterface& scriptInterface) const
{
	TOJSVAL_SETUP();
	SET_MSG_PROPERTY(entity);
	SET_MSG_PROPERTY(from);
	SET_MSG_PROPERTY(to);
	return JS::ObjectValue(*obj);
}

CMessage* CMessageOwnershipChanged::FromJSVal(const ScriptInterface& scriptInterface, JS::HandleValue val)
{
	FROMJSVAL_SETUP();
	GET_MSG_PROPERTY(entity_id_t, entity);
	GET_MSG_PROPERTY(player_id_t, from);
	GET_MSG_PROPERTY(player_id_t, to);
	return new CMessageOwnershipChanged(entity, from, to);
}

////////////////////////////////

JS::Value CMessagePositionChanged::ToJSVal(const ScriptInterface& scriptInterface) const
{
	TOJSVAL_SETUP();
	SET_MSG_PROPERTY(entity);
	SET_MSG_PROPERTY(inWorld);
	SET_MSG_PROPERTY(x);
	SET_MSG_PROPERTY(z);
	SET_MSG_PROPERTY(a);
	return JS::ObjectValue(*obj);
}

CMessage* CMessagePositionChanged::FromJSVal(const ScriptInterface& scriptInterface, JS::HandleValue val)
{
	FROMJSVAL_SETUP();
	GET_MSG_PROPERTY(entity_id_t, entity);
	GET_MSG_PROPERTY(bool, inWorld);
	GET_MSG_PROPERTY(entity_pos_t, x);
	GET_MSG_PROPERTY(entity_pos_t, z);
	GET_MSG_PROPERTY(entity_angle_t, a);
	return new CMessagePositionChanged(entity, inWorld, x, z, a);
}

////////////////////////////////

JS::Value CMessageInterpolatedPositionChanged::ToJSVal(const ScriptInterface& UNUSED(scriptInterface)) const
{
	LOGWARNING("CMessageInterpolatedPositionChanged::ToJSVal not implemented");
	return JS::UndefinedValue();
}

CMessage* CMessageInterpolatedPositionChanged::FromJSVal(const ScriptInterface& UNUSED(scriptInterface), JS::HandleValue UNUSED(val))
{
	LOGWARNING("CMessageInterpolatedPositionChanged::FromJSVal not implemented");
	return NULL;
}

////////////////////////////////

JS::Value CMessageTerritoryPositionChanged::ToJSVal(const ScriptInterface& scriptInterface) const
{
	TOJSVAL_SETUP();
	SET_MSG_PROPERTY(entity);
	SET_MSG_PROPERTY(newTerritory);
	return JS::ObjectValue(*obj);
}

CMessage* CMessageTerritoryPositionChanged::FromJSVal(const ScriptInterface& scriptInterface, JS::HandleValue val)
{
	FROMJSVAL_SETUP();
	GET_MSG_PROPERTY(entity_id_t, entity);
	GET_MSG_PROPERTY(player_id_t, newTerritory);
	return new CMessageTerritoryPositionChanged(entity, newTerritory);
}

////////////////////////////////

const std::array<const char*, CMessageMotionUpdate::UpdateType::LENGTH> CMessageMotionUpdate::UpdateTypeStr = { {
	"likelySuccess", "likelyFailure", "obstructed"
} };

JS::Value CMessageMotionUpdate::ToJSVal(const ScriptInterface& scriptInterface) const
{
	TOJSVAL_SETUP();
	JS::RootedValue prop(cx);

	if (!JS_SetProperty(cx, obj, UpdateTypeStr[updateType], JS::TrueHandleValue))
		return JS::UndefinedValue();

	return JS::ObjectValue(*obj);
}

CMessage* CMessageMotionUpdate::FromJSVal(const ScriptInterface& scriptInterface, JS::HandleValue val)
{
	FROMJSVAL_SETUP();
	GET_MSG_PROPERTY(std::wstring, updateString);

	if (updateString == L"likelySuccess")
		return new CMessageMotionUpdate(CMessageMotionUpdate::LIKELY_SUCCESS);
	if (updateString == L"likelyFailure")
		return new CMessageMotionUpdate(CMessageMotionUpdate::LIKELY_FAILURE);
	if (updateString == L"obstructed")
		return new CMessageMotionUpdate(CMessageMotionUpdate::OBSTRUCTED);

	LOGWARNING("CMessageMotionUpdate::FromJSVal passed wrong updateString");
	return NULL;
}

////////////////////////////////

JS::Value CMessageTerrainChanged::ToJSVal(const ScriptInterface& scriptInterface) const
{
	TOJSVAL_SETUP();
	SET_MSG_PROPERTY(i0);
	SET_MSG_PROPERTY(j0);
	SET_MSG_PROPERTY(i1);
	SET_MSG_PROPERTY(j1);
	return JS::ObjectValue(*obj);
}

CMessage* CMessageTerrainChanged::FromJSVal(const ScriptInterface& scriptInterface, JS::HandleValue val)
{
	FROMJSVAL_SETUP();
	GET_MSG_PROPERTY(int32_t, i0);
	GET_MSG_PROPERTY(int32_t, j0);
	GET_MSG_PROPERTY(int32_t, i1);
	GET_MSG_PROPERTY(int32_t, j1);
	return new CMessageTerrainChanged(i0, i1, j0, j1);
}

////////////////////////////////

JS::Value CMessageVisibilityChanged::ToJSVal(const ScriptInterface& scriptInterface) const
{
	TOJSVAL_SETUP();
	SET_MSG_PROPERTY(player);
	SET_MSG_PROPERTY(ent);
	SET_MSG_PROPERTY(oldVisibility);
	SET_MSG_PROPERTY(newVisibility);
	return JS::ObjectValue(*obj);
}

CMessage* CMessageVisibilityChanged::FromJSVal(const ScriptInterface& scriptInterface, JS::HandleValue val)
{
	FROMJSVAL_SETUP();
	GET_MSG_PROPERTY(player_id_t, player);
	GET_MSG_PROPERTY(entity_id_t, ent);
	GET_MSG_PROPERTY(int, oldVisibility);
	GET_MSG_PROPERTY(int, newVisibility);
	return new CMessageVisibilityChanged(player, ent, oldVisibility, newVisibility);
}

////////////////////////////////

JS::Value CMessageWaterChanged::ToJSVal(const ScriptInterface& scriptInterface) const
{
	TOJSVAL_SETUP();
	return JS::ObjectValue(*obj);
}

CMessage* CMessageWaterChanged::FromJSVal(const ScriptInterface& UNUSED(scriptInterface), JS::HandleValue UNUSED(val))
{
	return new CMessageWaterChanged();
}

////////////////////////////////

JS::Value CMessageObstructionMapShapeChanged::ToJSVal(const ScriptInterface& scriptInterface) const
{
	TOJSVAL_SETUP();
	return JS::ObjectValue(*obj);
}

CMessage* CMessageObstructionMapShapeChanged::FromJSVal(const ScriptInterface& UNUSED(scriptInterface), JS::HandleValue UNUSED(val))
{
	return new CMessageObstructionMapShapeChanged();
}

////////////////////////////////

JS::Value CMessageTerritoriesChanged::ToJSVal(const ScriptInterface& scriptInterface) const
{
	TOJSVAL_SETUP();
	return JS::ObjectValue(*obj);
}

CMessage* CMessageTerritoriesChanged::FromJSVal(const ScriptInterface& UNUSED(scriptInterface), JS::HandleValue UNUSED(val))
{
	return new CMessageTerritoriesChanged();
}

////////////////////////////////

JS::Value CMessageRangeUpdate::ToJSVal(const ScriptInterface& scriptInterface) const
{
	TOJSVAL_SETUP();
	SET_MSG_PROPERTY(tag);
	SET_MSG_PROPERTY(added);
	SET_MSG_PROPERTY(removed);
	return JS::ObjectValue(*obj);
}

CMessage* CMessageRangeUpdate::FromJSVal(const ScriptInterface& UNUSED(scriptInterface), JS::HandleValue UNUSED(val))
{
	LOGWARNING("CMessageRangeUpdate::FromJSVal not implemented");
	return NULL;
}

////////////////////////////////

JS::Value CMessagePathResult::ToJSVal(const ScriptInterface& UNUSED(scriptInterface)) const
{
	LOGWARNING("CMessagePathResult::ToJSVal not implemented");
	return JS::UndefinedValue();
}

CMessage* CMessagePathResult::FromJSVal(const ScriptInterface& UNUSED(scriptInterface), JS::HandleValue UNUSED(val))
{
	LOGWARNING("CMessagePathResult::FromJSVal not implemented");
	return NULL;
}

////////////////////////////////

JS::Value CMessageValueModification::ToJSVal(const ScriptInterface& scriptInterface) const
{
	TOJSVAL_SETUP();
	SET_MSG_PROPERTY(entities);
	SET_MSG_PROPERTY(component);
	SET_MSG_PROPERTY(valueNames);
	return JS::ObjectValue(*obj);
}

CMessage* CMessageValueModification::FromJSVal(const ScriptInterface& scriptInterface, JS::HandleValue val)
{
	FROMJSVAL_SETUP();
	GET_MSG_PROPERTY(std::vector<entity_id_t>, entities);
	GET_MSG_PROPERTY(std::wstring, component);
	GET_MSG_PROPERTY(std::vector<std::wstring>, valueNames);
	return new CMessageValueModification(entities, component, valueNames);
}

////////////////////////////////

JS::Value CMessageTemplateModification::ToJSVal(const ScriptInterface& scriptInterface) const
{
	TOJSVAL_SETUP();
	SET_MSG_PROPERTY(player);
	SET_MSG_PROPERTY(component);
	SET_MSG_PROPERTY(valueNames);
	return JS::ObjectValue(*obj);
}

CMessage* CMessageTemplateModification::FromJSVal(const ScriptInterface& scriptInterface, JS::HandleValue val)
{
	FROMJSVAL_SETUP();
	GET_MSG_PROPERTY(player_id_t, player);
	GET_MSG_PROPERTY(std::wstring, component);
	GET_MSG_PROPERTY(std::vector<std::wstring>, valueNames);
	return new CMessageTemplateModification(player, component, valueNames);
}

////////////////////////////////

JS::Value CMessageVisionRangeChanged::ToJSVal(const ScriptInterface& scriptInterface) const
{
	TOJSVAL_SETUP();
	SET_MSG_PROPERTY(entity);
	SET_MSG_PROPERTY(oldRange);
	SET_MSG_PROPERTY(newRange);
	return JS::ObjectValue(*obj);
}

CMessage* CMessageVisionRangeChanged::FromJSVal(const ScriptInterface& scriptInterface, JS::HandleValue val)
{
	FROMJSVAL_SETUP();
	GET_MSG_PROPERTY(entity_id_t, entity);
	GET_MSG_PROPERTY(entity_pos_t, oldRange);
	GET_MSG_PROPERTY(entity_pos_t, newRange);
	return new CMessageVisionRangeChanged(entity, oldRange, newRange);
}

JS::Value CMessageVisionSharingChanged::ToJSVal(const ScriptInterface& scriptInterface) const
{
	TOJSVAL_SETUP();
	SET_MSG_PROPERTY(entity);
	SET_MSG_PROPERTY(player);
	SET_MSG_PROPERTY(add);
	return JS::ObjectValue(*obj);
}

CMessage* CMessageVisionSharingChanged::FromJSVal(const ScriptInterface& scriptInterface, JS::HandleValue val)
{
	FROMJSVAL_SETUP();
	GET_MSG_PROPERTY(entity_id_t, entity);
	GET_MSG_PROPERTY(player_id_t, player);
	GET_MSG_PROPERTY(bool, add);
	return new CMessageVisionSharingChanged(entity, player, add);
}

////////////////////////////////

JS::Value CMessageMinimapPing::ToJSVal(const ScriptInterface& scriptInterface) const
{
	TOJSVAL_SETUP();
	return JS::ObjectValue(*obj);
}

CMessage* CMessageMinimapPing::FromJSVal(const ScriptInterface& UNUSED(scriptInterface), JS::HandleValue UNUSED(val))
{
	return new CMessageMinimapPing();
}

////////////////////////////////

JS::Value CMessageCinemaPathEnded::ToJSVal(const ScriptInterface& scriptInterface) const
{
	TOJSVAL_SETUP();
	SET_MSG_PROPERTY(name);
	return JS::ObjectValue(*obj);
}

CMessage* CMessageCinemaPathEnded::FromJSVal(const ScriptInterface& scriptInterface, JS::HandleValue val)
{
	FROMJSVAL_SETUP();
	GET_MSG_PROPERTY(CStrW, name);
	return new CMessageCinemaPathEnded(name);
}

////////////////////////////////

JS::Value CMessageCinemaQueueEnded::ToJSVal(const ScriptInterface& scriptInterface) const
{
	TOJSVAL_SETUP();
	return JS::ObjectValue(*obj);
}

CMessage* CMessageCinemaQueueEnded::FromJSVal(const ScriptInterface& scriptInterface, JS::HandleValue val)
{
	FROMJSVAL_SETUP();
	return new CMessageCinemaQueueEnded();
}

////////////////////////////////////////////////////////////////

JS::Value CMessagePlayerColorChanged::ToJSVal(const ScriptInterface& scriptInterface) const
{
	TOJSVAL_SETUP();
	SET_MSG_PROPERTY(player);
	return JS::ObjectValue(*obj);
}

CMessage* CMessagePlayerColorChanged::FromJSVal(const ScriptInterface& scriptInterface, JS::HandleValue val)
{
	FROMJSVAL_SETUP();
	GET_MSG_PROPERTY(player_id_t, player);
	return new CMessagePlayerColorChanged(player);
}

////////////////////////////////////////////////////////////////

CMessage* CMessageFromJSVal(int mtid, const ScriptInterface& scriptingInterface, JS::HandleValue val)
{
	switch (mtid)
	{
#define MESSAGE(name) case MT_##name: return CMessage##name::FromJSVal(scriptingInterface, val);
#define INTERFACE(name)
#define COMPONENT(name)
#include "simulation2/TypeList.h"
#undef COMPONENT
#undef INTERFACE
#undef MESSAGE
	}

	return NULL;
}
