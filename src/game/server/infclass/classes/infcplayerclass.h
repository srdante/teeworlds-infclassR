#ifndef GAME_SERVER_INFCLASS_CLASSES_PLAYER_CLASS_H
#define GAME_SERVER_INFCLASS_CLASSES_PLAYER_CLASS_H

#include <base/vmath.h>
#include <game/server/entity.h>

class CConfig;
class CGameContext;
class CGameWorld;
class CInfClassCharacter;
class CInfClassGameContext;
class CInfClassGameController;
class CInfClassPlayer;
class IServer;

class CInfClassPlayerClass
{
public:
	CInfClassPlayerClass(CInfClassPlayer *pPlayer);
	virtual ~CInfClassPlayerClass() = default;

	void SetCharacter(CInfClassCharacter *character);

	virtual bool IsHuman() const = 0;
	bool IsZombie() const;

	// Temp stuff
	int PlayerClass() const;
	void OnPlayerClassChanged();

	void Poison(int Count, int From);

	// Events
	virtual void Tick();
	virtual void OnCharacterSpawned();

	virtual void OnSlimeEffect(int Owner) = 0;

	CGameContext *GameContext() const;
	CGameContext *GameServer() const;
	CGameWorld *GameWorld() const;
	CInfClassGameController *GameController() const;
	CConfig *Config();
	IServer *Server();
	CInfClassPlayer *GetPlayer();
	int GetCID();
	vec2 GetPos() const;
	vec2 GetDirection() const;
	float GetProximityRadius() const;

protected:
	virtual void GiveClassAttributes();

	CInfClassPlayer *m_pPlayer = nullptr;
	CInfClassCharacter *m_pCharacter = nullptr;

	int m_Poison = 0;
	int m_PoisonTick = 0;
	int m_PoisonFrom = 0;
};

#endif // GAME_SERVER_INFCLASS_CLASSES_PLAYER_CLASS_H