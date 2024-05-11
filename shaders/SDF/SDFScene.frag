#include "../GLSLversion.h"

#include "SDFCommon.h"
#include "SDFShapes.h"
#include "SDFOperations.h"
#include "SDFUtils.h"


// Settings and flags

//#define ANTIALIASING 2
#define USE_AABB


// Different scenes

#define TWIST_TORUS
//#define BEND_BOX
//#define SINGLE_CLOUD
//#define REPEATED_CLOUDS
//#define TEMPLE_AND_HOUSE
//#define TEMPLE_AND_LARGE_TERRAIN


uniform sampler2D uRockSampler;

uniform mat4 invCameraMatrix;
uniform float windowWidth;
uniform float windowHeight;
uniform float time;

out vec4 outColor;



const uint INVALID_MAT_ID = 0;
const uint PLANE_MAT_ID = 1;
const uint HOUSE_CASING_MAT_ID = 2;
const uint HOUSE_ROOF_MAT_ID = 3;
const uint HOUSE_WINDOW_MAT_ID = 4;
const uint TEMPLE_MAT_ID = 5;
const uint ROCK_MAT_ID = 6;
const uint TORUS_MAT_ID = 7;

struct Material {
    vec4 basecolor;
    bool opaque;
    float Kd;
    float Ks;
};

Material materialPool[] = {
    { vec4(0.0f), true, 1.0f, 0.0f },                           // Invalid
    { vec4(0.0f), true, 0.8f, 0.2f },                           // Special case
    { vec4(1.2f, 0.5f, 0.3f, 1.0f), true, 0.8f, 0.2f },         // House casing
    { vec4(0.2f, 0.1f, 0.1f, 1.0f), true, 0.95f, 0.05f },       // House roof
    { vec4(0.1f, 0.2f, 0.8f, 0.3f), false, 0.6f, 0.9f },        // Windows
    { vec4(0.76f, 0.7f, 0.5f, 1.0f), true, 1.0f, 0.0f },        // Temple
    { vec4(1.0f, 1.0f, 1.0f, 1.0f), true, 1.0f, 0.0f },         // Rocks
    { vec4(0.123f, 0.54f, 0.76f, 1.0f), true, 0.8f, 0.2f }      // Torus
};


vec4 rockTexture(vec2 uv) {
    return texture(uRockSampler, uv);
}


// Temporary define four functions for better performance

float SDF_house(vec3 pos) {
    float outerRoundBox = sdfBox(pos, vec3(5.0f, 3.0f, 0.0f), vec3(1.0f, 6.0f, 5.0f));
    float innerBox = sdfBox(pos, vec3(5.0f, 1.9f, 0.0f), vec3(100000.0f, 4.2f, 3.2f));

    float arc = sdfOpDiff(outerRoundBox, innerBox);
    float cylinder = sdfCylinder(pos, vec3(5.0f, 3.4f, 0.0f), vec3(1.0f, 0.0f, 0.0f), 1.6f);
    arc = sdfOpDiff(arc, cylinder);

    float wall1 = sdfBox(pos, vec3(5.0f, 3.0f, 6.5f), vec3(1.0f, 6.0f, 8.0f));
    wall1 = sdfOpDiff(wall1, sdfBox(pos, vec3(5.0f, 3.4f, 7.0f), vec3(100000.0f, 3.0f, 3.0f)));

    float glass1 = sdfBox(pos, vec3(5.0f, 3.4f, 7.0f), vec3(0.5f, 3.0f, 3.0f));
    wall1 = sdfOpUnion(wall1, glass1);

    float wall2 = sdfBox(pos, vec3(5.0f, 3.0f, -6.5f), vec3(1.0f, 6.0f, 8.0f));
    wall2 = sdfOpDiff(wall2, sdfBox(pos, vec3(5.0f, 3.4f, -7.0f), vec3(100000.0f, 3.0f, 3.0f)));

    float glass2 = sdfBox(pos, vec3(5.0f, 3.4f, -7.0f), vec3(0.5f, 3.0f, 3.0f));
    wall2 = sdfOpUnion(wall2, glass2);

    float wall3 = sdfBox(pos, vec3(0.0f, 3.0f, 10.0f), vec3(10.0f, 6.0f, 1.0f));
    float wall4 = sdfBox(pos, vec3(0.0f, 3.0f, -10.0f), vec3(10.0f, 6.0f, 1.0f));
    float wall5 = sdfBox(pos, vec3(-5.0f, 3.0f, 0.0f), vec3(1.0f, 6.0f, 21.0f));
    
    vec3 scale = vec3(3.7f, 1.0f, 1.0f);
    mat3 rotation = mat3(vec3(0, 0, 1), vec3(0, 1, 0), vec3(-1, 0, 0));
    float roof = sdfTriPrism(rotation * (pos / scale), vec3(0.0f, 7.0f, 0.0f), vec2(4.0f, 23.0f)) + 0.02 * sin(10 * pos.z);

    float house =   sdfOpUnion(arc,
                    sdfOpUnion(wall1,
                    sdfOpUnion(wall2,
                    sdfOpUnion(wall3,
                    sdfOpUnion(wall4,
                    sdfOpUnion(wall5, roof))))));

    return house;
}

