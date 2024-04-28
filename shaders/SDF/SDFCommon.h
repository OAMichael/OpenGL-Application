/*
    Common structures and functions for SDF
*/

#ifndef SDF_COMMON_H
#define SDF_COMMON_H

struct ObjectDesc {
    float dist;
    uint matID;
};

float getSceneSDF(vec3 pos);
ObjectDesc getSceneSDFMat(vec3 pos);

#endif