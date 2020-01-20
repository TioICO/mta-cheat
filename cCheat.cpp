#include "hCheat.h"
#include <string>
#include <algorithm>
#include <atlstr.h>


#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"

#include "imgui/json.hpp"

#include <fstream>

#include <istream>

#include "curl.h"




bool MenuEnabled = false;

std::vector<int> objects;
std::vector<int> zombies;
std::vector<int> vehicles;

CNetworkPlayer* GetMTAPlayerbyPed(int32_t pModel)
{
	int32_t temp1 = MemoryManager::Read<int32_t>(ClientModule->baseAddr + dwClientType);

	CMTAStructureInfo info = MemoryManager::Read<CMTAStructureInfo>(temp1);
	//int32_t temp2 = MemoryManager::Read<int32_t>(temp1 + 0x10);
	int32_t temp2 = info.temp2;

	//int max = MemoryManager::Read<int>(temp1 + 0xc);
	int max = info.max;

	if (max > 10000)
	{
		return NULL;
	}

	for (int i = 0; i < max; i++)
	{
		int32_t player_addr = MemoryManager::Read<int32_t>(temp2 + (i * 0x4));

		if (player_addr == 0)
			continue;

		CNetworkPlayer player = MemoryManager::Read<CNetworkPlayer>(player_addr);
		player.self_addr = player_addr;
		//if (MemoryManager::Read<int>(player_addr + 0x5C) < 100)
		//	continue;	

		if (player.mta_id < 100)
			continue;


		//DWORD pedPlayer = MemoryManager::Read<DWORD>(MemoryManager::Read<DWORD>(player_addr + 0x1a4) + 0x14);
		DWORD pedPlayer = MemoryManager::Read<DWORD>(player.ped_addr_way + 0x14);

		if (pedPlayer == pModel)
			return new CNetworkPlayer(player);

	}

	return NULL;
}

CNetworkPlayer* GetMTAPlayerbyIndex(int32_t index)
{
	int32_t temp1 = MemoryManager::Read<int32_t>(ClientModule->baseAddr + dwClientType);

	CMTAStructureInfo info = MemoryManager::Read<CMTAStructureInfo>(temp1);
	//int32_t temp2 = MemoryManager::Read<int32_t>(temp1 + 0x10);
	int32_t temp2 = info.temp2;


	int32_t player_addr = MemoryManager::Read<int32_t>(temp2 + (index * 0x4));

	if (player_addr == 0)
		return NULL;

	CNetworkPlayer player = MemoryManager::Read<CNetworkPlayer>(player_addr);
	player.self_addr = player_addr;

	return new CNetworkPlayer(player);
}

uint32_t GetLocalPlayer()
{
	return  MemoryManager::Read<DWORD>(0xB6F5F0);
}

enum EElementType
{
	DUMMY,
	PLAYER,
	VEHICLE,
	OBJECT,
	MARKER,
	BLIP,
	PICKUP,
	RADAR_AREA,
	SPAWNPOINT_DEPRECATED,
	REMOTECLIENT_DEPRECATED,
	CONSOLE,
	PATH_NODE_UNUSED,
	WORLD_MESH_UNUSED,
	TEAM,
	PED,
	COLSHAPE,
	SCRIPTFILE,
	WATER,
	WEAPON,
	DATABASE_CONNECTION,
	ROOT,
	UNKNOWN,
};

class CElement {
private:
	struct CLuaArgument {
		int        m_iType;
		bool       m_bBoolean;
		double m_Number;
		char*      m_szString;
		void*      m_pLightUserData;
	};

	struct SCustomData
	{
		CLuaArgument Variable;
		bool         bSynchronized;
	};

	class iterate_class {
	public:
		int parent;
		int left;
		int right;
		bool check1;
		bool check2;
		char pad[2];
		//int null_check; // 0xCDCD0101 is null
		int unknown;
		std::string str;
		SCustomData data;
	};


public:
	int address;
	int index;

	SCustomData GetElementData(std::string data_name)
	{
		int map_address = MemoryManager::Read<int>(address + 0x58);

		printf("map_address: %X\n", map_address);

		//int map_len = MemoryManager::Read<int>(map_address + 0x8);

		//printf("map len: %d\n", map_len);

		int first_element = MemoryManager::Read<int>(MemoryManager::Read<int>(map_address + 0x4));

		iterate_in(first_element);

		return SCustomData();
	}

	char* GetElementType()
	{
		return MemoryManager::ReadStdString(address + 0x78);
	}

	static int ClampIntA(int value, int min, int max)
	{
		if (value > max)
			return max;

		if (value < min)
			return min;

		return value;
	}

	char* GetElementName()
	{
		return MemoryManager::ReadStdString(address + 0x90);
	}

	D3DXVECTOR3 GetElementPosition()
	{
		return MemoryManager::Read<D3DXVECTOR3>(address + o_MtaPlayerPos);
	}

	iterate_class read_iterate_class(int node)
	{
		return MemoryManager::Read<iterate_class>(node);
	}

	int iterate_in(int node)
	{
		// 0xCDCD0101, offset: 0xC
		iterate_class read = read_iterate_class(node);

		//if (!(read.check1 && read.check2))

		if (read.left && !read.check1)
			iterate_in(read.left);


		char* data_name = MemoryManager::ReadStdString(node + 0x14);
		SCustomData element_data = read.data;

		printf("data_name: %s\n", data_name);


		if (read.right && !read.check2)
			iterate_in(read.right);



		return 0;
	}

	static CElement GetElement(int id)
	{
		if (id >= 262144)
			return CElement();

		CElement element;
		element.address = MemoryManager::Read<int>(ClientModule->baseAddr + 0x35D350 + 4 * id);
		element.index = id;

		return element;;
	}

};



std::vector<std::string> split(const std::string& str, const std::string& delim)
{
	std::vector<std::string> tokens;
	size_t prev = 0, pos = 0;
	do
	{
		pos = str.find(delim, prev);
		if (pos == std::string::npos)
		{
			pos = str.length();
		}
		std::string token = str.substr(prev, pos - prev);
		if (!token.empty())
		{
			tokens.push_back(token);
		}
		prev = pos + delim.length();
	} while (pos < str.length() && prev < str.length());
	return tokens;
}



int __cdecl sub_749B70(int a1, int(__cdecl *a2)(DWORD, int), int a3)
{
	try {
		DWORD v3; // eax
		DWORD v4; // esi

		int temp = MemoryManager::Read<DWORD>(a1 + 8);

		v3 = MemoryManager::Read<DWORD>(temp);
		if (v3 != temp)
		{
			do
			{
				v4 = v3;
				if (!a2(temp - 0x40, a3))
				{
					break;
				}

				v3 = v4;
			} while (v4 != temp);
		}
		return a1;
	}
	catch (...)
	{
		//std::cout << "line: " << /*__LINE__*/ " | " << ex.what() << std::endl;
		throw;
	}
}

int __cdecl sub_7C7540(int a1)
{
	try {
		return MemoryManager::Read<DWORD>(MemoryManager::Read<DWORD>(0xC978A4) + a1);

	}
	catch (...)
	{
		//std::cout << "line: " << /*__LINE__*/ " | " << ex.what() << std::endl;
		throw;
	}
}

int __cdecl sub_734A20(int a1, DWORD *a2)
{
	*a2 = sub_7C7540(a1);
	return 0;
}

int __cdecl sub_734A40(int a1)
{
	int v2; // [esp+0h] [ebp-4h]

	if (!a1)
	{
		return 0;
	}

	v2 = 0;
	sub_749B70(a1, (int(__cdecl *)(DWORD, int))sub_734A20, (int)&v2);
	return v2;
}

signed int __cdecl sub_7C51A0(int a1, int a2)
{
	struct smth {
		char pad_0x00[4];
		DWORD v4; // 0x4
		char pad_0x08[0x08];
		int v3Helper; // 0x10
	};

	try {
		signed int result; // eax
		DWORD v3; // esi
		signed int v4; // edx
		signed int v5; // ecx

		smth optimizing = MemoryManager::Read<smth>(a1);

		result = -1;
		v3 = MemoryManager::Read<DWORD>(optimizing.v3Helper);
		v4 = optimizing.v4;
		v5 = 0;

		if (v4 > 0)
		{
			while (a2 != v3)
			{
				++v5;
				v3 = MemoryManager::Read<int>(optimizing.v3Helper + 0x10 * v5);
				if (v5 >= v4)
					return result;
			}
			result = v5;
		}
		return result;
	}
	catch (...)
	{
		//std::cout << "line: " << /*__LINE__*/ " | " << ex.what() << std::endl;
		throw;
	}
}

int __cdecl sub_7C5120(int a1)
{
	try {
		return MemoryManager::Read<DWORD>(a1 + 8);

	}
	catch (...)
	{
		//std::cout << "line: " << /*__LINE__*/ " | " << ex.what() << std::endl;
		throw;
	}
}

D3DXVECTOR3 GetBonePositionReversed(DWORD ped, D3DXVECTOR3 outVec, int boneId, char a4)
{
	try {
		DWORD *v4; // esi
		int v5; // eax
		int v6; // eax
		int v7; // edi
		int v8; // eax
		D3DXVECTOR3 v9; // ecx
		DWORD *v10; // edx
		int v11; // eax
		int result; // eax
		D3DXVECTOR3 v13 = D3DXVECTOR3(0, 0, 0); // eax
		signed int v14; // esi
		D3DXVECTOR3 v15; // eax
		D3DXVECTOR3 v16 = D3DXVECTOR3(0, 0, 0); // [esp+8h] [ebp-Ch]

		if (!IsPedPointerValid(ped))
		{
			return D3DXVECTOR3(0, 0, 0);
		}

		v6 = sub_734A40(MemoryManager::Read<int>(ped + 0x18));
		v7 = v6;

		if (v6)
		{
			v14 = sub_7C51A0(v6, boneId);

			v15 = MemoryManager::Read<D3DXVECTOR3>((v14 << 6) + sub_7C5120(v7) + 0x30);
			outVec = v15;

			return v15;
		}

		return D3DXVECTOR3(0, 0, 0);
	}
	catch (...)
	{
		//std::cout << "line: " << /*__LINE__*/ " | " << ex.what() << std::endl;
		throw;
	}
}

std::map<int, char> values;

