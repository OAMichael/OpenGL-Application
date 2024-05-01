/*
    Utility functions for working with
    Signed distance field scenes
*/

#include "SDFCommon.h"


/*
 *	Query scene normal with given precision
 *	Parameters:
 *		p - sample position
 *      eps - precision
 */
vec3 getSceneNormalPrecision(vec3 position, float eps) {
    vec2 e = vec2(eps, 0.0f);
    vec3 n = vec3(
        getSceneSDF(position + e.xyy) - getSceneSDF(position - e.xyy),
        getSceneSDF(position + e.yxy) - getSceneSDF(position - e.yxy),
        getSceneSDF(position + e.yyx) - getSceneSDF(position - e.yyx)
    );
    return normalize(n);
}


/*
 *	Query scene normal in relaxed way 
 *  with given precision
 *	Parameters:
 *		p - sample position
 *      eps - precision
 */
vec3 getSceneNormalRelaxedPrecision(vec3 position, float eps) {
    vec2 e = vec2(eps, 0.0f);
    vec3 n = vec3(
        getSceneSDF(position + e.xyy),
        getSceneSDF(position + e.yxy),
        getSceneSDF(position + e.yyx)
    );
    n = n - getSceneSDF(position);
    return normalize(n);
}


/*
 *	Query scene normal
 *	Parameters:
 *		p - sample position
 */
vec3 getSceneNormal(vec3 position) {
    return getSceneNormalPrecision(position, 0.001f);
}


/*
 *	Query scene normal in relaxed way
 *	Parameters:
 *		p - sample position
 */
vec3 getSceneNormalRelaxed(vec3 position) {
    return getSceneNormalRelaxedPrecision(position, 0.001f);
}


/*
 *  Maximum amount of steps to perform Ray Marching
 */
const uint MAX_STEPS = 10000;


/*
 *  Minimum distance to object to be counted as intersection
 */
const float INTERSECT_DIST = 0.001f;


/*
 *  Main function which performs Ray Marching
 *  Parameters:
 *      ro - ray origin
 *      rd - ray direction
 *      maxDist - maximum distance which ray can travel
 */
float rayMarching(vec3 ro, vec3 rd, float maxDist) {
    float d = 0.0f;
    for (uint i = 0; i < MAX_STEPS; i++) {
        vec3 pos = ro + rd * d;
        float dS = getSceneSDF(pos);
        d += dS;
        if (d > maxDist || dS < INTERSECT_DIST) {
            break;
        }
    }
    return d;
}


/*
 *  Main function which performs Ray Marching for objects with material
 *  Parameters:
 *      ro - ray origin
 *      rd - ray direction
 *      maxDist - maximum distance which ray can travel
 */
ObjectDesc rayMarchingMat(vec3 ro, vec3 rd, float maxDist) {
    ObjectDesc objDesc = ObjectDesc(0.0f, 0);
    ObjectDesc dS;
    for (uint i = 0; i < MAX_STEPS; i++) {
        vec3 pos = ro + rd * objDesc.dist;
        dS = getSceneSDFMat(pos);
        objDesc.dist += dS.dist;
        if (objDesc.dist > maxDist || dS.dist < INTERSECT_DIST) {
            break;
        }
    }
    objDesc.matID = dS.matID;
    return objDesc;
}


/*
 *  Main function which performs Ray Marching for opaque objects
 *  Parameters:
 *      ro - ray origin
 *      rd - ray direction
 *      maxDist - maximum distance which ray can travel
 */
ObjectDesc rayMarchingMatOpaque(vec3 ro, vec3 rd, float maxDist) {
    ObjectDesc objDesc = ObjectDesc(0.0f, 0);
    ObjectDesc dS;
    for (uint i = 0; i < MAX_STEPS; i++) {
        vec3 pos = ro + rd * objDesc.dist;
        dS = getSceneSDFMatOpaque(pos);
        objDesc.dist += dS.dist;
        if (objDesc.dist > maxDist || dS.dist < INTERSECT_DIST) {
            break;
        }
    }
    objDesc.matID = dS.matID;
    return objDesc;
}


/*
 *  Function to calculate hard shadow from light
 *  Parameters:
 *      ro - ray origin (world pixel position)
 *      rd - ray direction (direction to light source)
 *      startDist - start distance from point (to avoid immediate self intersection)
 *      maxDist - maximum distance which ray can travel
 */
float sceneShadow(vec3 ro, vec3 rd, float startDist, float maxDist) {
    float d = startDist;
    while (d < maxDist) {
        float h = getSceneSDF(ro + rd * d);
        if (h < INTERSECT_DIST)
            return 0.0f;
        d += h;
    }
    return 1.0f;
}


/*
 *  Function to calculate soft shadow from light
 *  Parameters:
 *      ro - ray origin (world pixel position)
 *      rd - ray direction (direction to light source)
 *      startDist - start distance from point (to avoid immediate self intersection)
 *      maxDist - maximum distance which ray can travel
 *      w - degree of softness
 */
float sceneSoftshadow(vec3 ro, vec3 rd, float startDist, float maxDist, float w) {
    float res = 1.0;
    float d = startDist;
    for (uint i = 0; d < maxDist; i++) {
        float h = getSceneSDF(ro + d * rd);
        if (h < INTERSECT_DIST)
            return 0.0f;
        res = min(res, h / (w * d));
        d += clamp(h, 0.005, 0.50);
        if (res < -1.0 || d > maxDist)
            break;
    }
    res = max(res, -1.0);
    return 0.25 * (1.0 + res) * (1.0 + res) * (2.0 - res);
}


/*
 *  Function to calculate ambient occlusion
 *  Parameters:
 *      pos - world position of the pixel
 *      normal - normal of the pixel
 */
float getSceneAO(vec3 pos, vec3 normal) {
    float occ = 0.0;
    for (int i = 0; i < 8; i++) {
        float h = 0.01 + 0.4 * float(i) / 7.0;
        vec3  w = normalize(normal + normalize(sin(float(i) + vec3(0, 2, 4))));
        float d = getSceneSDF(pos + h * w).x;
        occ += h - d;
    }
    return clamp(1.0 - 0.71 * occ, 0.0, 1.0);
}