ObjectDesc SDF_houseMatOpaque(vec3 pos) {
    float outerRoundBoxDist = sdfBox(pos, vec3(5.0f, 3.0f, 0.0f), vec3(1.0f, 6.0f, 5.0f));
    ObjectDesc outerRoundBox = ObjectDesc(outerRoundBoxDist, HOUSE_CASING_MAT_ID);

    float innerBoxDist = sdfBox(pos, vec3(5.0f, 1.9f, 0.0f), vec3(100000.0f, 4.2f, 3.2f));
    ObjectDesc innerBox = ObjectDesc(innerBoxDist, HOUSE_CASING_MAT_ID);

    ObjectDesc arc = sdfOpDiffMat(outerRoundBox, innerBox);

    float cylinderDist = sdfCylinder(pos, vec3(5.0f, 3.4f, 0.0f), vec3(1.0f, 0.0f, 0.0f), 1.6f);
    ObjectDesc cylinder = ObjectDesc(cylinderDist, HOUSE_CASING_MAT_ID);

    arc = sdfOpDiffMat(arc, cylinder);

    float wall1Dist = sdfBox(pos, vec3(5.0f, 3.0f, 6.5f), vec3(1.0f, 6.0f, 8.0f));
    ObjectDesc wall1 = ObjectDesc(wall1Dist, HOUSE_CASING_MAT_ID);

    float win1Dist = sdfBox(pos, vec3(5.0f, 3.4f, 7.0f), vec3(100000.0f, 3.0f, 3.0f));
    ObjectDesc win1 = ObjectDesc(win1Dist, HOUSE_CASING_MAT_ID);

    wall1 = sdfOpDiffMat(wall1, win1);

    float wall2Dist = sdfBox(pos, vec3(5.0f, 3.0f, -6.5f), vec3(1.0f, 6.0f, 8.0f));
    ObjectDesc wall2 = ObjectDesc(wall2Dist, HOUSE_CASING_MAT_ID);

    float win2Dist = sdfBox(pos, vec3(5.0f, 3.4f, -7.0f), vec3(100000.0f, 3.0f, 3.0f));
    ObjectDesc win2 = ObjectDesc(win2Dist, HOUSE_CASING_MAT_ID);

    wall2 = sdfOpDiffMat(wall2, win2);

    float wall3Dist = sdfBox(pos, vec3(0.0f, 3.0f, 10.0f), vec3(10.0f, 6.0f, 1.0f));
    ObjectDesc wall3 = ObjectDesc(wall3Dist, HOUSE_CASING_MAT_ID);

    float wall4Dist = sdfBox(pos, vec3(0.0f, 3.0f, -10.0f), vec3(10.0f, 6.0f, 1.0f));
    ObjectDesc wall4 = ObjectDesc(wall4Dist, HOUSE_CASING_MAT_ID);

    float wall5Dist = sdfBox(pos, vec3(-5.0f, 3.0f, 0.0f), vec3(1.0f, 6.0f, 21.0f));
    ObjectDesc wall5 = ObjectDesc(wall5Dist, HOUSE_CASING_MAT_ID);

    vec3 scale = vec3(3.7f, 1.0f, 1.0f);
    mat3 rotation = mat3(vec3(0, 0, 1), vec3(0, 1, 0), vec3(-1, 0, 0));
    float roofDist = sdfTriPrism(rotation * (pos / scale), vec3(0.0f, 7.0f, 0.0f), vec2(4.0f, 23.0f)) + 0.02 * sin(10 * pos.z);
    ObjectDesc roof = ObjectDesc(roofDist, HOUSE_ROOF_MAT_ID);

    ObjectDesc house =  sdfOpUnionMat(arc,
                        sdfOpUnionMat(wall1,
                        sdfOpUnionMat(wall2,
                        sdfOpUnionMat(wall3,
                        sdfOpUnionMat(wall4,
                        sdfOpUnionMat(wall5, roof))))));

    return house;
}