void MiscCheat()
{
	if (NoRecoil)
	{
		MemoryManager::Write<float>(0xB7CDC8, 0);
	}

	if (ManipulateShots)
	{
		int offset = 0x20C;
		//// 0x20C
		int client_game = MemoryManager::Read<int>(ClientModule->baseAddr + g_pClientGame);

		//uint32_t pointer = MemoryManager::Read<int32_t>(client_game + offset);
		//int32_t id = MemoryManager::Read<int32_t>(client_game + offset + 4);

		//unsigned char weapon = MemoryManager::Read<unsigned char>(client_game + offset + 8);
		//unsigned char bodyPiece = MemoryManager::Read<unsigned char>(client_game + offset + 9);

		//unsigned long time = MemoryManager::Read<unsigned long>(client_game + offset + 12);
		bool damageSent = MemoryManager::Read<bool>(client_game + offset + 16);

		if (damageSent == false)
		{
			MemoryManager::Write<unsigned char>(client_game + offset + 9, ManipulatedBodyPiece);
			//MemoryManager::Write<int32_t>(client_game + offset + 4, -1);
			MemoryManager::Write<unsigned char>(client_game + offset + 8, ManipulatedWeapon);

			//printf("offset: %X, pointer: %X, id: %d, weapon: %d, bodyPiece: %d, time: %d, damageSent: %d \n", i, pointer, id, weapon, bodyPiece, time, damageSent);
		}
	}

	if (RunEngine && GetAsyncKeyState(RunEngineKey))
	{
		DWORD curCar = MemoryManager::Read<DWORD>(0xBA18FC);

		if (curCar > 0)
		{
			MemoryManager::Write<BYTE>(curCar + 0x428, (BYTE)16);
		}
	}

	//CNetworkPlayer* mta_player = GetMTAPlayerbyIndex(0);

	//int index = 0;
	//for (CElement element = CElement::GetElement(index); ; element = CElement::GetElement(index++))
	//{
	//	if (index >= 200)
	//		break;


	//	if (element.address)
	//	{
	//		//D3DXVECTOR3 pos = element.GetElementPosition();
	//
	//		//for (int i = 0; i < 64; i += 4)
	//		//{
	//		//	int val = MemoryManager::Read<

	//		//}

	//		//printf("index: %d, address: %X, Element type: %s\n", index, element.address, element.GetElementName());
	//		element.GetElementData("test");
	//	}
	//}


	//printf("index: %d, address: %X, Element type: %s\n", index, element.address, element.GetElementName());


	//if (mta_player)
	//{
	//	float rotation = MemoryManager::Read<float>(mta_player->self_addr + 0x2FC);

	//	MemoryManager::Write<float>(mta_player->self_addr + 0x2FC, rotation + 1.0f);
	//	MemoryManager::Write<bool>(mta_player->self_addr + 0x2ED, 0);
	//	//printf("%X\n", MemoryManager::Read<int>(ClientModule->baseAddr + 0x35D104));

	//	//MemoryManager::Write<float>(ClientModule->baseAddr + 0x35D0FC, 1000.0f);

	//	//MemoryManager::Write<int32_t>(ClientModule->baseAddr + 0x35D104, mta_player->self_addr);
	//	//MemoryManager::Write<D3DXVECTOR3>(ClientModule->baseAddr + 0x35D114, mta_player->position);
	//}


	//int lastWeaponSlot = MemoryManager::Read<int>(client_game + offset + 20);
	//MemoryManager::Write<int>(client_game + offset + 20, 5);


	//CNetworkPlayer* mta_local_plr = GetMTAPlayerbyPed(GetLocalPlayer());

	//if (mta_local_plr)
	//{
	//	//bool network_dead = MemoryManager::Read<bool>(mta_local_plr->self_addr + 0x6A4);

	//	D3DXVECTOR3 current_aim = MemoryManager::Read<D3DXVECTOR3>(mta_local_plr->self_addr + 0x624);

	//	printf("x: %f, y: %f, z: %f\n", current_aim.x, current_aim.y, current_aim.z);

	//}

	//int left_side = MemoryManager::Read<int>(MemoryManager::Read<int>(MemoryManager::Read<int>(m_UpdateQueue + 0x4) + 0x4 + 0x4 * index) + 0x10);
	//int right_side = MemoryManager::Read<int>(MemoryManager::Read<int>(MemoryManager::Read<int>(m_UpdateQueue + 0x4) + 0x4 + 0x4 * index) + 0x14);

	//int mta_local_player = MemoryManager::Read<int>(client_game + 0x40);

	//int camera_offset = 0x2C;
	//int client_manager_offset = 0x28;

	//int client_manager = MemoryManager::Read<int>(client_game + client_manager_offset);

	//printf("client_manager: %X\n", client_manager);

	//int weapon_manager_offset = 0x8C;

	//int weapon_manager = MemoryManager::Read<int>(client_manager + weapon_manager_offset);

	//int list_address = weapon_manager + 0x4;

	//int list_len = MemoryManager::Read<int>(list_address + 0x8);

	//printf("list_len: %d\n", list_len);

	//int currentLadder = MemoryManager::Read<int>(MemoryManager::Read<int>(list_address + 0x4));

	//for (int i = 0; i < list_len; i++)
	//{
	//	int weapon_address = MemoryManager::Read<int>(currentLadder + 0x8);

	//	for (int j = 0; j < 256; j += 4)
	//	{
	//		if (client_manager == MemoryManager::Read<int>(weapon_address + j))
	//			printf("offset: %X\n", j);
	//	}

	//	currentLadder = MemoryManager::Read<int>(currentLadder);
	//}


	//if (sa_camera)
	//{
	//	//D3DXMATRIX matrix = MemoryManager::Read<D3DXMATRIX>(sa_camera + 0x9F4);

	//	//matrix._11 = 1.0f;
	//	//matrix._12 = 1.0f;
	//	//matrix._13 = 1.0f;
	//	//matrix._14 = 1.0f;

	//	//MemoryManager::Write<D3DXMATRIX>(sa_camera + 0x9F4, matrix);

	//	D3DXVECTOR3 aim = MemoryManager::Read<D3DXVECTOR3>(sa_camera + 0x908);

	//	printf("%f %f %f\n", aim.x, aim.y, aim.z);

	//	//0xB6F258
	//}


	//for (int i = 0x15C; i < 0x15C + 48; i += 4)
	//{
	//	float roll = MemoryManager::Read<float>(camera + i);

	//	printf("offset: %X, roll: %f\n", i, roll);

	//	//MemoryManager::Write<float>(camera + i, 1.0f);
	//}


	//MemoryManager::Write<float>(camera + 0x15c, 5000.0f);

	//for (int i = 0; i < 1024; i += 4)
	//{
	//	int pointer = MemoryManager::Read<int>(camera + i);

	//	if (pointer == mta_local_player)
	//		printf("offset: %X\n", i);
	//}

	//printf("offset of ammo: %X\n", offsetof(CNetworkPlayer, m_usWeaponAmmo));

	//int addrs = MemoryManager::Read<int>(client_game + 0x168);
	//CNetworkPlayer mta_player = MemoryManager::Read<CNetworkPlayer>(addrs);//GetMTAPlayerbyPed(GetLocalPlayer());

	//printf("addr: %X\n", addrs);
	//if (mta_player)
	{

		//MemoryManager::Write<float>(addrs + 0x2F0, 100.0f);
		//MemoryManager::Write<float>(GetLocalPlayer() + 0x540, 100.0f);

		//static int addr = 0x360;

		//float move_speed = MemoryManager::Read<float>(mta_player->self_addr + 0x300);
		//printf("speed: %f\n", move_speed);

		//int cur_slot = MemoryManager::Read<int>(mta_player->self_addr + 0x360);
		//printf("offset %X, cur_slot: %d\n", 0x35C, cur_slot);
		//int cur_slot = MemoryManager::Read<int>(mta_player->self_addr + 0x398);
		//printf("offset %X, cur_slot: %f\n", 0x398, mta_player->m_fMoveSpeed);

		//if (GetAsyncKeyState('K') & 1) // first scan
		//{
		//	values.clear();

		//	for (int i = 0; i < 8196; i++)
		//	{
		//		char slot = MemoryManager::Read<char>(mta_player->ped_addr_way + i);

		//		values[i] = slot;
		//	}
		//}

		//if (GetAsyncKeyState('U') & 1) // next scan
		//{
		//	std::map<int, char> temp_map = values;

		//	values.clear();

		//	for (int i = 0; i < 8196; i++)
		//	{
		//		char slot = MemoryManager::Read<char>(mta_player->ped_addr_way + i);

		//		if (temp_map[i] != slot)
		//			values[i] = slot;
		//	}

		//
		//}

		//if (GetAsyncKeyState('L') & 1) // show what changed
		//{
		//	for (int i = 0; i < 8196; i++)
		//	{
		//		char slot = MemoryManager::Read<char>(mta_player->ped_addr_way + i);

		//		if (values[i] != slot)
		//			printf("offset %d, old value: %d, new value: %d\n", i, values[i], slot);
		//	}
		//}

		//

		// health locked 0x486

		//bool health_locked = MemoryManager::Read<bool>(mta_player->self_addr + 0x486);
		//printf("health locked: %d\n", health_locked);

		//MemoryManager::Write<bool>(mta_player->self_addr + 0x486, true);
		//MemoryManager::Write<float>(mta_player->self_addr + 0x2F0, 100.0f);

	}

}

void MTAVehicleWallhack()
{
	try {
		{
			DWORD local_player =  MemoryManager::Read<DWORD>(0xB6F5F0);
			D3DXVECTOR3 LocalPlayerPos = MemoryManager::Read<D3DXVECTOR3>(MemoryManager::Read<DWORD>(local_player + 0x14) + 0x30);

			DWORD temp1 = MemoryManager::Read<DWORD>(ClientModule->baseAddr + dwVehicleType);
			CMTAStructureInfo info = MemoryManager::Read<CMTAStructureInfo>(temp1);
			
			//DWORD temp2 = MemoryManager::Read<DWORD>(temp1 + 0x10);
			DWORD temp2 = info.temp2;

			//int max = MemoryManager::Read<int>(temp1 + 0xc);
			int max = info.max;

			if (max > 10000)
			{
				return;
			}


			for (int i = 0; i < max; i++)
			{
				DWORD vehicle = MemoryManager::Read<DWORD>(temp2 + (i * 0x4));

				if (vehicle == 0)
				{
					continue;
				}

				//if (MemoryManager::Read<int>(vehicle + 0x5C) < 100)
				//{
				//	continue;
				//}


				//for (int n = 0; n < 500; n++)
				//{
				//	short carId = MemoryManager::Read<short>(vehicle + n);

				//	if (carId >= 300 && carId <= 800)
				//		printf("n: %d, carId: %d\n", n, carId);
				//}

				short carID = MemoryManager::Read<short>(vehicle + o_MTACarId);

				if (carID < 300 || carID > 800)
				{
					continue;
				}

				if (vehicleFilter)
				{
					if (std::find(vehicles.begin(), vehicles.end(), carID) == vehicles.end())
					{
						continue;
					}

				}

				D3DXVECTOR3 vehicle_position = MemoryManager::Read<D3DXVECTOR3>(vehicle + o_MtaPlayerPos);

				if ((int)vehicle_position.x == 0 && (int)vehicle_position.y == 0 && (int)vehicle_position.z == 0)
					continue;

				D3DXVECTOR2 screen_position = WorldToScreen(vehicle_position);

				if (screen_position.x <= -10 || screen_position.x > Width + 10 || screen_position.y <= -10 || screen_position.y > Height + 10)
					continue;

				float theValuev2 = (LocalPlayerPos - vehicle_position).sqrMagnitude();
				if (theValuev2 >= INT_MAX || theValuev2 <= INT_MIN || theValuev2 >= FLT_MAX || theValuev2 <= -FLT_MAX)
				{
					continue;
				}

				std::string myBetchv2 = "( " + std::to_string((int)theValuev2) + " m )";


				try
				{

					if (vehicleList.find(carID) != vehicleList.end())
					{
						std::string vehname = vehicleList[carID];
						std::string towriteonscr = vehname;
						DrawString((char*)towriteonscr.c_str(), (int)screen_position.x, (int)screen_position.y, 0, 255, 0, pFontSmall);
						DrawString((char*)myBetchv2.c_str(), (int)screen_position.x, (int)screen_position.y + 14, 0, 255, 0, pFontSmall);
					}
					else {
						std::string towriteonscr = "vehicle";
						DrawString((char*)towriteonscr.c_str(), (int)screen_position.x, (int)screen_position.y, 0, 255, 0, pFontSmall);
						DrawString((char*)myBetchv2.c_str(), (int)screen_position.x, (int)screen_position.y + 14, 0, 255, 0, pFontSmall);
					}

				}
				catch (...)
				{
					//std::cout << "line: " << /*__LINE__*/ " | " << ex.what() << std::endl;
				}

				//std::string nName = (char*)"P";


				//try {
				//	char* xnName = (char*)"";
				//	int len = MemoryManager::Read<int>(vehicle + o_Nickname + 0x10);
				//	if (len <= 15)
				//	{
				//		xnName = MemoryManager::ReadString(vehicle + o_Nickname, len);
				//	}
				//	else {
				//		xnName = MemoryManager::ReadString(MemoryManager::Read<DWORD>(vehicle + o_Nickname), len);
				//	}

				//	std::string daNAMEX = "";
				//	if (xnName > 0)
				//	{
				//		//len = strlen(xnName);
				//		memcpy(xnName, xnName, len);
				//		memset(xnName + len - 1, 0, 1);
				//		daNAMEX = xnName;
				//		RemoveSpecials(daNAMEX);
				//	}

				//	nName = "P ( " + daNAMEX + " )";


				//}
				//catch (...) {
				//	nName = "P";
				//}



				//D3DXVECTOR3 vPos = MemoryManager::ReadVirtualMemory<D3DXVECTOR3>(vehicle + o_MtaPlayerPos), scrPos = D3DXVECTOR3(0, 0, 0);

				//CalcScreenCoors(&vPos, &scrPos);

				//if (scrPos.z <= 1.0f || scrPos.x > s_width || scrPos.y > s_height || scrPos.x < 0 || scrPos.y < 0)
				//{
				//	continue;
				//}


				//DWORD LocalPlayerF = MemoryManager::Read<DWORD>(0xB6F5F0);
				//D3DXVECTOR3 LocalPlayerPos = MemoryManager::ReadVirtualMemory<D3DXVECTOR3>(MemoryManager::Read<DWORD>(LocalPlayerF + 0x14) + 0x30);
				//float theValuev2 = GetDistance(vPos, LocalPlayerPos);
				//string myBetchv2 = "( " + to_string((int)theValuev2) + " m )";
				//string myBetchv1 = "";

				//if (nName != "")
				//{
				//	myBetchv1 = nName;
				//	RemoveSpecials(myBetchv1);
				//}


				//int R = 255, G = 255, B = 255;



				//try
				//{
				//	DrawString(scrPos.x, scrPos.y, D3DCOLOR_ARGB(255, R, G, B), pFont, myBetchv1.c_str());
				//	DrawString(scrPos.x, scrPos.y + 14, D3DCOLOR_ARGB(255, R, G, B), pFont, myBetchv2.c_str());
				//}
				//catch (int err)
				//{

				//}


			}
		}
	}

	catch (...)
	{
		//std::cout << "line: " << /*__LINE__*/ " | " << ex.what() << std::endl;
	}

}

