#include "VxVector.h"
#include "VxMatrix.h"
#include <cstdint>
#include <cstring>

// Retail VxMath: 0x2429E360/370/390, table initialized at 0x2429A4F1.
// Angles are 14-bit turns, not degrees. Initialization sets x87 PC=24;
// round the angular product to binary32 before evaluating the sine.
static constexpr unsigned SIN_TABLE_SIZE = 16384;
static float g_SinTable[SIN_TABLE_SIZE];

static bool InitializeTrigTablesOnce() {
    const float radiansPerUnit = 0.00038349520764313638f; // original 0x39c90fdb
    for (unsigned i = 0; i < SIN_TABLE_SIZE; ++i) {
        const float angle = static_cast<float>(i) * radiansPerUnit;
        g_SinTable[i] = static_cast<float>(std::sin(static_cast<double>(angle)));
    }
    return true;
}

void InitializeTables() {
    static const bool initialized = InitializeTrigTablesOnce();
    (void) initialized;
}

int radToAngle(float val) {
    // Original default PC=24 multiplication followed by _ftol (truncate
    // to int64, return its low DWORD). Explicit range checks avoid C++ UB
    // for NaN, infinity and values outside the signed 64-bit range.
    const float product = val * 2607.594482421875f; // original 0x4522f983
    const double wide = product;
    if (!(wide >= -9223372036854775808.0 && wide < 9223372036854775808.0))
        return 0;
    const uint32_t low = static_cast<uint32_t>(static_cast<int64_t>(wide));
    int32_t result;
    std::memcpy(&result, &low, sizeof(result));
    return result;
}

float Tsin(int angle) {
    InitializeTables();
    return g_SinTable[static_cast<unsigned>(angle) & (SIN_TABLE_SIZE - 1)];
}

float Tcos(int angle) {
    InitializeTables();
    return g_SinTable[(static_cast<unsigned>(angle) + SIN_TABLE_SIZE / 4) & (SIN_TABLE_SIZE - 1)];
}

const VxVector VxVector::m_AxisX(1.0f, 0.0f, 0.0f);
const VxVector VxVector::m_AxisY(0.0f, 1.0f, 0.0f);
const VxVector VxVector::m_AxisZ(0.0f, 0.0f, 1.0f);
const VxVector VxVector::m_Axis0(0.0f, 0.0f, 0.0f);
const VxVector VxVector::m_Axis1(1.0f, 1.0f, 1.0f);

VxVector &VxVector::operator=(const VxCompressedVector &v) {
    const int yaAngle = static_cast<int>(v.ya);
    const int xaAngle = static_cast<int>(v.xa);

    const float sinYa = Tsin(yaAngle);
    const float cosYa = Tcos(yaAngle);
    const float sinXa = Tsin(xaAngle);
    const float cosXa = Tcos(xaAngle);

    x = sinYa * cosXa;
    y = -sinXa;
    z = cosYa * cosXa;

    return *this;
}

const VxVector &VxVector::axisX() { return m_AxisX; }
const VxVector &VxVector::axisY() { return m_AxisY; }
const VxVector &VxVector::axisZ() { return m_AxisZ; }
const VxVector &VxVector::axis0() { return m_Axis0; }
const VxVector &VxVector::axis1() { return m_Axis1; }
