#include "../GLSLversion.h"

#include "SDFCommon.h"
#include "SDFShapes.h"
#include "SDFOperations.h"
#include "SDFUtils.h"

//#define ANTIALIASING 2

uniform mat4 invCameraMatrix;
uniform float windowWidth;
uniform float windowHeight;
uniform float time;

out vec4 outColor;

const uint DIRECTIONAL_LIGHTS_COUNT = 3;
const uint POINT_LIGHS_COUNT = 1;

vec3 directionalLigthDirs[DIRECTIONAL_LIGHTS_COUNT] = {
    vec3(0.5f, 0.7f, 0.5f),
    vec3(-0.5f, 0.4f, 0.5f),
    vec3(-0.5f, 0.3f, -0.5f)
};

vec3 pointLightPositions[POINT_LIGHS_COUNT] = {
    vec3(-5.0f, 5.0f, 0.0f)
};


const uint INVALID_MAT_ID = 0;
const uint PLANE_MAT_ID = 1;
const uint HOUSE_CASING_MAT_ID = 2;
const uint HOUSE_ROOF_MAT_ID = 3;

struct Material {
    vec3 basecolor;
};

Material materialPool[] = {
    { vec3(0.0f) },     // Invalid
    { vec3(0.0f) },     // Special case
    { vec3(1.2f, 0.5f, 0.3f) },
    { vec3(0.2f, 0.6f, 0.3f) }
};

// Temporary define two functions for better performance
float getSceneSDF(vec3 pos) {
    float plane = sdfPlane(pos, vec3(0.0f, 1.0f, 0.0f), 0.0f);

    float outerRoundBox = sdfBox(pos, vec3(0.0f, 3.0f, 0.0f), vec3(1.0f, 6.0f, 5.0f));
    float innerBox = sdfBox(pos, vec3(0.0f, 1.9f, 0.0f), vec3(100000.0f, 4.2f, 3.2f));

    float arc = sdfOpDiff(outerRoundBox, innerBox);
    float cylinder = sdfCylinder(pos, vec3(0.0f, 3.4f, 0.0f), vec3(1.0f, 0.0f, 0.0f), 1.6f);
    arc = sdfOpDiff(arc, cylinder);

    float wall1 = sdfBox(pos, vec3(0.0f, 3.0f, 6.5f), vec3(1.0f, 6.0f, 8.0f));
    wall1 = sdfOpDiff(wall1, sdfBox(pos, vec3(0.0f, 3.4f, 7.0f), vec3(100000.0f, 3.0f, 3.0f)));

    float wall2 = sdfBox(pos, vec3(0.0f, 3.0f, -6.5f), vec3(1.0f, 6.0f, 8.0f));
    wall2 = sdfOpDiff(wall2, sdfBox(pos, vec3(0.0f, 3.4f, -7.0f), vec3(100000.0f, 3.0f, 3.0f)));

    float wall3 = sdfBox(pos, vec3(-5.0f, 3.0f, 10.0f), vec3(10.0f, 6.0f, 1.0f));
    float wall4 = sdfBox(pos, vec3(-5.0f, 3.0f, -10.0f), vec3(10.0f, 6.0f, 1.0f));
    float wall5 = sdfBox(pos, vec3(-10.5f, 3.0f, 0.0f), vec3(1.0f, 6.0f, 21.0f));
    
    float roof = sdfTriPrism(pos, vec3(-5.25f, 10.0f, 0.0f), vec2(8.0f, 12.0f));

    float house = sdfOpUnion(arc,
                    sdfOpUnion(wall1,
                        sdfOpUnion(wall2,
                            sdfOpUnion(wall3,
                                sdfOpUnion(wall4,
                                    sdfOpUnion(wall5, roof))))));

    float wholeSolidScene = sdfOpUnion(plane, house);
    return wholeSolidScene;
}