void MTAColWallhack()
{
	DWORD local_player = MemoryManager::Read<DWORD>(0xB6F5F0);
	D3DXVECTOR3 LocalPlayerPos = MemoryManager::Read<D3DXVECTOR3>(MemoryManager::Read<DWORD>(local_player + 0x14) + 0x30);

	//for (int i = -512; i <= 512; i += 4)
	{


		DWORD temp1 = MemoryManager::Read<DWORD>(ClientModule->baseAddr + dwColType);
		CMTAColStructureInfo info = MemoryManager::Read<CMTAColStructureInfo>( temp1 );

		int max = (info.to_calculate_len - info.pointer_to_members) / 4;


		if (max > 500000)
		{
			return;
		}


		for (int i = 0; i < max; i++)
		{
			DWORD object_addr = MemoryManager::Read<DWORD>(info.pointer_to_members + (i * 0x4));

			if (object_addr == 0)
			{
				continue;
			}

			D3DXVECTOR3 object_position = MemoryManager::Read<D3DXVECTOR3>(object_addr + o_MtaColPos);

			if ((int)object_position.x == 0 && (int)object_position.y == 0 && (int)object_position.z == 0)
				continue;

			float theValuev2 = (LocalPlayerPos - object_position).sqrMagnitude();
			if (theValuev2 >= INT_MAX || theValuev2 <= INT_MIN || theValuev2 >= FLT_MAX || theValuev2 <= -FLT_MAX)
			{
				continue;
			}

			if (theValuev2 > EveryWHDistance)
				continue;

			D3DXVECTOR2 screen_position = WorldToScreen(object_position);

			if (screen_position.x <= -10 || screen_position.x > Width + 10 || screen_position.y <= -10 || screen_position.y > Height + 10)
				continue;

	

			std::string myBetchv2 = "( " + std::to_string((int)theValuev2) + " m )";



			std::string towriteonscr = "collider";
			DrawString((char*)towriteonscr.c_str(), (int)screen_position.x, (int)screen_position.y, 0, 255, 0, pFontSmall);
			DrawString((char*)myBetchv2.c_str(), (int)screen_position.x, (int)screen_position.y + 14, 0, 255, 0, pFontSmall);

		}
	}
}
class Friend {
public:
	int mta_id;
	int index;
	std::string name;
	bool IsSelected;
	D3DXVECTOR3 position;

	bool operator==(const Friend& rhs) {
		return rhs.mta_id == this->mta_id;
	}
};

std::vector<D3DXVECTOR3>* objectsformap = new std::vector<D3DXVECTOR3>();

int init_objects_for_map = 0;

void MTAObjectWallhack()
{


	DWORD local_player = MemoryManager::Read<DWORD>(0xB6F5F0);
	D3DXVECTOR3 LocalPlayerPos = MemoryManager::Read<D3DXVECTOR3>(MemoryManager::Read<DWORD>(local_player + 0x14) + 0x30);

	//for (int i = -512; i <= 512; i += 4)
	{


		DWORD temp1 = MemoryManager::Read<DWORD>(ClientModule->baseAddr + dwColType + 0x4);
		CMTAObjectStructureInfo info = MemoryManager::Read<CMTAObjectStructureInfo>(temp1);


		int max = info.max;


		if (max > 500000)
		{
			return;
		}

		bool init_objects = false;

		int cur_time = GetTickCount();
		
		if (MapObjects && cur_time - init_objects_for_map > 500)
		{
			objectsformap->clear();
			init_objects = true;
			init_objects_for_map = cur_time;
		}

		for (int i = 0; i < max; i++)
		{
			DWORD object_addr = MemoryManager::Read<DWORD>(info.temp2 + (i * 0x4));

			if (object_addr == 0)
			{
				continue;
			}

			int object_id = MemoryManager::Read<unsigned short>(object_addr + o_MtaObjectId);

			if (WHObjectIDs)
			{
				if (std::find(objects.begin(), objects.end(), object_id) == objects.end())
					continue;
			}


			D3DXVECTOR3 object_position = MemoryManager::Read<D3DXVECTOR3>(object_addr + o_MtaPlayerPos);

			if ((int)object_position.x == 0 && (int)object_position.y == 0 && (int)object_position.z == 0)
				continue;


			float theValuev2 = (LocalPlayerPos - object_position).sqrMagnitude();
			if (theValuev2 >= INT_MAX || theValuev2 <= INT_MIN || theValuev2 >= FLT_MAX || theValuev2 <= -FLT_MAX)
			{
				continue;
			}

			if (MapObjects && WHObjectIDs && init_objects)
			{
				objectsformap->push_back(object_position);
			}

			if (theValuev2 > EveryWHDistance)
				continue;




			D3DXVECTOR2 screen_position = WorldToScreen(object_position);

			if (screen_position.x <= -10 || screen_position.x > Width + 10 || screen_position.y <= -10 || screen_position.y > Height + 10)
				continue;


			std::string myBetchv2 = "( " + std::to_string((int)theValuev2) + " m )";
			std::string myBetchv3 = "ID: " + std::to_string((int)object_id);



			DrawString((char*)myBetchv3.c_str(), (int)screen_position.x, (int)screen_position.y, 0, 255, 0, pFontSmall);
			DrawString((char*)myBetchv2.c_str(), (int)screen_position.x, (int)screen_position.y + 14, 0, 255, 0, pFontSmall);
			//DrawString((char*)myBetchv3.c_str(), (int)screen_position.x, (int)screen_position.y + 28, 0, 255, 0, pFontSmall);

		}
	}
}



bool invalidChar(char c)
{
	return !(c > 32 && c < 128 && c != 44 && c != 96 && c != 126 && c != 94 && c != 37);
}


void RemoveSpecials(std::string& str)
{
	//str.erase(std::remove_if(str.begin(), str.end(), invalidChar), str.end());
}


using json = nlohmann::json;

void SaveSettings()
{
	json myJson;

	myJson["AimAssist"]["Enabled"] = AimEnabled;
	myJson["AimAssist"]["Smooth"] = Smooth;
	myJson["AimAssist"]["FOV"] = FOV;
	myJson["AimAssist"]["DontAimAtZombies"] = DontAimAtZombies;
	myJson["AimAssist"]["Key"] = aimKey;
	myJson["AimAssist"]["FriendKey"] = friendAimKey;
	myJson["AimAssist"]["AimMod"] = CurrentAimMode;
	myJson["AimAssist"]["LatencyAdjusting"] = m_LatencyAdjusting;
	myJson["AimAssist"]["VehicleLatencyAdjusting"] = m_VehicleLatencyAdjusting;

	myJson["WH"]["Enabled"] = WHEnabled;
	myJson["WH"]["Peds"] = WHPeds;
	myJson["WH"]["PedColors"] = WHPedColors;
	myJson["WH"]["Cars"] = WHCars;

	myJson["WH"]["Object"] = WHObject;
	myJson["WH"]["ObjectIDsBool"] = WHObjectIDs;
	myJson["WH"]["ObjectIDs"] = objectIDs;

	myJson["WH"]["Zombies"] = WHZombies;
	myJson["WH"]["ZombieIDs"] = zombieIDs;

	myJson["WH"]["showZombies"] = showZombies;
	myJson["WH"]["showCorpses"] = showCorpses;
	myJson["WH"]["everyWHKey"] = everyWHKey;


	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			if (ColorsForPeds[i][j] != 0)
				myJson["WH"]["PedColor"][i][j] = ColorsForPeds[i][j];
			else
				myJson["WH"]["PedColor"][i][j] = 0;
		}
	}

	for (int j = 0; j < (sizeof(WHObjectList) / sizeof(*WHObjectList)); j++)
	{
		myJson["WH"]["ObjectList"][j] = WHObjectList[j];
	}

	std::ofstream out;
	out.open("settings.json");
	if (out.is_open())
	{
		std::string thatsIt = myJson.dump(4);

		out << thatsIt;
		out.close();
		MessageBeep(MB_OK);
	}
	else
	{
		MessageBeep(MB_ICONERROR);
	}
}

void LoadSettings()
{
	if (std::ifstream("settings.json").fail())
	{
		return;
	}

	json myJson;

	std::ifstream outtoin;
	outtoin.open("settings.json");
	if (outtoin.is_open())
	{
		myJson << outtoin;
		outtoin.close();
		MessageBeep(MB_OK);
	}
	else
	{
		MessageBeep(MB_ICONERROR);
		SaveSettings();
	}

	try
	{
		AimEnabled = myJson["AimAssist"]["Enabled"];
		Smooth = myJson["AimAssist"]["Smooth"];
		FOV = myJson["AimAssist"]["FOV"];
		DontAimAtZombies = myJson["AimAssist"]["DontAimAtZombies"];
		aimKey = myJson["AimAssist"]["Key"];
		friendAimKey = myJson["AimAssist"]["FriendKey"];

		CurrentAimMode = myJson["AimAssist"]["AimMod"];
		m_LatencyAdjusting = myJson["AimAssist"]["LatencyAdjusting"];
		m_VehicleLatencyAdjusting = myJson["AimAssist"]["VehicleLatencyAdjusting"];

		WHEnabled = myJson["WH"]["Enabled"];
		WHPeds = myJson["WH"]["Peds"];
		WHPedColors = myJson["WH"]["PedColors"];
		WHCars = myJson["WH"]["Cars"];

		WHObject = myJson["WH"]["Object"];

		WHObjectIDs = myJson["WH"]["ObjectIDsBool"];

		memcpy(objectIDs, myJson["WH"]["ObjectIDs"].get<std::string>().c_str(), strlen(myJson["WH"]["ObjectIDs"].get<std::string>().c_str()) + 1);

		WHZombies = myJson["WH"]["Zombies"];

		memcpy(zombieIDs, myJson["WH"]["ZombieIDs"].get<std::string>().c_str(), strlen(myJson["WH"]["ZombieIDs"].get<std::string>().c_str()) + 1);


		showZombies = myJson["WH"]["showZombies"];
		showCorpses = myJson["WH"]["showCorpses"];

		everyWHKey = myJson["WH"]["everyWHKey"];

		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				ColorsForPeds[i][j] = myJson["WH"]["PedColor"][i][j];
			}
		}

		for (int j = 0; j < (sizeof(WHObjectList) / sizeof(*WHObjectList)); j++)
		{
			WHObjectList[j] = myJson["WH"]["ObjectList"][j];
		}
	}
	catch (int err) {
		
	}

}

