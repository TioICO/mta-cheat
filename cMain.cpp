#include "hMain.h"

#include "hCheat.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"

#include <string>

#include "curl.h"

#include <cstring>
#include <algorithm>

//Module* MainModule = NULL;
Module* ClientModule = NULL;

int Width = 1360;
int Height = 768;

MARGINS Margin;

char* tModuleName = "proxy_sa.exe";

char lWindowName[256] = ".";
HWND hWnd;

char tWindowName[256] = "MTA: San Andreas"; /* tWindowName ? Target Window Name */
HWND tWnd;
RECT tSize;

MSG Message;

FILE _iob[] = { *stdin, *stdout, *stderr };
extern "C" FILE * __cdecl __iob_func(void) { return _iob; }

int Clamp(int value, int min, int max)
{
	if (value > max)
		return max;

	if (value < min)
		return min;

	return value;
}


#define INRANGE(x,a,b)  (x >= a && x <= b) 
#define getBits( x )    (INRANGE((x&(~0x20)),'A','F') ? ((x&(~0x20)) - 'A' + 0xa) : (INRANGE(x,'0','9') ? x - '0' : 0))
#define getByte( x )    (getBits(x[0]) << 4 | getBits(x[1]))

DWORD FindSignature2(const char* szSignature) {

	if (ClientModule == NULL)
		return 0;

	//MODULEENTRY32 modInfo = Manager.GetModule2(module_name);
	DWORD startAddress = ClientModule->baseAddr;
	DWORD endAddress = startAddress + 0xFFFFFF;

	printf("end Address: %X\n", endAddress);

	const char* pat = szSignature;
	DWORD firstMatch = 0;
	for (DWORD pCur = startAddress; pCur < endAddress; pCur++) {
		if (!*pat) {
			return firstMatch;
		}
		if (*(PBYTE)pat == '\?' || MemoryManager::Read<BYTE>(pCur) == getByte(pat)) {
			if (!firstMatch) {
				firstMatch = pCur;
			}
			if (!pat[2]) {
				return firstMatch;
			}
			if (*(PWORD)pat == '\?\?' || *(PBYTE)pat != '\?') {
				pat += 3;
			}
			else {
				pat += 2;    //one ?
			}
		}
		else {
			pat = szSignature;
			firstMatch = 0;
		}
	}
	return NULL;
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WinProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	if (MenuEnabled && ImGui_ImplWin32_WndProcHandler(hWnd, Message, wParam, lParam))
	{
		return true;
	}

	switch (Message)
	{	

	case WM_SIZE:
		if (p_Device != NULL && wParam != SIZE_MINIMIZED)
		{
			ImGui_ImplDX9_InvalidateDeviceObjects();
			ImGui_ImplDX9_CreateDeviceObjects();
			break;
		}
		

	case WM_PAINT:
		//DwmExtendFrameIntoClientArea(hWnd, &Margin);
		//Render();
		break;

	case WM_CREATE:
		DwmExtendFrameIntoClientArea(hWnd, &Margin);
		break;

	case WM_DESTROY:
		PostQuitMessage(1);
		return 0;

	default:
		return DefWindowProc(hWnd, Message, wParam, lParam);
		break;
	}
	return 0;
}


void Thread2()
{
	while (true)
	{
		if (bigHake(thingx))
			MainCheat();

		Sleep(1);
	}
}


void Thread3()
{
//	const char to_find[] = R"(
//--[[
//    SERVER AND CLIENT.
//)";
//
//	for (int i = ClientModule->baseAddr; i < ClientModule->baseAddr + 0xFFFFFFFFFF; i += 2048)
//	{
//		char* memory = MemoryManager::ReadString(i, 2048);
//
//		char* found = std::search(memory, memory + 2048, to_find, to_find + strlen(to_find));
//
//		if (found < memory + 2048)
//		{
//			int offset = found - memory;
//			printf("found at %X, string:\n%s\n", offset + i - ClientModule->baseAddr, found);
//			break;
//		}
//
//		free(memory);
//	}
//
//	printf("finished searching\n");
}


