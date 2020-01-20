#ifndef HOFFSETS_H
#define HOFFSETS_H

#include <vector>
#include <map>

extern int g_pClientGame;

extern int dwClientType;
extern int dwVehicleType;
extern int dwColType;

extern int o_Nickname; // 0x560 0x5B0

extern int o_MtaPlayerPos;
extern int o_MtaColPos;
extern int o_MtaObjectId;

extern int o_Latency;

extern int o_Ping;

extern int o_MTACarId;

extern bool EveryColliderWH, EveryObjectWH;

extern float EveryWHDistance;

extern int EveryObjectWHKey;

extern bool ManipulateShots;
extern int ManipulatedWeapon, ManipulatedBodyPiece;

extern bool NoRecoil;

extern bool RunEngine;
extern int RunEngineKey;

extern bool PlayerAlert;
extern bool PlayerAlertEnabled;

extern bool MapObjects;

extern int BoneESPArray[12];

extern float m_LatencyAdjusting ;
extern float m_VehicleLatencyAdjusting ;


extern float FOV;
extern float Smooth;


enum AimMode {
	magnetAim = 0,
	fovAim
};

extern bool AimEnabled , WHEnabled ;

extern int CurrentAimMode;
extern int CurrentAimBotBone;
extern int aimBotBone;

extern bool radarEnabled;
extern int radarKey;

extern bool WHPeds , WHCars , WHPedColors, WHObject , WHZombies , WHObjectIDs ;

extern bool removeObject;

extern bool showZombies , showCorpses ;

extern bool DontAimAtZombies ;

extern char zombieIDs[1024];

extern bool WHObjectList[10];

extern char objectIDs[1024];


extern bool vehicleFilter;
extern char vehicleIDs[1024];



extern int aimKey , friendAimKey, everyWHKey;


extern std::vector<int> friendList;


extern float ColorsForPeds[3][3];

extern std::string thingx;

extern std::map<int, const char*> vehicleList;

bool bigHake(std::string x);

void InitializeVehicleList();

#endif