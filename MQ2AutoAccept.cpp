// MQ2AutoAccept.cpp : RedGuides exclusive auto accept group/raid invite, clicking of dialog boxes (group task adds, expeditions, and wizard teleports among others)
// v1.0 - Sym - 04-23-2012
// v1.01 - Sym - 08-05-2012 - Site name update
// v1.02 - Sym - 09-27-2012 - Added GlobalNames ini section. Needs to be edited in ini file, add/del does not affect globals.
// v1.03 - Eqmule - 04-23-2016 - removed ParseMacroData because this is a plugin not a macro ;) added accept for another dialog.
// v2.0 - Eqmule 07-22-2016 - Added string safety.
// v2.1 - Sym - 07-23-2017 - Added specific anchor name checking and addanchor/delanchor commands.
//    /autoaccept addanchor VALUE :: Add VALUE as a valid anchor target. Proper case matters for matching later. Put the entire address inside quotes as it shows in the portal dialog box such as "Willow Circle Bay, 100 Vanward Heights"
//    /autoaccept delanchor VALUE :: Delete VALUE from your valid anchor target list. Put the entire address inside quotes as it shows in the portal dialog box such as "Willow Circle Bay, 100 Vanward Heights"
// v2.11 - Sym - 07-28-2017 - Cleaned up anchor port code, added selfanchor option
//    /autoaccept selfanchor on|off :: Toggle acceptance of primary/secondary real estate anchor port when you cast it.  Default *OFF*
// v2.12 - Sym - 08-07-2017 - Added wizard translocate and druid zephyr casts to translocate toggle. Previously it was translocate to bind only.

#include "../MQ2Plugin.h"
using namespace std;
#include <vector>
PLUGIN_VERSION(2.12);

#pragma warning(disable:4800)
PreSetup("MQ2AutoAccept");

vector <string> vIniNames;
vector <string> vGlobalNames;
vector <string> vNames;
vector <string> vAnchors;
#define SKIP_PULSES 40

long SkipPulse = 0;

char szTemp[MAX_STRING];
char szName[MAX_STRING];
char szName2[MAX_STRING];
char szList[MAX_STRING];
char szAnchors[MAX_STRING];
bool bAutoAccept = false;
bool bTranslocate = false;
bool bAnchor = false;
bool bSelfAnchor = false;
bool bTrade = true;
bool bTradeAlways = false;
bool bGroup = true;
bool bRaid = true;
bool bInitDone = false;
bool bTradeReject = false;

ULONGLONG rejectTimer = 0;


void CombineNames() {
    vNames.clear();
    for (register unsigned int a = 0; a < vGlobalNames.size(); a++) {
        string& vRef = vGlobalNames[a];
        vNames.push_back(vRef.c_str());
    }
    for (register unsigned int a = 0; a < vIniNames.size(); a++) {
        string& vRef = vIniNames[a];
        vNames.push_back(vRef.c_str());
    }
}

bool WindowOpen(PCHAR WindowName) {
    PCSIDLWND pWnd=(PCSIDLWND)FindMQ2Window(WindowName);
    return (!pWnd)?false:(BOOL)pWnd->dShow;
}

void Update_INIFileName() {
    //sprintf_s(INIFileName,"%s\\%s_%s.ini",gszINIPath, EQADDR_SERVERNAME, GetCharInfo()->Name);
    sprintf_s(INIFileName,"%s\\MQ2AutoAccept.ini",gszINIPath);
}

void ListUsers() {
    WriteChatf("User list contains \ay%d\ax %s", vNames.size(), vNames.size() > 1 ? "entries" : "entry");
    for (unsigned int a = 0; a < vNames.size(); a++) {
        string& vRef = vNames[a];
        WriteChatf("\at%d: %s\ax", a+1, vRef.c_str());
    }
}

void ListAnchors() {
    WriteChatf("Anchor list contains \ay%d\ax %s", vAnchors.size(), vAnchors.size() > 1 ? "entries" : "entry");
    for (unsigned int a = 0; a < vAnchors.size(); a++) {
        string& vRef = vAnchors[a];
        WriteChatf("\at%d: %s\ax", a+1, vRef.c_str());
    }
}

