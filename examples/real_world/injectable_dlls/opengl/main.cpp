#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

BOOL APIENTRY DllMain(HMODULE, DWORD call_reason_id, LPVOID) {
    // TODO: write an OpenGL/ImGui sample to draw overlays
    // in order to use ImGui, I need to figure out how to
    // hook the processes input

    if (call_reason_id == DLL_PROCESS_ATTACH)
        MessageBox(0, "Injected!", "Success!", 0);
    return TRUE;
}
