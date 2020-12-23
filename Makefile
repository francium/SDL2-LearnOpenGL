CXX = g++
CXX_FLAGS = -std=c++11
LIBS =
INCLUDES =
CXX_FLAGS += -g -O0


## SDL2
LIBS += $$(sdl2-config --libs --cflags)


## GLAD
INCLUDES += -Ilib/glad/include
LIBS += -ldl


COMPILE = $(CXX) $(CXX_FLAGS) $(LIBS) $(INCLUDES)


# TARGETS #####################################################################


all: bin/ bin/main


bin/main: bin/glad.o bin/main.o
	$(COMPILE) -o $@ $^


bin/glad.o: lib/glad/src/glad.c
	$(COMPILE) -c -o $@ $^


bin/main.o: src/main.cpp
	$(COMPILE) -c -o $@ $^


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
		make bin/main;\
	done


watch-run:
	@clear;
	@echo -n "Ready"
	@while ,watchdo .runfile; do\
		clear;\
		make run;\
	done