VOID SaveINI(VOID)
{
    Update_INIFileName();
    // write on/off settings
    sprintf_s(szTemp,"%s_Settings",GetCharInfo()->Name);
    WritePrivateProfileSection(szTemp, "", INIFileName);
    WritePrivateProfileString(szTemp,"Enabled",bAutoAccept ? "1" : "0",INIFileName);
    WritePrivateProfileString(szTemp,"Translocate",bTranslocate ? "1" : "0",INIFileName);
    WritePrivateProfileString(szTemp,"Anchor",bAnchor ? "1" : "0",INIFileName);
    WritePrivateProfileString(szTemp,"SelfAnchor",bSelfAnchor ? "1" : "0",INIFileName);
    WritePrivateProfileString(szTemp,"Trade",bTrade ? "1" : "0",INIFileName);
    WritePrivateProfileString(szTemp,"TradeAlways",bTradeAlways ? "1" : "0",INIFileName);
    WritePrivateProfileString(szTemp,"TradeReject",bTradeReject ? "1" : "0",INIFileName);
    WritePrivateProfileString(szTemp,"Group",bGroup ? "1" : "0",INIFileName);
    WritePrivateProfileString(szTemp,"Raid",bRaid ? "1" : "0",INIFileName);


    // write all names
    sprintf_s(szTemp,"%s_Names",GetCharInfo()->Name);
    WritePrivateProfileSection(szTemp, "", INIFileName);
    for (unsigned int a = 0; a < vIniNames.size(); a++) {
        string& vRef = vIniNames[a];
        WritePrivateProfileString(szTemp,vRef.c_str(),"1",INIFileName);
    }

    char szA[MAX_STRING];
    // write all anchors
    sprintf_s(szTemp,"%s_Anchors",GetCharInfo()->Name);
    WritePrivateProfileSection(szTemp, "", INIFileName);
    for (unsigned int a = 0; a < vAnchors.size(); a++) {
        string& vRef = vAnchors[a];
        sprintf_s(szA,"Anchor%d",a);
        WritePrivateProfileString(szTemp,szA,vRef.c_str(),INIFileName);
    }
    WriteChatf("MQ2AutoAccept :: Settings updated");
}

