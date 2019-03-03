LIBTCODDIR=libtcod-1.5.1
SDLDIR=SDL-1.2.14
CFLAGS=$(FLAGS) -I$(LIBTCODDIR)/include -I$(SDLDIR)/include -IUnBrogueCode -IPlatformCode -DBROGUE-TCOD -Wall

%.o : %.c
	gcc $(CFLAGS) -O2 -s -o $@ -c $< 

OBJS=UnBrogueCode/Architect.o \
	UnBrogueCode/Combat.o \
	UnBrogueCode/Dijkstra.o \
	UnBrogueCode/Globals.o \
	UnBrogueCode/IO.o \
	UnBrogueCode/Buttons.o \
	UnBrogueCode/MainMenu.o \
	UnBrogueCode/Items.o \
	UnBrogueCode/Light.o \
	UnBrogueCode/Monsters.o \
	UnBrogueCode/Movement.o \
	UnBrogueCode/RogueMain.o \
	UnBrogueCode/Random.o \
	UnBrogueCode/Recordings.o \
	PlatformCode/main.o \
	PlatformCode/platformdependent.o 
	PlatformCode/tcod-platform.o 

all : UnBrogue

UnBrogue-icon.o : UnBrogue-icon.ico icon.rc
	windres icon.rc UnBrogue-icon.o

UnBrogue : ${OBJS} UnBrogue-icon.o 
	g++ -o UnBrogue.exe ${OBJS} UnBrogue-icon.o -L. -ltcod-mingw -lSDL.dll -mwindows
