/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/server/gamecontext.h>
#include <engine/shared/config.h>

#include "merc-bomb.h"

#include <game/server/infclass/damage_type.h>
#include <game/server/infclass/infcgamecontroller.h>

#include "growingexplosion.h"
#include "infccharacter.h"

CMercenaryBomb::CMercenaryBomb(CGameContext *pGameContext, vec2 Pos, int Owner)
	: CPlacedObject(pGameContext, CGameWorld::ENTTYPE_MERCENARY_BOMB, Pos, Owner)
{
	m_InfClassObjectType = INFCLASS_OBJECT_TYPE_MERCENARY_BOMB;
	GameWorld()->InsertEntity(this);
	m_LoadingTick = Server()->TickSpeed();
	m_Damage = 0;
	
	for(int i=0; i<NUM_IDS; i++)
	{
		m_IDs[i] = Server()->SnapNewID();
	}

	GameServer()->CreateSound(GetPos(), SOUND_PICKUP_ARMOR);
}

CMercenaryBomb::~CMercenaryBomb()
{
	for(int i=0; i<NUM_IDS; i++)
	{
		Server()->SnapFreeID(m_IDs[i]);
	}
}

void CMercenaryBomb::Upgrade(float Points)
{
	float MaxDamage = Config()->m_InfMercBombs;
	float NewDamage = minimum(MaxDamage, m_Damage + Points);
	if(NewDamage <= m_Damage)
	{
		return;
	}

	m_Damage = NewDamage;

	GameServer()->CreateSound(GetPos(), SOUND_PICKUP_ARMOR);
}

void CMercenaryBomb::Tick()
{
	if(m_MarkedForDestroy) return;
	
	if(m_Damage >= Config()->m_InfMercBombs && m_LoadingTick > 0)
		m_LoadingTick--;
	
	// Find other players
	bool MustExplode = false;
	for(CInfClassCharacter *p = (CInfClassCharacter*) GameWorld()->FindFirst(CGameWorld::ENTTYPE_CHARACTER); p; p = (CInfClassCharacter *)p->TypeNext())
	{
		if(p->IsHuman()) continue;
		if(!p->CanDie()) continue;

		float Len = distance(p->m_Pos, m_Pos);
		if(Len < p->m_ProximityRadius + GetMaxRadius())
		{
			MustExplode = true;
			break;
		}
	}
	
	if(MustExplode)
		Explode();
}

void CMercenaryBomb::Explode()
{
	float Factor = static_cast<float>(m_Damage)/Config()->m_InfMercBombs;
	
	if(m_Damage > 1)
	{
		GameServer()->CreateSound(m_Pos, SOUND_GRENADE_EXPLODE);
		new CGrowingExplosion(GameServer(), m_Pos, vec2(0.0, -1.0), m_Owner, 16.0f * Factor, DAMAGE_TYPE::MERCENARY_BOMB);
	}
				
	GameServer()->m_World.DestroyEntity(this);
}

bool CMercenaryBomb::ReadyToExplode()
{
	return m_LoadingTick <= 0;
}

float CMercenaryBomb::GetMaxRadius()
{
	return 80;
}

void CMercenaryBomb::Snap(int SnappingClient)
{
	if(!DoSnapForClient(SnappingClient))
		return;

	//CPlayer* pClient = GameServer()->m_apPlayers[SnappingClient];
	//if(pClient->IsZombie()) // invisible for zombies
	//	return;

	if(Server()->GetClientInfclassVersion(SnappingClient))
	{
		CNetObj_InfClassObject *pInfClassObject = SnapInfClassObject();
		if(!pInfClassObject)
			return;
	}

	float AngleStart = (2.0f * pi * Server()->Tick()/static_cast<float>(Server()->TickSpeed()))/10.0f;
	float AngleStep = 2.0f * pi / CMercenaryBomb::NUM_SIDE;
	float R = 50.0f*static_cast<float>(m_Damage)/Config()->m_InfMercBombs;
	for(int i=0; i<CMercenaryBomb::NUM_SIDE; i++)
	{
		vec2 PosStart = m_Pos + vec2(R * cos(AngleStart + AngleStep*i), R * sin(AngleStart + AngleStep*i));

		CNetObj_Pickup *pP = static_cast<CNetObj_Pickup *>(Server()->SnapNewItem(NETOBJTYPE_PICKUP, m_IDs[i], sizeof(CNetObj_Pickup)));
		if(!pP)
			return;

		pP->m_X = (int)PosStart.x;
		pP->m_Y = (int)PosStart.y;
		pP->m_Type = POWERUP_HEALTH;
		pP->m_Subtype = 0;
	}

	if(SnappingClient == m_Owner && m_LoadingTick > 0)
	{
		R = GetMaxRadius();
		AngleStart = AngleStart*2.0f;
		for(int i=0; i<CMercenaryBomb::NUM_SIDE; i++)
		{
			vec2 PosStart = m_Pos + vec2(R * cos(AngleStart + AngleStep*i), R * sin(AngleStart + AngleStep*i));
			GameController()->SendHammerDot(PosStart, m_IDs[CMercenaryBomb::NUM_SIDE+i]);
		}
	}
}
