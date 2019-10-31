SAMPLE = sdl_opengl_simple
SRCS   = 

SDL_CFLAGS  := $(shell sdl-config --cflags)
SDL_LDFLAGS := $(shell sdl-config --libs)

CC     = gcc
COPTS  = -g -Wall
CFLAGS = $(SDL_CFLAGS)
LIBS   = -lstdc++ -lSDL $(SDL_LDFLAGS) -L/usr/X11R6/lib -lXi -lGL -lGLU

all : $(SAMPLE)

clean : 
	rm -f $(SAMPLE)

run : $(SAMPLE)
	./$(SAMPLE)

$(SAMPLE) : $(SAMPLE).cpp
	$(CC) $(SRCS) -o $@ $(COPTS) $< $(LIBS) $(CFLAGS)
