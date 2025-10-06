#include "rasterizer.hpp"

constexpr RoundedRectangle rectangle = RoundedRectangle(50, 50, 10, 100, 100, 10);
constexpr Vector camera = Vector(100, 100, 500);
constexpr Color background_color = Color::rgb(0x00, 0x00, 0x00);
constexpr Color foreground_color = Color::rgb(0xFF, 0xFF, 0xFF);
constexpr Color shadow_color = foreground_color * 1.0f;
constexpr int iterations = 1024;

int main() {
	constexpr Vector normal(0, 0, 1);
	Pixmap pixmap(200, 200);
	Random random;
	for (int x = 0; x < 200; ++x) {
		for (int y = 0; y < 200; ++y) {
			Color color;
			for (int i = 0; i < iterations; ++i) {
				Vector origin = Vector(x, y, 0);
				origin.x += random.next_float();
				origin.y += random.next_float();
				Vector direction = random_vector(random);
				if (direction.z < 0.f) direction.z = -direction.z;
				/*if (intersect(Ray(origin, camera - origin), rectangle)) {
					color = color + foreground_color;
					continue;
				}*/
				if (intersect(Ray(origin, direction), rectangle)) {
					color = color + shadow_color;
				}
			}
			color = blend(background_color, color * (1.f / iterations));
			pixmap.add_pixel(x, y, color);
		}
	}
	pixmap.write_png("simulation.png");
}
