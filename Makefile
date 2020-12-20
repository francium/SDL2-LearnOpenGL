all:
	g++ -o main main.cpp -lGLEW -lX11 -lGLU -lGL $$(sdl2-config --libs --cflags)