DWORD GetPhysicalDriveSerialNumber(UINT nDriveNumber, CString& strSerialNumber)
{
	DWORD dwResult = NO_ERROR;
	strSerialNumber.Empty();

	// Format physical drive path (may be '\\.\PhysicalDrive0', '\\.\PhysicalDrive1' and so on).
	CString strDrivePath;
	strDrivePath.Format(_T("\\\\.\\PhysicalDrive%u"), nDriveNumber);

	// call CreateFile to get a handle to physical drive
	HANDLE hDevice = ::CreateFile(strDrivePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, 0, NULL);

	if (INVALID_HANDLE_VALUE == hDevice)
	{
		return ::GetLastError();
	}


	// set the input STORAGE_PROPERTY_QUERY data structure
	STORAGE_PROPERTY_QUERY storagePropertyQuery;
	ZeroMemory(&storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY));
	storagePropertyQuery.PropertyId = StorageDeviceProperty;
	storagePropertyQuery.QueryType = PropertyStandardQuery;

	// get the necessary output buffer size
	STORAGE_DESCRIPTOR_HEADER storageDescriptorHeader = { 0 };
	DWORD dwBytesReturned = 0;
	if (!::DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
		&storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
		&storageDescriptorHeader, sizeof(STORAGE_DESCRIPTOR_HEADER),
		&dwBytesReturned, NULL))
	{
		dwResult = ::GetLastError();
		::CloseHandle(hDevice);
		return dwResult;
	}

	// allocate the necessary memory for the output buffer
	const DWORD dwOutBufferSize = storageDescriptorHeader.Size;
	BYTE* pOutBuffer = new BYTE[dwOutBufferSize];
	ZeroMemory(pOutBuffer, dwOutBufferSize);

	// get the storage device descriptor
	if (!::DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
		&storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
		pOutBuffer, dwOutBufferSize,
		&dwBytesReturned, NULL))
	{
		dwResult = ::GetLastError();
		delete[]pOutBuffer;
		::CloseHandle(hDevice);
		return dwResult;
	}

	// Now, the output buffer points to a STORAGE_DEVICE_DESCRIPTOR structure
	// followed by additional info like vendor ID, product ID, serial number, and so on.
	STORAGE_DEVICE_DESCRIPTOR* pDeviceDescriptor = (STORAGE_DEVICE_DESCRIPTOR*)pOutBuffer;
	const DWORD dwSerialNumberOffset = pDeviceDescriptor->SerialNumberOffset;
	if (dwSerialNumberOffset != 0)
	{
		// finally, get the serial number
		strSerialNumber = CString(pOutBuffer + dwSerialNumberOffset);
	}

	// perform cleanup and return
	delete[]pOutBuffer;
	::CloseHandle(hDevice);
	return dwResult;
}

int ClampInt(int value, int min, int max)
{
	if (value > max)
		return max;

	if (value < min)
		return min;

	return value;
}


std::vector<Friend> friends, players;

void RefreshPlayerList()
{
	players.clear();

	int32_t temp1 = MemoryManager::Read<int32_t>(ClientModule->baseAddr + dwClientType);

	CMTAStructureInfo info = MemoryManager::Read< CMTAStructureInfo>(temp1);
	//int32_t temp2 = MemoryManager::Read<int32_t>(temp1 + 0x10);
	int32_t temp2 = info.temp2;
	//int max = MemoryManager::Read<int>(temp1 + 0xc);
	int max = info.max;

	if (max > 50000)
	{
		return;
	}


	for (int i = 0; i < max; i++)
	{
		int32_t player_addr = MemoryManager::Read<int32_t>(temp2 + (i * 0x4));

		if (player_addr == 0)
			continue;
		
			
		
		CNetworkPlayer player = MemoryManager::Read<CNetworkPlayer>(player_addr);


		//int mta_id = MemoryManager::Read<int>(player_addr + 0x5C);
		int mta_id = player.mta_id;
		if (mta_id < 100)
		{
			continue;
		}
			
		

		char* nName = "";

		//int name_len = MemoryManager::Read<int>(player_addr + o_Nickname + 0x10);
		int name_len = player.nickname_len;

		name_len = ClampInt(name_len, 0, 22);

		if (name_len <= 15)
		{
			nName = MemoryManager::ReadString(player_addr + o_Nickname, name_len + 1);
		}
		else {
			nName = MemoryManager::ReadString(player.nickname_addr, name_len + 1);
		}

		std::string name_info = "";

		if (nName)
		{
			name_info = nName;
			RemoveSpecials(name_info);
		}

		Friend afriend;
		afriend.IsSelected = false;
		afriend.index = i;
		afriend.mta_id = mta_id;
		afriend.name = name_info;
		afriend.position = player.position;
		players.push_back(afriend);

		if (nName)
		{
			free(nName);
		}
	}


}

char* to_lower(char* str)
{
	int len = strlen(str);

	for (int i = 0; i < len; i++)
		str[i] = tolower(str[i]);

	return str;
}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

//std::vector<std::string> split(const std::string& str, const std::string& delim)
//{
//	std::vector<std::string> tokens;
//	size_t prev = 0, pos = 0;
//	do
//	{
//		pos = str.find(delim, prev);
//		if (pos == std::string::npos) pos = str.length();
//		std::string token = str.substr(prev, pos - prev);
//		if (!token.empty()) tokens.push_back(token);
//		prev = pos + delim.length();
//	} while (pos < str.length() && prev < str.length());
//
//	return tokens;
//}

bool Log_in(std::string name, std::string pass, std::string hwid)
{
	CURL *curl;
	CURLcode res;
	std::string readBuffer;

	hwid.erase(remove(hwid.begin(), hwid.end(), ' '), hwid.end());

	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, "https://raw.githubusercontent.com/Aeonblue7/ss/master/ss.txt");
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);


		std::stringstream ss(readBuffer);
		std::string to;

		while (std::getline(ss, to, '\n')) {
			if (to.find(name) != std::string::npos)
			{
				std::vector<std::string> strings = split(to, ":");

				if (strings[2].find(hwid) != std::string::npos && strings[1] == pass)
				{
					return true;
				}
			}
		}

	}
	else {
		printf("CURL init failed!\n");
	}

	return false;
}

LPDIRECT3DTEXTURE9 map_texture = nullptr;
char searchName[23];

//const char* DecryptString(const char* str)
//{
//	char* decrypted_string = (char*)str;
//
//	for (int i = 0; i < strlen(decrypted_string); i++)
//		if(decrypted_string[i] != 32)
//			decrypted_string[i] -= 1;
//
//	return (const char*)decrypted_string;
//}

int last_reset_time = 0;