VOID LoadINI(VOID)
{
    Update_INIFileName();
    // get on/off settings
    sprintf_s(szTemp,"%s_Settings",GetCharInfo()->Name);
    bAutoAccept = GetPrivateProfileInt(szTemp, "Enabled", 1, INIFileName) > 0 ? true : false;
    bTranslocate = GetPrivateProfileInt(szTemp, "Translocate", 0, INIFileName) > 0 ? true : false;
    bAnchor = GetPrivateProfileInt(szTemp, "Anchor", 0, INIFileName) > 0 ? true : false;
    bSelfAnchor = GetPrivateProfileInt(szTemp, "SelfAnchor", 0, INIFileName) > 0 ? true : false;
    bTrade = GetPrivateProfileInt(szTemp, "Trade", 1, INIFileName) > 0 ? true : false;
    bTradeAlways = GetPrivateProfileInt(szTemp, "TradeAlways", 0, INIFileName) > 0 ? true : false;
    bTradeReject = GetPrivateProfileInt(szTemp, "TradeReject", 0, INIFileName) > 0 ? true : false;
    bGroup = GetPrivateProfileInt(szTemp, "Group", 1, INIFileName) > 0 ? true : false;
    bRaid = GetPrivateProfileInt(szTemp, "Raid", 1, INIFileName) > 0 ? true : false;

    // get all names
    sprintf_s(szTemp,"%s_Names",GetCharInfo()->Name);
    GetPrivateProfileSection(szTemp,szList,MAX_STRING,INIFileName);

    // clear list
    vIniNames.clear();

    char* p = (char*)szList;
    char *pch = 0;
    char *Next_Token1 = 0;
    size_t length = 0;
    int i = 0;
    // loop through all entries under _Names
    // values are terminated by \0, final value is teminated with \0\0
    // values look like
    // Charactername=1
    while (*p)
    {
        length = strlen(p);
        // split entries on =
        pch = strtok_s(p,"=",&Next_Token1);
        while (pch != NULL)
        {
            // Odd entries are the names. Add it to the list
            vIniNames.push_back(pch);

            // next is value. Don't use it so skip it
            pch = strtok_s(NULL, "=",&Next_Token1);

            // next name
            pch = strtok_s(NULL, "=",&Next_Token1);
            i++;
        }
        p += length;
        p++;
    }
    // if we have entries show them
    GetPrivateProfileSection("Global_Names",szList,MAX_STRING,INIFileName);
    vGlobalNames.clear();

    p = (char*)szList;
    length = 0;
    i = 0;
    // loop through all entries under _Names
    // values are terminated by \0, final value is teminated with \0\0
    // values look like
    // Charactername=1
    while (*p)
    {
        length = strlen(p);
        // split entries on =
        pch = strtok_s(p,"=",&Next_Token1);
        while (pch != NULL)
        {
            // Odd entries are the names. Add it to the list
            vGlobalNames.push_back(pch);

            // next is value. Don't use it so skip it
            pch = strtok_s(NULL, "=",&Next_Token1);

            // next name
            pch = strtok_s(NULL, "=",&Next_Token1);
            i++;
        }
        p += length;
        p++;
    }

    // if we have entries show them
    CombineNames();
    if (vNames.size()) ListUsers();

    // get all anchors
    sprintf_s(szTemp,"%s_Anchors",GetCharInfo()->Name);
    GetPrivateProfileSection(szTemp,szList,MAX_STRING,INIFileName);

    // clear list
    vAnchors.clear();

    p = (char*)szList;
    length = 0;
    i = 0;
    // loop through all entries under _Names
    // values are terminated by \0, final value is teminated with \0\0
    // values look like
    // Anchor0=anchorname
    while (*p)
    {
        length = strlen(p);
        // split entries on =
        pch = strtok_s(p,"=",&Next_Token1);
        while (pch != NULL)
        {
            // Odd values are the numbered entries. Don't use it so skip it
            pch = strtok_s(NULL, "=",&Next_Token1);

            // Even entries are the anchor values. Add it to the list
            vAnchors.push_back(pch);

            // next anchor
            pch = strtok_s(NULL, "=",&Next_Token1);
            i++;
        }
        p += length;
        p++;
    }
    if (vAnchors.size()) ListAnchors();
    // flag first load init as done

    bInitDone = true;
}

PLUGIN_API VOID SetGameState(DWORD GameState) {
    if(GameState==GAMESTATE_INGAME) {
        if (!bInitDone) LoadINI();
    } else if(GameState!=GAMESTATE_LOGGINGIN) {
        if (bInitDone) bInitDone=false;
    }
}

