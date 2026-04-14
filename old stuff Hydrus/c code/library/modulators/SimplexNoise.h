#pragma once

#include <stdbool.h>
#include <math.h>
#include "../math.h"

double step(double a, double b) {
	return b >= a ? 1 : 0;
}

double frac(double v) {
	return v - floor(v);
}

double saturate(double v) {
	return dmax(0, dmin(1, v));
}

typedef struct Uint3 {
	unsigned x;
	unsigned y;
	unsigned z;
} Uint3;

Uint3 Uint3_make(unsigned x, unsigned y, unsigned z) {
	Uint3 ret = { x, y, z };
	return ret;
}

Uint3 Uint3_modify(Uint3* o, unsigned x, unsigned y, unsigned z) {
	o->x = x;
	o->y = y;
	o->z = z;
}

typedef struct Uint4 {
	unsigned x;
	unsigned y;
	unsigned z;
	unsigned w;
} Uint4;

Uint4 Uint4_modify(Uint4* o, unsigned x, unsigned y, unsigned z, unsigned w) {
	o->x = x;
	o->y = y;
	o->z = z;
	o->w = w;
}

typedef struct Float2 {
	double x;
	double y;
} Float2;

Float2 Float2_make(double x, double y) {
	Float2 ret = { x, y };
	return ret;
}

Float2 Float2_modify(Float2* o, double x, double y) {
	o->x = x;
	o->y = y;
}

double Float2_dot(Float2* a, const Float2* b) {
	return a->x * b->x + a->y * b->y;
}

Float2 Float2_cubed(const Float2* o) {
	Float2 ret = {
		o->x * o->x * o->x,
		o->y * o->y * o->y
	};

	return ret;
}

typedef struct Float3 {
	double x;
	double y;
	double z;
} Float3;

Float3 Float3_make(double x, double y, double z) {
	Float3 ret = { x, y, z };
	return ret;
}

Float3 Float3_modify(Float3* o, double x, double y, double z) {
	o->x = x;
	o->y = y;
	o->z = z;
}

void Float3_add_modify(Float3* o, double v) {
	o->x += v;
	o->y += v;
	o->z += v;
}

Float3 Float3_scalar(Float3* o, double v) {
	Float3 ret = {
		o->x + v,
		o->y + v,
		o->z + v
	};

	return ret;
}

double Float3_dot(const Float3* a, const Float3* b) {
	return a->x*b->x + a->y*b->y + a->z*b->z;
}

Float3 Float3_floor(const Float3* o) {
	Float3 ret = {
		floor(o->x),
		floor(o->y),
		floor(o->z)
	};

	return ret;
}

Float3 Float3_cubed(const Float3* o) {
	Float3 ret = {
		o->x * o->x * o->x,
		o->y * o->y * o->y,
		o->z * o->z * o->z
	};

	return ret;
}

typedef struct Float4 {
	double x;
	double y;
	double z;
	double w;
} Float4;

Float4 Float4_make(double x, double y, double z, double w) {
	Float4 ret = { x, y, z, w };
	return ret;
}

Float4 Float4_modify(Float4* o, double x, double y, double z, double w) {
	o->x = x;
	o->y = y;
	o->z = z;
	o->w = w;
}

double Float4_dot(const Float4* a, const Float4* b) {
	return a->x*b->x + a->y*b->y + a->z*b->z + a->w*b->w;
}

Float4 Float4_squared(Float4* o) {
	Float4 ret = {
		o->x * o->x,
		o->y * o->y,
		o->z * o->z,
		o->w * o->w
	};

	return ret;
}

void Float4_add_modify(Float4* o, double v) {
	o->x += v;
	o->y += v;
	o->z += v;
	o->w += v;
}

unsigned pcg3d16(Uint3* p) {
	//uint3 v = p * 1664525u + 1013904223u;

	Uint3 v;
	v.x = p->x * 1664525u + 1013904223u; 
	v.y = p->y * 1664525u + 1013904223u;
	v.z = p->z * 1664525u + 1013904223u;

	v.x += v.y * v.z; 
	v.y += v.z * v.x; 
	v.z += v.x * v.y;
	v.x += v.y * v.z;

	return v.x;
}

unsigned pcg4d16(Uint4* p) {
	//uint4 v = p * 1664525u + 1013904223u;

	Uint4 v;
	v.x = p->x * 1664525u + 1013904223u;
	v.y = p->y * 1664525u + 1013904223u;
	v.z = p->z * 1664525u + 1013904223u;
	v.w = p->z * 1664525u + 1013904223u;

	v.x += v.y * v.w; 
	v.y += v.z * v.x; 
	v.z += v.x * v.y; 
	v.w += v.y * v.z;
	v.x += v.y * v.w;

	return v.x;
}

