CC = g++
CFLAGS  = -I include -Wall -Wextra -Werror 
EFLAGS = -lglfw3 -lopengl32 -lgdi32

PROG = sunset
SRCS = main.cpp shader.cpp model.cpp glad.c
OBJS = main.o shader.o model.o glad.o

all: ${PROG}

${PROG}: ${OBJS}
	${CC} ${CFLAGS} -o $@ ${OBJS} ${EFLAGS}

%.o: %.cpp
	${CC} ${CFLAGS} -c -o $@ $< ${EFLAGS}
	
%.o: %.c
	${CC} ${CFLAGS}-c -o $@ $< ${EFLAGS}

clean:
	del -f *.o