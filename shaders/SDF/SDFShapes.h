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
	h /= 2.0f;
	return max(q.x - h.y, max(q.z * 0.866025 + dif.y * 0.5, -dif.y) - h.x * 0.5);
}


/*
 *	Capped cone
 *	Parameters:
 *		p - sample position
 *		c - center of the cone
 *		h - height of the cone
 *		r1 - lower radius of the cone
 *		r2 - higher radius of the cone
 */
float sdfCappedCone(vec3 p, vec3 c, float h, float r1, float r2) {
	vec3 full = p - c;
	h /= 2.0f;

	vec2 q = vec2(length(full.xz), full.y);
	vec2 k1 = vec2(r2, h);
	vec2 k2 = vec2(r2 - r1, 2.0 * h);
	vec2 ca = vec2(q.x - min(q.x, (q.y < 0.0) ? r1 : r2), abs(q.y) - h);
	vec2 cb = q - k1 + k2 * clamp(dot(k1 - q, k2) / dot2(k2), 0.0, 1.0);
	float s = (cb.x < 0.0 && ca.y < 0.0) ? -1.0 : 1.0;
	return s * sqrt(min(dot2(ca), dot2(cb)));
}


/*
 *	Capped cone
 *	Parameters:
 *		p - sample position
 *		c - center of the cone
 *		h - height of the cone
 *		r - radius of the cone
 */
float sdfCone(vec3 p, vec3 c, float h, float r) {
	return sdfCappedCone(p, c, h, r, 0.0f);
}


float sdfBoxFrame(vec3 p, vec3 c, vec3 b, float e) {
	p = abs(p - c) - b / 2.0f;
	e /= 2.0f;
	vec3 q = abs(p + e) - e;
	return min(min(
		length(max(vec3(p.x, q.y, q.z), 0.0)) + min(max(p.x, max(q.y, q.z)), 0.0),
		length(max(vec3(q.x, p.y, q.z), 0.0)) + min(max(q.x, max(p.y, q.z)), 0.0)),
		length(max(vec3(q.x, q.y, p.z), 0.0)) + min(max(q.x, max(q.y, p.z)), 0.0));
}


float sdfTorus(vec3 p, vec2 t) {
	vec2 q = vec2(length(p.xz) - t.x, p.y);
	return length(q) - t.y;
}
