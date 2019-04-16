#pragma newdecls required
#pragma semicolon 1 

#include <sourcemod>
#include <sdktools>
#include <sdkhooks>
#include <dhooks>

Handle g_hWeapon_ShootPosition = INVALID_HANDLE;
float  g_vecOldWeaponShootPos[MAXPLAYERS + 1][3];

public Plugin myinfo =
{
	name = "Bullet position fix",
	author = "xutaxkamay",
	description = "Fixes shoot position",
	version = "1.0",
	url = "https://forums.alliedmods.net/showthread.php?p=2646571"
};

public void OnPluginStart()
{
	Handle gameData = LoadGameConfigFile("dhooks.weapon_shootposition");
	
	if (gameData == INVALID_HANDLE)
	{
		SetFailState("[FireBullets Fix] No game data present");
	}
		
	int offset = GameConfGetOffset(gameData, "Weapon_ShootPosition");

	if (offset == -1)
	{
		SetFailState("[FireBullets Fix] failed to find offset");
	}

	LogMessage("Found offset for Weapon_ShootPosition %d", offset);

	CloseHandle(gameData);

	g_hWeapon_ShootPosition = DHookCreate(offset, HookType_Entity, ReturnType_Vector, ThisPointer_CBaseEntity);

	if (g_hWeapon_ShootPosition == INVALID_HANDLE)
	{
		SetFailState("[FireBullets Fix] couldn't hook Weapon_ShootPosition");
	}

	for (int client = 1; client <= MaxClients; client++)
		OnClientPutInServer(client);
}

public void OnClientPutInServer(int client)
{
	if (IsClientConnected(client) && IsClientInGame(client) && !IsFakeClient(client))
	{
		DHookEntity(g_hWeapon_ShootPosition, true, client, RemovalCB, Weapon_ShootPosition_Post);
	}
}

void RemovalCB(int hookId)
{
	PrintToServer("Removed hook %i", hookId);
}

public Action OnPlayerRunCmd(int client)
{	
	if (IsClientConnected(client) && IsClientInGame(client) && !IsFakeClient(client))
		GetClientEyePosition(client, g_vecOldWeaponShootPos[client]); // or SDKCall(g_hWeapon_ShootPositionCall, client, g_vecOldWeaponShootPos[client]); 
		
	return Plugin_Continue;
}

public MRESReturn Weapon_ShootPosition_Post(int client, Handle hReturn)
{
	PrintToChat(client, "Calling old");
	float vec[3];
	DHookGetReturnVector(hReturn, vec);
	PrintToChat(client, "Calling new %f %f %f - %f %f %f", vec[0], vec[1], vec[2], g_vecOldWeaponShootPos[client][0], g_vecOldWeaponShootPos[client][1], g_vecOldWeaponShootPos[client][2]);
	// Still crashes, dhooks might need some recompiling soon.. If it's the problem.
	DHookSetReturnVector(hReturn, vec);

	return MRES_Ignored;
}

