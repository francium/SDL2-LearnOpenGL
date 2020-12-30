CXX = g++
CXX_FLAGS = -std=c++11 -Wall -Wno-unused-variable -Wno-unused-function
LIBS =
INCLUDES =
CXX_FLAGS += -g -O0
# CXX_FLAGS += -g -O2
# CXX_FLAGS += -fsanitize=address


## Assimp
LIBS += -L./lib/assimp/bin -lassimp


## SDL2
LIBS += $$(sdl2-config --libs --cflags)


## GLAD
INCLUDES += -isystem lib/glad/include
LIBS += -ldl


## GLM
INCLUDES += -isystem lib/glm


## STB
INCLUDES += -isystem lib/stb/include


COMPILE = $(CXX) $(CXX_FLAGS) $(LIBS) $(INCLUDES)


# TARGETS #####################################################################


all: bin/ bin/main


bin/main: bin/glad.o bin/main.o
	$(COMPILE) -o $@ $^


bin/glad.o: lib/glad/src/glad.c
	$(COMPILE) -c -o $@ $^


bin/main.o: $(wildcard src/*)
	$(COMPILE) -c -o $@ src/main.cpp


bin/:
	@mkdir -p bin


.PHONY: clean
clean:
	rm -rf bin/


.PHONY: run
run: bin/main
	bin/main


watch-build:
	@clear;
	@echo -n "Ready"
	@while ,watchdo .watchfile; do\
		clear;\
		time make bin/main;\
	done


watch-run:
	@clear;
	@echo -n "Ready"
	@while ,watchdo .runfile; do\
		clear;\
		make run;\
	done