ObjectDesc SDF_houseMat(vec3 pos) {
    float outerRoundBoxDist = sdfBox(pos, vec3(5.0f, 3.0f, 0.0f), vec3(1.0f, 6.0f, 5.0f));
    ObjectDesc outerRoundBox = ObjectDesc(outerRoundBoxDist, HOUSE_CASING_MAT_ID);

    float innerBoxDist = sdfBox(pos, vec3(5.0f, 1.9f, 0.0f), vec3(100000.0f, 4.2f, 3.2f));
    ObjectDesc innerBox = ObjectDesc(innerBoxDist, HOUSE_CASING_MAT_ID);

    ObjectDesc arc = sdfOpDiffMat(outerRoundBox, innerBox);
    
    float cylinderDist = sdfCylinder(pos, vec3(5.0f, 3.4f, 0.0f), vec3(1.0f, 0.0f, 0.0f), 1.6f);
    ObjectDesc cylinder = ObjectDesc(cylinderDist, HOUSE_CASING_MAT_ID);

    arc = sdfOpDiffMat(arc, cylinder);

    float wall1Dist = sdfBox(pos, vec3(5.0f, 3.0f, 6.5f), vec3(1.0f, 6.0f, 8.0f));
    ObjectDesc wall1 = ObjectDesc(wall1Dist, HOUSE_CASING_MAT_ID);

    float win1Dist = sdfBox(pos, vec3(5.0f, 3.4f, 7.0f), vec3(100000.0f, 3.0f, 3.0f));
    ObjectDesc win1 = ObjectDesc(win1Dist, HOUSE_CASING_MAT_ID);

    wall1 = sdfOpDiffMat(wall1, win1);

    float glass1Dist = sdfBox(pos, vec3(5.0f, 3.4f, 7.0f), vec3(0.5f, 3.0f, 3.0f));
    ObjectDesc glass1 = ObjectDesc(glass1Dist, HOUSE_WINDOW_MAT_ID);

    wall1 = sdfOpUnionMat(wall1, glass1);

    float wall2Dist = sdfBox(pos, vec3(5.0f, 3.0f, -6.5f), vec3(1.0f, 6.0f, 8.0f));
    ObjectDesc wall2 = ObjectDesc(wall2Dist, HOUSE_CASING_MAT_ID);

    float win2Dist = sdfBox(pos, vec3(5.0f, 3.4f, -7.0f), vec3(100000.0f, 3.0f, 3.0f));
    ObjectDesc win2 = ObjectDesc(win2Dist, HOUSE_CASING_MAT_ID);

    wall2 = sdfOpDiffMat(wall2, win2);

    float glass2Dist = sdfBox(pos, vec3(5.0f, 3.4f, -7.0f), vec3(0.5f, 3.0f, 3.0f));
    ObjectDesc glass2 = ObjectDesc(glass2Dist, HOUSE_WINDOW_MAT_ID);

    wall2 = sdfOpUnionMat(wall2, glass2);

    float wall3Dist = sdfBox(pos, vec3(0.0f, 3.0f, 10.0f), vec3(10.0f, 6.0f, 1.0f));
    ObjectDesc wall3 = ObjectDesc(wall3Dist, HOUSE_CASING_MAT_ID);

    float wall4Dist = sdfBox(pos, vec3(0.0f, 3.0f, -10.0f), vec3(10.0f, 6.0f, 1.0f));
    ObjectDesc wall4 = ObjectDesc(wall4Dist, HOUSE_CASING_MAT_ID);

    float wall5Dist = sdfBox(pos, vec3(-5.0f, 3.0f, 0.0f), vec3(1.0f, 6.0f, 21.0f));
    ObjectDesc wall5 = ObjectDesc(wall5Dist, HOUSE_CASING_MAT_ID);
    
    vec3 scale = vec3(3.7f, 1.0f, 1.0f);
    mat3 rotation = mat3(vec3(0, 0, 1), vec3(0, 1, 0), vec3(-1, 0, 0));
    float roofDist = sdfTriPrism(rotation * (pos / scale), vec3(0.0f, 7.0f, 0.0f), vec2(4.0f, 23.0f)) + 0.02 * sin(10 * pos.z);
    ObjectDesc roof = ObjectDesc(roofDist, HOUSE_ROOF_MAT_ID);

    ObjectDesc house =  sdfOpUnionMat(arc,
                        sdfOpUnionMat(wall1,
                        sdfOpUnionMat(wall2,
                        sdfOpUnionMat(wall3,
                        sdfOpUnionMat(wall4,
                        sdfOpUnionMat(wall5, roof))))));

    return house;
}

float SDF_houseAABB(vec3 pos) {
    return sdfBox(pos, vec3(0.0f, 5.1f, 0.0f), vec3(13.2f, 10.2f, 23.02f) + 2 * INTERSECT_DIST);
}


float SDF_column(vec3 pos) {
    float base = sdfRoundBox(pos, vec3(0.0f, 0.05f, 0.0f), vec3(1.0f, 0.1f, 1.0f), 0.02f);
    float cyl1 = sdfCappedCone(pos, vec3(0.0f, 0.25f, 0.0f), 0.3f, 0.4f, 0.4f);
    float cone1 = sdfCappedCone(pos, vec3(0.0f, 0.5f, 0.0f), 0.2f, 0.4f, 0.3f);
    float cyl2 = sdfCappedCone(pos, vec3(0.0f, 2.6f, 0.0f), 4.0f, 0.27f, 0.27f);
    float cone2 = sdfCappedCone(pos, vec3(0.0f, 4.7f, 0.0f), 0.2f, 0.3f, 0.4f);

    cyl2 -= 0.05 * pow(0.2 + 0.2 * sin(atan(pos.x, pos.z) * 16.0), 2.0);

    float column =  sdfOpUnion(base,
                    sdfOpUnion(cyl1,
                    sdfOpUnion(cone1,
                    sdfOpUnion(cyl2, cone2))));

    return column;
}