void MenuDraw()
{
	if (!map_texture)
	{
		D3DXCreateTextureFromFile(p_Device, "map.png", &map_texture);
	}


	if (PlayerAlertEnabled && PlayerAlert)
	{
		DrawString("Player Alert!", 0, 0, 255, 0, 0, pFontMedium);
	}


	MenuEnabled = GetKeyState(VK_F7);

	ImGui_ImplDX9_NewFrame();


	if (MenuEnabled || (radarEnabled && GetKeyState(radarKey)))
	{
		int cur_time = GetTickCount();
		if (cur_time- last_reset_time > 500)
		{
			RefreshPlayerList();
			last_reset_time = cur_time;
		}
	}
		


	if (MenuEnabled)
	{
		if (bigHake(thingx))
		{

			ImGui::Begin("Friendlist");

			std::string player_list = "Player List ( " + std::to_string(players.size()) + " )";
			ImGui::ListBoxHeader(player_list.c_str());
			for (auto item : players)
			{
				std::string& item_name = item.name + " : " + std::to_string(item.index) + " : " + std::to_string(item.mta_id);
				if (ImGui::Selectable(item_name.c_str(), item.IsSelected))
				{
					int id = item.mta_id;
					if (!(std::find(friendList.begin(), friendList.end(), id) != friendList.end()))
					{
						friendList.push_back(id);
						friends.push_back(item);
					}

				}
			}
			ImGui::ListBoxFooter();

			ImGui::ListBoxHeader("Friend List");
			for (auto item : friends)
			{
				std::string& item_name = item.name + " : " + std::to_string(item.index);
				if (ImGui::Selectable(item_name.c_str(), item.IsSelected))
				{
					int id = item.mta_id;
					if (std::find(friendList.begin(), friendList.end(), id) != friendList.end())
					{
						friendList.erase(std::find(friendList.begin(), friendList.end(), id));
						friends.erase(std::find(friends.begin(), friends.end(), item));
					}

				}
			}
			ImGui::ListBoxFooter();

			ImGui::End();
		}


		{
			ImGui::Begin("With");

			static int m_CurrentPane = 0;

			if (m_CurrentPane == 0)
			{
				static char username[256], password[256];
				ImGui::PushItemWidth(200);
				ImGui::InputText("E-mail", username, 256);
				ImGui::PushItemWidth(200);
				ImGui::InputText("Password", password, 256, ImGuiInputTextFlags_::ImGuiInputTextFlags_Password);

				if (ImGui::Button("Login"))
				{

					UINT nDriveNumber = 0;
					CString strSerialNumber;
					DWORD dwResult = GetPhysicalDriveSerialNumber(nDriveNumber, strSerialNumber);

					if (NO_ERROR != dwResult)
					{
						MessageBox(0, "Something went wrong. App is going to close.", "Error", 0);
						exit(0);
						return;
					}

					std::string testediyorum = (std::string)strSerialNumber;
					char* HWIDNumber = 0;
					std::string HWIDString;
					if (testediyorum != "") {
						HWIDNumber = (char *)testediyorum.c_str();
						HWIDString = HWIDNumber;
					}
					else
					{
						MessageBox(0, "Something went wrong. App is going to close.", "Error", 0);
						exit(0);
						return;
					}

					std::string uname = username, pass = password;

					if (Log_in(uname, pass, HWIDString))
					{
						m_CurrentPane = 1;
						thingx = "Wrong username or password.";
						LoadSettings();
					}
				}

			}
			else
			{
				if (ImGui::Button("Wallhack"))
				{
					m_CurrentPane = 2;
				}
				ImGui::SameLine();
				if (ImGui::Button("Aim Assist"))
				{
					m_CurrentPane = 3;
				}
				ImGui::SameLine();

				if (ImGui::Button("Misc"))
				{
					m_CurrentPane = 4;
				}


				ImGui::Separator();

				if (m_CurrentPane == 1)
				{
					ImGui::Text("Thanks for buying this application, enjoy game :)");
				}

				if (m_CurrentPane == 2)
				{
					ImGui::Checkbox("Wall hack", &WHEnabled);
					if (WHEnabled)
					{
						ImGui::Checkbox("Peds", &WHPeds);
						if (WHPeds)
						{
							ImGui::Checkbox("Ped Colors", &WHPedColors);
							if (WHPedColors)
							{
								ImGui::Spacing();
								ImGui::ColorEdit3("Normal ( zombies, npcs etc. )", ColorsForPeds[0]);
								ImGui::ColorEdit3("Player", ColorsForPeds[1]);
								ImGui::ColorEdit3("Corpse", ColorsForPeds[2]);
							}
							ImGui::Checkbox("Show corpses", &showCorpses);
							ImGui::Spacing();
						}

						ImGui::Checkbox("Cars", &WHCars);

						ImGui::Checkbox("Objects", &WHObject);

						if (WHObject)
						{
			/*				for (int i = 0; i < (sizeof(WHObjectList) / sizeof(*WHObjectList)); i++)

							{
								ImGui::Checkbox(std::to_string(i).c_str(), &WHObjectList[i]);
								if ((sizeof(WHObjectList) / sizeof(*WHObjectList)) / 2 != i + 1)
								{
									if ((sizeof(WHObjectList) / sizeof(*WHObjectList)) - 1 != i)
									{
										ImGui::SameLine();
									}

								}
							}*/
							//ImGui::Spacing();
							ImGui::Checkbox("Filter: ID 2", &WHObjectIDs);
							if (WHObjectIDs)
							{
								ImGui::InputText("Object IDs", objectIDs, 1024);

								objects.clear();

								std::vector<std::string> splitd = split(objectIDs, ",");

								for (int i = 0; i < splitd.size(); i++)
								{
									objects.push_back(atoi(splitd[i].c_str()));
								}
							}

							//ImGui::Checkbox("Remove Object (Needs Driver)", &removeObject);

						}

						ImGui::Checkbox("Zombies", &WHZombies);

						if (WHZombies)
						{
							ImGui::Checkbox("Show zombies", &showZombies);
							ImGui::InputText("Zombie Skin IDs", zombieIDs, 1024);
							ImGui::TextWrapped("You must use \",\" between ids.");

							zombies.clear();

							std::vector<std::string> splitd = split(zombieIDs, ",");

							for (int i = 0; i < splitd.size(); i++)
							{
								zombies.push_back(atoi(splitd[i].c_str()));
							}
						}

						ImGui::Spacing();

						ImGui::Hotkey("World Wallhack", &everyWHKey);
						ImGui::Checkbox("Vehicle Filter", &vehicleFilter);
						if (vehicleFilter)
						{
							ImGui::InputText("Vehicle IDs", vehicleIDs, 1024);
							ImGui::TextWrapped("You must use ',' between ids.");

							vehicles.clear();

							std::vector<std::string> splitd = split(vehicleIDs, ",");

							for (int i = 0; i < splitd.size(); i++)
							{
								vehicles.push_back(atoi(splitd[i].c_str()));
							}
						}
						ImGui::Spacing();

						ImGui::Hotkey("Object Key", &EveryObjectWHKey);

						ImGui::Checkbox("World Object Wallhack", &EveryObjectWH);
						ImGui::Checkbox("World Collider Wallhack", &EveryColliderWH);

						ImGui::SliderFloat("Max Distance For Objects", &EveryWHDistance, 100, 10000);

					}
				}

				if (m_CurrentPane == 3)
				{
					ImGui::Checkbox("Aimbot", &AimEnabled);
					if (AimEnabled)
					{
						ImGui::Spacing();
						const char* items[] = { "Magnet Based Aim", "FOV Based Aim" };

						ImGui::Combo("Aim Mode", &CurrentAimMode, items, IM_ARRAYSIZE(items));					static const char* items2[] = { "Head", "Chest", "Spine" };
						ImGui::Combo("Aim Bone", &CurrentAimBotBone, items2, IM_ARRAYSIZE(items2));
						if (CurrentAimBotBone == 0)
						{
							aimBotBone = 6;
						}
						if (CurrentAimBotBone == 1)
						{
							aimBotBone = 4;
						}
						if (CurrentAimBotBone == 2)
						{
							aimBotBone = 2;
						}
						ImGui::Spacing();
						ImGui::SliderFloat("Smooth", &Smooth, 0.0f, 1000.0f);
						if (CurrentAimMode == fovAim)
						{
							ImGui::SliderFloat("FOV", &FOV, 0.0f, 1000.0f);
						}

						ImGui::SliderFloat("Latency Adjusting", &m_LatencyAdjusting, -0.5f, 0.5f);
						ImGui::SliderFloat("Vehicle Latency Adjusting", &m_VehicleLatencyAdjusting, -0.5f, 0.5f);

						ImGui::Spacing();
						ImGui::TextWrapped("Smooth: Aiming speed. It depends screen resolution and can be flickering after a value, set it by trying.");
						ImGui::Spacing();
						if (CurrentAimMode == fovAim)
						{
							ImGui::TextWrapped("FOV: It's called field of view of your aim bot so it can be described as a distance to aim peds.");
							ImGui::Spacing();
						}

						ImGui::Checkbox("Do not Aim At Zombies", &DontAimAtZombies);
						if (DontAimAtZombies)
						{
							ImGui::InputText("Zombie Skin IDs", zombieIDs, 1024);
							ImGui::TextWrapped("You must use \",\" between ids.");
							ImGui::Spacing();

							zombies.clear();

							std::vector<std::string> splitd = split(zombieIDs, ",");

							for (int i = 0; i < splitd.size(); i++)
							{
								zombies.push_back(atoi(splitd[i].c_str()));
							}
						}
						ImGui::Spacing();

						ImGui::Hotkey("Aim key   ", &aimKey);
						//ImGui::Hotkey("Friend key", &friendAimKey);

						//ImGui::Spacing();

						//ImGui::TextWrapped("Aim at your friend and press Mouse-3 ( selected key ) to add him to your friend list.");
					}
				}

				if (m_CurrentPane == 4)
				{
					if (ImGui::Button("Save settings"))
					{
						SaveSettings();
					}

					if (ImGui::Button("Load settings"))
					{
						LoadSettings();
					}

					ImGui::Checkbox("Map", &radarEnabled);
					ImGui::Hotkey("Map Key", &radarKey);

					ImGui::Checkbox("Player Alert", &PlayerAlertEnabled);

					ImGui::Spacing();

					bool temp1 = MapObjects;
					ImGui::Checkbox("Map Objects", &MapObjects);

					if (MapObjects != temp1)
					{
						if (MapObjects && WHObjectIDs)
							MTAObjectWallhack();
					}

					if (ImGui::Button("Reset Map Objects"))
					{
						objectsformap->clear();

						if (MapObjects && WHObjectIDs)
							MTAObjectWallhack();
					}

					ImGui::Checkbox("Manipulate Shots", &ManipulateShots);
					if (ManipulateShots)
					{
						ImGui::InputInt("Manipulated Body Piece", &ManipulatedBodyPiece);
						ImGui::InputInt("Manipulated Weapon", &ManipulatedWeapon);
					}

					ImGui::Checkbox("Run Engine", &RunEngine);
					if (RunEngine)
					{
						ImGui::Hotkey("Run Engine Key", &RunEngineKey);
					}

					ImGui::Checkbox("No Recoil", &NoRecoil);
				}
			}

			ImGui::End();
		}
	}
	ImVec2 radar_window_pos = ImVec2(0, 0), radar_window_size = ImVec2(0, 0);

	if (radarEnabled && bigHake(thingx) && GetKeyState(radarKey))
	{

		{
			ImGui::Begin("Radar");
			radar_window_size = ImGui::GetWindowSize();
			radar_window_size = ImVec2(radar_window_size.x - 25, radar_window_size.y - 60);
			ImGui::Image(map_texture, radar_window_size);
			radar_window_pos = ImGui::GetWindowPos();

			ImGui::InputText("Search player", searchName, 23);

			ImGui::End();
		}
	}


	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

	if (radarEnabled && bigHake(thingx) && GetKeyState(radarKey)) {
		int mta_local_id = -1;
		int32_t local_player = MemoryManager::Read<int32_t>(0xB6F5F0);
		if (local_player > 0)
		{
			CNetworkPlayer* mta_local_player = GetMTAPlayerbyPed(local_player);

			if (mta_local_player != NULL)
			{
				mta_local_id = mta_local_player->mta_id;

				free(mta_local_player);
			}
		}



		POINT cursor_pos;
		GetCursorPos(&cursor_pos);
		int count = 0;
		for (auto player : players)
		{
			if (player.mta_id != mta_local_id)
				if (strlen(searchName) > 0 && !strstr(to_lower((char*)player.name.c_str()), to_lower(searchName))) continue;

			player.position.x += 3000;
			player.position.y = 3000 - player.position.y;

			player.position.x *= radar_window_size.x / 6000.0f;
			player.position.y *= radar_window_size.y / 6000.0f;


			if (player.mta_id == mta_local_id)
			{
				DrawGUIBox(radar_window_pos.x + 7 + player.position.x, radar_window_pos.y + 27 + player.position.y, 8, 8, 0, 0, 0, 255, 0, 0, 255, 255);

			}
			else if (std::find(friendList.begin(), friendList.end(), player.mta_id) != friendList.end())
			{
				DrawGUIBox(radar_window_pos.x + 7 + player.position.x, radar_window_pos.y + 27 + player.position.y, 7, 7, 0, 0, 0, 255, 255, 0, 255, 255);
			}
			else {
				DrawGUIBox(radar_window_pos.x + 7 + player.position.x, radar_window_pos.y + 27 + player.position.y, 5, 5, 0, 0, 0, 255, 255, 0, 0, 255);
			}

			if (cursor_pos.x >= radar_window_pos.x + 7 + player.position.x && cursor_pos.x <= radar_window_pos.x + 7 + player.position.x + 5 && cursor_pos.y >= radar_window_pos.y + 27 + player.position.y && cursor_pos.y <= radar_window_pos.y + 27 + player.position.y + 5)
			{
				DrawString((char*)player.name.c_str(), 0, count * 24, 255, 255, 255, pFontMedium);
				count++;
			}

			/*ImGui::Image(player_texture, ImVec2(10, 10));
			ImVec2 window_pos = ImGui::GetWindowPos(), window_size = ImGui::GetWindowSize();
			ImVec2 parameter2 = ImVec2(window_pos.x + window_size.x, window_pos.y + window_size.y);
			ImGui::GetWindowDrawList()->AddImage(
				(void *)player_texture,
				ImVec2(window_pos.x + 100, window_pos.y)
			, ImVec2(window_pos.x + 100, window_pos.y));*/
		}

		if (MapObjects && objectsformap->size() > 0)
		{
			auto objs = *objectsformap;
			for (auto object : objs)
			{
				object.x += 3000;
				object.y = 3000 - object.y;

				object.x *= radar_window_size.x / 6000.0f;
				object.y *= radar_window_size.y / 6000.0f;


				DrawGUIBox(radar_window_pos.x + 7 + object.x, radar_window_pos.y + 27 + object.y, 5, 5, 0, 0, 0, 255, 0, 255, 0, 255);

			}
		}

	}


}


void MTAPedWallhack()
{
	int32_t local_player = MemoryManager::Read<int32_t>(0xB6F5F0);
	D3DXVECTOR3 local_player_pos = MemoryManager::Read<D3DXVECTOR3>(MemoryManager::Read<DWORD>(local_player + 0x14) + 0x30);

	int32_t temp1 = MemoryManager::Read<int32_t>(ClientModule->baseAddr + dwClientType);

	CMTAStructureInfo info = MemoryManager::Read<CMTAStructureInfo>(temp1);
	//char* temp_to_keep = MemoryManager::ReadString(temp1 + 0xC, 0x8);

	//int32_t temp2 = MemoryManager::Read<int32_t>(temp1 + 0x10); //
	int32_t temp2 = info.temp2; //

	//int32_t max = MemoryManager::Read<int>(temp1 + 0xc); // 
	int32_t max = info.max; // 

	if (max > 10000)
	{
		return;
	}

	for (int i = 0; i < max; i++)
	{
		int32_t player = MemoryManager::Read<int32_t>(temp2 + (i * 0x4));
		if (player == 0)
			continue;

		CNetworkPlayer mta_player = MemoryManager::Read<CNetworkPlayer>(player);
		
		//int32_t mta_id = MemoryManager::Read<int32_t>(player + 0x5C);
		int32_t mta_id = mta_player.mta_id;

		if (mta_id < 100)
			continue;

		//D3DXVECTOR3 player_position = MemoryManager::Read<D3DXVECTOR3>(player + o_MtaPlayerPos);
		D3DXVECTOR3 player_position = mta_player.position;

		D3DXVECTOR2 screen_position = WorldToScreen(player_position);
		if (screen_position.x <= -10 || screen_position.x > Width + 10 || screen_position.y <= -10 || screen_position.y > Height + 10)
			continue;

		float theValuev2 = (local_player_pos - player_position).sqrMagnitude();
		std::string distance_info = "( " + std::to_string((int)theValuev2) + " m )";

		int R = 255, G = 255, B = 255;

		if (theValuev2 < 900)
		{
			R = 255; G = 0; B = 0;
		}
		else
		{
			R = 200; G = 200; B = 200;
		}

		char* nName = "";

		//int name_len = MemoryManager::Read<int>(player + o_Nickname + 0x10);
		int name_len = mta_player.nickname_len;

		name_len = ClampInt(name_len, 0, 22);

		if (name_len <= 15)
		{
			nName = MemoryManager::ReadString(player + o_Nickname, name_len + 1);
		}
		else {
			nName = MemoryManager::ReadString(mta_player.nickname_addr, name_len + 1);
		}



		std::string name_info = "";

		if (nName)
		{
			name_info = nName;
			RemoveSpecials(name_info);
		}

		if (std::find(friendList.begin(), friendList.end(), mta_id) != friendList.end())
		{
			name_info = ("Friend ( " + name_info + " )");
			R = 255;
			G = 75;
			B = 141;
		}


		DrawString((char*)name_info.c_str(), (int)screen_position.x, (int)screen_position.y, R, G, B, pFontSmall);
		DrawString((char*)distance_info.c_str(), (int)screen_position.x, (int)screen_position.y + 14, R, G, B, pFontSmall);

		if (nName)
		{
			free(nName);
		}
	}
}

