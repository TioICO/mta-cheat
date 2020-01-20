// some parts not shared

#pragma once

#include <Windows.h>
#include <cstdio>
#include <string>


class Module {
public:
	Module()
	{
		baseAddr = 0;
		size = 0;
	}

	Module(DWORD baseAd, DWORD sz)
	{
		baseAddr = baseAd;
		size = sz;
	}
	DWORD baseAddr;
	DWORD size;
};

class MemoryManager
{
public:
	static bool Attach(const char* app_name, LPCSTR RegistryPath);
	static Module* GetModule(const char* module_name);

	static bool ReadVirtualMemory(ULONG ReadAddress, void* buffer, UINT ReadSize)
	{
		SIZE_T bytesRead;
		ReadProcessMemory(hHandle, (LPCVOID)ReadAddress, buffer, ReadSize, &bytesRead);

		return bytesRead > 0;
	}

	static bool WriteVirtualMemory(ULONG WriteAddress,
		CHAR* WriteValue, SIZE_T WriteSize)
	{
		SIZE_T writtenBytes;
		WriteProcessMemory(hHandle, (LPVOID)WriteAddress, WriteValue, WriteSize, &writtenBytes);

		return writtenBytes > 0;
	}

	template <class T>
	static T Read(ULONG addr) {
		T _read;
		ReadVirtualMemory(addr, &_read, sizeof(T));
		return _read;
	}
	template <class T>
	static void Write(DWORD addr, T val) {
		WriteVirtualMemory(addr, (CHAR*)&val, sizeof(T));
	}


	static char* ReadString(DWORD addr, int size)
	{
		char* str = new char[size];
		ReadVirtualMemory(addr, str, size);

		return str;
	}


	static char* ReadStdString(DWORD addr)
	{
		int name_len = MemoryManager::Read<int>(addr + 0x10);

		name_len = ClampIntA(name_len, 0, 120);

		char* nName;


		if (name_len <= 15)
		{
			nName = (MemoryManager::ReadString(addr, name_len + 1));
		}
		else {
			nName = (MemoryManager::ReadString(MemoryManager::Read<int>(addr), name_len + 1));
		}

		return nName;
	}

	static byte* ReadBytes(DWORD addr, int size)
	{
		byte* bytes = new byte[size];
		ReadVirtualMemory(addr, bytes, size);

		return bytes;
	}


	static int m_PID;
	static HANDLE hDriver;
	static HANDLE hHandle;
};