float SDF_temple(vec3 pos) {
    float stair1 = sdfBox(pos, vec3(0.0f, 0.35f, 0.0f), vec3(14.0f, 0.7f, 9.5f));
    float stair2 = sdfBox(pos, vec3(0.0f, 1.05f, 0.0f), vec3(13.0f, 0.7f, 8.5f));
    float stair3 = sdfBox(pos, vec3(0.0f, 1.75f, 0.0f), vec3(12.0f, 0.7f, 7.5f));

    if (stair1 < 0.1) {
        stair1 -= 0.02 * smoothstep(0.5, 1.0, fbm4(pos.zxy));
        stair1 -= 0.01 * smoothstep(0.4, 0.8, fbm4(pos * 3.0));
        stair1 += 0.005;
    }

    if (stair2 < 0.1) {
        stair2 -= 0.02 * smoothstep(0.5, 1.0, fbm4(pos.zxy));
        stair2 -= 0.01 * smoothstep(0.4, 0.8, fbm4(pos * 3.0));
        stair2 += 0.005;
    }

    if (stair3 < 0.1) {
        stair3 -= 0.02 * smoothstep(0.5, 1.0, fbm4(pos.zxy));
        stair3 -= 0.01 * smoothstep(0.4, 0.8, fbm4(pos * 3.0));
        stair3 += 0.005;
    }

    vec3 leftBottomColumn = vec3(5.25f, 2.1f, 3.0f);
    vec3 rightUpperColumn = vec3(-5.25f, 2.1f, -3.0f);

    vec3 rep1 = sdfOpLimRepeatPos(pos - leftBottomColumn, vec3(1.5f, 0.0f, 0.0f), ivec3(8, 0, 0), ivec3(0));
    vec3 rep2 = sdfOpLimRepeatPos(pos - leftBottomColumn, vec3(0.0f, 0.0f, 1.5f), ivec3(0, 0, 5), ivec3(0));
    vec3 rep3 = sdfOpLimRepeatPos(pos - rightUpperColumn, vec3(1.5f, 0.0f, 0.0f), ivec3(0), ivec3(7, 0, 0));
    vec3 rep4 = sdfOpLimRepeatPos(pos - rightUpperColumn, vec3(0.0f, 0.0f, 1.5f), ivec3(0), ivec3(0, 0, 4));

    float column1 = SDF_column(rep1);
    float column2 = SDF_column(rep2);
    float column3 = SDF_column(rep3);
    float column4 = SDF_column(rep4);

    float colcap1 = sdfBox(rep1, vec3(0.0f, 4.85f, 0.0f), vec3(1.0f, 0.1f, 1.0f));
    float colcap2 = sdfBox(rep2, vec3(0.0f, 4.85f, 0.0f), vec3(1.0f, 0.1f, 1.0f));
    float colcap3 = sdfBox(rep3, vec3(0.0f, 4.85f, 0.0f), vec3(1.0f, 0.1f, 1.0f));
    float colcap4 = sdfBox(rep4, vec3(0.0f, 4.85f, 0.0f), vec3(1.0f, 0.1f, 1.0f));

    column1 = sdfOpUnion(column1, colcap1);
    column2 = sdfOpUnion(column2, colcap2);
    column3 = sdfOpUnion(column3, colcap3);
    column4 = sdfOpUnion(column4, colcap4);

    float roofrect1 = sdfBoxFrame(pos, vec3(0.0f, 7.45f, 0.0f), vec3(11.3f, 0.9f, 6.8f), 0.9f);
    float roofrect2 = sdfBoxFrame(pos, vec3(0.0f, 7.35f, 0.0f), vec3(11.4f, 0.1f, 6.9f), 0.1f);
    vec3 scale1 = vec3(1.0f, 1.0f, 8.2f);
    float roofprism1 = sdfTriPrism(pos / scale1, vec3(0.0f, 8.15f, 0.0f) / scale1, vec2(1.0f, 11.6f));

    // Front and back
    vec3 reporn1 = sdfOpLimRepeatPos(pos - vec3(5.58f, 7.65f, 3.375f), vec3(11.16f, 0.0f, 1.105f), ivec3(2, 0, 7), ivec3(0));
    vec3 reporn2 = sdfOpLimRepeatPos(pos - vec3(5.58f, 7.65f, 3.315f), vec3(11.16f, 0.0f, 1.105f), ivec3(2, 0, 7), ivec3(0));
    vec3 reporn3 = sdfOpLimRepeatPos(pos - vec3(5.58f, 7.65f, 3.255f), vec3(11.16f, 0.0f, 1.105f), ivec3(2, 0, 7), ivec3(0));

    float rooforn1 = sdfBox(reporn1, vec3(0.0f, 0.0f, 0.0f), vec3(0.2f, 0.5f, 0.05f));
    float rooforn2 = sdfBox(reporn2, vec3(0.0f, 0.0f, 0.0f), vec3(0.2f, 0.5f, 0.05f));
    float rooforn3 = sdfBox(reporn3, vec3(0.0f, 0.0f, 0.0f), vec3(0.2f, 0.5f, 0.05f));

    // Right and left
    vec3 reporn4 = sdfOpLimRepeatPos(pos - vec3(5.625f, 7.65f, 3.33f), vec3(1.113f, 0.0f, 6.66f), ivec3(11, 0, 2), ivec3(0));
    vec3 reporn5 = sdfOpLimRepeatPos(pos - vec3(5.565f, 7.65f, 3.33f), vec3(1.113f, 0.0f, 6.66f), ivec3(11, 0, 2), ivec3(0));
    vec3 reporn6 = sdfOpLimRepeatPos(pos - vec3(5.505f, 7.65f, 3.33f), vec3(1.113f, 0.0f, 6.66f), ivec3(11, 0, 2), ivec3(0));

    float rooforn4 = sdfBox(reporn4, vec3(0.0f, 0.0f, 0.0f), vec3(0.05f, 0.5f, 0.2f));
    float rooforn5 = sdfBox(reporn5, vec3(0.0f, 0.0f, 0.0f), vec3(0.05f, 0.5f, 0.2f));
    float rooforn6 = sdfBox(reporn6, vec3(0.0f, 0.0f, 0.0f), vec3(0.05f, 0.5f, 0.2f));

    float roof =    sdfOpUnion(rooforn1,
                    sdfOpUnion(rooforn2,
                    sdfOpUnion(rooforn3,
                    sdfOpUnion(rooforn4,
                    sdfOpUnion(rooforn5,
                    sdfOpUnion(rooforn6,
                    sdfOpUnion(roofrect1,
                    sdfOpUnion(roofrect2, roofprism1))))))));

    float temple =  sdfOpUnion(roof,
                    sdfOpUnion(stair1,
                    sdfOpUnion(stair2,
                    sdfOpUnion(stair3,
                    sdfOpUnion(column1,
                    sdfOpUnion(column2,
                    sdfOpUnion(column3, column4)))))));

    return temple;
}

