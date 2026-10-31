#include "VxLegacyMatrix.h"
#include "VxLegacyMatrixMultiply.h"
#include <cstdio>
#include <cstdint>
#include <cstring>

#ifdef TEST_PHYSICS_MODE_SWITCH
extern "C" int TT_PhysicsSetSolverMode(int);
extern "C" int TT_PhysicsGetSolverMode();
#endif

static bool Select(bool precise) {
#ifdef TEST_PHYSICS_MODE_SWITCH
    return TT_PhysicsSetSolverMode(precise ? 1 : 0) == 1 &&
           TT_PhysicsGetSolverMode() == (precise ? 1 : 0);
#else
    VxSetLegacyEntityMatrixMultiply(precise ? LegacyNonAliasedSSEMatrixMultiply : nullptr);
    return true;
#endif
}

static bool ReadWords(FILE *file, float *values, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        unsigned char bytes[4];
        if (std::fread(bytes, 1, 4, file) != 4) return false;
        const std::uint32_t word = std::uint32_t(bytes[0]) |
            (std::uint32_t(bytes[1]) << 8) | (std::uint32_t(bytes[2]) << 16) |
            (std::uint32_t(bytes[3]) << 24);
        std::memcpy(values + i, &word, sizeof(word));
    }
    return true;
}

int main(int argc, char **argv) {
    if (argc != 3) return 2;
    FILE *inputs = std::fopen(argv[1], "rb");
    FILE *expected = std::fopen(argv[2], "rb");
    if (!inputs || !expected) return 3;
    size_t count = 0;
    bool ok = true;
    while (true) {
        const int first = std::fgetc(inputs);
        if (first == EOF) break;
        std::ungetc(first, inputs);
        float a[16], b[16], ref[16], result[16], originalA[16], originalB[16];
        if (!ReadWords(inputs, a, 16) || !ReadWords(inputs, b, 16) ||
            !ReadWords(expected, ref, 16)) { ok = false; break; }
        std::memcpy(originalA, a, sizeof(a));
        std::memcpy(originalB, b, sizeof(b));
        std::memcpy(result, ref, sizeof(result));
        // Disabled calls must not write even when they would produce a result.
        if (!Select(false) || VxTryLegacyEntityMatrixMultiply(result, a, b) ||
            std::memcmp(result, ref, sizeof(result))) { ok = false; break; }
        std::memset(result, 0xa5, sizeof(result));
        if (!Select(true) || !VxTryLegacyEntityMatrixMultiply(result, a, b) ||
            std::memcmp(result, ref, sizeof(result)) ||
            VxTryLegacyEntityMatrixMultiply(a, a, b) ||
            VxTryLegacyEntityMatrixMultiply(b, a, b) ||
            std::memcmp(a, originalA, sizeof(a)) ||
            std::memcmp(b, originalB, sizeof(b))) { ok = false; break; }
        if (!Select(false) || VxTryLegacyEntityMatrixMultiply(result, a, b) ||
            std::memcmp(result, ref, sizeof(result))) { ok = false; break; }
        ++count;
    }
    ok = Select(false) && ok && count == 1025 &&
         !std::ferror(inputs) && std::fgetc(expected) == EOF && !std::ferror(expected);
    std::fclose(inputs);
    std::fclose(expected);
    std::printf("legacy entity matrix cases=%zu pass=%d\n", count, ok);
    return ok ? 0 : 1;
}