void ShowHelp(void) {
    WriteChatf("\atMQ2AutoAccept :: v%1.2f :: by Sym for RedGuides.com\ax", MQ2Version);
    WriteChatf("/autoaccept :: Lists command syntax");
    WriteChatf("/autoaccept on|off :: Main accept toggle. Nothing else will accept if this is off. Default \ag*ON*\ax");
    WriteChatf("/autoaccept translocate on|off :: Toggle acceptance of translocate or zephyr port.  Default \ar*OFF*\ax");
    WriteChatf("/autoaccept anchor on|off :: Toggle acceptance of primary/secondary real estate anchor port.  Default \ar*OFF*\ax");
    WriteChatf("/autoaccept selfanchor on|off :: Toggle acceptance of primary/secondary real estate anchor port when you cast it.  Default \ar*OFF*\ax");
    WriteChatf("/autoaccept trade on|off :: Toggle acceptance of trades by people on the auto accept list. Default \ag*ON*\ax");
    WriteChatf("/autoaccept trade always on|off :: Toggles always accept all trades. Default \ar*OFF*\ax");
    WriteChatf("/autoaccept trade reject on|off :: Reject trades for people not on auto accept list after 5 seconds. Default \ar*OFF*\ax");
    WriteChatf("/autoaccept group on|off :: Toggles accept group invites. Default \ag*ON*\ax");
    WriteChatf("/autoaccept raid on|off :: Toggles accept raid invites. Default \ag*ON*\ax");
    WriteChatf("/autoaccept status :: Lists status of toggles.");
    WriteChatf("/autoaccept list :: Lists users on your auto accept list.");
    WriteChatf("/autoaccept save :: Saves settings to ini. Changes \arDO NOT\ax auto save.");
    WriteChatf("/autoaccept load :: Loads settings from ini");
    WriteChatf("/autoaccept add NAME :: Add NAME to the auto accept list. Proper case matters for matching later.");
    WriteChatf("/autoaccept del NAME :: Delete NAME from your auto accept list.");
    WriteChatf("/autoaccept addanchor VALUE :: Add VALUE as a valid anchor target. Proper case matters for matching later. Put the entire address inside quotes as it shows in the portal dialog box such as \ag\"Willow Circle Bay, 100 Vanward Heights\"\ax");
    WriteChatf("/autoaccept delanchor VALUE :: Delete VALUE from your valid anchor target list. Put the entire address inside quotes as it shows in the portal dialog box such as \ag\"Willow Circle Bay, 100 Vanward Heights\"\ax");
}