float SDF_templeAABB(vec3 pos) {
    return sdfBox(pos, vec3(0.0f, 4.5f, 0.0f), vec3(14.1f, 9.0f, 9.6f) + 2 * INTERSECT_DIST);
}


float SDF_cloud(vec3 pos) {
    mat3 m = mat3( 0.00,  0.80,  0.60,
                  -0.80,  0.36, -0.48,
                  -0.60, -0.48,  0.64 );
	vec3 q = pos - vec3(0.0f, 0.5f, 1.0f) * time / 300.0f;
    float f = fbm4Mat(q, m);
    float d = sdfTorus(pos, vec2(6.0f, 0.005f)) - f * 3.5f - 1.0f;
	return d;
}



float getSceneSDFAO(vec3 pos) {



#ifdef BEND_BOX
    vec3 bent = sdfOpBend(pos, 0.05f * cos(time / 3000.0));
    float d = sdfRoundBox(bent, vec3(0.0f, 5.0f, 0.0f), vec3(10.0f, 1.0f, 4.0f), 0.1f);
    return d;
#endif



#ifdef TWIST_TORUS
    mat3 m = mat3(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f);
    vec3 twisted = sdfOpTwist(pos, 0.5f * cos(time / 3000.0));
    float d = sdfTorus(m * twisted, vec2(2.0f, 0.8f));
    return d;
#endif



#ifdef SINGLE_CLOUD
    float d = SDF_cloud(pos);
    return d;
#endif



#ifdef REPEATED_CLOUDS
    vec3 q = sdfOpLimRepeatPos(pos, vec3(25.0f), ivec3(2), ivec3(2));
    float d = SDF_cloud(q);
    return d;
#endif



#ifdef TEMPLE_AND_HOUSE
    float plane = sdfPlane(pos, vec3(0.0f, 1.0f, 0.0f), 0.0f);

    float house = 0.0f;
    float temple = 0.0f;

#ifdef USE_AABB
    house = SDF_houseAABB(pos - vec3(1.0f, 0.0f, 25.0f));
    if (house < 0.42f) {
        house = SDF_house(pos - vec3(1.0f, 0.0f, 25.0f));
    }

    temple = SDF_templeAABB(pos);
    if (temple < 0.42f) {
        temple = SDF_temple(pos);
    }
#else
    house = SDF_house(pos - vec3(1.0f, 0.0f, 25.0f));
    temple = SDF_temple(pos);
#endif

    float wholeSolidScene = sdfOpUnion(plane,
                            sdfOpUnion(house, temple));

    return wholeSolidScene;
#endif



#ifdef TEMPLE_AND_LARGE_TERRAIN
    float d = length(pos / vec3(10.0f) - vec3(0.0, -250.0, 0.0)) - 250.0;
    float fbm = sdfRandomTerrain(pos / vec3(10.0f), d);

    float temple = 0.0f;

#ifdef USE_AABB
    temple = SDF_templeAABB(pos);
    if (temple < 0.42f) {
        temple = SDF_temple(pos);
    }
#else
    temple = SDF_temple(pos);
#endif

    float wholeSolidScene = sdfOpUnion(fbm, temple);
    return wholeSolidScene;
#endif



}