ObjectDesc getSceneSDFMat(vec3 pos) {
    float planeDist = sdfPlane(pos, vec3(0.0f, 1.0f, 0.0f), 0.0f);
    ObjectDesc plane = ObjectDesc(planeDist, PLANE_MAT_ID);

    float outerRoundBoxDist = sdfBox(pos, vec3(0.0f, 3.0f, 0.0f), vec3(1.0f, 6.0f, 5.0f));
    ObjectDesc outerRoundBox = ObjectDesc(outerRoundBoxDist, HOUSE_CASING_MAT_ID);

    float innerBoxDist = sdfBox(pos, vec3(0.0f, 1.9f, 0.0f), vec3(100000.0f, 4.2f, 3.2f));
    ObjectDesc innerBox = ObjectDesc(innerBoxDist, HOUSE_CASING_MAT_ID);

    ObjectDesc arc = sdfOpDiffMat(outerRoundBox, innerBox);
    
    float cylinderDist = sdfCylinder(pos, vec3(0.0f, 3.4f, 0.0f), vec3(1.0f, 0.0f, 0.0f), 1.6f);
    ObjectDesc cylinder = ObjectDesc(cylinderDist, HOUSE_CASING_MAT_ID);

    arc = sdfOpDiffMat(arc, cylinder);

    float wall1Dist = sdfBox(pos, vec3(0.0f, 3.0f, 6.5f), vec3(1.0f, 6.0f, 8.0f));
    ObjectDesc wall1 = ObjectDesc(wall1Dist, HOUSE_CASING_MAT_ID);

    float win1Dist = sdfBox(pos, vec3(0.0f, 3.4f, 7.0f), vec3(100000.0f, 3.0f, 3.0f));
    ObjectDesc win1 = ObjectDesc(win1Dist, HOUSE_CASING_MAT_ID);

    wall1 = sdfOpDiffMat(wall1, win1);

    float wall2Dist = sdfBox(pos, vec3(0.0f, 3.0f, -6.5f), vec3(1.0f, 6.0f, 8.0f));
    ObjectDesc wall2 = ObjectDesc(wall2Dist, HOUSE_CASING_MAT_ID);

    float win2Dist = sdfBox(pos, vec3(0.0f, 3.4f, -7.0f), vec3(100000.0f, 3.0f, 3.0f));
    ObjectDesc win2 = ObjectDesc(win2Dist, HOUSE_CASING_MAT_ID);

    wall2 = sdfOpDiffMat(wall2, win2);

    float wall3Dist = sdfBox(pos, vec3(-5.0f, 3.0f, 10.0f), vec3(10.0f, 6.0f, 1.0f));
    ObjectDesc wall3 = ObjectDesc(wall3Dist, HOUSE_CASING_MAT_ID);

    float wall4Dist = sdfBox(pos, vec3(-5.0f, 3.0f, -10.0f), vec3(10.0f, 6.0f, 1.0f));
    ObjectDesc wall4 = ObjectDesc(wall4Dist, HOUSE_CASING_MAT_ID);

    float wall5Dist = sdfBox(pos, vec3(-10.5f, 3.0f, 0.0f), vec3(1.0f, 6.0f, 21.0f));
    ObjectDesc wall5 = ObjectDesc(wall5Dist, HOUSE_CASING_MAT_ID);
    
    float roofDist = sdfTriPrism(pos, vec3(-5.25f, 10.0f, 0.0f), vec2(8.0f, 12.0f));
    ObjectDesc roof = ObjectDesc(roofDist, HOUSE_ROOF_MAT_ID);


    ObjectDesc house = sdfOpUnionMat(arc,
                            sdfOpUnionMat(wall1,
                                sdfOpUnionMat(wall2,
                                    sdfOpUnionMat(wall3,
                                        sdfOpUnionMat(wall4,
                                            sdfOpUnionMat(wall5, roof))))));

    ObjectDesc wholeSolidScene = sdfOpUnionMat(plane, house);
    return wholeSolidScene;
}


vec3 getPixelDirection(vec2 screenUV) {
    vec3 rayOrigin = (invCameraMatrix * vec4(screenUV - 0.5f, -1.0f, 1.0f)).xyz;
    vec3 camPos    = (invCameraMatrix * vec4(0, 0, 0, 1)).xyz;
    vec3 rayDir    = rayOrigin - camPos;
    return normalize(rayDir);
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
        vec3 col = vec3(0.5f, 0.8f, 0.9f) - max(rd.y, 0.0f) * 0.5f;
        vec2 uv = 1.5f * rd.xz / rd.y;
        float cl = sin(uv.x) + sin(uv.y);
        uv *= mat2(0.8f, 0.6f, -0.6f, 0.8f) * 2.1f;
        cl += 0.5f * (sin(uv.x) + sin(uv.y));
        col += 0.1f * (-1.0f + 2.0f * smoothstep(-0.1f, 0.1f, cl - 0.4f));
	    col = mix(col, vec3(0.5f, 0.7f, 0.9f), exp(-10.0f * max(rd.y, 0.0f)));
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
    vec3 normal = getSceneNormal(position);

    Material objMaterial = materialPool[hitObj.matID];

    vec3 basecolor = objMaterial.basecolor;
    if (hitObj.matID == PLANE_MAT_ID) {
        if ((int(floor(position.x)) + int(floor(position.z))) % 2 == 0) {
            basecolor = vec3(0.0f);
        }
        else {
            basecolor = vec3(2.5f);
        }
    }

    vec3 color = vec3(0.0f);

    for (uint i = 0; i < DIRECTIONAL_LIGHTS_COUNT; i++) {
        // Phong model
        vec3 lightDir = normalize(directionalLigthDirs[i]);
        vec3 diffuse = basecolor * max(0.0f, dot(lightDir, normal));

        float specularStrength = 0.5;
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(-rd, reflectDir), 0.0), 256);
        vec3 specular = specularStrength * spec * vec3(10.0f);

        // Shadows
        vec3 lightro = position;
        vec3 lightrd = lightDir;
        float lightStartDist = 0.01f;
        float lightDist = 100.0f;
        float shadow = sceneShadow(lightro, lightrd, lightStartDist, lightDist);

        color += 0.8f * diffuse * shadow + 0.2f * specular;
    }

    for (uint i = 0; i < POINT_LIGHS_COUNT; i++) {
        // Phong model
        vec3 lightDir = normalize(pointLightPositions[i] - position);
        vec3 diffuse = basecolor * max(0.0f, dot(lightDir, normal));

        float specularStrength = 0.5;
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(-rd, reflectDir), 0.0), 256);
        vec3 specular = specularStrength * spec * vec3(10.0f);

        // Shadows
        vec3 lightro = position;
        vec3 lightrd = lightDir;
        float lightStartDist = 0.01f;
        float lightDist = length(pointLightPositions[i] - position);
        float shadow = sceneShadow(lightro, lightrd, lightStartDist, lightDist);

        diffuse /= lightDist + 0.01f;
        specular /= lightDist + 0.01f;
        color += 0.8f * diffuse * shadow + 0.2f * specular;
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