CC = gcc -g
INCLUDE = -I. -Iinclude/ -I/usr/include
GTK2_FLAGS=`pkg-config --cflags --libs gtk+-2.0 gthread-2.0`
XINE_LIBS= `pkg-config --cflags --libs libxine gthread-2.0`
X_LIBS= -lXtst
CFLAGS = -g -Wall $(GTK2_FLAGS)
LIBS = -L. -L/usr/lib $(X_LIBS) $(XINE_LIBS)

SRC = lxine.c callbacks.c gtkxine.c mixer.c actions.c
HEADERS = globals.h
OBJ = $(SRC:.c=.o)

OUTPUT_OPTION = 
COMPILE = $(CC) $(CFLAGS) $(INCLUDE) $(LIBS)

.c.o:
	$(COMPILE) -c $< $(OUTPUT_OPTION)

all: $(OBJ)
	$(COMPILE) -o lxine $?
