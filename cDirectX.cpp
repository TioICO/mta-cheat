#include "hDirectX.h"

#include "hCheat.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include <string>

IDirect3D9Ex* p_Object = 0;
IDirect3DDevice9Ex* p_Device = 0;
D3DPRESENT_PARAMETERS p_Params;

ID3DXLine* p_Line;
ID3DXFont* pFontSmall = 0;
ID3DXFont* pFontMedium = 0;

int DirectXInit(HWND hWnd)
{
	if(FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &p_Object)))
		exit(1);

	ZeroMemory(&p_Params, sizeof(p_Params));    
    p_Params.Windowed = TRUE;   
    p_Params.SwapEffect = D3DSWAPEFFECT_DISCARD;    
    p_Params.hDeviceWindow = hWnd;    
	p_Params.MultiSampleQuality   = D3DMULTISAMPLE_NONE;
    p_Params.BackBufferFormat = D3DFMT_A8R8G8B8 ;     
    p_Params.BackBufferWidth = Width;    
    p_Params.BackBufferHeight = Height;    
    p_Params.EnableAutoDepthStencil = TRUE;
    p_Params.AutoDepthStencilFormat = D3DFMT_D16;

	if(FAILED(p_Object->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &p_Params, 0, &p_Device)))
		exit(1);

	if(!p_Line)
		D3DXCreateLine(p_Device, &p_Line);
		//p_Line->SetAntialias(1); *removed cuz crosshair was blurred*

	D3DXCreateFont(p_Device, 14, 0, 100, 1, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Calibri", &pFontSmall);
	D3DXCreateFont(p_Device, 24, 0, 100, 1, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Calibri", &pFontMedium);

	return 0;
}

int last_time = 0;

int Render()
{
	if (p_Device != NULL)
	{
		p_Device->Clear(0, 0, D3DCLEAR_TARGET, 0, 1.0f, 0);
		p_Device->BeginScene();

		//int diff = (GetTickCount() - last_time);
		//if (diff == 0)
		//	diff = 1;
		//int fps = 1000.0f / diff;

		//last_time = GetTickCount();

		//DrawString((char*)(std::to_string(fps) + " FPS").c_str(), 5, 5, 255, 255, 255, pFontMedium);

		if(/*tWnd == GetForegroundWindow()*/ bigHake(thingx))
		{
			if (WHEnabled)
			{
				if (WHPeds)
					PedWallhack();

				if (WHCars)
					VehicleWallhack();

				if (WHObject)
					ObjectWallhack();

				if (GetAsyncKeyState(everyWHKey))
				{
					if (WHPeds)
						MTAPedWallhack();

					if (WHCars)
						MTAVehicleWallhack();

				}

				if (GetAsyncKeyState(EveryObjectWHKey))
				{
					if (EveryColliderWH)
						MTAColWallhack();

					if (EveryObjectWH)
						MTAObjectWallhack();
				}

				if (MapObjects && WHObjectIDs && GetAsyncKeyState(radarKey))
					MTAObjectWallhack();

			}


			MiscCheat();


		}

		MenuDraw();


		p_Device->EndScene();

		p_Device->PresentEx(0, 0, 0, 0, 0);
	}
	Sleep(1);
	return 0;
}
