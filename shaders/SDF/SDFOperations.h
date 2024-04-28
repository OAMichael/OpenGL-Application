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