void PedWallhack()
{
	PlayerAlert = false;

	int32_t local_player = MemoryManager::Read<int32_t>(0xB6F5F0);
	D3DXVECTOR3 local_player_pos = MemoryManager::Read<D3DXVECTOR3>(MemoryManager::Read<int32_t>(local_player + 0x14) + 0x30);

	CGTAStructureInfo info = MemoryManager::Read<CGTAStructureInfo>(MemoryManager::Read<int32_t>(0xB74490) + 0x0);
	//char* ped_list_temp = MemoryManager::ReadString(MemoryManager::Read<int32_t>(0xB74490) + 0x0, 0xC);

	//int32_t ped_list = *(int32_t*)(ped_list_temp + 0x0);
	int32_t ped_list = info.address;

	if (ped_list == NULL)
	{
		//free(ped_list_temp);
		return;
	}
		

	//int32_t max_peds = *(int32_t*)(ped_list_temp + 0x8);
	int32_t max_peds = info.max;

	if (max_peds > 10000 || max_peds <= 0)
	{
		//free(ped_list_temp);
		return;
	}
		

	int R = 255, G = 255, B = 255;

	for (int i = 0; i < max_peds; i++)
	{
		int32_t ped_addr = ped_list + (i * 0x7C4);

		if (ped_addr == 0 /*|| ped_addr == local_player*/)
			continue;

		CPed ped = MemoryManager::Read<CPed>(ped_addr);

		if (!IsPedPointerValid(ped, ped_addr, info))
			continue;

		//int health = (int)MemoryManager::Read<float>(ped_addr + 0x540);
		int health = ped.health;

		if (!showCorpses && health <= 0)
			continue;

		//int skinID = MemoryManager::Read<short>(ped_addr + 0x22);
		int skinID = ped.skin_id;

		bool isZombie = false;

		if (WHZombies)
		{
			//for (int j = 0; j < zombies.size(); j++)
			//{
			//	int id = zombies[j];

			//	if (id == skinID)
			//	{
			//		isZombie = true;
			//	}
			//}
			if (std::find(zombies.begin(), zombies.end(), skinID) != zombies.end())
			{
				isZombie = true;
			}
		}


		std::string str = "( " + std::to_string(health) + " )";


		if (WHPedColors)
		{
			str = ("( " + std::to_string((int)health) + " )");
			R = ColorsForPeds[0][0] * 255.f;
			G = ColorsForPeds[0][1] * 255.f;
			B = ColorsForPeds[0][2] * 255.f;


			if ((int)health <= 0)
			{
				str = ("D ( " + std::to_string((int)health) + " )");
				R = ColorsForPeds[2][0] * 255.f;
				G = ColorsForPeds[2][1] * 255.f;
				B = ColorsForPeds[2][2] * 255.f;

				if (!showCorpses)
				{
					continue;
				}

			}
		}


		if (isZombie)
		{
			str = ("Z ( " + std::to_string((int)health) + " )");
			R = (ColorsForPeds[0][0]) * 255.f;
			G = (ColorsForPeds[0][1]) * 255.f;
			B = (ColorsForPeds[0][2]) * 255.f;

			if (!showZombies)
			{
				continue;
			}

		}

		//int32_t position_struct = MemoryManager::Read<int32_t>(ped_addr + 0x14);
		int32_t position_struct = ped.matrix;
		D3DXVECTOR3 ped_position = MemoryManager::Read<D3DXVECTOR3>(position_struct + 0x30);

		float distance = (local_player_pos - ped_position).sqrMagnitude();

		D3DXVECTOR2 screen_position = WorldToScreen(ped_position);
		//if (screen_position.x <= -10 || screen_position.x > Width + 10 || screen_position.y <= -10 || screen_position.y > Height + 10)
		//	continue;

		CNetworkPlayer* mta_player = GetMTAPlayerbyPed(ped_addr);

		if (mta_player != NULL)
		{
			char* nName = "";

			int name_len = mta_player->nickname_len;

			if (name_len <= 15)
			{
				nName = MemoryManager::ReadString(mta_player->self_addr + o_Nickname, name_len + 1);
			}
			else {
				nName = MemoryManager::ReadString(mta_player->nickname_addr, name_len + 1);
			}

			std::string name_info = "";

			if (nName)
			{
				name_info = nName;
				RemoveSpecials(name_info);
			}

			R = ColorsForPeds[1][0] * 255;
			G = ColorsForPeds[1][1] * 255;
			B = ColorsForPeds[1][2] * 255;


			if (std::find(friendList.begin(), friendList.end(), mta_player->mta_id) != friendList.end())
			{
				name_info = ("Friend ( " + name_info + " )");
				R = 255;
				G = 75;
				B = 141;
			}
			else {
				PlayerAlert = true;
			}

			DrawString((char*)name_info.c_str(), (int)screen_position.x, (int)screen_position.y, R, G, B, pFontSmall);

			if (nName)
				free(nName);

			free(mta_player);

			// bone esp
			D3DXVECTOR2 last_position = WorldToScreen(GetBonePositionReversed(ped_addr, ped_position, BoneESPArray[0], 0));

			for (int j = 0; j < 8; j++)
			{
				D3DXVECTOR3 bone_position = GetBonePositionReversed(ped_addr, bone_position, BoneESPArray[j], 0);
				D3DXVECTOR2 screen_position = WorldToScreen(bone_position);

				if (screen_position.x < -10 || screen_position.y < -10)
					continue;
				//DrawString((char*)std::to_string(j).c_str(), screen_position.x, screen_position.y, 255, 255, 255, pFontSmall);
				DrawLine(last_position.x, last_position.y, screen_position.x, screen_position.y, R, G, B, 255);
				last_position = screen_position;
			}

			last_position = WorldToScreen(GetBonePositionReversed(ped_addr, ped_position, BoneESPArray[3], 0));

			for (int j = 8; j < 12; j++)
			{
				D3DXVECTOR3 bone_position = GetBonePositionReversed(ped_addr, bone_position, BoneESPArray[j], 0);
				D3DXVECTOR2 screen_position = WorldToScreen(bone_position);

				if (screen_position.x < -10 || screen_position.y < -10)
					continue;
				//DrawString((char*)std::to_string(j).c_str(), screen_position.x, screen_position.y, 255, 255, 255, pFontSmall);
				DrawLine(last_position.x, last_position.y, screen_position.x, screen_position.y, R, G, B, 255);
				last_position = screen_position;
			}

			// bone esp
		}

		std::string skin_info = "Skin ID: " + std::to_string(skinID);
		std::string distance_info = std::to_string((int)distance) + " m";

		DrawString((char*)str.c_str(), (int)screen_position.x, (int)screen_position.y+14, R, G, B, pFontSmall);
		DrawString((char*)distance_info.c_str(), (int)screen_position.x, (int)screen_position.y+28, R, G, B, pFontSmall);
		DrawString((char*)skin_info.c_str(), (int)screen_position.x, (int)screen_position.y+42, R, G, B, pFontSmall);
	}

	//free(ped_list_temp);
}


void VehicleWallhack()
{
	//char* vehicle_list_temp = MemoryManager::ReadString(MemoryManager::Read<int32_t>(0xB74494) + 0x0, 0xC);
	CGTAStructureInfo info = MemoryManager::Read<CGTAStructureInfo>(MemoryManager::Read<int32_t>(0xB74494) + 0x0);

	//int32_t vehicle_list = *(int32_t*)(vehicle_list_temp + 0x0);
	int32_t vehicle_list = info.address;

	//int maxNum = *(int32_t*)(vehicle_list_temp + 0x8);
	int maxNum = info.max;
	if (maxNum > 10000)
	{
		return;
	}

	int32_t current_vehicle = MemoryManager::Read<DWORD>(0xBA18FC);

	for (int i = 0; i < maxNum; i++)
	{
		DWORD vehicle_addr = vehicle_list + (i * 0xA18);
		if (vehicle_addr == 0 || vehicle_addr == current_vehicle)
		{
			continue;
		}

		CVehicle vehicle = MemoryManager::Read<CVehicle>(vehicle_addr);

		//short vehicle_id = MemoryManager::Read<short>(vehicle_addr + 34);
		short vehicle_id = vehicle.vehicle_id;

		if (vehicle_id < 300 || vehicle_id > 800)
			continue;
		

		//float vehicle_health = MemoryManager::Read<float>(vehicle_addr + 1216);
		float vehicle_health = vehicle.health;

		if (vehicle_health > 100000 || vehicle_health < -10000)
			continue;

		D3DXVECTOR3 vehicle_position = MemoryManager::Read<D3DXVECTOR3>( vehicle.addressToMatrix  + 0x30);

		if ((int)vehicle_position.x == 0 && (int)vehicle_position.y == 0 && (int)vehicle_position.z == 0)
			continue;

		D3DXVECTOR2 screen_position = WorldToScreen(vehicle_position);

		if (screen_position.x <= -10 || screen_position.x > Width + 10 || screen_position.y <= -10 || screen_position.y > Height + 10)
			continue;

		if (vehicleList.find(vehicle_id) != vehicleList.end())
		{
			std::string vehname = vehicleList[vehicle_id];
			std::string towriteonscr =  vehname;
			DrawString((char*)towriteonscr.c_str(), (int)screen_position.x, (int)screen_position.y, 0, 255, 0, pFontSmall);
		}
		else {
			std::string towriteonscr = "vehicle";
			DrawString((char*)towriteonscr.c_str(), (int)screen_position.x, (int)screen_position.y, 0, 255, 0, pFontSmall);
		}
	}
}

void ObjectWallhack()
{
	int32_t local_player = MemoryManager::Read<int32_t>(0xB6F5F0);
	D3DXVECTOR3 local_player_pos = MemoryManager::Read<D3DXVECTOR3>(MemoryManager::Read<DWORD>(local_player + 0x14) + 0x30);

	//char* object_list_temp = MemoryManager::ReadString(MemoryManager::Read<int32_t>(0xB7449C) + 0x0, 0xC);
	CGTAStructureInfo info = MemoryManager::Read<CGTAStructureInfo>(MemoryManager::Read<int32_t>(0xB7449C) + 0x0);
	//int32_t object_list = *(int32_t*)(object_list_temp + 0x0);
	int32_t object_list = info.address;

	//int maxNum = *(int32_t*)(object_list_temp + 0x8);
	int maxNum = info.max;

	if (maxNum > 10000)
		return;

	for (int i = 0; i < maxNum; i++)
	{
		int32_t object_addr = object_list + (i * 0x19C);
		if (object_addr == 0)
			continue;

		CObject object = MemoryManager::Read<CObject>(object_addr);
		
		//float health = MemoryManager::Read<float>(object + 0x154);

		//if (object.health <= 0)
		//	continue;
		
		//int object_id = MemoryManager::Read<int>(object + 0x13c);

		//if ((sizeof(WHObjectList) / sizeof(*WHObjectList)) - 1 < object.objectId)
		//	continue;
		//

		//if (!WHObjectList[object.objectId])
		//	continue;

		//int skin_id = MemoryManager::Read<short>(object + 0x22);

		if (WHObjectIDs)
		{
			//std::vector<bool> temp3 = { 0 };

			//for (int j = 0; j < objects.size(); j++)
			//{
			//	int id = atoi(objects[j].c_str());

			//	if (id != skin_id)
			//	{
			//		temp3.push_back(0);
			//	}
			//}

			//if (temp3.size() != objects.size())
			//{
			//	continue;
			//}
			if (std::find(objects.begin(), objects.end(), object.skinId) == objects.end())
				continue;

		}

		D3DXVECTOR3 object_pos = MemoryManager::Read<D3DXVECTOR3>(object.addressToPosition + 0x30);

		if ((int)object_pos.x == 0 && (int)object_pos.y == 0 && (int)object_pos.z == 0)
			continue;

		//if (removeObject)
		//	MemoryManager::WriteVirtualMemory((ULONG)MemoryManager::pID, PosStruct_ + 0x34, 0, 4);

		D3DXVECTOR2 screen_position = WorldToScreen(object_pos);

		if (screen_position.x <= -10 || screen_position.x > Width + 10 || screen_position.y <= -10 || screen_position.y > Height + 10)
			continue;

		int R = 255, G = 255, B = 255;

		std::string myBetch = "ID: " + std::to_string(object.objectId);

		R = 0;
		G = 255;
		B = 0;

		float theValuev2 = (object_pos - local_player_pos).sqrMagnitude();
		std::string myBetchv2 = "( " + std::to_string((int)theValuev2) + " m )";


		std::string myBetchv3 = "ID 2: " + std::to_string(object.skinId);

		DrawString((char*)myBetch.c_str(), screen_position.x, screen_position.y, R, G, B, pFontSmall);
		DrawString((char*)myBetchv2.c_str(), screen_position.x, screen_position.y + 14, R, G, B, pFontSmall);
		DrawString((char*)myBetchv3.c_str(), screen_position.x, screen_position.y + 28, R, G, B, pFontSmall);
	}
}


