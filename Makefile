CXX = g++
CXX_FLAGS =


SRCS = main.cpp
OUT_NAME = main
OUT = -o $(OUT_NAME)
LIBS =
INCLUDES =


# Debug
CXX_FLAGS += -g -O0


## SDL2
LIBS += $$(sdl2-config --libs --cflags)


## GLAD
SRCS += lib/glad/src/glad.c
INCLUDES = -Ilib/glad/include
LIBS += -ldl


# TARGETS #####################################################################


$(OUT_NAME): $(SRCS)
	$(CXX) $(CXX_FLAGS) $(LIBS) $(INCLUDES) $(OUT) $(SRCS)