// Get random gradient from hash value.
Float3 gradient3d(unsigned hash) {
	//float3 g = float3(hash.xxx & uint3(0x80000, 0x40000, 0x20000));
	Float3 g = { hash & 0x80000, hash & 0x40000, hash & 0x20000 };

	//return g * float3(1.0 / 0x40000, 1.0 / 0x20000, 1.0 / 0x10000) - 1.0;
	return Float3_make(g.x * (1.0 / 0x40000) - 1.0, g.y * (1.0 / 0x20000) - 1.0, g.z * (1.0 / 0x10000) - 1.0);
}

Float4 gradient4d(unsigned hash) {
	//float4 g = float4(hash.xxxx & uint4(0x80000, 0x40000, 0x20000, 0x10000));
	Float4 g = { hash & 0x80000, hash & 0x40000, hash & 0x20000, hash & 0x10000 };

	//return g * float4(1.0 / 0x40000, 1.0 / 0x20000, 1.0 / 0x10000, 1.0 / 0x8000) - 1.0;
	return Float4_make(g.x* (1.0 / 0x40000) - 1.0, g.y* (1.0 / 0x20000) - 1.0, g.z* (1.0 / 0x10000) - 1.0, g.w* (1.0 / 0x8000) - 1.0);
}

// 3D Simplex Noise. Approximately 71 instruction slots used.
// Assume p is in the range [-32768, 32767].
double SimplexNoise3D(Float3 p) {
	double dotproduct;

	const Float2 c = { 1.0 / 6.0, 1.0 / 3.0 };
	const Float4 d = { 0.0, 0.5, 1.0, 2.0 };

	// First corner
	//float3 i = floor(p + dot(p, C.yyy));
	Float3 c_y = { c.y, c.y, c.y };
	dotproduct = Float3_dot(&p, &c_y);
	Float3 i = { p.x + dotproduct, p.y + dotproduct, p.z + dotproduct };	

	//float3 x0 = p - i + dot(i, C.xxx);
	Float3 c_x = { c.x, c.x, c.x };
	dotproduct = Float3_dot(&i, &c_x);
	Float3 x0 = { p.x - i.x + dotproduct, p.y - i.y + dotproduct, p.z - i.z + dotproduct };

	// Other corners
	// Float3 g = step(x0.yzx, x0.xyz);
	Float3 g = { step(x0.y, x0.x), step(x0.z, x0.y), step(x0.x, x0.z) };

	//Float3 l = 1.0 - g;
	Float3 l = { 1.0 - g.x, 1.0 - g.y, 1.0 - g.z };

	//Float3 i1 = min(g.xyz, l.zxy);
	Float3 i1 = { dmin(g.x, l.z), dmin(g.y, l.x), dmin(g.z, l.y) };

	//Float3 i2 = max(g.xyz, l.zxy);
	Float3 i2 = { dmax(g.x, l.z), dmax(g.y, l.x), dmax(g.z, l.y) };

	//float3 x1 = x0 - i1 + C.xxx;
	Float3 x1 = { x0.x - i1.x + c_x.x, x0.y - i1.y + c_x.x, x0.z - i1.z + c_x.x };

	//float3 x2 = x0 - i2 + C.yyy;
	Float3 x2 = { x0.x - i2.x + c_y.y, x0.y - i2.y + c_y.y, x0.z - i2.z + c_y.y };

	//float3 x3 = x0 - D.yyy;
	Float3 x3 = { x0.x - d.y, x0.y - d.y, x0.z - d.y };

	//i = i + 32768.5;
	Float3_add_modify(&i, 32768.5);

	Uint3 uint3;

	//uint hash0 = pcg3d16((uint3)i);
	Uint3_modify(&uint3, (unsigned)i.x, (unsigned)i.y, (unsigned)i.z);
	unsigned hash0 = pcg3d16(&uint3);

	//uint hash1 = pcg3d16((uint3)(i + i1));
	Uint3_modify(&uint3, (unsigned)(i.x + i1.x), (unsigned)(i.y + i1.y), (unsigned)(i.z + i1.z));
	unsigned hash1 = pcg3d16(&uint3);

	//uint hash2 = pcg3d16((uint3)(i + i2));
	Uint3_modify(&uint3, (unsigned)(i.x + i2.x), (unsigned)(i.y + i2.y), (unsigned)(i.z + i2.z));
	unsigned hash2 = pcg3d16(&uint3);

	//uint hash3 = pcg3d16((uint3)(i + 1 ));
	Uint3_modify(&uint3, (unsigned)(i.x + 1), (unsigned)(i.y + 1), (unsigned)(i.z + 1));
	unsigned hash3 = pcg3d16(&uint3);

	Float3 p0 = gradient3d(hash0);
	Float3 p1 = gradient3d(hash1);
	Float3 p2 = gradient3d(hash2);
	Float3 p3 = gradient3d(hash3);

	// Mix final noise value.
	//float4 m = saturate(0.5 - float4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)));
	Float4 dotproduct4 = { Float3_dot(&x0, &x0), Float3_dot(&x1, &x1), Float3_dot(&x2, &x2), Float3_dot(&x3, &x3) };
	Float4 m = { saturate(0.5 - dotproduct4.x), saturate(0.5 - dotproduct4.y), saturate(0.5 - dotproduct4.z),saturate(0.5 - dotproduct4.w) };
	//float4 mt = m * m;
	Float4 mt = Float4_squared(&m);
	//float4 m4 = mt * mt;
	Float4 m4 = Float4_squared(&mt);
	//return 62.6 * dot(m4, float4(dot(x0, p0), dot(x1, p1), dot(x2, p2), dot(x3, p3)));
	Float4 x_p_dot4 = { Float3_dot(&x0, &p0), Float3_dot(&x1, &p1), Float3_dot(&x2, &p2), Float3_dot(&x3, &p3) };
	return 62.6 * Float4_dot(&m4, &x_p_dot4);
}

