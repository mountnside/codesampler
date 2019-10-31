OBJECTS= oglu_glx_dds.o dds.o

CC= gcc

all: oglu_glx_dds

%.o: %.c
	$(CC) -o $@ $<

clean:
	rm -f *.o;rm oglu_glx_dds

oglu_glx_dds: $(OBJECTS)
	$(CC) -g -o oglu_glx_dds $(OBJECTS) -lm -lGL -lGLU
