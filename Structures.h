#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <Windows.h>
#include "Offsets.h"

#include <d3dx9math.h>


#define STR_MERGE_IMPL(x, y)				x##y
#define STR_MERGE(x,y)						STR_MERGE_IMPL(x,y)
#define MAKE_PAD(size)						BYTE STR_MERGE(pad_, __COUNTER__) [ size ]

#define DEFINE_MEMBER_0(x, y)				x // y is for making it more readable
#define DEFINE_MEMBER_N(x,offset)			struct { MAKE_PAD(offset); x; }

class CMTAStructureInfo {
public:
	char pad_0x00[0xC];
	int max;
	int temp2;
};

class CMTAObjectStructureInfo {
public:
	char pad_0x00[0x2C];
	int max;
	int temp2;
};

class CMTAColStructureInfo {
public:
	int pointer_to_members;
	int unknown;
	int to_calculate_len;
};

class CGTAStructureInfo {
public:
	int address;
	int unknown;
	int max;
};

class CNetworkPlayer { // 0x2F0 health
public:


	char pad_0x00[0x5C];
	int mta_id; // 0x5C
	char pad_0x60[0xE8];
	D3DXVECTOR3 position; // 0x148
	char pad_0x154[0x50];
	int ped_addr_way; // 0x1A4
	char pad_0x1A8[0x148];
	float health; // 0x2F0
	//float                                    m_fArmor;
	//bool                                     m_bDead;
	//bool                                     m_bWorldIgnored;
	//float                                    m_fCurrentRotation;
	//float                                    m_fMoveSpeed;
	//bool                                     m_bCanBeKnockedOffBike;
	//float                                  m_Matrix[16];
	//float                                  m_vecMoveSpeed[3];
	//float                                  m_vecTurnSpeed[3];
	//int                              m_CurrentWeaponSlot;
	//int m_WeaponTypes[13];
	//unsigned short     m_usWeaponAmmo[13];
	char pad_0x2F4[0x2C4];
	int nickname_addr; // 0x5B8
	char pad_0x5BC[0xC];
	int nickname_len; // 0x5B8 + 0x10
	char pad_0x5CC[0x4];
	unsigned short ping; // 0x5D0
	char pad_0x5D2[0x36];
	unsigned short latency;

	int self_addr;
	//union {
	//	DEFINE_MEMBER_0(void* start, 0x0);
	//	DEFINE_MEMBER_N(int mta_id, 0x5C);
	//	DEFINE_MEMBER_N(D3DXVECTOR3 position, 0x148);
	//	DEFINE_MEMBER_N(int ped_addr_way, 0x1A4);
	//	DEFINE_MEMBER_N(int nickname_addr, 0x5B8);
	//	DEFINE_MEMBER_N(int nickname_len, 0x5B8 + 0x10);
	//	DEFINE_MEMBER_N(unsigned short ping, 0x5D0);
	//	DEFINE_MEMBER_N(unsigned short latency, 0x608);

	//	DEFINE_MEMBER_N(int self_addr, 0x608 + 0x2);
	//};
};

class CPed {
private:
	char pad_0x00[0x14];

public:
	int matrix;
	char pad_0x18[0xA];
	short skin_id; // 0x22
	char pad_0x24[0x20];
	D3DXVECTOR3 speed; // 0x44
	char pad_0x50[0x60];
	int unknown1; // 0xB0
	char pad_0xB4[0x384];
	int unknown2; // 0x438
	char pad_0x43C[0x104];
	float health; // 0x540
	char pad_0x544[0x48];
	int current_vehicle;
	//DEFINE_MEMBER_N(int weapon_id, 0x740);
};


class CObject {
public:
	char pad[0x14];
	DWORD addressToPosition; // 0x14
	char pad_0x18[0xA];
	short skinId; // 0x22
	char pad_0x24[0x118];
	int objectId; // 0x13C
	char pad_0x140[0x14];
	float health; // 0x154
};

class CVehicle {
public:
	char pad_0x00[20];
	DWORD addressToMatrix; // 0x14 20
	char pad_0x18[0xA];  // 24
	WORD vehicle_id; // 0x22
	char pad_0x24[0x49C];
	float health; // 0x4A0
};


#endif