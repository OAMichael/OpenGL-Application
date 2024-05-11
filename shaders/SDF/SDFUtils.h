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
const uint MAX_STEPS = 1000;


/*
 *  Minimum distance to object to be counted as intersection
 */
const float INTERSECT_DIST = 0.001f;


const uint AABB_MATERIAL_ID = 65536u;
const uint CLOUD_MATERIAL_ID = 65537u;


const uint CLOUD_MARCH_MAX_STEPS = 100;
const uint CLOUD_MARCH_MAX_SHADOW_STEPS = 8;
const float CLOUD_MARCH_STEP = 0.15;
const float CLOUD_MARCH_SHADOW_STEP = 0.125f;

const float DENSITY_COEFF = 0.2f;
const float SHADOW_COEFF = 0.375f;
const vec3 CLOUD_COLOR = vec3(0.15, 0.45, 1.1);
const float MAX_DISTANCE_IN_CLOUD = 1.0f;


const uint DIRECTIONAL_LIGHTS_COUNT = 1;
const uint POINT_LIGHS_COUNT = 1;

vec3 directionalLigthDirs[DIRECTIONAL_LIGHTS_COUNT] = {
    vec3(0.5f, 0.7f, 0.5f)
};

vec3 pointLightPositions[POINT_LIGHS_COUNT] = {
    vec3(1.0f, 5.0f, 25.0f)
};


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
        if (objDesc.dist > maxDist) {
            break;
        }
        if (dS.dist < INTERSECT_DIST) {
            if (dS.matID == AABB_MATERIAL_ID) {
                objDesc.dist += INTERSECT_DIST;
                continue;
            }
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
        if (objDesc.dist > maxDist) {
            break;
        }
        if (dS.dist < INTERSECT_DIST) {
            if (dS.matID == AABB_MATERIAL_ID) {
                objDesc.dist += INTERSECT_DIST;
                continue;
            }
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
    float occ = 0.0f;
    for (int i = 0; i < 8; i++) {
        float h = 0.01 + 0.4 * float(i) / 7.0;
        vec3  w = normalize(normal + normalize(sin(float(i) + vec3(0, 2, 4))));
        ObjectDesc obj = getSceneSDFMat(pos + h * w);
        if (obj.matID == AABB_MATERIAL_ID) {
            obj.dist = getSceneSDFAO(pos + h * w);
        }
        occ += h - obj.dist;
    }
    return clamp(1.0 - 0.71 * occ, 0.0, 1.0);
}



vec3 performBumpMap(vec3 position, vec3 normal, float e, float b, float r) {
    float ref = fbm6(r * position);
    vec3 gradient = b * (ref - vec3(fbm6(r * vec3(position.x + e, position.y, position.z)),
                                    fbm6(r * vec3(position.x, position.y + e, position.z)),
                                    fbm6(r * vec3(position.x, position.y, position.z + e)))) / e;

    vec3 tgrad = gradient - normal * dot(normal, gradient);
    return normalize(normal - tgrad);
}


float sdfRandomRepeatedSpheres(vec3 pos) {
    ivec3 i = ivec3(floor(pos));
    vec3 f = fract(pos);

    float sphere1 = sdfSphere(f, vec3(0, 0, 0), 0.5 * rand(i + vec3(0, 0, 0)));
    float sphere2 = sdfSphere(f, vec3(0, 0, 1), 0.5 * rand(i + vec3(0, 0, 1)));
    float sphere3 = sdfSphere(f, vec3(0, 1, 0), 0.5 * rand(i + vec3(0, 1, 0)));
    float sphere4 = sdfSphere(f, vec3(0, 1, 1), 0.5 * rand(i + vec3(0, 1, 1)));
    float sphere5 = sdfSphere(f, vec3(1, 0, 0), 0.5 * rand(i + vec3(1, 0, 0)));
    float sphere6 = sdfSphere(f, vec3(1, 0, 1), 0.5 * rand(i + vec3(1, 0, 1)));
    float sphere7 = sdfSphere(f, vec3(1, 1, 0), 0.5 * rand(i + vec3(1, 1, 0)));
    float sphere8 = sdfSphere(f, vec3(1, 1, 1), 0.5 * rand(i + vec3(1, 1, 1)));

    float random =  sdfOpUnion(sphere1,
                    sdfOpUnion(sphere2,
                    sdfOpUnion(sphere3,
                    sdfOpUnion(sphere4,
                    sdfOpUnion(sphere5,
                    sdfOpUnion(sphere6,
                    sdfOpUnion(sphere7, sphere8)))))));

    return random;
}


float sdfRandomTerrain(vec3 pos, float d) {
    float s = 1.0f;
    vec3 q = pos;

    mat3 m = mat3(0.00,  1.60,  1.20,
                 -1.60,  0.72, -0.96,
                 -1.20, -0.96,  1.28);

    for (int i = 0; i < 11; i++) {
        if (d > s * 0.866f)
            break;

        float n = s * sdfRandomRepeatedSpheres(q);

        n = smax(n, d - 0.1f * s, 0.3f * s);
        d = smin(n, d, 0.3f * s);

        q = m * q;
        s = 0.415 * s;
    }
    return d;
}


vec4 rayMarchingVolumetric(vec3 ro, vec3 rd) {
    vec4 sum = vec4(0.0f, 0.0f, 0.0f, 1.0f);

    vec3 pos = ro;
    for (int i = 0; i < CLOUD_MARCH_MAX_STEPS; i++) {
        if (sum.a < 0.1f) {
            break;
        }

        pos += rd * CLOUD_MARCH_STEP;

        float d = getSceneSDF(pos);
        if (d < -0.001f) {
            float shadow = 0.0f;
            vec3 lpos = pos + directionalLigthDirs[0] * CLOUD_MARCH_SHADOW_STEP;
            for (int s = 0; s < CLOUD_MARCH_MAX_SHADOW_STEPS; s++) {
                lpos += directionalLigthDirs[0] * CLOUD_MARCH_SHADOW_STEP;
                shadow += clamp(-getSceneSDF(lpos) / MAX_DISTANCE_IN_CLOUD, 0.0f, 1.0f);
            }

            d = clamp(-d / MAX_DISTANCE_IN_CLOUD, 0.0f, 1.0f);
            float density = clamp(d * DENSITY_COEFF, 0.0f, 1.0f);
            float s = exp(-shadow * SHADOW_COEFF);
            sum.rgb += vec3(s * density) * sum.a;
            sum.a *= 1.0f - density;

            sum.rgb += exp(d * 0.2f) * density * CLOUD_COLOR * sum.a;
        }
    }
    return sum;
}
