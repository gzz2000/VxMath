/**
 * @file SIMDMatrixTest.cpp
 * @brief Tests for SIMD matrix operations.
 *
 * Tests inline SIMD operations defined in VxSIMD.h/VxSIMD.inl:
 * - VxSIMDMultiplyMatrixM, VxSIMDTransposeMatrixM
 * - VxSIMDMultiplyMatrixVector, VxSIMDMultiplyMatrixVector4
 * - VxSIMDRotateVector
 * - VxSIMDMatrixIdentity
 */

#include <gtest/gtest.h>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

#include "VxSIMD.h"
#include "SIMDTestCommon.h"

#if defined(VX_SIMD_SSE)

namespace {

using namespace SIMDTest;

class SIMDMatrixTest : public SIMDTest::SIMDTestBase {};

TEST_F(SIMDMatrixTest, MultiplyVector_MatchesOriginalTransformerCoordinates) {
    // Original DLL oracle, SR02 wood-to-paper activation. Bitwise equality
    // matters: these coordinates become the next physical ball's initial state.
    const uint32_t matrixBits[16] = {0x3f78ed3cu, 0xbe6f0c4eu, 0x322f4ea4u, 0x00000000u, 0x3e6f0c4eu, 0x3f78ed3cu, 0x30a5fde6u, 0x00000000u, 0xb22f4ea4u, 0x30a5fde6u, 0x3f800000u, 0x00000000u, 0xc463db4bu, 0x432ee7e0u, 0x4382c337u, 0x3f800000u};
    const uint32_t vectorBits[3] = {0x4467f800u, 0x423375d4u, 0xc3847297u};
    const uint32_t expected[3] = {0x3fa47600u, 0x3ff62780u, 0xc057b000u};
    VxMatrix matrix;
    VxVector input, output;
    std::memcpy(&matrix, matrixBits, sizeof(matrixBits));
    std::memcpy(&input, vectorBits, sizeof(vectorBits));
    Vx3DMultiplyMatrixVector(&output, matrix, &input);
    uint32_t actual[3];
    std::memcpy(actual, &output, sizeof(actual));
    for (int i=0; i<3; ++i) EXPECT_EQ(actual[i], expected[i]);
    Vx3DMultiplyMatrixVector(&input, matrix, &input);
    std::memcpy(actual, &input, sizeof(actual));
    for (int i=0; i<3; ++i) EXPECT_EQ(actual[i], expected[i]);
}


//=============================================================================
// Matrix Multiply Tests
//=============================================================================

TEST_F(SIMDMatrixTest, MultiplyMatrix_IdentityLeft) {
    VxMatrix identity;
    identity.SetIdentity();

    VxMatrix m = RandomTRSMatrix();
    VxMatrix result;

    VxSIMDMultiplyMatrixM(&result, &identity, &m);

    EXPECT_SIMD_MAT_NEAR(result, m, SIMD_SCALAR_TOL);
}

TEST_F(SIMDMatrixTest, MultiplyMatrix_IdentityRight) {
    VxMatrix identity;
    identity.SetIdentity();

    VxMatrix m = RandomTRSMatrix();
    VxMatrix result;

    VxSIMDMultiplyMatrixM(&result, &m, &identity);

    EXPECT_SIMD_MAT_NEAR(result, m, SIMD_SCALAR_TOL);
}

TEST_F(SIMDMatrixTest, MultiplyMatrix_MatchesScalar) {
    for (int i = 0; i < 50; ++i) {
        VxMatrix a = RandomTRSMatrix();
        VxMatrix b = RandomTRSMatrix();

        VxMatrix expected;
        Vx3DMultiplyMatrix(expected, a, b);

        VxMatrix result;
        VxSIMDMultiplyMatrixM(&result, &a, &b);

        EXPECT_SIMD_MAT_NEAR(result, expected, SIMD_SCALAR_TOL);
    }
}

//=============================================================================
// Matrix Transpose Tests
//=============================================================================

TEST_F(SIMDMatrixTest, TransposeMatrix_Identity) {
    VxMatrix identity;
    identity.SetIdentity();
    VxMatrix result;

    VxSIMDTransposeMatrixM(&result, &identity);

    EXPECT_SIMD_MAT_NEAR(result, identity, SIMD_EXACT_TOL);
}

TEST_F(SIMDMatrixTest, TransposeMatrix_InvolutoryProperty) {
    // Transpose of transpose should equal original
    VxMatrix m = RandomTRSMatrix();
    VxMatrix temp, result;

    VxSIMDTransposeMatrixM(&temp, &m);
    VxSIMDTransposeMatrixM(&result, &temp);

    EXPECT_SIMD_MAT_NEAR(result, m, SIMD_SCALAR_TOL);
}

TEST_F(SIMDMatrixTest, TransposeMatrix_MatchesScalar) {
    for (int i = 0; i < 50; ++i) {
        VxMatrix m = RandomTRSMatrix();

        VxMatrix expected;
        Vx3DTransposeMatrix(expected, m);

        VxMatrix result;
        VxSIMDTransposeMatrixM(&result, &m);

        EXPECT_SIMD_MAT_NEAR(result, expected, SIMD_SCALAR_TOL);
    }
}

//=============================================================================
// Matrix-Vector Multiply Tests
//=============================================================================

TEST_F(SIMDMatrixTest, MultiplyMatrixVector_Identity) {
    VxMatrix identity;
    identity.SetIdentity();
    VxVector v = RandomVector();
    VxVector result;

    VxSIMDMultiplyMatrixVector(&result, &identity, &v);

    EXPECT_SIMD_VEC3_NEAR(result, v, SIMD_SCALAR_TOL);
}

TEST_F(SIMDMatrixTest, MultiplyMatrixVector_Translation) {
    VxMatrix trans;
    trans.SetIdentity();
    trans[3][0] = 10.0f;
    trans[3][1] = 20.0f;
    trans[3][2] = 30.0f;

    VxVector v(1.0f, 2.0f, 3.0f);
    VxVector result;

    VxSIMDMultiplyMatrixVector(&result, &trans, &v);

    EXPECT_NEAR(result.x, 11.0f, SIMD_SCALAR_TOL);
    EXPECT_NEAR(result.y, 22.0f, SIMD_SCALAR_TOL);
    EXPECT_NEAR(result.z, 33.0f, SIMD_SCALAR_TOL);
}

TEST_F(SIMDMatrixTest, MultiplyMatrixVector_MatchesScalar) {
    for (int i = 0; i < 100; ++i) {
        VxMatrix m = RandomTRSMatrix();
        VxVector v = RandomVector();

        VxVector expected;
        Vx3DMultiplyMatrixVector(&expected, m, &v);

        VxVector result;
        VxSIMDMultiplyMatrixVector(&result, &m, &v);

        EXPECT_SIMD_VEC3_NEAR(result, expected, SIMD_SCALAR_TOL);
    }
}

TEST_F(SIMDMatrixTest, MultiplyMatrixVector4_MatchesScalar) {
    for (int i = 0; i < 100; ++i) {
        VxMatrix m = RandomTRSMatrix();
        VxVector4 v = RandomVector4();

        VxVector4 expected;
        Vx3DMultiplyMatrixVector4(&expected, m, &v);

        VxVector4 result;
        VxSIMDMultiplyMatrixVector4(&result, &m, &v);

        EXPECT_SIMD_VEC4_NEAR(result, expected, SIMD_SCALAR_TOL);
    }
}

//=============================================================================
// Rotate Vector Operation Tests
//=============================================================================

TEST_F(SIMDMatrixTest, RotateVectorOp_Identity) {
    VxMatrix identity;
    identity.SetIdentity();
    VxVector v = RandomVector();
    VxVector result;

    VxSIMDRotateVector(&result, &identity, &v);

    EXPECT_SIMD_VEC3_NEAR(result, v, SIMD_SCALAR_TOL);
}

TEST_F(SIMDMatrixTest, RotateVectorOp_IgnoresTranslation) {
    VxMatrix trans;
    trans.SetIdentity();
    trans[3][0] = 100.0f;
    trans[3][1] = 200.0f;
    trans[3][2] = 300.0f;

    VxVector v(1.0f, 2.0f, 3.0f);
    VxVector result;

    VxSIMDRotateVector(&result, &trans, &v);

    // Rotation ignores translation
    EXPECT_SIMD_VEC3_NEAR(result, v, SIMD_SCALAR_TOL);
}

TEST_F(SIMDMatrixTest, RotateVectorOp_PreservesLength) {
    for (int i = 0; i < 50; ++i) {
        VxMatrix rot = RandomRotationMatrix();
        VxVector v = RandomVector();

        float originalLen = v.Magnitude();

        VxVector result;
        VxSIMDRotateVector(&result, &rot, &v);

        float newLen = result.Magnitude();
        EXPECT_SIMD_NEAR(newLen, originalLen, SIMD_SCALAR_TOL);
    }
}

TEST_F(SIMDMatrixTest, RotateVectorOp_MatchesScalar) {
    for (int i = 0; i < 100; ++i) {
        VxMatrix m = RandomTRSMatrix();
        VxVector v = RandomVector();

        VxVector expected;
        Vx3DRotateVector(&expected, m, &v);

        VxVector result;
        VxSIMDRotateVector(&result, &m, &v);

        EXPECT_SIMD_VEC3_NEAR(result, expected, SIMD_SCALAR_TOL);
    }
}

//=============================================================================
// Matrix Identity Tests
//=============================================================================

TEST_F(SIMDMatrixTest, Identity_CreatesCorrectMatrix) {
    VxMatrix result;
    // Initialize with garbage
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result[i][j] = 99.0f;
        }
    }

    VxSIMDMatrixIdentity(&result);

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            float expected = (i == j) ? 1.0f : 0.0f;
            EXPECT_FLOAT_EQ(result[i][j], expected)
                << "Mismatch at [" << i << "][" << j << "]";
        }
    }
}

//=============================================================================
// Edge Cases
//=============================================================================

TEST_F(SIMDMatrixTest, StressTest_ChainMultiply) {
    VxMatrix result;
    result.SetIdentity();

    // Chain many small rotations
    VxVector axis(0.0f, 1.0f, 0.0f);
    float smallAngle = PI / 100.0f;

    for (int i = 0; i < 200; ++i) {
        VxMatrix rot;
        Vx3DMatrixFromRotation(rot, axis, smallAngle);

        VxMatrix temp;
        VxSIMDMultiplyMatrixM(&temp, &result, &rot);
        result = temp;
    }

    // Should have rotated 2*PI (full circle), back to identity-ish
    // Note: Due to accumulated error, we use loose tolerance
    VxVector testVec(1.0f, 0.0f, 0.0f);
    VxVector transformed;
    Vx3DRotateVector(&transformed, result, &testVec);

    // Should be close to original
    EXPECT_NEAR(transformed.x, testVec.x, 0.1f);
    EXPECT_NEAR(transformed.y, testVec.y, 0.1f);
    EXPECT_NEAR(transformed.z, testVec.z, 0.1f);
}

} // namespace

#endif // VX_SIMD_SSE
