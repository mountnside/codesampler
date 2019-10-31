SAMPLE = ogl_basic_collision

CC ?= gcc
COPTS ?= -g

LIBS = -L/usr/X11R6/lib -lGL -lGLU -lXi -lXmu -lXext -lX11 -lm

all : $(SAMPLE)

clean : 
	rm -f $(SAMPLE)

run : $(SAMPLE)
	./$(SAMPLE)

$(SAMPLE) : $(SAMPLE).cpp
	$(CC) -o $@ $(COPTS) $< $(LIBS)
