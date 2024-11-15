// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <Windows.h>
#include <string>
#include "injector/injector.hpp"
#include "ini.h"

#pragma comment(lib, "XInput9_1_0.lib")
#pragma comment(lib, "XInput.lib")
#include <xinput.h>

static auto eCreateLookAtMatrix = (int(__cdecl*)(void*, void*, void*, void*))0x006CF0A0;
void* HookAddr = (void*)0x00468B2F;

struct Vec3
{
    float x;
    float y;
    float z;
};

float CamYaw = 0.0f;
float CamPitch = 20.0f;
float MouseSensitivity = 0.2f;
float OrbitDistance = 5.0f;

POINT screenCenter;

void InitScreenCenter()
{
    RECT screenRect;
    GetClientRect(GetDesktopWindow(), &screenRect);
    screenCenter.x = (screenRect.right - screenRect.left) / 2;
    screenCenter.y = (screenRect.bottom - screenRect.top) / 2;
}

int __cdecl CreateLookAtMatrixHook(void* outMatrix, Vec3* from, Vec3* to, Vec3* up)
{
    // Get the current mouse position
    POINT mousePos;
    GetCursorPos(&mousePos);

    int deltaX = mousePos.x - screenCenter.x;
    int deltaY = mousePos.y - screenCenter.y;

    CamYaw += deltaX * MouseSensitivity;
    CamPitch += deltaY * MouseSensitivity;

    if (CamPitch > 80.0f) CamPitch = 80.0f;
    if (CamPitch < -80.0f) CamPitch = -80.0f;

    Vec3 offset;
    offset.x = OrbitDistance * cos(CamYaw * 0.0174533f) * cos(CamPitch * 0.0174533f);
    offset.y = OrbitDistance * sin(CamYaw * 0.0174533f) * cos(CamPitch * 0.0174533f);
    offset.z = OrbitDistance * sin(CamPitch * 0.0174533f);

    from->x = to->x + offset.x;
    from->y = to->y + offset.y;
    from->z = to->z + offset.z;

    SetCursorPos(screenCenter.x, screenCenter.y);
    return eCreateLookAtMatrix(outMatrix, from, to, up);
}

int tCamera;

void Main()
{
    mINI::INIFile file("MWGTACamera.ini");
    mINI::INIStructure ini;
    file.read(ini);

    std::string& gEnable = ini["Main"]["ToggleCamera"];
    tCamera = std::stoi(gEnable);

    if (tCamera == 1)
    {
        InitScreenCenter();
        SetCursorPos(screenCenter.x, screenCenter.y);
        injector::MakeCALL(HookAddr, CreateLookAtMatrixHook, true);
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        Main();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}