float getSceneSDF(vec3 pos) {



#ifdef BEND_BOX
    vec3 bent = sdfOpBend(pos, 0.05f * cos(time / 3000.0));
    float d = sdfRoundBox(bent, vec3(0.0f, 5.0f, 0.0f), vec3(10.0f, 1.0f, 4.0f), 0.1f);
    return d;
#endif



#ifdef TWIST_TORUS
    mat3 m = mat3(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f);
    vec3 twisted = sdfOpTwist(pos, 0.5f * cos(time / 3000.0));
    float d = sdfTorus(m * twisted, vec2(2.0f, 0.8f));
    return d;
#endif



#ifdef SINGLE_CLOUD
    float d = SDF_cloud(pos);
    return d;
#endif



#ifdef REPEATED_CLOUDS
    vec3 q = sdfOpLimRepeatPos(pos, vec3(25.0f), ivec3(2), ivec3(2));
    float d = SDF_cloud(q);
    return d;
#endif



#ifdef TEMPLE_AND_HOUSE
    float plane = sdfPlane(pos, vec3(0.0f, 1.0f, 0.0f), 0.0f);

    float house = 0.0f;
    float temple = 0.0f;

#ifdef USE_AABB
    house = SDF_houseAABB(pos - vec3(1.0f, 0.0f, 25.0f));
    if (house < INTERSECT_DIST) {
        house = SDF_house(pos - vec3(1.0f, 0.0f, 25.0f));
    }

    temple = SDF_templeAABB(pos);
    if (temple < INTERSECT_DIST) {
        temple = SDF_temple(pos);
    }
#else
    house = SDF_house(pos - vec3(1.0f, 0.0f, 25.0f));
    temple = SDF_temple(pos);
#endif

    float wholeSolidScene = sdfOpUnion(plane,
                            sdfOpUnion(house, temple));

    return wholeSolidScene;
#endif



#ifdef TEMPLE_AND_LARGE_TERRAIN
    float d = length(pos / vec3(10.0f) - vec3(0.0, -250.0, 0.0)) - 250.0;
    float fbm = sdfRandomTerrain(pos / vec3(10.0f), d);

    float temple = 0.0f;

#ifdef USE_AABB
    temple = SDF_templeAABB(pos);
    if (temple < INTERSECT_DIST) {
        temple = SDF_temple(pos);
    }
#else
    temple = SDF_temple(pos);
#endif

    float wholeSolidScene = sdfOpUnion(fbm, temple);
    return wholeSolidScene;
#endif



}

ObjectDesc getSceneSDFMatOpaque(vec3 pos) {



#ifdef BEND_BOX
    vec3 bent = sdfOpBend(pos, 0.05f * cos(time / 3000.0));
    float d = sdfRoundBox(bent, vec3(0.0f, 5.0f, 0.0f), vec3(10.0f, 1.0f, 4.0f), 0.1f);
    return ObjectDesc(d, TORUS_MAT_ID);
#endif



#ifdef TWIST_TORUS
    mat3 m = mat3(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f);
    vec3 twisted = sdfOpTwist(pos, 0.5f * cos(time / 3000.0));
    float d = sdfTorus(m * twisted, vec2(2.0f, 0.8f));
    return ObjectDesc(d, TORUS_MAT_ID);
#endif



#ifdef SINGLE_CLOUD
    float d = SDF_cloud(pos);
    return ObjectDesc(d, CLOUD_MATERIAL_ID);
#endif



#ifdef REPEATED_CLOUDS
    vec3 q = sdfOpLimRepeatPos(pos, vec3(25.0f), ivec3(2), ivec3(2));
    float d = SDF_cloud(q);
    return ObjectDesc(d, CLOUD_MATERIAL_ID);
#endif



#ifdef TEMPLE_AND_HOUSE
    float planeDist = sdfPlane(pos, vec3(0.0f, 1.0f, 0.0f), 0.0f);
    ObjectDesc plane = ObjectDesc(planeDist, PLANE_MAT_ID);

    ObjectDesc house;
    ObjectDesc temple;

#ifdef USE_AABB
    float houseAABB = SDF_houseAABB(pos - vec3(1.0f, 0.0f, 25.0f));
    house = ObjectDesc(houseAABB, AABB_MATERIAL_ID);
    if (house.dist < 0) {
        house = SDF_houseMatOpaque(pos - vec3(1.0f, 0.0f, 25.0f));
    }

    float templeAABB = SDF_templeAABB(pos);
    temple = ObjectDesc(templeAABB, AABB_MATERIAL_ID);
    if (temple.dist < 0) {
        float templeDist = SDF_temple(pos);
        temple = ObjectDesc(templeDist, TEMPLE_MAT_ID);
    }
#else
    house = SDF_houseMatOpaque(pos - vec3(1.0f, 0.0f, 25.0f));

    float templeDist = SDF_temple(pos);
    temple = ObjectDesc(templeDist, TEMPLE_MAT_ID);
#endif

    ObjectDesc wholeSolidScene =    sdfOpUnionMat(plane,
                                    sdfOpUnionMat(house, temple));
    return wholeSolidScene;
#endif



#ifdef TEMPLE_AND_LARGE_TERRAIN
    float d = length(pos / vec3(10.0f) - vec3(0.0, -250.0, 0.0)) - 250.0;
    ObjectDesc fbm = ObjectDesc(sdfRandomTerrain(pos / vec3(10.0f), d), ROCK_MAT_ID);

    ObjectDesc temple;

#ifdef USE_AABB
    float templeAABB = SDF_templeAABB(pos);
    temple = ObjectDesc(templeAABB, AABB_MATERIAL_ID);
    if (temple.dist < INTERSECT_DIST) {
        float templeDist = SDF_temple(pos);
        temple = ObjectDesc(templeDist, TEMPLE_MAT_ID);
    }
#else
    float templeDist = SDF_temple(pos);
    temple = ObjectDesc(templeDist, TEMPLE_MAT_ID);
#endif

    ObjectDesc wholeSolidScene = sdfOpUnionMat(fbm, temple);
    return wholeSolidScene;
#endif



}

