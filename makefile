main: main.cpp png.cpp rasterizer.hpp
	c++ -o $@ main.cpp png.cpp

simulation: simulation.cpp png.cpp rasterizer.hpp
	c++ -o $@ simulation.cpp png.cpp -O3

simulation.png: simulation
	./simulation
