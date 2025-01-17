#include "infc-placed-object.h"

#include "infccharacter.h"

#include <engine/server.h>
#include <game/server/infclass/infcgamecontroller.h>

CPlacedObject::CPlacedObject(CGameContext *pGameContext, int ObjectType, vec2 Pos, int Owner, int ProximityRadius)
	: CInfCEntity(pGameContext, ObjectType, Pos, Owner, ProximityRadius)
{
	m_InfClassObjectID = Server()->SnapNewID();
	m_InfClassObjectType = INFCLASS_OBJECT_TYPE_CUSTOM;
}

CPlacedObject::~CPlacedObject()
{
	Server()->SnapFreeID(m_InfClassObjectID);
}

bool CPlacedObject::DoSnapForClient(int SnappingClient)
{
	if(!CInfCEntity::DoSnapForClient(SnappingClient))
		return false;

	CInfClassCharacter *pCharacter = GameController()->GetCharacter(SnappingClient);
	if(pCharacter && pCharacter->IsBlind())
		return false;

	return true;
}

CNetObj_InfClassObject *CPlacedObject::SnapInfClassObject()
{
	CNetObj_InfClassObject *pInfClassObject = static_cast<CNetObj_InfClassObject *>(Server()->SnapNewItem(NETOBJTYPE_INFCLASSOBJECT, m_InfClassObjectID, sizeof(CNetObj_InfClassObject)));
	if(!pInfClassObject)
		return nullptr;

	pInfClassObject->m_X = m_Pos.x;
	pInfClassObject->m_Y = m_Pos.y;

	pInfClassObject->m_Type = m_InfClassObjectType;
	pInfClassObject->m_Flags = 0;

	pInfClassObject->m_Owner = GetOwner();

	pInfClassObject->m_StartTick = 0;
	pInfClassObject->m_LifeSpan = 0;

	pInfClassObject->m_X2 = 0;
	pInfClassObject->m_Y2 = 0;

	return pInfClassObject;
}