void AutoAcceptCommand(PSPAWNINFO pCHAR, PCHAR zLine) {
    int b = 0;
    GetArg(szTemp,zLine,1);

    if(!_strnicmp(szTemp,"load",4)) {
        LoadINI();
        return;
    }
    if(!_strnicmp(szTemp,"save",4)) {
        SaveINI();
        return;
    }

    if(!_strnicmp(szTemp,"status",6)) {
        WriteChatf("MQ2AutoAccept :: %s", bAutoAccept ? "\agENABLED\ax" : "\arDISABLED\ax");
        WriteChatf("MQ2AutoAccept :: Anchor portal accept is %s", bAnchor ? "\agON\ax" : "\arOFF\ax");
        WriteChatf("MQ2AutoAccept :: Self anchor portal accept is %s", bSelfAnchor ? "\agON\ax" : "\arOFF\ax");
        WriteChatf("MQ2AutoAccept :: Group accept is %s", bGroup ? "\agON\ax" : "\arOFF\ax");
        WriteChatf("MQ2AutoAccept :: Raid accept is %s", bRaid ? "\agON\ax" : "\arOFF\ax");
        WriteChatf("MQ2AutoAccept :: Trade accept is %s", bTrade ? "\agON\ax" : "\arOFF\ax");
        WriteChatf("MQ2AutoAccept :: Trade reject is %s", bTradeReject ? "\agON\ax" : "\arOFF\ax");
        WriteChatf("MQ2AutoAccept :: Trade always accept is %s", bTradeAlways ? "\agON\ax" : "\arOFF\ax");
        WriteChatf("MQ2AutoAccept :: Translocate accept is %s", bTranslocate ? "\agON\ax" : "\arOFF\ax");
        return;
    }

    if(!_strnicmp(szTemp,"on",2)) {
        bAutoAccept = true;
        WriteChatf("MQ2AutoAccept :: \agEnabled\ax");
    }
    else if(!_strnicmp(szTemp,"off",3)) {
        bAutoAccept = false;
        WriteChatf("MQ2AutoAccept :: \arDisabled\ax");
    }
    else if(!_strnicmp(szTemp,"list",4)) {
        ListUsers();
        ListAnchors();
    }
    else if(!_strnicmp(szTemp,"addanchor",9)) {
        GetArg(szTemp,zLine,2);
        if(!_strcmpi(szTemp, "")) {
            WriteChatf("Usage: /autoaccept addanchor \"VALUE\"");
            return;
        }
        for (unsigned int a = 0; a < vAnchors.size(); a++) {
            string& vRef = vAnchors[a];
            if (!_strcmpi(szTemp,vRef.c_str())) {
                WriteChatf("MQ2AutoAccept :: Anchor \ay%s\ax already exists", szTemp);
                return;
            }
        }
        vAnchors.push_back(szTemp);
        WriteChatf("MQ2AutoAccept :: Added \ay%s\ax to anchor list", szTemp);
    }
    else if(!_strnicmp(szTemp,"add",3)) {
        GetArg(szTemp,zLine,2);
        if(!_strcmpi(szTemp, "")) {
            WriteChatf("Usage: /autoaccept add NAME");
            return;
        }
        for (unsigned int a = 0; a < vIniNames.size(); a++) {
            string& vRef = vIniNames[a];
            if (!_strcmpi(szTemp,vRef.c_str())) {
                WriteChatf("MQ2AutoAccept :: User \ay%s\ax already exists", szTemp);
                return;
            }
        }
        vIniNames.push_back(szTemp);
        CombineNames();
        WriteChatf("MQ2AutoAccept :: Added \ay%s\ax to name list", szTemp);
    }
    else if(!_strnicmp(szTemp,"delanchor",9)) {
        int delIndex = -1;
        GetArg(szTemp,zLine,2);
        for (unsigned int a = 0; a < vAnchors.size(); a++) {
            string& vRef = vAnchors[a];
            if (!_strcmpi(szTemp,vRef.c_str())) {
                delIndex = a;
            }
        }
        if (delIndex >= 0) {
            vAnchors.erase(vAnchors.begin() + delIndex);
            WriteChatf("MQ2AutoAccept :: Deleted anchor \ay%s\ax", szTemp);
        } else {
            WriteChatf("MQ2AutoAccept :: Anchor \ay%s\ax not found", szTemp);
        }
    }
    else if(!_strnicmp(szTemp,"del",3)) {
        int delIndex = -1;
        GetArg(szTemp,zLine,2);
        for (unsigned int a = 0; a < vIniNames.size(); a++) {
            string& vRef = vIniNames[a];
            if (!_strcmpi(szTemp,vRef.c_str())) {
                delIndex = a;
            }
        }
        if (delIndex >= 0) {
            CombineNames();
            vIniNames.erase(vIniNames.begin() + delIndex);
            WriteChatf("MQ2AutoAccept :: Deleted user \ay%s\ax", szTemp);
        } else {
            WriteChatf("MQ2AutoAccept :: User \ay%s\ax not found", szTemp);
        }
    }
    else if(!_strnicmp(szTemp,"selfanchor",10)) {
        GetArg(szTemp,zLine,2);
        if(!_strnicmp(szTemp,"on",2)) {
            bSelfAnchor = true;
        } else if(!_strnicmp(szTemp,"off",2)) {
            bSelfAnchor = false;
        }
        WriteChatf("MQ2AutoAccept :: Self anchor portal accept is %s", bSelfAnchor ? "\agON\ax" : "\arOFF\ax");
    }
    else if(!_strnicmp(szTemp,"anchor",6)) {
        GetArg(szTemp,zLine,2);
        if(!_strnicmp(szTemp,"on",2)) {
            bAnchor = true;
        } else if(!_strnicmp(szTemp,"off",2)) {
            bAnchor = false;
        }
        WriteChatf("MQ2AutoAccept :: Anchor portal accept is %s", bAnchor ? "\agON\ax" : "\arOFF\ax");
    }
    else if(!_strnicmp(szTemp,"group",5)) {
        GetArg(szTemp,zLine,2);
        if(!_strnicmp(szTemp,"on",2)) {
            bGroup = true;
        } else if(!_strnicmp(szTemp,"off",2)) {
            bGroup = false;
        }
        WriteChatf("MQ2AutoAccept :: Group accept is %s", bGroup ? "\agON\ax" : "\arOFF\ax");
    }
    else if(!_strnicmp(szTemp,"raid",4)) {
        GetArg(szTemp,zLine,2);
        if(!_strnicmp(szTemp,"on",2)) {
            bRaid = true;
        } else if(!_strnicmp(szTemp,"off",2)) {
            bRaid = false;
        }
        WriteChatf("MQ2AutoAccept :: Raid accept is %s", bRaid ? "\agON\ax" : "\arOFF\ax");
    }
    else if(!_strnicmp(szTemp,"trade",5)) {
        GetArg(szTemp,zLine,2);
        if(!_strnicmp(szTemp,"on",2)) {
            bTrade = true;
        } else if(!_strnicmp(szTemp,"off",2)) {
            bTrade = false;
            bTradeAlways = false;
        }
        else if(!_strnicmp(szTemp,"reject",6)) {
            GetArg(szTemp,zLine,3);
            if(!_strcmpi(szTemp, "")) {
                WriteChatf("Usage: /autoaccept trade reject on|off");
                return;
            }
            if(!_strnicmp(szTemp,"on",2)) {
                bTrade = true;
                bTradeReject = true;
            } else if(!_strnicmp(szTemp,"off",2)) {
                bTradeReject = false;
            }
            WriteChatf("MQ2AutoAccept :: Trade reject is %s", bTradeReject ? "\agON\ax" : "\arOFF\ax");
            return;
        }
        else if(!_strnicmp(szTemp,"always",6)) {
            GetArg(szTemp,zLine,3);
            if(!_strcmpi(szTemp, "")) {
                WriteChatf("Usage: /autoaccept trade always on|off");
                return;
            }
            if(!_strnicmp(szTemp,"on",2)) {
                bTrade = true;
                bTradeAlways = true;
            } else if(!_strnicmp(szTemp,"off",2)) {
                bTradeAlways = false;
            }
            WriteChatf("MQ2AutoAccept :: Trade always accept is %s", bTradeAlways ? "\agON\ax" : "\arOFF\ax");
            return;
        }
        WriteChatf("MQ2AutoAccept :: Trade accept is %s", bTrade ? "\agON\ax" : "\arOFF\ax");
    }
    else if(!_strnicmp(szTemp,"translocate",11)) {
        GetArg(szTemp,zLine,2);
        if(!_strnicmp(szTemp,"on",2)) {
            bTranslocate = true;
        } else if(!_strnicmp(szTemp,"off",2)) {
            bTranslocate = false;
        }
        WriteChatf("MQ2AutoAccept :: Translocate accept is %s", bTranslocate ? "\agON\ax" : "\arOFF\ax");
    }
    else {
        ShowHelp();
    }
}