// 4D Simplex Noise. Approximately 113 instruction slots used.
// Assume p is in the range [-32768, 32767].
float SimplexNoise4D(Float4* p) {
	Float4 f4 = { 0.309016994374947451, 0.309016994374947451, 0.309016994374947451, 0.309016994374947451 };

	Float4 c = {
		0.138196601125011, // (5 - sqrt(5))/20  G4
		0.276393202250021, // 2 * G4
		0.414589803375032, // 3 * G4
	   -0.447213595499958  // -1 + 4 * G4
	};

	// First corner
	//float4 i = floor(p + dot(p, F4));
	double dotproduct;
	dotproduct = Float4_dot(p, &f4);
	Float4 i = { floor(p->x + dotproduct), floor(p->y + dotproduct), floor(p->z + dotproduct), floor(p->w + dotproduct) };
	Float4 c_x = { c.x, c.x, c.x, c.x };
	//Float4 x0 = p - i + dot(i, c.xxxx);
	dotproduct = Float4_dot(&i, &c_x);
	Float4 x0 = { p->x - i.x + dotproduct, p->y - i.y + dotproduct, p->z - i.z + dotproduct, p->w - i.w + dotproduct };

	// Other corners
	// Rank sorting originally contributed by Bill Licea-Kane, AMD (formerly ATI)
	Float4 i0;
	//Float3 isX = step(x0.yzw, x0.xxx);
	Float3 isX = { step(x0.y, x0.x), step(x0.z, x0.x), step(x0.w, x0.x) };
	//Float3 isYZ = step(x0.zww, x0.yyz);
	Float3 isYZ = { step(x0.z, x0.y), step(x0.w, x0.y), step(x0.w, x0.z) };

	i0.x = isX.x + isX.y + isX.z;
	//i0.yzw = 1.0 - isX;
	i0 = Float4_modify(&i0, i0.x, 1.0 - isX.x, 1.0 - isX.y, 1.0 - isX.z);

	i0.y += isYZ.x + isYZ.y;
	//i0.zw += 1.0 - isYZ.xy;
	Float4_modify(&i0, i0.x, i0.y, 1.0 - isYZ.x, 1.0 - isYZ.y);
	i0.z += isYZ.z;
	i0.w += 1.0 - isYZ.z;

	// i0 now contains the unique values 0,1,2,3 in each channel
	Float4 i3 = { saturate(i0.x), saturate(i0.y), saturate(i0.z) };                   //float4 i3 = saturate( i0 );
	Float4 i2 = { saturate(i0.x - 1.0), saturate(i0.y - 1.0), saturate(i0.z - 1.0) }; //float4 i2 = saturate(i0 - 1.0);
	Float4 i1 = { saturate(i0.x - 2.0), saturate(i0.y - 2.0), saturate(i0.z - 2.0) }; //float4 i1 = saturate(i0 - 2.0);

	Float4 x1 = { x0.x - i1.x + c.x, x0.y - i1.y + c.x, x0.z - i1.z + c.x, x0.w - i1.w + c.x }; //float4 x1 = x0 - i1 + C.xxxx;
	Float4 x2 = { x0.x - i2.x + c.y, x0.y - i2.y + c.y, x0.z - i2.z + c.y, x0.w - i2.w + c.y }; //float4 x2 = x0 - i2 + C.yyyy;
	Float4 x3 = { x0.x - i3.x + c.z, x0.y - i3.y + c.z, x0.z - i3.z + c.z, x0.w - i3.w + c.z }; //float4 x3 = x0 - i3 + C.zzzz;
	Float4 x4 = { x0.x + c.w, x0.y + c.w, x0.z + c.w, x0.w + c.w }; //float4 x4 = x0 + C.wwww;

	Float4_add_modify(&i, 32768.5); //i = i + 32768.5;

	Uint4 uint4;

	//uint hash0 = pcg4d16((uint4)i);
	Uint4_modify(&uint4, (unsigned)i.x, (unsigned)i.y, (unsigned)i.z, (unsigned)i.w);
	unsigned hash0 = pcg4d16(&uint4);

	//uint hash1 = pcg4d16((uint4)(i + i1));
	Uint4_modify(&uint4, (unsigned)(i.x + i1.x), (unsigned)(i.y + i1.y), (unsigned)(i.z + i1.z), (unsigned)(i.w + i1.w));
	unsigned hash1 = pcg4d16(&uint4);

	//uint hash2 = pcg4d16((uint4)(i + i2));
	Uint4_modify(&uint4, (unsigned)(i.x + i2.x), (unsigned)(i.y + i2.y), (unsigned)(i.z + i2.z), (unsigned)(i.w + i2.w));
	unsigned hash2 = pcg4d16(&uint4);

	//uint hash3 = pcg4d16((uint4)(i + i3));
	Uint4_modify(&uint4, (unsigned)(i.x + i3.x), (unsigned)(i.y + i3.y), (unsigned)(i.z + i3.z), (unsigned)(i.w + i3.w));
	unsigned hash3 = pcg4d16(&uint4);

	//uint hash4 = pcg4d16((uint4)(i + 1));
	Uint4_modify(&uint4, (unsigned)(i.x + 1), (unsigned)(i.y + 1), (unsigned)(i.z + 1), (unsigned)(i.w + 1));
	unsigned hash4 = pcg4d16(&uint4);

	Float4 p0 = gradient4d(hash0);
	Float4 p1 = gradient4d(hash1);
	Float4 p2 = gradient4d(hash2);
	Float4 p3 = gradient4d(hash3);
	Float4 p4 = gradient4d(hash4);

	// Mix contributions from the five corners
	//float3 m0 = saturate(0.6 - float3(dot(x0, x0), dot(x1, x1), dot(x2, x2)));
	Float3 dot3 = { Float4_dot(&x0, &x0), Float4_dot(&x1, &x1), Float4_dot(&x2, &x2) };
	Float3 m0 = { 0.6 - dot3.x, 0.6 - dot3.y, 0.6 - dot3.z };

	//float2 m1 = saturate(0.6 - float2(dot(x3, x3), dot(x4, x4)));
	Float2 dot2 = { Float4_dot(&x3, &x3), Float4_dot(&x4, &x4) };
	Float2 m1 = { saturate(0.6 - dot2.x), saturate(0.6 - dot2.y) };

	Float3 m03 = Float3_cubed(&m0); //float3 m03 = m0 * m0 * m0;
	Float2 m13 = Float2_cubed(&m1); //float2 m13 = m1 * m1 * m1;

	//return (dot(m03, float3(dot(p0, x0), dot(p1, x1), dot(p2, x2))) + dot(m13, float2(dot(p3, x3), dot(p4, x4)))) * 9.0;
	Float3_modify(&dot3, Float4_dot(&p0, &x0), Float4_dot(&p1, &x1), Float4_dot(&p2, &x2));
	Float2_modify(&dot2, Float4_dot(&p3, &x3), Float4_dot(&p4, &x4));
	return Float3_dot(&m03, &dot3) + Float2_dot(&m13, &dot2) * 9.0;
}

double fbm(Float3 x) {
	double v = 0.0;
	double a = 0.5;
	Float3 shift = { 100, 100, 100 };
	for (int i = 0; i < 5; ++i) {
		double n = fabs(SimplexNoise3D(x));
		n = 1 - pow(1 - n, 3);
		v += a * n;
		x = Float3_make(x.x * 2.0 + shift.x, x.y * 2.0 + shift.y, x.z * 2.0 + shift.z);
		a *= 0.5;
	}
	return v;
}