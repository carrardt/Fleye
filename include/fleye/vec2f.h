#ifndef __FLEYE_VEC2F_H
#define __FLEYE_VEC2F_H

#include <cmath>

struct Vec2f
{
	float x,y;

	inline Vec2f() : x(0.0f) , y(0.0f) {}
	inline Vec2f(float u, float v) : x(u) , y(v) {}
	inline Vec2f(const Vec2f& v) : x(v.x) , y(v.y) {}

	inline Vec2f operator + (Vec2f v) const { return Vec2f(x+v.x,y+v.y); }
	inline Vec2f& operator += (Vec2f v) { *this = *this + v; return *this; }
	inline Vec2f operator - (Vec2f v) const { return Vec2f(x-v.x,y-v.y); }
	inline Vec2f operator * (float s) const { return Vec2f(x*s,y*s); }
	inline Vec2f operator / (float s) const { return Vec2f(x/s,y/s); }
	
	inline float norm2() const { return x*x+y*y; }
	inline float norm() const { return std::sqrt( norm2() ); }
	inline float dist2(Vec2f v) const {	return (v-*this).norm2(); }
	inline float dist(Vec2f v) const {	return (v-*this).norm(); }
	
	inline Vec2f normalize() const { return *this / norm(); }
};

template<typename OStream>
OStream& operator << (OStream& out, const Vec2f& v)
{
	out << v.x << ',' << v.y;
	return out;
}

#endif
