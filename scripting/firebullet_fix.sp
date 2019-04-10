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
	url = "https://steamcommunity.com/profiles/76561198171398301"
};

public void OnPluginStart()
{
	Handle gameData = LoadGameConfigFile("dhooks.weapon_shootposition");
	
	if(gameData == INVALID_HANDLE)
	{
		SetFailState("[FireBullets Fix] No game data present");
	}
		
	int offset = GameConfGetOffset(gameData, "Weapon_ShootPosition");
	g_hWeapon_ShootPosition = DHookCreate(offset, HookType_Entity, ReturnType_Vector, ThisPointer_CBaseEntity, Weapon_ShootPosition);
	
	CloseHandle(gameData);

	if (g_hWeapon_ShootPosition == INVALID_HANDLE)
	{
		SetFailState("[FireBullets Fix] couldn't hook Weapon_ShootPosition");
	}
}

public void OnClientPutInServer(int client)
{
	if(IsClientConnected(client) && IsClientInGame(client) && !IsFakeClient(client))
	{
		DHookEntity(g_hWeapon_ShootPosition, true, client, RemovalCB);
	}
}

void RemovalCB(int hookId)
{
	PrintToServer("Removed hook %i", hookId);
}

public Action OnPlayerRunCmd(int client)
{	
	if(IsClientConnected(client) && IsClientInGame(client) && !IsFakeClient(client))
		GetClientEyePosition(client, g_vecOldWeaponShootPos[client]); // or SDKCall(g_hWeapon_ShootPositionCall, client, g_vecOldWeaponShootPos[client]); 
		
	return Plugin_Continue;
}

public MRESReturn Weapon_ShootPosition(int client, Handle hReturn)
{
	// float vec[3];
	// DHookGetReturnVector(hReturn, vec);
	// PrintToChat(client, "Calling new %f %f %f - %f %f %f", vec[0], vec[1], vec[2], g_vecOldWeaponShootPos[client][0], g_vecOldWeaponShootPos[client][1], g_vecOldWeaponShootPos[client][2]);
	DHookSetReturnVector(hReturn, g_vecOldWeaponShootPos[client]);
	return MRES_Supercede;
}

