CC = g++
CFLAGS  = -I include -Wall -Wextra -Werror 
EFLAGS = -lglfw3 -lopengl32 -lgdi32

PROG = sunset
SRCS = main.cpp shader.cpp model.cpp texture.cpp glad.c
OBJS = main.o shader.o model.o texture.o glad.o

all: ${PROG}

${PROG}: ${OBJS}
	${CC} ${CFLAGS} -o $@ ${OBJS} ${EFLAGS}

%.o: %.cpp
	${CC} ${CFLAGS} -c -o $@ $< ${EFLAGS}
	
%.o: %.c
	${CC} ${CFLAGS}-c -o $@ $< ${EFLAGS}

clean:
	del -f *.o