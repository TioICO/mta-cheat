#include "Offsets.h"
#include <Windows.h>

int g_pClientGame = 0x35CF44;

int dwClientType = 0x0;
int dwVehicleType = 0x0;
int dwColType = 0x0;

int o_MtaPlayerPos = 0x148;
int o_MtaColPos = 0x13C;
int o_MtaObjectId = 0x194;

int o_Nickname = 0x5B8; // 0x560 0x5B0, 0x5B8 0x67C

int o_Ping = 0x5D0;

int o_Latency = 0x608;

int o_MTACarId = 0x198;

bool ManipulateShots = false;
int ManipulatedWeapon = 255, ManipulatedBodyPiece = 255;

bool NoRecoil = false;

bool RunEngine = false;
int RunEngineKey = 'E';

bool vehicleFilter = false;
char vehicleIDs[1024] = "487";

bool EveryColliderWH = false, EveryObjectWH = true;

float EveryWHDistance = 1000.0f;

int EveryObjectWHKey = 'O';

bool PlayerAlert = false;
bool PlayerAlertEnabled = true;

bool MapObjects = false;


float FOV = 150;
float Smooth = 100;


float m_LatencyAdjusting = 0.02f;
float m_VehicleLatencyAdjusting = 0.003f;


bool AimEnabled = 1, WHEnabled = 1;

bool radarEnabled = true;
int radarKey = VK_F11;

int CurrentAimMode = fovAim;

int CurrentAimBotBone = 0;
int aimBotBone = 8;

int BoneESPArray[12] = { 6,4,3, 2, 51, 52, 53, 54, 41, 42, 43 ,44 };

bool WHPeds = 1, WHCars = 1, WHPedColors = 1, WHObject = 1, WHZombies = 0, WHObjectIDs = 0;

bool removeObject = 0;

bool showZombies = 0, showCorpses = 0;

bool DontAimAtZombies = 0;

char zombieIDs[1024] = "59,61,72";

bool WHObjectList[10] = { 1,1,1,1,1,1,1,1,1,1 };

char objectIDs[1024] = "0,1,2,3";


int aimKey = 0x2, friendAimKey = 0x4, everyWHKey = VK_CAPITAL;


std::vector<int> friendList = { 0 };

float ColorsForPeds[3][3] = { 0,0,1, 1,0,0, 1,1,1 };

std::string thingx = "x";

bool bigHake(std::string x)
{
	return (strstr(x.c_str(), "Wrong"));
}

std::map<int, const char*> vehicleList = std::map<int, const char*>();