int gEnemy_addr = 0;

FLOAT GetDistance(const D3DXVECTOR3& From, const D3DXVECTOR3& To)
{
	float Angle[3] = { 0,0,0 };
	Angle[0] = To.x - From.x;
	Angle[1] = To.y - From.y;
	Angle[2] = To.z - From.z;

	return sqrtf(Angle[0] * Angle[0] + Angle[1] * Angle[1] + Angle[2] * Angle[2]);
}

int findTarget(int FOV)
{
	using namespace std;

	try {
		double closestDelta = 999999;
		double closestDelta2 = 999999;
		int closestPlayer = NULL;

		DWORD local_player = MemoryManager::Read<DWORD>(0xB6F5F0);
		if (!local_player)
		{
			return 0;
		}

		CGTAStructureInfo info = MemoryManager::Read<CGTAStructureInfo>(MemoryManager::Read<DWORD>(0xB74490) + 0x0);

		//DWORD PlayerList = MemoryManager::Read<DWORD>(MemoryManager::Read<DWORD>(0xB74490) + 0x0);
		DWORD PlayerList = info.address;

		//int maxNum = MemoryManager::Read<int>(MemoryManager::Read<DWORD>(0xB74490) + 0x8);
		int maxNum = info.max;

		if (maxNum > 10000)
		{
			return 0;
		}

		for (int i = 0; i < maxNum; i++)
		{

			DWORD player_addr = PlayerList + (i * 0x7C4);
			if (player_addr == 0)
				continue;

			CPed player = MemoryManager::Read<CPed>(player_addr);

			if (!IsPedPointerValid(player, player_addr, info) || player_addr == local_player)
				continue;

			//float player_hp = MemoryManager::Read<float>(player_addr + 0x540);
			int player_hp = (int)player.health;

			if (player_hp <= 0)
				continue;


			//DWORD_PTR matrix = MemoryManager::Read<DWORD_PTR>(player_addr + 0x14);
			D3DXVECTOR3 PlayerPos = MemoryManager::Read<D3DXVECTOR3>(player.matrix + 0x30);


			if ((int)PlayerPos.x != 0 && (int)PlayerPos.y != 0 && (int)PlayerPos.z != 0)
			{

				if (DontAimAtZombies)
				{
					//int skin_id = MemoryManager::Read<short>(player_addr + 0x22);
					int skin_id = player.skin_id;

					bool isZombie = false;

					//for (int j = 0; j < zombies.size(); j++)
					//{
					//	int id = atoi(zombies[j].c_str());

					//	if (id == skinID)
					//	{
					//		isZombie = true;
					//	}

					//}

					if (std::find(zombies.begin(), zombies.end(), skin_id) != zombies.end())
						isZombie = true;

					if (isZombie)
					{
						continue;
					}

				}


				D3DXVECTOR2 screen_position;

				float myDelta = 9999999;
				float myDelta2 = 9999999;

				CNetworkPlayer* adam = GetMTAPlayerbyPed(player_addr);
				if (adam != NULL)
				{

					D3DXVECTOR3 playerSpeed = MemoryManager::Read<D3DXVECTOR3>(player_addr + 0x44);
					//unsigned short latency1 = MemoryManager::Read<unsigned short>(adam + o_Latency);
					unsigned short latency1 = adam->latency;

					int latency = latency1;

					PlayerPos -= playerSpeed * latency * m_LatencyAdjusting;

					DWORD curVehicle = MemoryManager::Read<DWORD>(player_addr + 0x58C);

					if (curVehicle > 0)
					{
						D3DXVECTOR3 vehSpeed = MemoryManager::Read<D3DXVECTOR3>(curVehicle + 0x44);

						PlayerPos += vehSpeed * latency * m_VehicleLatencyAdjusting;
					}

					free(adam);
				}

				screen_position = WorldToScreen(PlayerPos);

				if (screen_position.x <= -10 || screen_position.x > Width + 10 || screen_position.y <= -10 || screen_position.y > Height + 10)
					continue;

				int weapon_id = MemoryManager::Read<int>(local_player + 0x740);

				myDelta = abs(screen_position.x - Width / 2);
				myDelta2 = abs(screen_position.y - Height / 2);

				if (weapon_id != 358)
				{
					myDelta = abs((screen_position.x - ((23.f / 800.f) * Width) - (Width / 2)));
					myDelta2 = abs(screen_position.y + ((79.f / 768.f) * Height) - (Height / 2));
				}
				else
				{
					myDelta = abs(screen_position.x - Width / 2);
					myDelta2 = abs(screen_position.y - Height / 2);
				}

				if (myDelta < closestDelta && myDelta2 < closestDelta2 && myDelta <= FOV && myDelta2 <= FOV) {
					closestDelta = myDelta;
					closestDelta2 = myDelta2;
					closestPlayer = player_addr;
				}

			}


		}

		if (closestDelta == 999999 || closestDelta < 0)
		{
			return 0;
		}

		return closestPlayer;
	}
	catch (...)
	{
		//std::cout << "line: " << /*__LINE__*/ " | " << ex.what() << std::endl;
		return 0;
	}
}



