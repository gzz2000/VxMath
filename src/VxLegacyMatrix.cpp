#include "VxLegacyMatrix.h"
namespace {
VxLegacyMatrixBackend current{};
VxLegacyExtraBackend extra{};
VxLegacyOrientationNormalizer orientationNormalizer=nullptr;
VxLegacyVectorMagnitude vectorMagnitude=nullptr;
VxLegacyQuaternionFromMatrix quaternionFromMatrix=nullptr;
VxLegacyDynamicPosition dynamicPosition=nullptr;
VxLegacyEntityMatrixMultiply entityMatrixMultiply=nullptr;
std::uint64_t generation=0;
}
void VxSetLegacyMatrixBackend(const VxLegacyMatrixBackend *backend) {
    const VxLegacyMatrixBackend next=backend ? *backend : VxLegacyMatrixBackend{};
    if (current.inverse==next.inverse &&
        current.updateInverseTranslation==next.updateInverseTranslation) return;
    current=next;
    ++generation;
}
std::uint64_t VxGetLegacyMatrixGeneration() { return generation; }
bool VxTryLegacyInverse(float *out,const float *matrix) {
    if (!current.inverse) return false;
    current.inverse(out,matrix);
    return true;
}
bool VxTryLegacyInverseTranslation(float *inverse,const float *world) {
    if (!current.updateInverseTranslation) return false;
    current.updateInverseTranslation(inverse,world);
    return true;
}

void VxSetLegacyOrientationNormalizer(VxLegacyOrientationNormalizer normalize) {
    orientationNormalizer=normalize;
}
bool VxTryLegacyNormalizeOrientation(const float *input,float *output) {
    if (!orientationNormalizer) return false;
    orientationNormalizer(input,output);
    return true;
}

void VxSetLegacyVectorMagnitude(VxLegacyVectorMagnitude magnitude) {
    vectorMagnitude=magnitude;
}
bool VxTryLegacyVectorMagnitude(const float *input,float *output) {
    if (!vectorMagnitude) return false;
    *output=vectorMagnitude(input);
    return true;
}

void VxSetLegacyQuaternionFromMatrix(VxLegacyQuaternionFromMatrix convert) {
    quaternionFromMatrix=convert;
}
bool VxTryLegacyQuaternionFromMatrix(float *quaternion,const float *matrix,
                                    int matrixIsUnit,int restoreMatrix) {
    if (!quaternionFromMatrix) return false;
    quaternionFromMatrix(quaternion,matrix,matrixIsUnit,restoreMatrix);
    return true;
}

void VxSetLegacyDynamicPosition(VxLegacyDynamicPosition update) {
    dynamicPosition=update;
}
bool VxTryLegacyDynamicPosition(float *target,const float *object,
    const float *previous,const float *force,const float *damping,
    const float *offset,float deltaTime,float maximumDistance) {
    if (!dynamicPosition) return false;
    dynamicPosition(target,object,previous,force,damping,offset,deltaTime,maximumDistance);
    return true;
}

void VxSetLegacyEntityMatrixMultiply(VxLegacyEntityMatrixMultiply multiply) {
    entityMatrixMultiply=multiply;
}
bool VxTryLegacyEntityMatrixMultiply(float *out,const float *a,const float *b) {
    if (!entityMatrixMultiply || out==a || out==b) return false;
    entityMatrixMultiply(out,a,b);
    return true;
}

void VxSetLegacyExtraBackend(const VxLegacyExtraBackend *backend) {
    extra=backend ? *backend : VxLegacyExtraBackend{};
}
bool VxTryLegacyExtraNormalize(float *position) {
    if (!extra.normalizeOrbit) return false;
    extra.normalizeOrbit(position); return true;
}
bool VxTryLegacyExtraMove(float *out,const float *position,const float *previous,
    const float *target,float damping,float force,float timeValue,int flyaway) {
    if (!extra.move) return false;
    extra.move(out,position,previous,target,damping,force,timeValue,flyaway); return true;
}
bool VxTryLegacyExtraWithinDistance(const float *a,const float *b,
    float squaredThreshold,bool *within) {
    if (!extra.withinDistance) return false;
    *within=extra.withinDistance(a,b,squaredThreshold)!=0; return true;
}

bool VxTryLegacyExtraRotation(float *matrix,const float *axis,float angle) {
    if (!extra.rotationMatrix) return false;
    extra.rotationMatrix(matrix,axis,angle); return true;
}

bool VxTryLegacyExtraRotateVector(float *out,const float *matrix,const float *position) {
    if (!extra.rotateVector) return false;
    extra.rotateVector(out,matrix,position); return true;
}