void InitializeVehicleList()
{
	vehicleList[577] = "AT-400";
	vehicleList[511] = "Beagle";
	vehicleList[512] = "Cropduster";
	vehicleList[593] = "Dodo";
	vehicleList[520] = "Hydra";
	vehicleList[553] = "Nevada";
	vehicleList[476] = "Rustler";
	vehicleList[519] = "Shamal";
	vehicleList[460] = "Skimmer";
	vehicleList[513] = "Stuntplane";
	vehicleList[548] = "Cargobob";
	vehicleList[417] = "Leviathan";
	vehicleList[487] = "Maverick";
	vehicleList[488] = "News Chopper";
	vehicleList[497] = "Police Maverick";
	vehicleList[563] = "Raindance";
	vehicleList[447] = "Seasparrow";
	vehicleList[469] = "Sparrow";
	vehicleList[472] = "Coastguard";
	vehicleList[473] = "Dinghy";
	vehicleList[493] = "Jetmax";
	vehicleList[595] = "Launch";
	vehicleList[484] = "Marquis";
	vehicleList[430] = "Predator";
	vehicleList[453] = "Reefer";
	vehicleList[452] = "Speeder";
	vehicleList[446] = "Squalo";
	vehicleList[454] = "Tropic";
	vehicleList[485] = "Baggage";
	vehicleList[431] = "Bus";
	vehicleList[438] = "Cabbie";
	vehicleList[437] = "Coach";
	vehicleList[574] = "Sweeper";
	vehicleList[420] = "Taxi";
	vehicleList[525] = "Towtruck";
	vehicleList[408] = "Trashmaster";
	vehicleList[552] = "Utility Van";
	vehicleList[416] = "Ambulance";
	vehicleList[433] = "Barracks";
	vehicleList[427] = "Enforcer";
	vehicleList[490] = "FBI Rancher";
	vehicleList[528] = "FBI Truck";
	vehicleList[407] = "Fire Truck";
	vehicleList[544] = "Fire Truck";
	vehicleList[523] = "HPV1000";
	vehicleList[470] = "Patriot";
	vehicleList[596] = "Police LS";
	vehicleList[598] = "Police LV";
	vehicleList[599] = "Police Ranger";
	vehicleList[597] = "Police SF";
	vehicleList[432] = "Rhino";
	vehicleList[601] = "S.W.A.T.";
	vehicleList[428] = "Securicar";
	vehicleList[499] = "Benson";
	vehicleList[609] = "Black Boxville";
	vehicleList[498] = "Boxville";
	vehicleList[524] = "Cement Truck";
	vehicleList[532] = "Combine Harvester";
	vehicleList[578] = "DFT-30";
	vehicleList[486] = "Dozer";
	vehicleList[406] = "Dumper";
	vehicleList[573] = "Dune";
	vehicleList[455] = "Flatbed";
	vehicleList[588] = "Hotdog";
	vehicleList[403] = "Linerunner";
	vehicleList[423] = "Mr.Whoopee";
	vehicleList[443] = "Packer";
	vehicleList[515] = "Roadtrain";
	vehicleList[510] = "Mountain Bike";
	vehicleList[514] = "Tanker";
	vehicleList[531] = "Tractor";
	vehicleList[456] = "Yankee";
	vehicleList[536] = "Blade";
	vehicleList[575] = "Broadway";
	vehicleList[534] = "Remington";
	vehicleList[567] = "Savanna";
	vehicleList[535] = "Slamvan";
	vehicleList[576] = "Tornado";
	vehicleList[412] = "Voodoo";
	vehicleList[402] = "Buffalo";
	vehicleList[542] = "Clover";
	vehicleList[603] = "Phoenix";
	vehicleList[475] = "Sabre";
	vehicleList[581] = "BF-400";
	vehicleList[400] = "Landstalker";
	vehicleList[509] = "Bike";
	vehicleList[481] = "BMX";
	vehicleList[462] = "Faggio";
	vehicleList[521] = "FCR-900";
	vehicleList[463] = "Freeway";
	vehicleList[522] = "NRG-500";
	vehicleList[461] = "PCJ-600";
	vehicleList[448] = "Pizza Boy";
	vehicleList[468] = "Sanchez";
	vehicleList[586] = "Wayfarer";
	vehicleList[602] = "Alpha";
	vehicleList[496] = "Blista Compact";
	vehicleList[401] = "Bravura";
	vehicleList[527] = "Cadrona";
	vehicleList[589] = "Club";
	vehicleList[419] = "Esperanto";
	vehicleList[587] = "Euros";
	vehicleList[533] = "Feltzer";
	vehicleList[526] = "Fortune";
	vehicleList[474] = "Hermes";
	vehicleList[545] = "Hustler";
	vehicleList[517] = "Majestic";
	vehicleList[410] = "Manana";
	vehicleList[600] = "Picador";
	vehicleList[436] = "Previon";
	vehicleList[439] = "Stallion";
	vehicleList[549] = "Tampa";
	vehicleList[491] = "Virgo";
	vehicleList[445] = "Admiral";
	vehicleList[604] = "Damaged Glendale";
	vehicleList[507] = "Elegant";
	vehicleList[585] = "Emperor";
	vehicleList[466] = "Glendale";
	vehicleList[492] = "Greenwood";
	vehicleList[546] = "Intruder";
	vehicleList[516] = "Nebula";
	vehicleList[467] = "Oceanic";
	vehicleList[426] = "Premier";
	vehicleList[547] = "Primo";
	vehicleList[405] = "Sentinel";
	vehicleList[580] = "Stafford";
	vehicleList[409] = "Stretch";
	vehicleList[550] = "Sunrise";
	vehicleList[566] = "Tahoma";
	vehicleList[540] = "Vincent";
	vehicleList[421] = "Washington";
	vehicleList[529] = "Willard";
	vehicleList[459] = "Berkley's RC Van";
	vehicleList[422] = "Bobcat";
	vehicleList[482] = "Burrito";
	vehicleList[605] = "Damaged Sadler";
	vehicleList[530] = "Forklift";
	vehicleList[418] = "Moonbeam";
	vehicleList[572] = "Mower";
	vehicleList[582] = "News Van";
	vehicleList[413] = "Pony";
	vehicleList[440] = "Rumpo";
	vehicleList[543] = "Sadler";
	vehicleList[583] = "Tug";
	vehicleList[478] = "Walton";
	vehicleList[554] = "Yosemite";
	vehicleList[429] = "Banshee";
	vehicleList[541] = "Bullet";
	vehicleList[415] = "Cheetah";
	vehicleList[480] = "Comet";
	vehicleList[562] = "Elegy";
	vehicleList[565] = "Flash";
	vehicleList[434] = "Hotknife";
	vehicleList[494] = "Hotring Racer";
	vehicleList[502] = "Hotring Racer 2";
	vehicleList[503] = "Hotring Racer 3";
	vehicleList[411] = "Infernus";
	vehicleList[559] = "Jester";
	vehicleList[561] = "Stratum";
	vehicleList[560] = "Sultan";
	vehicleList[506] = "Super GT";
	vehicleList[451] = "Turismo";
	vehicleList[558] = "Uranus";
	vehicleList[555] = "Windsor";
	vehicleList[477] = "ZR-350";
	vehicleList[441] = "RC Bandit";
	vehicleList[464] = "RC Baron";
	vehicleList[594] = "RC Cam";
	vehicleList[501] = "RC Goblin";
	vehicleList[465] = "RC Raider";
	vehicleList[564] = "RC Tiger";
	vehicleList[606] = "Baggage Trailer";
	vehicleList[607] = "Baggage Trailer";
	vehicleList[610] = "Farm Trailer";
	vehicleList[584] = "Petrol Trailer";
	vehicleList[611] = "Trailer";
	vehicleList[608] = "Trailer";
	vehicleList[435] = "Trailer 1";
	vehicleList[450] = "Trailer 2";
	vehicleList[591] = "Trailer 3";
	vehicleList[590] = "Box Freight";
	vehicleList[538] = "Brown Streak";
	vehicleList[570] = "Brown Streak Carriage";
	vehicleList[569] = "Flat Freight";
	vehicleList[537] = "Freight";
	vehicleList[449] = "Tram";
	vehicleList[568] = "Bandito";
	vehicleList[424] = "BF Injection";
	vehicleList[504] = "Bloodring Banger";
	vehicleList[457] = "Caddy";
	vehicleList[483] = "Camper";
	vehicleList[508] = "Journey";
	vehicleList[571] = "Kart";
	vehicleList[500] = "Mesa";
	vehicleList[444] = "Monster";
	vehicleList[556] = "Monster 2";
	vehicleList[557] = "Monster 3";
	vehicleList[471] = "Quadbike";
	vehicleList[495] = "Sandking";
	vehicleList[539] = "Vortex";
	vehicleList[579] = "Huntley";
	vehicleList[404] = "Perennial";
	vehicleList[489] = "Rancher";
	vehicleList[505] = "Rancher";
	vehicleList[479] = "Regina";
	vehicleList[442] = "Romero";
	vehicleList[458] = "Solair";

}