// Called once, when the plugin is to initialize
PLUGIN_API VOID InitializePlugin() {
    DebugSpewAlways("Initializing MQ2AutoAccept");
    AddCommand("/autoaccept",AutoAcceptCommand);
    ShowHelp();
}

// Called once, when the plugin is to shutdown
PLUGIN_API VOID ShutdownPlugin() {
    DebugSpewAlways("Shutting down MQ2AutoAccept");
    RemoveCommand("/autoaccept");
}

PLUGIN_API DWORD OnIncomingChat(PCHAR Line, DWORD Color)
{
    // No users, abort
    if (!vNames.size()) return 0;
    CHAR szName[MAX_STRING];
    if (strstr(Line,"invites you to join a group.") && bGroup) {
        GetArg(szName,Line,1);
        // loop through user list and find a match for inviter. If found join group
        for (unsigned int a = 0; a < vNames.size(); a++) {
            string& vRef = vNames[a];
            if (!_strcmpi(szName,vRef.c_str())) {
                DoCommand(GetCharInfo()->pSpawn,"/timed 3s /invite");
                WriteChatf("\agMQ2AutoAccept :: Joining group with %s\ax",szName);
            }
        }
    }
    else if (strstr(Line,"invites you to join a raid") && bRaid) {
        GetArg(szName,Line,1);
        // loop through user list and find a match for inviter. If found join raid
        for (unsigned int a = 0; a < vNames.size(); a++) {
            string& vRef = vNames[a];
            if (!_strcmpi(szName,vRef.c_str())) {
                DoCommand(GetCharInfo()->pSpawn,"/timed 3s /raidaccept");
                WriteChatf("\agMQ2AutoAccept :: Joining raid with %s\ax",szName);
            }
        }
    }
    return 0;
}

