/*
	Signed distance functions for different
	mathematical geometric shapes in 3D
*/


/*
 *	Sphere
 *	Parameters:
 *		p - sample position
 *		c - sphere center
 *		r - sphere radius
 */
float sdfSphere(vec3 p, vec3 c, float r) {
	return length(p - c) - r;
}


/*
 *	Plane
 *	Parameters:
 *		p - sample position
 *		n - plane normal (normalized)
 *		h - plane displacement from origin
 */
float sdfPlane(vec3 p, vec3 n, float h) {
	return dot(p, n) + h;
}


/*
 *	Box
 *	Parameters:
 *		p - sample position
 *		c - box center
 *		b - box dimensions
 */
float sdfBox(vec3 p, vec3 c, vec3 b) {
	vec3 q = abs(p - c) - b / 2.0f;
	return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}


/*
 *	Rounded box
 *	Parameters:
 *		p - sample position
 *		c - box center
 *		b - box dimensions
 *		r - radius of rounding
 */
float sdfRoundBox(vec3 p, vec3 c, vec3 b, float r) {
	vec3 q = abs(p - c) - b / 2.0f + r;
	return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0) - r;
}


/*
 *	Infinite cylinder
 *	Parameters:
 *		p - sample position
 *		r0 - point which is passed by directing line
 *		a - directing vector
 *		r - radius
 */
float sdfCylinder(vec3 p, vec3 r0, vec3 a, float r) {
	vec3 n = p - r0 - dot(a, p - r0) / dot(a, a) * a;
	return length(n) - r;
}


/*
 *	Triangular prism
 *	Parameters:
 *		p - sample position
 *		c - center of the prism
 *		h.x - height of the prism
 *		h.y - length of the prism
 */
float sdfTriPrism(vec3 p, vec3 c, vec2 h) {
	vec3 dif = p - c;
	vec3 q = abs(dif);
	return max(q.z - h.y, max(q.x * 0.866025 + dif.y * 0.5, -dif.y) - h.x * 0.5);
}
