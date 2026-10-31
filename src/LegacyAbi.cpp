#include "VxMatrix.h"

#if defined(_WIN32)
#include <windows.h>
#include <cstdio>

namespace {
using RetailFromMatrix = void (__thiscall *)(
    VxQuaternion *, const VxMatrix *, int, int);

RetailFromMatrix GetRetailFromMatrix() {
    static RetailFromMatrix function = [] {
        if (!GetEnvironmentVariableA("BALLANCE_USE_RETAIL_FROM_MATRIX", nullptr, 0))
            return static_cast<RetailFromMatrix>(nullptr);
        HMODULE module = LoadLibraryA("VxMathRetail.dll");
        if (!module) {
            if (FILE *log = std::fopen("vxlegacy.log", "ab")) {
                std::fprintf(log, "load failed error=%lu\n", GetLastError());
                std::fclose(log);
            }
            return static_cast<RetailFromMatrix>(nullptr);
        }
        using ModifyFeatures = void (__cdecl *)(unsigned long, unsigned long);
        auto modifyFeatures = reinterpret_cast<ModifyFeatures>(GetProcAddress(
            module, "?ModifyProcessorFeatures@@YAXKK@Z"));
        if (modifyFeatures)
            modifyFeatures(0, 0x02000000UL);
        auto result = reinterpret_cast<RetailFromMatrix>(GetProcAddress(
            module, "?FromMatrix@VxQuaternion@@QAEXABVVxMatrix@@HH@Z"));
        if (FILE *log = std::fopen("vxlegacy.log", "ab")) {
            std::fprintf(log, "module=%p function=%p modify=%p\n",
                         module, result, modifyFeatures);
            std::fclose(log);
        }
        return result;
    }();
    return function;
}
}

// A free __fastcall wrapper has the same x86 register/stack layout as the old
// member __thiscall here: ECX is `this`, EDX is ignored, and the three real
// arguments remain on the stack.  The DEF file exposes it under the original
// VC6 member-function spelling.
extern "C" __declspec(dllexport) void __fastcall VxQuaternionFromMatrixLegacy(
    VxQuaternion *self, void *, const VxMatrix *matrix,
    int matrixIsUnit, int restoreMatrix) {
    if (RetailFromMatrix retail = GetRetailFromMatrix()) {
        retail(self, matrix, matrixIsUnit, restoreMatrix);
        static unsigned calls = 0;
        if (calls++ < 16) {
            if (FILE *log = std::fopen("vxlegacy.log", "ab")) {
                std::fprintf(log, "call=%u unit=%d restore=%d q=(%a,%a,%a,%a)\n",
                             calls, matrixIsUnit, restoreMatrix,
                             self->x, self->y, self->z, self->w);
                std::fclose(log);
            }
        }
        return;
    }
    self->FromMatrix(*matrix, matrixIsUnit, restoreMatrix);
}
#endif