ObjectDesc getSceneSDFMat(vec3 pos) {



#ifdef BEND_BOX
    vec3 bent = sdfOpBend(pos, 0.05f * cos(time / 3000.0));
    float d = sdfRoundBox(bent, vec3(0.0f, 5.0f, 0.0f), vec3(10.0f, 1.0f, 4.0f), 0.1f);
    return ObjectDesc(d, TORUS_MAT_ID);
#endif



#ifdef TWIST_TORUS
    mat3 m = mat3(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f);
    vec3 twisted = sdfOpTwist(pos, 0.5f * cos(time / 3000.0));
    float d = sdfTorus(m * twisted, vec2(2.0f, 0.8f));
    return ObjectDesc(d, TORUS_MAT_ID);
#endif



#ifdef SINGLE_CLOUD
    float d = SDF_cloud(pos);
    return ObjectDesc(d, CLOUD_MATERIAL_ID);
#endif



#ifdef REPEATED_CLOUDS
    vec3 q = sdfOpLimRepeatPos(pos, vec3(25.0f), ivec3(2), ivec3(2));
    float d = SDF_cloud(q);
    return ObjectDesc(d, CLOUD_MATERIAL_ID);
#endif



#ifdef TEMPLE_AND_HOUSE
    float planeDist = sdfPlane(pos, vec3(0.0f, 1.0f, 0.0f), 0.0f);
    ObjectDesc plane = ObjectDesc(planeDist, PLANE_MAT_ID);

    ObjectDesc house;
    ObjectDesc temple;

#ifdef USE_AABB
    float houseAABB = SDF_houseAABB(pos - vec3(1.0f, 0.0f, 25.0f));
    house = ObjectDesc(houseAABB, AABB_MATERIAL_ID);
    if (house.dist < 0) {
        house = SDF_houseMat(pos - vec3(1.0f, 0.0f, 25.0f));
    }

    float templeAABB = SDF_templeAABB(pos);
    temple = ObjectDesc(templeAABB, AABB_MATERIAL_ID);
    if (temple.dist < 0) {
        float templeDist = SDF_temple(pos);
        temple = ObjectDesc(templeDist, TEMPLE_MAT_ID);
    }
#else
    house = SDF_houseMat(pos - vec3(1.0f, 0.0f, 25.0f));

    float templeDist = SDF_temple(pos);
    temple = ObjectDesc(templeDist, TEMPLE_MAT_ID);
#endif
    ObjectDesc wholeSolidScene =    sdfOpUnionMat(plane,
                                    sdfOpUnionMat(house, temple));
    return wholeSolidScene;
#endif



#ifdef TEMPLE_AND_LARGE_TERRAIN
    float d = length(pos / vec3(10.0f) - vec3(0.0, -250.0, 0.0)) - 250.0;
    ObjectDesc fbm = ObjectDesc(sdfRandomTerrain(pos / vec3(10.0f), d), ROCK_MAT_ID);

    ObjectDesc temple;

#ifdef USE_AABB
    float templeAABB = SDF_templeAABB(pos);
    temple = ObjectDesc(templeAABB, AABB_MATERIAL_ID);
    if (temple.dist < INTERSECT_DIST) {
        float templeDist = SDF_temple(pos);
        temple = ObjectDesc(templeDist, TEMPLE_MAT_ID);
    }
#else
    float templeDist = SDF_temple(pos);
    temple = ObjectDesc(templeDist, TEMPLE_MAT_ID);
#endif

    ObjectDesc wholeSolidScene = sdfOpUnionMat(fbm, temple);
    return wholeSolidScene;
#endif



}


vec3 getPixelDirection(vec2 screenUV) {
    vec3 rayOrigin = (invCameraMatrix * vec4(screenUV - 0.5f, -1.0f, 1.0f)).xyz;
    vec3 camPos    = (invCameraMatrix * vec4(0, 0, 0, 1)).xyz;
    vec3 rayDir    = rayOrigin - camPos;
    return normalize(rayDir);
}

vec3 getSkyColor(vec3 rd) {
    vec3 col = vec3(0.5f, 0.8f, 0.9f) - max(rd.y, 0.0f) * 0.5f;
    vec2 uv = 1.5f * rd.xz / rd.y;
    float cl = sin(uv.x) + sin(uv.y);
    uv *= mat2(0.8f, 0.6f, -0.6f, 0.8f) * 2.1f;
    cl += 0.5f * (sin(uv.x) + sin(uv.y));
    col += 0.1f * (-1.0f + 2.0f * smoothstep(-0.1f, 0.1f, cl - 0.4f));
    col = mix(col, vec3(0.5f, 0.7f, 4.1f), exp(-10.0f * max(rd.y, 0.0f)));

    return col;
}

vec3 getPlaneColor(vec3 pos) {
    if ((int(floor(pos.x)) + int(floor(pos.z))) % 2 == 0) {
        return vec3(0.0f);
    }
    return vec3(2.5f);
}


