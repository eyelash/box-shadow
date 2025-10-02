#include "rasterizer.hpp"

int main() {
	Pixmap pixmap(100, 100);
	for (size_t x = 0; x < pixmap.get_width(); ++x) {
		for (size_t y = 0; y < pixmap.get_height(); ++y) {
			pixmap.add_pixel(x, y, Color(1, 1, 0));
		}
	}
	pixmap.write_png("out.png");
}
