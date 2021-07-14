// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include "base-dllmain.h"
#include "Logger.h"
#include "ScanMemory.h"

const std::string targetModule = "nioh2.exe";
const std::string outputFile = "Wrapper.log";

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    return BaseDllMain(hModule, ul_reason_for_call, lpReserved);
}

void RemoveSpecialEffectMaxConstraints();

void DoInjection() {
    RemoveSpecialEffectMaxConstraints();
}

void RemoveSpecialEffectMaxConstraints() {
    auto moduleAddress = static_cast<BYTE*>((void*) GetModuleHandle(targetModule.c_str()));
    auto targetAddress = moduleAddress + 0xF4E69B;
    auto injectAddress = moduleAddress + 0x450; // Hijack a section that's all 0s.

    auto targetBytes = std::vector<BYTE> {
        0xE8, 0xB0, 0x1D, 0x0B, 0xFF, // call nioh2.exe+450
        0x90 // nop (Since we're replacing code that's 6 bytes.)
    };

    auto injectBytes = std::vector<BYTE> {
        0x66, 0x83, 0x7B, 0x04, 0x00, // cmp word ptr [rbx+04],00
        0x0F, 0x84, 0x03, 0x00, 0x00, 0x00, // je +3 (Jumps to the copied original code.)
        0x38, 0xC0, // cmp al,al
        0xC3, // ret
        // Original code:
        0x89, 0x43, 0x04, // mov [rbx+04],eax
        0x45, 0x84, 0xF6, // test r14l,r14l
        0xC3 // ret
    };

    VirtualProtect(targetAddress, targetBytes.size(), PAGE_EXECUTE_READWRITE);

    // Replace original code with a `call` to out own.
    memcpy((BYTE*) targetAddress, targetBytes.data(), targetBytes.size());

    VirtualProtect(injectAddress, injectBytes.size(), PAGE_EXECUTE_READWRITE);

    // Inject the code we're calling.
    memcpy((BYTE*) injectAddress, injectBytes.data(), injectBytes.size());

    Log("Remove Special Effect Max Constraints (target address): 0x" << (void*) targetAddress);
    Log("Remove Special Effect Max Constraints (inject address): 0x" << (void*) injectAddress);
    Log("Remove Special Effect Max Constraints patched.");
}