void MainCheat()
{
	using namespace std;

	if (CurrentAimMode == fovAim)
	{
		if (GetAsyncKeyState(aimKey) && AimEnabled && bigHake(thingx))
		{
			if (!gEnemy_addr)
			{
				gEnemy_addr = findTarget(FOV);
			}


			if (gEnemy_addr != 0)
			{
				CPed enemy_player = MemoryManager::Read<CPed>(gEnemy_addr);

				//float enemy_hp = MemoryManager::Read<float>(TheEnemy + 0x540);
				int enemy_hp = (int)enemy_player.health;


				if (enemy_hp <= 0 || !IsPedPointerValid(enemy_player, gEnemy_addr))
				{
					gEnemy_addr = 0;
					return;
				}

				CNetworkPlayer* MTAPlayer = GetMTAPlayerbyPed(gEnemy_addr);

				if (MTAPlayer != NULL/* && GetMTAPlayerByID(MTAPlayerId) != -1*/)
				{
					if (std::find(friendList.begin(), friendList.end(), MTAPlayer->mta_id) != friendList.end())
					{
						free(MTAPlayer);
						gEnemy_addr = 0;
						return;
					}
				}


				//float gameSensx = MemoryManager::Read<float>(0xB6EC1C);
				//float gameSensy = MemoryManager::Read<float>(0xB6EC18);
				
				DWORD local_player = MemoryManager::Read<DWORD>(0xB6F5F0);

				if (gEnemy_addr == local_player)
				{
					if(MTAPlayer)
						free(MTAPlayer);
					return;
				}

				if (DontAimAtZombies)
				{
					//int skin_id = MemoryManager::Read<short>(gEnemy_addr + 0x22);
					int skin_id = enemy_player.skin_id;

					bool isZombie = false;

					//for (int j = 0; j < zombies.size(); j++)
					//{
					//	int id = atoi(zombies[j].c_str());

					//	if (id == skinID)
					//	{
					//		isZombie = true;
					//	}

					//}



					if (isZombie)
					{
						if (MTAPlayer)
							free(MTAPlayer);
						gEnemy_addr = 0;
						return;
					}
				}


				int weapon_id = MemoryManager::Read<int>(local_player + 0x740);

				D3DXVECTOR3 PlayerPos;
				D3DXVECTOR2 delta;

				try
				{
					//if (WeaponID == 358)
					//{
					//	D3DXVECTOR3 x;
					//	PlayerPos = GetBonePositionReversed(TheEnemy, x, 4, 0);
					//}
					//else
					//{
					//	D3DXVECTOR3 x;
					//	PlayerPos = GetBonePositionReversed(TheEnemy, x, 6, 0);
					//}
					D3DXVECTOR3 x;
					PlayerPos = GetBonePositionReversed(gEnemy_addr, x, aimBotBone, 0);
				}
				catch (...)
				{

					//std::cout << "line: " << /*__LINE__*/ " | " << ex.what() << std::endl;
					if (MTAPlayer)
						free(MTAPlayer);
					gEnemy_addr = 0;
					return;
				}

				CNetworkPlayer* adam = MTAPlayer;
				if (adam != NULL)
				{

					//D3DXVECTOR3 playerSpeed = MemoryManager::ReadVirtualMemory<D3DXVECTOR3>(gEnemy_addr + 0x44);
					D3DXVECTOR3 playerSpeed = enemy_player.speed;
					//unsigned short latency1 = MemoryManager::Read<unsigned short>(adam + o_Latency);
					unsigned short latency1 = adam->latency;
					//unsigned short ping1 = MemoryManager::Read<unsigned short>(adam + o_Ping);
					unsigned short ping1 = adam->ping;


					int latency = latency1 + 125;
					int ping = ping1 + 125;

					//printf("LATENCY: %d\nPING: %d\n", latency, ping);

					PlayerPos -= playerSpeed * ping * m_LatencyAdjusting;

					//DWORD curVehicle = MemoryManager::Read<DWORD>(gEnemy_addr + 0x58C);
					DWORD curVehicle = enemy_player.current_vehicle;

					if (curVehicle > 0)
					{
						D3DXVECTOR3 vehSpeed = MemoryManager::Read<D3DXVECTOR3>(curVehicle + 0x44);

						PlayerPos += vehSpeed * ping * m_VehicleLatencyAdjusting;
					}

					free(adam);
				}

				delta = WorldToScreen(PlayerPos);

				if (delta.x <= -10 || delta.x > Width + 10 || delta.y <= -10 || delta.y > Height + 10)
					return;

				int CENTERX = Width / 2, CENTERY = Height / 2;


				int myDelta = abs((delta.x - ((23.f / 800.f) * Width) - (Width/2 )));
				int myDelta2 = abs(delta.y + ((79.f / 768.f) * Height) - (Height/2));

				if (weapon_id != 358)
				{
					myDelta = abs((delta.x - ((23.f / 800.f) * Width) - (Width/2)));
					myDelta2 = abs(delta.y + ((79.f / 768.f) * Height) - (Height/2));
				}
				else
				{
					myDelta = abs(delta.x - CENTERX);
					myDelta2 = abs(delta.y - CENTERY);
				}

				//float factorx = gameSensx * 1000.0f / Smooth;
				//float factory = gameSensy * 1000.0f / Smooth;

				float factorx = Smooth / 1000.0f;
				float factory = Smooth / 1000.0f;

				if (myDelta <= FOV && myDelta2 <= FOV)
				{
					if (weapon_id != 358)
					{
						int nx = roundf(delta.x - ((23.f / 800.f) * Width) - CENTERX); // + 41
						int ny = roundf(delta.y + ((79.f / 768.f) * Height) - CENTERY); // - 67

						nx *= factorx;
						ny *= factory;

						mouse_event(MOUSEEVENTF_MOVE, nx, ny, 0, 0);
					}
					else
					{
						int nx = roundf((delta.x - CENTERX) * factorx * 0.5F);
						int ny = roundf((delta.y - CENTERY) * factory * 0.5F);
						mouse_event(MOUSEEVENTF_MOVE, nx, ny, 0, 0);
					}
				}
				else
				{
					gEnemy_addr = 0;
				}
			}
			else
			{
				gEnemy_addr = 0;
			}

		}
		else
		{
			gEnemy_addr = 0;
		}
	}
	else if (CurrentAimMode == magnetAim)
	{
		if (GetAsyncKeyState(aimKey) && AimEnabled && bigHake(thingx))
		{
			DWORD local_player = MemoryManager::Read<DWORD>(0xB6F5F0);

			gEnemy_addr = MemoryManager::Read<DWORD>(local_player + 0x79C);

			if (gEnemy_addr != 0 && IsPedPointerValid(gEnemy_addr))
			{
				float enemy_hp = MemoryManager::Read<float>(gEnemy_addr + 0x540);

				if (enemy_hp < 1.0f)
				{
					gEnemy_addr = 0;
					return;
				}

				CNetworkPlayer* MTAPlayer = GetMTAPlayerbyPed(gEnemy_addr);

				if ( MTAPlayer != NULL/* && GetMTAPlayerByID(MTAPlayerId) != -1*/)
				{
					if (std::find(friendList.begin(), friendList.end(), MTAPlayer->mta_id) != friendList.end())
					{
						free(MTAPlayer);
						gEnemy_addr = 0;
						return;
					}
				}



				if (gEnemy_addr == local_player)
				{
					if (MTAPlayer)
						free(MTAPlayer);
					gEnemy_addr = 0;
					return;
				}

				if (DontAimAtZombies)
				{
					int skinID = MemoryManager::Read<short>(gEnemy_addr + 0x22);

					//vector<string> zombies = split(zombieIDs, ",");

					bool isZombie = false;

					//for (int j = 0; j < zombies.size(); j++)
					//{
					//	int id = atoi(zombies[j].c_str());

					//	if (id == skinID)
					//	{
					//		isZombie = true;
					//	}

					//}

					if (std::find(zombies.begin(), zombies.end(), skinID) != zombies.end())
						isZombie = true;

					if (isZombie)
					{
						if (MTAPlayer)
							free(MTAPlayer);
						gEnemy_addr = 0;
						return;
					}
				}


				int WeaponID = MemoryManager::Read<int>(local_player + 0x740);

				D3DXVECTOR3 PlayerPos;
				D3DXVECTOR2 delta;

				try
				{
					//if (WeaponID == 358)
					//{
					//	D3DXVECTOR3 x;
					//	PlayerPos = GetBonePositionReversed(TheEnemy, x, 4, 0);
					//}
					//else
					//{
					//	D3DXVECTOR3 x;
					//	PlayerPos = GetBonePositionReversed(TheEnemy, x, 6, 0);
					//}
					D3DXVECTOR3 x;
					PlayerPos = GetBonePositionReversed(gEnemy_addr, x, aimBotBone, 0);
				}
				catch (...)
				{

					//std::cout << "line: " << /*__LINE__*/ " | " << ex.what() << std::endl;
					if (MTAPlayer)
						free(MTAPlayer);
					gEnemy_addr = 0;
					return;
				}

				int CENTERX = Width / 2, CENTERY = Height / 2;

				delta = WorldToScreen(PlayerPos);

				if (delta.x <= -10 || delta.x > Width + 10 || delta.y <= -10 || delta.y > Height + 10)
				{
					if (MTAPlayer)
						free(MTAPlayer);

					return;
				}
				

				int myDelta = abs((delta.x - ((23.f / 800.f) * Width) - (CENTERX)));
				int myDelta2 = abs(delta.y + ((79.f / 768.f) * Height) - (CENTERY));

				if (WeaponID != 358)
				{
					myDelta = abs((delta.x - ((23.f / 800.f) * Width) - (CENTERX)));
					myDelta2 = abs(delta.y + ((79.f / 768.f) * Height) - (CENTERY));
				}
				else
				{
					myDelta = abs(delta.x - CENTERX);
					myDelta2 = abs(delta.y - CENTERY);
				}


				float factorx = Smooth / 1000.0f;
				float factory = Smooth / 1000.0f;

				if (WeaponID != 358)
				{
					int nx = roundf(delta.x - ((23.f / 800.f) * Width) - CENTERX); // + 41
					int ny = roundf(delta.y + ((79.f / 768.f) * Height) - CENTERY); // - 67

					nx *= factorx;
					ny *= factory;

					mouse_event(MOUSEEVENTF_MOVE, nx, ny, 0, 0);
				}
				else
				{
					int nx = roundf((delta.x - CENTERX) * factorx * 0.5F);
					int ny = roundf((delta.y - CENTERY) * factory * 0.5F);
					mouse_event(MOUSEEVENTF_MOVE, nx, ny, 0, 0);
				}

				if (MTAPlayer)
					free(MTAPlayer);
			}
			else
			{
				gEnemy_addr = 0;
			}

		}
	}


	//if (false) // ClientModuleName != ModuleName
	//{
	//	if (GetAsyncKeyState(friendAimKey) & 1)
	//	{
	//		if (bigHake(thingx))
	//		{
	//			DWORD theFriend = findTarget(FOV);

	//			if (theFriend > 0)
	//			{
	//				float PlayerHP = MemoryManager::Read<float>(theFriend + 0x540);

	//				if (PlayerHP < 1.0f || !IsPedPointerValid(theFriend))
	//				{
	//					return;
	//				}

	//				if (GetMTAPlayerbyPed(theFriend) == -1)
	//				{
	//					return;
	//				}

	//				int id = MemoryManager::Read<int>((GetMTAPlayerbyPed(theFriend)) + 0x5c);
	//				if (std::find(friendList.begin(), friendList.end(), id) != friendList.end())
	//				{
	//					friendList.erase(std::find(friendList.begin(), friendList.end(), id));
	//				}
	//				else
	//				{
	//					friendList.push_back(id);
	//				}
	//			}
	//		}
	//	}
	//}


	//float cam_z_axis = MemoryManager::Read<float>(0xB6F258);

	//int32_t local_player = MemoryManager::Read<int32_t>(0xB6F5F0);
	//if (local_player > 0)
	//{
	//	int32_t mta_player = GetMTAPlayerbyPed(local_player);

	//	if (mta_player > 0)
	//	{
	//		short latency = MemoryManager::Read<short>(mta_player + o_Latency);
	//		short ping = MemoryManager::Read<short>(mta_player + o_Ping);

	//		printf("latency: %d, ping: %d\n", latency, ping);
	//		/*for (int i = 0; i < 2000; i += 1)
	//		{
	//			float value = MemoryManager::Read<float>(mta_player + i);
	//			if (value > cam_z_axis - 0.5f && value < cam_z_axis + 0.5f)
	//				printf("offset: %X, value: %f\n", i, value);
	//		}*/

	//	}
	//}
}








int __cdecl sub_56E210(int a1)
{
	int v1; // eax

	v1 = a1;
	if (a1 < 0)
	{
		v1 = MemoryManager::Read<unsigned __int8>(0xB7CD74);
	}

	return MemoryManager::Read<int>(0xB7CD98 + 100 * v1);
}

bool IsPedPointerValid(DWORD a1)
{
	int index; // edx
	int v3; // eax

	//char* ped_list_temp = MemoryManager::ReadString(MemoryManager::Read<int32_t>(0xB74490) + 0x0, 0xC);
	CGTAStructureInfo info = MemoryManager::Read<CGTAStructureInfo>(MemoryManager::Read<int32_t>(0xB74490) + 0x0);

	//int32_t ped_list = *(int32_t*)(ped_list_temp + 0x0);
	//int32_t max_peds = *(int32_t*)(ped_list_temp + 0x8);
	//int32_t unknown = *(int32_t*)(ped_list_temp + 0x4);
	int32_t ped_list = info.address;
	int32_t max_peds = info.max;
	int32_t unknown = info.unknown;

	index = (a1 - ped_list) / 0x7C4;
	if (index < 0 || index >= max_peds || MemoryManager::Read<BYTE>(index + unknown) < 0)
	{
		//free(ped_list_temp);

		return 0;
	}

	//v3 = MemoryManager::Read<BYTE>(a1 + 0x46C);
	//if (v3 & 0x100 && MemoryManager::Read<DWORD>(a1 + 0x58C) > 0)
	//	return 1;

	if (MemoryManager::Read<DWORD>(a1 + 0x438) == a1)
	{
		//free(ped_list_temp);

		return 1;
	}


	if (MemoryManager::Read<DWORD>(a1 + 0xB0)/* || a1 == sub_56E210(-1)*/)
	{
		//free(ped_list_temp);

		return 1;
	}

	//free(ped_list_temp);


	return 0;
}

bool IsPedPointerValid(CPed ped, int32_t ped_addr, CGTAStructureInfo ped_list_temp)
{
	int index; // edx

	int32_t ped_list = ped_list_temp.address;
	int32_t max_peds = ped_list_temp.max;
	int32_t unknown = ped_list_temp.unknown;


	index = (ped_addr - ped_list) / 0x7C4;
	if (index < 0 || index >= max_peds || MemoryManager::Read<BYTE>(index + unknown) < 0)
	{
		return 0;
	}


	if (ped.unknown2 == ped_addr || ped.unknown1)
	{
		return 1;
	}


	return 0;
}

bool IsPedPointerValid(CPed ped, int32_t ped_addr)
{
	int index; // edx
	CGTAStructureInfo ped_list_temp = MemoryManager::Read<CGTAStructureInfo>(MemoryManager::Read<int32_t>(0xB74490) + 0x0);

	int32_t ped_list = ped_list_temp.address;
	int32_t max_peds = ped_list_temp.max;
	int32_t unknown = ped_list_temp.unknown;


	index = (ped_addr - ped_list) / 0x7C4;
	if (index < 0 || index >= max_peds || MemoryManager::Read<BYTE>(index + unknown) < 0)
	{
		return 0;
	}


	if (ped.unknown2 == ped_addr || ped.unknown1)
	{
		return 1;
	}


	return 0;
}

void CalcScreenCoors(D3DXVECTOR3 * vecWorld, D3DXVECTOR2 * vecScreen)
{
	/** C++-ifyed function 0x71DA00, formerly called by CHudSA::CalcScreenCoors **/

	// Get the static view matrix as D3DXMATRIX
	D3DXMATRIX m = MemoryManager::Read<D3DXMATRIX>(0xB6FA2C);


	// Get the static virtual screen (x,y)-sizes
	DWORD dwLenX = Width;
	DWORD dwLenY = Height;

	// Do a transformation
	vecScreen->x = vecWorld->z * m._31 + vecWorld->y * m._21 + vecWorld->x * m._11 + m._41;
	vecScreen->y = vecWorld->z * m._32 + vecWorld->y * m._22 + vecWorld->x * m._12 + m._42;
	float zAxis = vecWorld->z * m._33 + vecWorld->y * m._23 + vecWorld->x * m._13 + m._43;

	// Get the correct screen coordinates

	float fRecip = 1.0f / zAxis;
	vecScreen->x *= fRecip * (dwLenX);
	vecScreen->y *= fRecip * (dwLenY);

	if (zAxis <= 1.0f)
	{
		vecScreen->x = -999;
		vecScreen->y = -999;
	}
}

D3DXVECTOR2 WorldToScreen(D3DXVECTOR3 vecWorld)
{
	D3DXVECTOR2 screen_coord = D3DXVECTOR2(0, 0);

	CalcScreenCoors(&vecWorld, &screen_coord);

	return screen_coord;
}