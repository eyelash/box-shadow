/*

Copyright (c) 2017-2025, Elias Aebi
All rights reserved.

*/

#include <vector>
#include <cstddef>
#include <cstdint>
#include <cmath>

constexpr float clamp(float value, float min, float max) {
	return value < min ? min : (max < value ? max : value);
}

constexpr bool between(float value, float min, float max) {
	return value >= min && value < max;
}

struct Point {
	float x, y;
	constexpr Point(float x, float y): x(x), y(y) {}
	constexpr Point operator +(const Point& p) const {
		return Point(x + p.x, y + p.y);
	}
	constexpr Point operator -(const Point& p) const {
		return Point(x - p.x, y - p.y);
	}
	constexpr Point operator -() const {
		return Point(-x, -y);
	}
	constexpr Point operator *(float f) const {
		return Point(x * f, y * f);
	}
	constexpr bool operator ==(const Point& p) const {
		return x == p.x && y == p.y;
	}
};

constexpr float dot(const Point& p0, const Point& p1) {
	return p0.x * p1.x + p0.y * p1.y;
}

struct Line {
	float m, x0;
	constexpr Line(float m, const Point& p): m(m), x0(p.x - m * p.y) {}
	constexpr Line(const Point& p0, const Point& p1): Line((p1.x - p0.x) / (p1.y - p0.y), p0) {}
	constexpr Line(float x): m(0.f), x0(x) {}
	constexpr float get_x(float y) const {
		return m * y + x0;
	}
};

constexpr float intersect(const Line& l0, const Line& l1) {
	return (l1.x0 - l0.x0) / (l0.m - l1.m);
}

struct Segment {
	float y0, y1;
	Line line;
	constexpr Segment(const Point& p0, const Point& p1): y0(p0.y), y1(p1.y), line(p0, p1) {}
	constexpr Segment(float y0, float y1, const Line& line): y0(y0), y1(y1), line(line) {}
};

struct Color {
	float r, g, b, a;
	constexpr Color(float r, float g, float b, float a = 1.f): r(r), g(g), b(b), a(a) {}
	constexpr Color(): Color(0.f, 0.f, 0.f, 0.f) {}
	static constexpr Color rgb(unsigned char r, unsigned char g, unsigned char b) {
		return Color(r / 255.f, g / 255.f, b / 255.f);
	}
	constexpr Color operator +(const Color& c) const {
		return Color(r + c.r, g + c.g, b + c.b, a + c.a);
	}
	constexpr Color operator *(float f) const {
		return Color(r * f, g * f, b * f, a * f);
	}
	constexpr Color unpremultiply() const {
		return a == 0.f ? Color() : Color(r/a, g/a, b/a, a);
	}
};

constexpr Color blend(const Color& dst, const Color& src) {
	return src + dst * (1.f - src.a);
}

class Random {
	uint64_t s[2] = {0xC0DEC0DEC0DEC0DE, 0xC0DEC0DEC0DEC0DE};
public:
	uint64_t next() {
		// xorshift128+
		const uint64_t result = s[0] + s[1];
		const uint64_t s1 = s[0] ^ (s[0] << 23);
		s[0] = s[1];
		s[1] = s1 ^ s[1] ^ (s1 >> 18) ^ (s[1] >> 5);
		return result;
	}
	float next_float() {
		return std::ldexp(static_cast<float>(next()), -64);
	}
};

inline unsigned char dither(Random& random, float value) {
	return clamp(value * 255.f + random.next_float(), 0.f, 255.f);
}

class Pixmap {
	std::vector<Color> pixels;
	size_t width;
public:
	Pixmap(size_t width, size_t height): pixels(width*height), width(width) {}
	size_t get_width() const {
		return width;
	}
	size_t get_height() const {
		return pixels.size() / width;
	}
	Color get_pixel(size_t x, size_t y) const {
		size_t i = y * width + x;
		return pixels[i];
	}
	void add_pixel(size_t x, size_t y, const Color& color) {
		size_t i = y * width + x;
		pixels[i] = pixels[i] + color;
	}
	void write_png(const char* file_name) const;
};

struct Vector {
	float x, y, z;
	constexpr Vector(float x, float y, float z): x(x), y(y), z(z) {}
	constexpr Vector(): Vector(0.f, 0.f, 0.f) {}
	constexpr Vector operator +(const Vector& v) const {
		return Vector(x + v.x, y + v.y, z + v.z);
	}
	constexpr Vector operator -(const Vector& v) const {
		return Vector(x - v.x, y - v.y, z - v.z);
	}
	constexpr Vector operator -() const {
		return Vector(-x, -y, -z);
	}
	constexpr Vector operator *(float t) const {
		return Vector(x * t, y * t, z * t);
	}
};

constexpr float dot(const Vector& v0, const Vector& v1) {
	return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
}

inline float length(const Vector& v) {
	return std::sqrt(dot(v, v));
}

inline Vector normalize(const Vector& v) {
	return v * (1.f / length(v));
}

inline Vector random_vector(Random& random) {
	while (true) {
		Vector v;
		v.x = random.next_float() * 2.f - 1.f;
		v.y = random.next_float() * 2.f - 1.f;
		v.z = random.next_float() * 2.f - 1.f;
		if (dot(v, v) <= 1.f) {
			return v;
		}
	}
}

struct Ray {
	Vector origin;
	Vector direction;
	constexpr Ray(const Vector& origin, const Vector& direction): origin(origin), direction(direction) {}
};

struct Sphere {
	Vector origin;
	float radius;
	constexpr Sphere(const Vector& origin, float radius): origin(origin), radius(radius) {}
};

struct ZPlane {
	float z;
	constexpr ZPlane(float z): z(z) {}
};

struct Rectangle {
	float x, y, z;
	float width, height;
	constexpr Rectangle(float x, float y, float z, float width, float height): x(x), y(y), z(z), width(width), height(height) {}
};

struct RoundedRectangle {
	float x, y, z;
	float width, height;
	float radius;
	constexpr RoundedRectangle(float x, float y, float z, float width, float height, float radius = 0.f): x(x), y(y), z(z), width(width), height(height), radius(radius) {}
};

inline Vector intersect(const Ray& ray, const ZPlane& z_plane) {
	const float t = (z_plane.z - ray.origin.z) / ray.direction.z;
	return ray.origin + ray.direction * t;
}

inline bool intersect(const Ray& ray, const Rectangle& rect) {
	if (ray.direction.z == 0.f) {
		return false;
	}
	const float t = (rect.z - ray.origin.z) / ray.direction.z;
	const Vector p = ray.origin + ray.direction * t;
	return p.x >= rect.x && p.x <= rect.x + rect.width && p.y >= rect.y && p.y <= rect.y + rect.height;
}

inline bool intersect(const Ray& ray, const RoundedRectangle& rect) {
	if (ray.direction.z == 0.f) {
		return false;
	}
	const float t = (rect.z - ray.origin.z) / ray.direction.z;
	Vector p = ray.origin + ray.direction * t;
	p.z = 0.f;
	p = p - Vector(rect.x, rect.y, 0.f);
	if (p.x < 0.f || p.y < 0.f) {
		return false;
	}
	if (p.x > rect.width || p.y > rect.height) {
		return false;
	}
	if (p.x > rect.width - rect.radius) {
		p.x = rect.width - p.x;
	}
	if (p.y > rect.height - rect.radius) {
		p.y = rect.height - p.y;
	}
	if (p.x < rect.radius && p.y < rect.radius) {
		Vector rv = p - Vector(rect.radius, rect.radius, 0.f);
		return dot(rv, rv) <= rect.radius * rect.radius;
	}
	return true;
}
