// Synthetic original-DLL oracle and native cross-check (E3368).
#include <cstdio>
#include <cstdint>
#include <cstring>
#ifdef _WIN32
#include <windows.h>
#else
#include "VxVector.h"
#endif
static uint32_t bits(float f) { uint32_t u; std::memcpy(&u,&f,4); return u; }
int main(int argc,char **argv) {
#ifdef _WIN32
    if(argc!=2) return 2;
    HMODULE dll=LoadLibraryA(argv[1]); if(!dll) return 3;
    const auto base=reinterpret_cast<uintptr_t>(dll);
    auto radToAngle=reinterpret_cast<int(__cdecl*)(float)>(base+0x1e360);
    auto Tsin=reinterpret_cast<float(__cdecl*)(int)>(base+0x1e370);
    auto Tcos=reinterpret_cast<float(__cdecl*)(int)>(base+0x1e390);
#endif
    for(int a=-32768;a<32768;++a)
        std::printf("t %d %08x %08x\n",a,bits(Tsin(a)),bits(Tcos(a)));
    // Include exact binary32 quadrant boundaries and all exponent classes.
    uint32_t u=0x3fc90fdb;
    for(int i=0;i<10024;++i) {
        if(i<24) {
            const uint32_t fixed[]={0,0x80000000,0x3fc90fda,0x3fc90fdb,0x3fc90fdc,
             0xbfc90fdb,0x40490fdb,0x40c90fdb,0x7f800000,0xff800000,0x7fc00000,
             0x7f7fffff,0xff7fffff,0x3f800000,0xbf800000,1,0x80000001,0x49ffffff,
             0x4a000000,0xcaffffff,0x59800000,0xd9800000,0x5a000000,0xda000000};
            u=fixed[i];
        } else u=u*1664525u+1013904223u;
        float f;std::memcpy(&f,&u,4);
        std::printf("r %08x %08x\n",u,static_cast<uint32_t>(radToAngle(f)));
    }
    return std::ferror(stdout)?4:0;
}