int last_time_checked = 0;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hSecInstance, LPSTR nCmdLine, INT nCmdShow)
{
	LPTSTR cmd = GetCommandLine();

	if (!strstr(cmd, "shx"))
		return -1;

	if (!FindWindow(0, tWindowName))
	{
		MessageBox(0, "Waiting for game...", "WAITING", 0);
	}

	while (!FindWindow(0, tWindowName))
		Sleep(100);

	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)SetWindowToTarget, 0, 0, 0);

	InitCheat();

	WNDCLASSEX wClass;
	wClass.cbClsExtra = NULL;
	wClass.cbSize = sizeof(WNDCLASSEX);
	wClass.cbWndExtra = NULL;
	wClass.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0));
	wClass.hCursor = LoadCursor(0, IDC_ARROW);
	wClass.hIcon = LoadIcon(0, IDI_APPLICATION);
	wClass.hIconSm = LoadIcon(0, IDI_APPLICATION);
	wClass.hInstance = hInstance;
	wClass.lpfnWndProc = WinProc;
	wClass.lpszClassName = lWindowName;
	wClass.lpszMenuName = lWindowName;
	wClass.style = CS_VREDRAW | CS_HREDRAW;

	if (!RegisterClassEx(&wClass))
	{
		MessageBox(0, "Failed, ERROR CODE: 100", "ERROR", 0);
		exit(1);
	}

	tWnd = FindWindow(0, tWindowName);

	//while (tWnd != GetForegroundWindow())
	//	Sleep(100);

	if (tWnd)
	{
		RECT desktop;
		const HWND hDesktop = GetDesktopWindow();
		GetWindowRect(hDesktop, &desktop);

		Width = Clamp(MemoryManager::Read<DWORD>(0xC17044), 0, desktop.right);
		Height = Clamp(MemoryManager::Read<DWORD>(0xC17048), 0, desktop.bottom);

		Margin = { 0, 0, Width, Height };
		hWnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED, lWindowName, lWindowName,  WS_POPUP, 1, 1, Width, Height, 0, 0, 0, 0);
		SetLayeredWindowAttributes(hWnd, 0, 1.0f, LWA_ALPHA);
		SetLayeredWindowAttributes(hWnd, 0, RGB(0, 0, 0), LWA_COLORKEY);
		ShowWindow( hWnd, SW_SHOW);
	}

	printf("Width: %d, Height: %d\n", Width, Height);

	DirectXInit(hWnd);

	InitMenu();

	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Thread2, 0, 0, 0);
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Thread3, 0, 0, 0);

	while(true)
	{
		//if (!FindWindow(0, "MTA: San Andreas"))
		//	return 0;

		Render();

		MenuEnabled ? SetWindowLong(hWnd, GWL_EXSTYLE, WS_EX_TOPMOST) : SetWindowLong(hWnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT);


		if(PeekMessage(&Message, hWnd, 0, 0, PM_REMOVE))
		{
			DispatchMessage(&Message);
			TranslateMessage(&Message);
		}

		Sleep(1);
	}
	return 0;
}

void InitMenu()
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	//io.DisplaySize = ImVec2(s_width, s_height);
	io.DisplaySize = ImVec2(-1.0f, -1.0f);
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	ImGui_ImplDX9_Init(hWnd, p_Device);
	ImGui_ImplDX9_CreateDeviceObjects();

	// Setup style
	ImGui::StyleColorsDark();
	printf("Menu initialized!\n");
}

void InitCheat()
{
	while (!MemoryManager::Attach(tModuleName, "not_shared"))
	{
		if (MemoryManager::m_PID == -1)
		{
			if (strcmp(tModuleName, "proxy_sa.exe") == 0)
			{
				tModuleName = "gta_sa.exe";
			}
			else {
				tModuleName = "proxy_sa.exe";
			}
		}

		Sleep(1000);
	}

	//MainModule = MemoryManager::GetModule(tModuleName);
	ClientModule = MemoryManager::GetModule("client.dll");

	if (ClientModule != NULL)
	{
		if (ClientModule->baseAddr == 0)
		{
			printf("Waiting for joining game...\n");
		}

		while (ClientModule->baseAddr == 0)
		{
			ClientModule = MemoryManager::GetModule("client.dll");
			Sleep(1000);
		}

		printf("client.dll base address: %X\n", ClientModule->baseAddr);

		dwClientType = FindSignature2("A3 ? ? ? ? 8B 41 48") + 1;

		dwClientType = MemoryManager::Read<DWORD>(dwClientType);

		dwClientType -= ClientModule->baseAddr;

		g_pClientGame = FindSignature2("A3 ? ? ? ? C7 45 ? ? ? ? ? 8B 01") + 1;

		g_pClientGame = MemoryManager::Read<DWORD>(g_pClientGame);

		g_pClientGame -= ClientModule->baseAddr;

		dwVehicleType = dwClientType + 0xC;
		dwColType = dwClientType + 0xC + 0x4;


		printf("dwClientType: %X\n", dwClientType);
		printf("dwVehicleType: %X\n", dwVehicleType);
		printf("dwColType: %X\n", dwColType);
		printf("g_pClientGame: %X\n", g_pClientGame);


		InitializeVehicleList();

		printf("Successfully Initialized!\n");
	}
	else {
		printf("ClientModule is NULL\n");
	}
}


void SetWindowToTarget()
{
	//while(true)
	//{
	//	tWnd = FindWindow(0, tWindowName);
	//	if (tWnd)
	//	{
	//		GetWindowRect(tWnd, &tSize);
	//		Width = tSize.right - tSize.left;
	//		Height = tSize.bottom - tSize.top;
	//		DWORD dwStyle = GetWindowLong(tWnd, GWL_STYLE);
	//		if(dwStyle & WS_BORDER)
	//		{
	//			tSize.top += 23;
	//			Height -= 23;
	//		}
	//		//MoveWindow(hWnd, tSize.left, tSize.top, Width, Height, true);
	//		SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, Width, Height, 0);
	//	}
	//	//else
	//	//{
	//	//	char ErrorMsg[125];
	//	//	sprintf(ErrorMsg, "Make sure %s is running!", tWindowName);
	//	//	MessageBox(0, ErrorMsg, "Error - Cannot find the game!", MB_OK | MB_ICONERROR);
	//	//	exit(1);
	//	//}
	//	Sleep(1000);
	//}
}