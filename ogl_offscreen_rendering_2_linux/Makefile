SAMPLE = ogl_offscreen_rendering_2

CC ?= gcc
COPTS ?= -g

LIBS = -lGL -lGLU

all : $(SAMPLE)

clean : 
	rm -f $(SAMPLE)

run : $(SAMPLE)
	./$(SAMPLE)

$(SAMPLE) : $(SAMPLE).cpp
	$(CC) -o $@ $(COPTS) $< $(LIBS)
