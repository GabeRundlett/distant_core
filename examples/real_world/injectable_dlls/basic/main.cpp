#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// MUST FIX LOADER FOR GLOBAL OBJECTS TO BEHAVE
#if 1
struct GlobalObject {
    GlobalObject() { MessageBox(0, "constructed", "Global", 0); }
    ~GlobalObject() { MessageBox(0, "destructed", "Global", 0); }
};

GlobalObject global_object;
#endif

BOOL APIENTRY DllMain(HMODULE, DWORD call_reason_id, LPVOID) {
    if (call_reason_id == DLL_PROCESS_ATTACH)
        MessageBox(0, "Injected!", "Success!", 0);
    return TRUE;
}
