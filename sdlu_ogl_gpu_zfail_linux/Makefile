OBJECTS= sdlu_ogl_gpu_zfail.o

CC= gcc

all: sdlu_ogl_gpu_zfail

%.o: %.c
	$(CC) -o $@ $<

clean:
	rm -f *.o;rm sdlu_ogl_gpu_zfail

sdlu_ogl_gpu_zfail: $(OBJECTS)
	$(CC) -g -o sdlu_ogl_gpu_zfail $(OBJECTS) -lm -lGL -lGLU -lSDL