void main() {
    vec3 totalColor = vec3(0.0f);


#ifdef ANTIALIASING
    for (uint AAx = 0; AAx < ANTIALIASING; AAx++) {
    for (uint AAy = 0; AAy < ANTIALIASING; AAy++) {
    vec2 screenUV = (gl_FragCoord.xy - vec2(0.5f) + (vec2(0.5f) + vec2(AAx, AAy)) / ANTIALIASING) / vec2(windowWidth, windowWidth);
#else
    vec2 screenUV = gl_FragCoord.xy / vec2(windowWidth, windowWidth);
#endif
    vec3 ro = (invCameraMatrix * vec4(0, 0, 0, 1)).xyz;
    vec3 rd = getPixelDirection(screenUV);


    ObjectDesc hitObj = rayMarchingMat(ro, rd, 100.0f);
    float dist = hitObj.dist;
    if (dist > 100.0f) {
        vec3 col = getSkyColor(rd);
        col = pow(col, vec3(1.0/2.2));
#ifdef ANTIALIASING
        totalColor += col;
        continue;
#else
        outColor = vec4(col, 1.0f);
        return;
#endif
    }

    // Lighting
    vec3 position = ro + rd * dist;
    if (hitObj.matID == CLOUD_MATERIAL_ID) {
        vec4 cloud = rayMarchingVolumetric(position, rd);
        cloud.xyz = cloud.xyz + getSkyColor(rd) * cloud.a;
        cloud.xyz = pow(cloud.xyz, vec3(1.0/2.2));
#ifdef ANTIALIASING
        totalColor += cloud.xyz;
        continue;
#else
        outColor = vec4(cloud.xyz, 1.0f);
        return;
#endif
    }

    vec3 normal = getSceneNormal(position);

    if (hitObj.matID == TEMPLE_MAT_ID) {
        normal = performBumpMap(position, normal, 0.002f, 0.015f, 4.0f);
    }
    if (hitObj.matID == HOUSE_CASING_MAT_ID) {
        normal = performBumpMap(position, normal, 0.025f, 0.025f, 8.0f);
    }
    if (hitObj.matID == HOUSE_ROOF_MAT_ID) {
        normal = performBumpMap(position, normal, 0.002f, 0.015f, 4.0f);
    }

    Material objMaterial = materialPool[hitObj.matID];

    vec4 basecolor = objMaterial.basecolor;
    if (hitObj.matID == PLANE_MAT_ID) {
        basecolor.xyz = getPlaneColor(position);
    }

    if (hitObj.matID == ROCK_MAT_ID) {
        basecolor = 0.25f * rockTexture(position.xz);
    }

    if (!objMaterial.opaque) {
        float maxOpaqueDist = 100.0f - dist;
        ObjectDesc opaqueObj = rayMarchingMatOpaque(position, rd, maxOpaqueDist);
        if (opaqueObj.dist > maxOpaqueDist) {
            vec3 skyColor = getSkyColor(rd);
            basecolor = mix(basecolor, vec4(skyColor, 1.0f), basecolor.a);
        }
        else {
            vec3 opaquePosition = position + rd * opaqueObj.dist;
            Material opaqueMat = materialPool[opaqueObj.matID];
            vec4 opaqueBasecolor = opaqueMat.basecolor;
            if (opaqueObj.matID == PLANE_MAT_ID) {
                opaqueBasecolor.xyz = getPlaneColor(opaquePosition);
            }
            basecolor = mix(basecolor, opaqueBasecolor, basecolor.a);
        }
    }

    vec3 color = vec3(0.0f);

    for (uint i = 0; i < DIRECTIONAL_LIGHTS_COUNT; i++) {
        // Phong model
        vec3 lightDir = normalize(directionalLigthDirs[i]);
        vec3 diffuse = basecolor.xyz * max(0.0f, dot(lightDir, normal));

        float specularStrength = 0.5;
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(-rd, reflectDir), 0.0), 256);
        vec3 specular = specularStrength * spec * vec3(10.0f);

        // Shadows
        vec3 lightro = position;
        vec3 lightrd = lightDir;
        float lightStartDist = 0.1f;
        float lightDist = 100.0f;
        float shadow = sceneShadow(lightro, lightrd, lightStartDist, lightDist) + 0.15f;

        color += objMaterial.Kd * diffuse * shadow + objMaterial.Ks * specular;
    }

    for (uint i = 0; i < POINT_LIGHS_COUNT; i++) {
        // Phong model
        vec3 lightDir = normalize(pointLightPositions[i] - position);
        vec3 diffuse = basecolor.xyz * max(0.0f, dot(lightDir, normal));

        float specularStrength = 0.5;
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(-rd, reflectDir), 0.0), 256);
        vec3 specular = specularStrength * spec * vec3(10.0f);

        // Shadows
        vec3 lightro = position;
        vec3 lightrd = lightDir;
        float lightStartDist = 0.1f;
        float lightDist = length(pointLightPositions[i] - position);
        float shadow = sceneShadow(lightro, lightrd, lightStartDist, lightDist) + 0.15f;

        diffuse /= lightDist + 0.01f;
        specular /= lightDist + 0.01f;
        color += objMaterial.Kd * diffuse * shadow + objMaterial.Ks * specular;
    }

    vec3 ambient = vec3(0.05f);
    float ao = getSceneAO(position, normal);
    color += ambient * ao;

    color = pow(color, vec3(1.0/2.2));
    totalColor += color;
#ifdef ANTIALIASING
    }
    }
    totalColor /= float(ANTIALIASING * ANTIALIASING);
#endif
    outColor = vec4(totalColor, 1.0f);
}
