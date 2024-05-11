/*
    Mathematical functions for shapes
    produced by signed distance fields
*/

#include "SDFCommon.h"


/// ======================== Regular signed distance functions operations ======================== ///


/*
 *  Union of two shapes (symmetric): (A or B)
 *  Parameters:
 *      d1 - distance to the first shape A
 *      d2 - distance to the second shape B
 */
float sdfOpUnion(float d1, float d2) {
    return min(d1, d2);
}


/*
 *  Intersection of two shapes (symmetric): (A and B)
 *  Parameters:
 *      d1 - distance to the first shape A
 *      d2 - distance to the second shape B
 */
float sdfOpIntersect(float d1, float d2) {
    return max(d1, d2);
}


/*
 *  Difference of two shapes (nonsymmetric): (A \ B)
 *  Parameters:
 *      d1 - distance to the first shape A
 *      d2 - distance to the second shape B
 */
float sdfOpDiff(float d1, float d2) {
    return max(d1, -d2);
}


/*
 *  Symmetric difference of two shapes (symmetric): (A or B) \ (A and B)
 *  Parameters:
 *      d1 - distance to the first shape A
 *      d2 - distance to the second shape B
 */
float sdfOpSymDiff(float d1, float d2) {
    return max(min(d1, d2), -max(d1, d2));
}


/*
 *  Complement of a shape: (not A)
 *  Parameters:
 *      d - distance to the shape A
 */
float sdfOpCompl(float d) {
    return -d;
}


/*
 *  Infinite repetition of a shape
 *  Produces position vector corresponding to infinite repetition
 *  Parameters:
 *      p - input position
 *      s - stride along each axis
 */
vec3 sdfOpInfRepeatPos(vec3 p, vec3 s) {
    vec3 q = p - s * round(p / s);
    return q;
}


/*
 *  Limited repetition of a shape
 *  Produces position vector corresponding to limited repetition
 *  Parameters:
 *      p - input position
 *      s - stride along each axis
 *      l - repetition count in negative axis directions
 *      h - repetition count in positive axis directions
 */
vec3 sdfOpLimRepeatPos(vec3 p, vec3 s, ivec3 l, ivec3 h) {
    vec3 q = p - s * clamp(round(p / s), min(-l + 1, ivec3(0)), max(ivec3(0), h - 1));
    return q;
}


/*
 *  Twist an object
 *  Produces position vector which performs twist of an object along Y axis
 *  Parameters:
 *      p - input position
 *      t - twist amount
 */
vec3 sdfOpTwist(vec3 p, float t) {
    float c = cos(t * p.y);
    float s = sin(t * p.y);
    vec2 q = mat2(c, -s, s, c) * p.xz;
    return vec3(q.x, p.y, q.y);
}



vec3 sdfOpBend(vec3 p, float b) {
    float c = cos(b * p.x);
    float s = sin(b * p.x);
    vec2 q = mat2(c, -s, s, c) * p.xy;
    return vec3(q, p.z);
}



/// ==================== Signed distance functions operations with materials ===================== ///


/*
 *  Union of two shapes (symmetric): (A or B)
 *  Parameters:
 *      o1 - object descrition for A
 *      o2 - object descrition for B
 */
ObjectDesc sdfOpUnionMat(ObjectDesc o1, ObjectDesc o2) {
    if (o1.dist < o2.dist) {
        return o1;
    }
    return o2;
}


/*
 *  Intersection of two shapes (symmetric): (A and B)
 *  Parameters:
 *      o1 - object descrition for A
 *      o2 - object descrition for B
 */
ObjectDesc sdfOpIntersectMat(ObjectDesc o1, ObjectDesc o2) {
    if (o1.dist > o2.dist) {
        return o1;
    }
    return o2;
}


/*
 *  Difference of two shapes (nonsymmetric): (A \ B)
 *  Parameters:
 *      o1 - object descrition for A
 *      o2 - object descrition for B
 */
ObjectDesc sdfOpDiffMat(ObjectDesc o1, ObjectDesc o2) {
    if (o1.dist > -o2.dist) {
        return o1;
    }
    o2.dist = -o2.dist;
    return o2;
}
