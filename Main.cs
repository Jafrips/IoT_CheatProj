using Swed64;
using System;
using System.Numerics;
using System.Threading;

// init memory for lib class
Swed swed = new("cs2");

//get client.dll
IntPtr client = swed.GetModuleBase("client.dll");

// offsets for entity list
int dwEntityList = 0x18C6268;   //check 27.04.24
int m_hPlayerPawn = 0x7E4;      // const
int m_iHealth = 0x334;          // const
int m_iszPlayerName = 0x638;    // const
int m_iTeamNum = 0x3CB;         // const
int m_pos = 0x127C;             // m_vOldOrigin, client.dll.cs check 27.04.24
int m_bIsLocalPlayerController = 0x6C8;
int m_entitySpottedState = 0x2278 + 0x8; // EntitySpottedState_t (spottedstate + mbspotted)
                                         // m_bSpotted = 0x8; // bool
                                         // m_bSpottedByMask = 0xC; // uint32[2]

// positioning vars
Vector3 Pos = new Vector3();
Vector3 LocalPos = new Vector3();
float Distance;
// other vars
String PlayerTeam = "";
String LocalPlayerTeam = "";
bool IsSpotted = false;
bool IsLocalPlayer = false;

// get entity list
IntPtr entityList = swed.ReadPointer(client, dwEntityList);

// first entry into entity list
IntPtr listEntry = swed.ReadPointer(entityList, 0x10); // we dont have any ID yet

// loop through entity list
for (int i = 0; i < 64; i++) // 64 controllers max on server
{
    if (listEntry == IntPtr.Zero) // skip iteration if entry invalid
        continue;

    //get current controller
    IntPtr currentController = swed.ReadPointer(listEntry, i * 0x78);

    if (currentController == IntPtr.Zero) // skip if controller invalid
        continue;

    // get pawn handle
    int pawnHandle = swed.ReadInt(currentController, m_hPlayerPawn);

    if (pawnHandle == 0)
        continue;

    // second entry, now we find the pawn !
    // apply bitmask (0x7FFF) and shift bits by 9
    IntPtr listEntry2 = swed.ReadPointer(entityList, 0x8 * ((pawnHandle & 0x7FFF) >> 9) + 0x10);

    // read current pawn, apply bitmask to stay inside range
    IntPtr currentPawn = swed.ReadPointer(listEntry2, 0x78 * (pawnHandle & 0x1FF));

    // check if its local pawn (local player)
    IsLocalPlayer = swed.ReadBool(currentController, m_bIsLocalPlayerController);

    // get player team
    uint team = swed.ReadUInt(currentPawn, m_iTeamNum);
    if (team == 3)
        PlayerTeam = "CT";
    else if (team == 2)
        PlayerTeam = "T";

    if (IsLocalPlayer)
    {
        // finding position
        LocalPos = swed.ReadVec(currentPawn, m_pos);

        // get local player team
        LocalPlayerTeam = PlayerTeam;
    }
    else
    {
        // get pawn health
        uint health = swed.ReadUInt(currentPawn, m_iHealth);
        if (!(health > 0))
            continue; // skip if not alive
        else if (LocalPlayerTeam == PlayerTeam)
            continue; // skip if its teammate

        // check if spotted (bool)
        IsSpotted = swed.ReadBool(currentPawn, m_entitySpottedState);

        // get controller attributes
        string name = swed.ReadString(currentController, m_iszPlayerName, 16); // 16 characters

        Pos = swed.ReadVec(currentPawn, m_pos);
        Distance = Vector3.Distance(LocalPos, Pos);
        //Console.WriteLine($"Distance to player: {Distance}");

        Console.WriteLine($"{name}: {health}hp, team: {PlayerTeam}, Distance: {Distance}, Spotted: {IsSpotted}");
    }
}
