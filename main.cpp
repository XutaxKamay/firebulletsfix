// clang-format off
#include "headers.h"
#include "main.h"
// clang-format on

CFireBulletFix g_FireBulletFix;

CDetour<void,
        int,
        int,
        int,
        Vector&,
        QAngle&,
        int,
        int,
        int,
        float,
        float,
        float,
        float,
        int,
        float>* CFireBulletFix::detourFireBullets;

CDetour<void, ptr_t, ptr_t, ptr_t, ptr_t>* CFireBulletFix::detourRunCommand;

Vector CFireBulletFix::vecOldShootPos[MAX_PLAYERS + 1];

Msg_t Msg = reinterpret_cast<Msg_t>(
    dlsym(dlopen("bin/engine.so", RTLD_NOW), "Msg"));

CPatternScan patternGlobals("csgo/bin/server.so",
                            "A1 ? ? ? ? F3 0F 10 40 ? 0F 2F 43 40 0F 86 ? ? ? "
                            "? 8B 06 8D 96");

static auto uGlobals = *reinterpret_cast<uintptr_t*>(
    patternGlobals.get<uintptr_t>() + 1);

InterfaceReg* InterfaceReg::s_pInterfaceRegs = nullptr;

InterfaceReg::InterfaceReg(InstantiateInterfaceFn fn, const char* pName) :
    m_pName(pName)
{
    m_CreateFn = fn;
    m_pNext = s_pInterfaceRegs;
    s_pInterfaceRegs = this;
}

void* CreateInterfaceInternal(const char* pName, int* pReturnCode)
{
    InterfaceReg* pCur;

    for (pCur = InterfaceReg::s_pInterfaceRegs; pCur; pCur = pCur->m_pNext)
    {
        if (strcmp(pCur->m_pName, pName) == 0)
        {
            if (pReturnCode)
            {
                *pReturnCode = IFACE_OK;
            }
            return pCur->m_CreateFn();
        }
    }

    if (pReturnCode)
    {
        *pReturnCode = IFACE_FAILED;
    }

    return NULL;
}

DLL_EXPORT void* CreateInterface(const char* pName, int* pReturnCode)
{
    return CreateInterfaceInternal(pName, pReturnCode);
}

void CFireBulletFix::Unload()
{
    delete detourFireBullets;
    delete detourRunCommand;
}

bool CFireBulletFix::Load(CreateInterfaceFn interfaceFactory,
                          CreateInterfaceFn gameServerFactory)
{
    auto pRunCommandAddress = CPatternScan("csgo/bin/server.so",
                                           "55 89 E5 57 56 53 81 EC ? ? ? ? 8B "
                                           "5D 0C 8B 7D 08 8B 75 10 8B 93")
                                  .get();

    detourRunCommand = new CDetour<void, ptr_t, ptr_t, ptr_t, ptr_t>(
        pRunCommandAddress, reinterpret_cast<ptr_t>(CPlayerMove_RunCommand));

    detourRunCommand->Detour();
    Msg("pRunCommand_Detour %p ...\n", pRunCommandAddress);

    auto pFX_FireBullets = CPatternScan("csgo/bin/server.so",
                                        "55 89 E5 57 56 53 81 EC ? ? ? ? 8B 5D "
                                        "10 8B 75 14 8B 7D 18 85 DB 0F 84")
                               .get();

    detourFireBullets = new CDetour<void,
                                    int,
                                    int,
                                    int,
                                    Vector&,
                                    QAngle&,
                                    int,
                                    int,
                                    int,
                                    float,
                                    float,
                                    float,
                                    float,
                                    int,
                                    float>(
        pFX_FireBullets, reinterpret_cast<ptr_t>(FX_FireBullets));

    detourFireBullets->Detour();

    Msg("pFX_FireBullets_Detour %p ...\n", pFX_FireBullets);

    return true;
}

// You gotta reverse yourself the other variables names heh fuck I'm incredibly
// too lazy for that. I guess a reference to the lastest csgo mac libs should be
// enough.