void WinClick(CXWnd *Wnd, PCHAR ScreenID, PCHAR ClickNotification, DWORD KeyState) {
    if (Wnd) {
        if (CXWnd *Child = Wnd->GetChildItem(ScreenID)) {
            BOOL KeyboardFlags[4];
            *(DWORD*)&KeyboardFlags = *(DWORD*)&((PCXWNDMGR)pWndMgr)->KeyboardFlags;
            *(DWORD*)&((PCXWNDMGR)pWndMgr)->KeyboardFlags = KeyState;
            SendWndClick2(Child, ClickNotification);
            *(DWORD*)&((PCXWNDMGR)pWndMgr)->KeyboardFlags = *(DWORD*)&KeyboardFlags;
        }
    }
}

// This is called every time MQ pulses
PLUGIN_API VOID OnPulse(VOID) {
    if (GetGameState() != GAMESTATE_INGAME)
        return;
    if (!bInitDone)
        return;
    if (!bAutoAccept)
        return;

    CXWnd *Child;
    CXWnd *pWnd;
    bool clickTrade = false;
    bool givingItem = false;

    if (SkipPulse == SKIP_PULSES) {
        SkipPulse = 0;
        // if we've clicked trade no need to check anything, let other person accept or reject
        if (pTradeWnd && pTradeWnd->dShow && ((PEQTRADEWINDOW)pTradeWnd)->MyTradeReady==0) {
            if (((PEQTRADEWINDOW)pTradeWnd)->HisTradeReady) {
                if (bTradeAlways) {
                    clickTrade = true;
                } else {
                    if (CXWnd* pHisNameWnd = pTradeWnd->GetChildItem("TRDW_HisName")) {
                        GetCXStr(pHisNameWnd->WindowText, szTemp, MAX_STRING - 1);
                        if (szTemp[0] != '\0') {
                            for (unsigned int a = 0; a < vNames.size(); a++) {
                                string& vRef = vNames[a];
                                if (!_strcmpi(szTemp, vRef.c_str())) {
                                    clickTrade = true;
                                    break;
                                }
                            }
                        }
                    }
                }
                givingItem = false;
                for (unsigned int n = 0; n < 8; n++) {
                    sprintf_s(szTemp, "TRDW_TradeSlot%d", n);
                    if (CXWnd* pTRDW_TradeSlotWnd = pTradeWnd->GetChildItem(szTemp)) {
                        GetCXStr(pTRDW_TradeSlotWnd->Tooltip, szTemp, MAX_STRING - 1);
                        if (szTemp[0] != '\0') {
                            if (strlen(szTemp) > 0) {
                                //DebugSpew("Giving %s in slot %d",szTemp, n);
                                givingItem = true;
                                break;
                            }
                        }
                    }
                }
                bool givingMoney = false;
                for (unsigned int n = 0; n < 4; n++) {
                    sprintf_s(szTemp, "TRDW_MyMoney%d", n);
                    if (CXWnd* pTRDW_MyMoneyWnd = pTradeWnd->GetChildItem(szTemp)) {
                        GetCXStr(pTRDW_MyMoneyWnd->WindowText, szTemp, MAX_STRING - 1);
                        if (szTemp[0] != '\0' && _stricmp(szTemp,"0")) {
                            if (strlen(szTemp) > 0) {
                                //DebugSpew("Giving %s in slot %d",szTemp, n);
                                givingMoney = true;
                                break;
                            }
                        }
                    }
                }
                if (  givingItem || givingMoney) {
                    // We're giving item or coin, don't auto do anything
                    //DebugSpew("We're giving item or coin, don't do anything");
                } else {
                    if (clickTrade) {
                        if (CXWnd* pHisNameWnd = pTradeWnd->GetChildItem("TRDW_HisName")) {
                            GetCXStr(pHisNameWnd->WindowText, szTemp, MAX_STRING - 1);
                            if (szTemp[0] != '\0') {
                                if (CXWnd* pTRDW_Trade_Button = pTradeWnd->GetChildItem("TRDW_Trade_Button")) {
                                    WriteChatf("\agMQ2AutoAccept :: Accepting trade from %s\ax", szTemp);
                                    SendWndClick2(pTRDW_Trade_Button,"leftmouseup");
                                    pTarget = NULL;
                                }
                            }
                        }
                    } else {
                        if (bTradeReject) {
                            if (rejectTimer == 0)
                                rejectTimer = GetTickCount642() + 5000;
                            if (rejectTimer < GetTickCount642()) {
                                rejectTimer = 0;
                                if (CXWnd* pTRDW_Cancel_Button = pTradeWnd->GetChildItem("TRDW_Cancel_Button")) {
                                    WriteChatf("\arMQ2AutoAccept :: Canceling Trade\ax");
                                    SendWndClick2(pTRDW_Cancel_Button,"leftmouseup");
                                }
                            }
                        }
                    }
                }
            }
        }
        pWnd=(CXWnd *)FindMQ2Window("ConfirmationDialogBox");
        if(pWnd && (BOOL)pWnd->dShow) {
            if(Child=pWnd->GetChildItem("CD_TextOutput")) {
                szTemp[0] = '\0';
                GetCXStr(((PCSIDLWND)Child)->SidlText,szTemp,sizeof(szTemp));
                if (bTranslocate && (strstr(szTemp,"translocated to your bind point") || strstr(szTemp,"wish to be translocated by") )) {
                    // Translocate request
                    WriteChatf("\agMQ2AutoAccept :: Accepting translocate\ax");
                    WinClick(FindMQ2Window("ConfirmationDialogBox"),"Yes_Button","leftmouseup",1);
                    return;
                } else if (bAnchor && strstr(szTemp,"to the real estate anchor in")) {
                    // Anchor portal request
                    for ( unsigned int a = 0; a < vAnchors.size (); a++ ) {
                        string& vRef = vAnchors[a];
                        if ( strstr (szTemp, vRef.c_str()) ) {
                            WriteChatf("\agMQ2AutoAccept :: Accepting anchor portal to \ax\at%s\ax", vRef.c_str());
                            WinClick(FindMQ2Window("ConfirmationDialogBox"),"Yes_Button","leftmouseup",1);
                            return;
                        }
                    }
                } else if ( bSelfAnchor && strstr (szTemp, "transport yourself to the real estate") ) {
                    // we cast the portal, accept it
                    WriteChatf("\agMQ2AutoAccept :: Accepting self anchor portal cast\ax");
                    WinClick(FindMQ2Window("ConfirmationDialogBox"), "Yes_Button", "leftmouseup", 1);
                    return;
                }
                //none of the above? do we have any names
                if (!vNames.size())
                    return;//nope then return

                // All other confirmation boxes
                for (unsigned int a = 0; a < vNames.size(); a++) {
                    string& vRef = vNames[a];
                    sprintf_s(szName,"%s ", vRef.c_str());
                    sprintf_s(szName2,"%s's", vRef.c_str());
                    if(strstr(szTemp,szName) || strstr(szTemp,szName2)) {
                        if (pWnd->GetChildItem("Yes_Button")) {
                            WriteChatf("\agMQ2AutoAccept :: Clicking Yes\ax");
                            WinClick(FindMQ2Window("ConfirmationDialogBox"),"Yes_Button","leftmouseup",1);
                        }
                        else if (pWnd->GetChildItem("OK_Button")) {
                            WriteChatf("\agMQ2AutoAccept :: Clicking OK\ax");
                            WinClick(FindMQ2Window("ConfirmationDialogBox"),"OK_Button","leftmouseup",1);
                        }
                        return;
                    }
                }
            }
        }
    }

   SkipPulse++;
}
