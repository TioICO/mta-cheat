#ifndef HCHEAT_H
#define HCHEAT_H

#include "hMain.h"
#include "hDrawings.h"
#include "Structures.h"

extern bool MenuEnabled;

void MenuDraw();
void FriendlistMenu();

void PedWallhack();
void VehicleWallhack();
void ObjectWallhack();

void MainCheat();
void MiscCheat();

void MTAPedWallhack();

void MTAColWallhack();
void MTAObjectWallhack();

void MTAVehicleWallhack();

int __cdecl sub_56E210(int a1);

bool IsPedPointerValid(DWORD a1);
bool IsPedPointerValid(CPed ped, int32_t a1, CGTAStructureInfo temp);
bool IsPedPointerValid(CPed ped, int32_t a1);

void CalcScreenCoors(D3DXVECTOR3 * vecWorld, D3DXVECTOR2 * vecScreen);

D3DXVECTOR2 WorldToScreen(D3DXVECTOR3 vecWorld);


#endif