/*

.text:00BB9F2C loc_BB9F2C:                             ; CODE XREF:
sub_BB9DF0+12C↑j .text:00BB9F2C mov     ecx, [ebp+var_44]
.text:00BB9F2F                 fstp    dword ptr [esp+28h] ; 11
.text:00BB9F33 ; 93:   *(&v13 + 1) = v26;
.text:00BB9F33                 movss   xmm0, [ebp+var_40]
.text:00BB9F38                 fstp    dword ptr [esp+24h] ; 10
.text:00BB9F3C ; 94:   *&v13 = v25;
.text:00BB9F3C                 fstp    dword ptr [esp+20h] ; 9
.text:00BB9F40                 movss   dword ptr [esp+30h], xmm0 ; 13
.text:00BB9F46                 mov     [esp+18h], ecx  ; 7
.text:00BB9F4A                 lea     ecx, [ebp+var_30]
.text:00BB9F4D                 mov     dword ptr [esp+2Ch], 1 ; 12
.text:00BB9F55                 fstp    dword ptr [esp+1Ch] ; 8
.text:00BB9F59                 mov     dword ptr [esp+14h], 0 ; 6
.text:00BB9F61                 mov     [esp+10h], edi  ; 5
.text:00BB9F65                 mov     [esp+0Ch], ecx  ; 4
.text:00BB9F69                 mov     [esp+8], eax    ; 3
.text:00BB9F6D                 mov     [esp+4], ebx    ; 2
.text:00BB9F71                 mov     [esp], edx      ; 1
.text:00BB9F74                 call    sub_B34410

*/

int GetPlayerIndex(uintptr_t Player)
{
    auto edict = *reinterpret_cast<uintptr_t*>(Player + 0x24);
    int index = 0;

    /*asm volatile( "mov eax, [%1]\n\t"
                              "mov edx, %2\n\t"
                              "sub edx, [eax+0x60]\n\t"
                              "mov eax, edx\n\t"
                              "sar eax, 4\n\t"
                              "mov %0, eax\n\t"
                              : "=r"( index )
                              : "r"( uEdicts ), "r"( edict )
                              : "%eax", "%edx" );*/

    if (edict)
        index = (edict - *reinterpret_cast<uintptr_t*>(
                             *reinterpret_cast<uintptr_t*>(uGlobals) + 0x60)) >>
                4;

    return index;
}

void CFireBulletFix::FX_FireBullets(int playerIndex,
                                    int a1,
                                    int a2,
                                    Vector& vOrigin,
                                    QAngle& qAngle,
                                    int a5,
                                    int a6,
                                    int a7,
                                    float a8,
                                    float a9,
                                    float a10,
                                    float a11,
                                    int a12,
                                    float a13)
{
    detourFireBullets->GetBeingCalled() = true;

    /* 
    Msg("%f %f %f - %f %f %f\n",
        vOrigin.x,
        vOrigin.y,
        vOrigin.z,
        qAngle.x,
        qAngle.y,
        qAngle.z);
    */

    // Here we ignore the last computed origin and viewoffset from
    // processmovement and use the old one instead.
    // vOrigin = vecOldShootPos[playerIndex];

    detourFireBullets->CallOriginal(playerIndex,
                                    a1,
                                    a2,
                                    vecOldShootPos[playerIndex],
                                    qAngle,
                                    a5,
                                    a6,
                                    a7,
                                    a8,
                                    a9,
                                    a10,
                                    a11,
                                    a12,
                                    a13);

    detourFireBullets->GetBeingCalled() = false;
}

void CFireBulletFix::CPlayerMove_RunCommand(ptr_t thisptr,
                                            ptr_t player,
                                            ptr_t pCmd,
                                            ptr_t moveHelper)
{
    detourRunCommand->GetBeingCalled() = true;

    int playerIndex = GetPlayerIndex(reinterpret_cast<uintptr_t>(player));

    // GetEyePosition
    vecOldShootPos[playerIndex] = reinterpret_cast<Vector (*)(ptr_t)>(
        sub_GetVTable(player)[131])(player);

    detourRunCommand->CallOriginal(thisptr, player, pCmd, moveHelper);
    detourRunCommand->GetBeingCalled() = false;
}

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CFireBulletFix,
                                  IServerPluginCallbacks,
                                  INTERFACEVERSION_ISERVERPLUGINCALLBACKS,
                                  g_FireBulletFix);
