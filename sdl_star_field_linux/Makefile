SAMPLE = sdl_star_field
SRCS   = 

SDL_CFLAGS  := $(shell sdl-config --cflags)
SDL_LDFLAGS := $(shell sdl-config --libs)

CC     = gcc
COPTS  = -g -Wall
CFLAGS = $(SDL_CFLAGS)
LIBS   = -lstdc++ -lSDL $(SDL_LDFLAGS) -L/usr/X11R6/lib -lXi

all : $(SAMPLE)

clean : 
	rm -f $(SAMPLE)

run : $(SAMPLE)
	./$(SAMPLE)

$(SAMPLE) : $(SAMPLE).cpp
	$(CC) $(SRCS) -o $@ $(COPTS) $< $(LIBS) $(CFLAGS)
