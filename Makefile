CC = g++
CFLAGS  = -I include -Wall -Wextra -Werror 
EFLAGS = -lglfw3 -lopengl32 -lgdi32

PROG = sunset
OBJS = main.cpp shader.cpp model.cpp glad.c

build: ${PROG}

${PROG}: ${OBJS}
	${CC} ${CFLAGS} -o $@ ${OBJS} ${EFLAGS}

clean:
	rm -f *.o sunset