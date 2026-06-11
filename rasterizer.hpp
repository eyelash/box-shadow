/*

Copyright (c) 2017-2026, Elias Aebi
All rights reserved.

*/

#include <vector>
#include <cstddef>
#include <cstdint>
#include <cmath>

template <class T> constexpr T clamp(T value, T min, T max) {
	return value < min ? min : (max < value ? max : value);
}

template <class T> constexpr T lerp(T v1, T v2, float f) {
	return v1 + (v2 - v1) * f;
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

inline float length(const Point& p) {
	return std::sqrt(dot(p, p));
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
	constexpr Color operator -(const Color& c) const {
		return Color(r - c.r, g - c.g, b - c.b, a - c.a);
	}
	constexpr Color operator *(float f) const {
		return Color(r * f, g * f, b * f, a * f);
	}
	constexpr Color unpremultiply() const {
		return a == 0.f ? Color() : Color(r/a, g/a, b/a, a);
	}
};
constexpr Color BLACK(0.f, 0.f, 0.f);
constexpr Color WHITE(1.f, 1.f, 1.f);
constexpr Color TRANSPARENT(0.f, 0.f, 0.f, 0.f);

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
	int width;
	int height;
public:
	Pixmap(int width, int height): pixels(width * height), width(width), height(height) {}
	int get_width() const {
		return width;
	}
	int get_height() const {
		return height;
	}
	Color get_pixel(int x, int y) const {
		x = clamp(x, 0, width - 1);
		y = clamp(y, 0, height - 1);
		return pixels[y * width + x];
	}
	void set_pixel(int x, int y, const Color& color) {
		pixels[y * width + x] = color;
	}
	Color get(int x, int y) const {
		x = clamp(x, 0, width - 1);
		y = clamp(y, 0, height - 1);
		return pixels[y * width + x];
	}
	Color get_linear(float x, float y) const {
		const int xi = x;
		const int yi = y;
		const float xf = x - xi;
		const float yf = y - yi;
		return lerp(
			lerp(get(xi, yi),     get(xi + 1, yi),     xf),
			lerp(get(xi, yi + 1), get(xi + 1, yi + 1), xf),
			yf
		);
	}
	void set(int x, int y, const Color& color) {
		pixels[y * width + x] = color;
	}
	void add(int x, int y, const Color& color) {
		const int i = y * width + x;
		pixels[i] = blend(pixels[i], color);
	}
	void add_pixel(int x, int y, const Color& color) {
		int i = y * width + x;
		pixels[i] = pixels[i] + color;
	}
	void clear(const Color& color) {
		for (size_t i = 0; i < pixels.size(); ++i) {
			pixels[i] = color;
		}
	}
	void fill_rectangle(int x_, int y_, int width, int height, const Color& color) {
		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				set_pixel(x_ + x, y_ + y, color);
			}
		}
	}
	void write_png(const char* file_name) const;
};

template <class F> void draw_graph(Pixmap& pixmap, int x_, int y_, int width, int height, const Color& color, F f) {
	const float line_width = 1.f;
	for (int x = 0; x < width; ++x) {
		const float x0 = x;
		const float x1 = x + 1;
		const float y0 = height - f(x0 / width) * height;
		const float y1 = height - f(x1 / width) * height;
		const float delta_y = length(Point(x0, y0) - Point(x1, y1)) * (line_width * .5f);
		float y2 = y0 - delta_y;
		float y3 = y1 - delta_y;
		float y4 = y0 + delta_y;
		float y5 = y1 + delta_y;
		if (y2 > y3) std::swap(y2, y3);
		if (y4 > y5) std::swap(y4, y5);
		for (int y = 0; y < height; ++y) {
			const float y6 = y;
			const float y7 = y + 1;
			if (y2 > y7 || y5 < y6) {
				continue;
			}
			float area = 0.f;
			if (y2 < y6) {
				if (y3 < y6) {
					area = 1.f;
				}
				else {
					const float x2 = 1.f / (y3 - y2) * (y6 - y2);
					if (y3 < y7) {
						// y3 is between y6 and y7
						area = (y7 - y3) + (y3 - y6) * (x2 + 1.f) * .5f;
					}
					else {
						// y3 > y7
						const float x3 = 1.f / (y3 - y2) * (y7 - y2);
						area = (x2 + x3) * .5f;
					}
				}
			}
			else {
				// y2 is between y6 and y7
				if (y3 < y7) {
					// y3 is between y6 and y7 (since y3 >= y2 >= y6)
					area = (y7 - y3) + (y3 - y2) * .5f;
				}
				else {
					// y3 > y7
					const float x3 = 1.f / (y3 - y2) * (y7 - y2);
					area = (y7 - y2) * x3 * .5f;
				}
			}
			if (y4 < y7) {
				if (y4 < y6) {
					const float x2 = 1.f / (y5 - y4) * (y6 - y4);
					if (y5 < y7) {
						// y5 is between y6 and y7
						area -= (y7 - y5) + (y5 - y6) * (x2 + 1.f) * .5f;
					}
					else {
						// y5 > y7
						const float x3 = 1.f / (y5 - y4) * (y7 - y4);
						area -= (x2 + x3) * .5f;
					}
				}
				else {
					// y4 is between y6 and y7
					if (y5 < y7) {
						// y5 is between y6 and y7
						area -= (y7 - y5) + (y5 - y4) * .5f;
					}
					else {
						// y5 > y7
						const float x3 = 1.f / (y5 - y4) * (y7 - y4);
						area -= (y7 - y4) * x3 * .5f;
					}
				}
			}
			pixmap.add(x_ + x, y_ + y, color * area);
		}
	}
}
template <class F> void draw_graph(Pixmap& pixmap, int x_, int y_, int width, int height, F f) {
	draw_graph(pixmap, x_, y_, width, height, BLACK, f);
}
template <class F> void draw_graph(Pixmap& pixmap, int x_, int y_, int width, int height, float x_min, float x_max, float y_min, float y_max, const Color& color, F f) {
	draw_graph(pixmap, x_, y_, width, height, color, [&](float x) {
		return (f(x_min + x * (x_max - x_min)) - y_min) / (y_max - y_min);
	});
}
template <class F> void draw_graph(Pixmap& pixmap, int x_, int y_, int width, int height, float x_min, float x_max, float y_min, float y_max, F f) {
	draw_graph(pixmap, x_, y_, width, height, x_min, x_max, y_min, y_max, BLACK, f);
}

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

constexpr float dot(const Vector& a, const Vector& b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

constexpr Vector cross(const Vector& a, const Vector& b) {
	return Vector(
		a.y * b.z - b.y * a.z,
		a.z * b.x - b.z * a.x,
		a.x * b.y - b.x * a.y
	);
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
