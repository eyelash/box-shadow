#include "rasterizer.hpp"

static std::vector<float> create_gaussian_kernel(int radius) {
	std::vector<float> kernel(radius * 2 + 1);
	const float sigma = radius / 3.f;
	const float sigma2 = sigma * sigma;
	const float factor = 1.0f / sqrtf(2.0f * M_PI * sigma2);
	for (int i = -radius; i <= radius; ++i) {
		kernel[i + radius] = factor * expf((i * i)/(-2.0f * sigma2));
	}
	return kernel;
}

static void gaussian_blur(Pixmap& pixmap, int radius) {
	const int width = pixmap.get_width();
	const int height = pixmap.get_height();
	std::vector<float> kernel = create_gaussian_kernel(radius);
	Pixmap tmp(width, height);
	// horizontal
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			Color sum;
			for (int i = -radius; i <= radius; ++i) {
				sum = sum + pixmap.get_pixel(x + i, y) * kernel[i + radius];
			}
			tmp.set_pixel(x, y, sum);
		}
	}
	// vertical
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			Color sum;
			for (int i = -radius; i <= radius; ++i) {
				sum = sum + tmp.get_pixel(x, y + i) * kernel[i + radius];
			}
			pixmap.set_pixel(x, y, sum);
		}
	}
}

int main() {
	Pixmap pixmap(300, 300);
	pixmap.clear(Color(0, 0, 0));
	pixmap.fill_rectangle(100, 100, 100, 100, Color(1, 1, 1));
	gaussian_blur(pixmap, 50);
	pixmap.write_png("gaussian-blur.png");
}
