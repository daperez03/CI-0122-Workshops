# use normal make for this Makefile
#
# Makefile for building user programs to run on top of Nachos
#
# Several things to be aware of:
#
#    Nachos assumes that the location of the program startup routine (the
# 	location the kernel jumps to when the program initially starts up)
#       is at location 0.  This means: start.o must be the first .o passed 
# 	to ld, in order for the routine "Start" to be loaded at location 0
#

# if you are cross-compiling, you need to point to the right executables
# and change the flags to ld and the build procedure for as
#GCCDIR = ~/gnu/local/decstation-ultrix/bin/
GCCDIR = ../bin/
LDFLAGS = -T script -N
ASFLAGS = -mips1
CPPFLAGS = $(INCDIR)


# if you aren't cross-compiling:
#GCCDIR =
#LDFLAGS = -N -T 0
#ASFLAGS =
#CPPFLAGS = -P $(INCDIR)

PATH := ../bin:$(PATH)

CC = $(GCCDIR)mips-gcc
AS = $(GCCDIR)as
LD = $(GCCDIR)mips-ld

CPP = /lib/cpp
INCDIR =-I../userprog -I../threads
# Corrects the error: dangerous relocation GP
CFLAGS = -c $(INCDIR) -mips1 -G 0

all: halt shell matmult sort RW

start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.s > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	/bin/rm strt.s

addrspacetest.o: addrspacetest.c
	$(CC) $(CFLAGS) -c addrspacetest.c
addrspacetest: addrspacetest.o start.o
	$(LD) $(LDFLAGS) start.o addrspacetest.o -o addrspacetest.coff
	../bin/coff2noff addrspacetest.coff addrspacetest

copy.o: copy.c
	$(CC) $(CFLAGS) -c copy.c
copy: copy.o start.o
	$(LD) $(LDFLAGS) start.o copy.o -o copy.coff
	../bin/coff2noff copy.coff copy

pingPong.o: pingPong.c
	$(CC) $(CFLAGS) -c pingPong.c
pingPong: pingPong.o start.o
	$(LD) $(LDFLAGS) start.o pingPong.o -o pingPong.coff
	../bin/coff2noff pingPong.coff pingPong

create.o: create.c
	$(CC) $(CFLAGS) -c create.c
create: create.o start.o
	$(LD) $(LDFLAGS) start.o create.o -o create.coff
	../bin/coff2noff create.coff create

RW.o: RW.c
	$(CC) $(CFLAGS) -c RW.c
RW: RW.o start.o
	$(LD) $(LDFLAGS) start.o RW.o -o RW.coff
	../bin/coff2noff RW.coff RW

socket.o: socket.c
	$(CC) $(CFLAGS) -c socket.c
socket: socket.o start.o
	$(LD) $(LDFLAGS) start.o socket.o -o socket.coff
	../bin/coff2noff socket.coff socket

halt.o: halt.c
	$(CC) $(CFLAGS) -c halt.c
halt: halt.o start.o
	$(LD) $(LDFLAGS) start.o halt.o -o halt.coff
	../bin/coff2noff halt.coff halt

shell.o: shell.c
	$(CC) $(CFLAGS) -c shell.c
shell: shell.o start.o
	$(LD) $(LDFLAGS) start.o shell.o -o shell.coff
	../bin/coff2noff shell.coff shell

sort.o: sort.c
	$(CC) $(CFLAGS) -c sort.c
sort: sort.o start.o
	$(LD) $(LDFLAGS) start.o sort.o -o sort.coff
	../bin/coff2noff sort.coff sort

matmult.o: matmult.c
	$(CC) $(CFLAGS) -c matmult.c
matmult: matmult.o start.o
	$(LD) $(LDFLAGS) start.o matmult.o -o matmult.coff
	../bin/coff2noff matmult.coff matmult

todos.o: todos.c
	$(CC) $(CFLAGS) -c todos.c
todos: todos.o start.o
	$(LD) $(LDFLAGS) start.o todos.o -o todos.coff
	../bin/coff2noff todos.coff todos

clean:
	rm -f *.coff *.o

# Estas reglas sirven para compilar programas simples, que consistan en un unico fuente
# Las reglas anteriores para construir los ejecutables
# halt, shell, sort y matmult se podrian suprimir

%: %.c 
	make start.o 
	$(CC) $(CFLAGS) -c $<
	$(LD) $(LDFLAGS) start.o $*.o -o $*.coff
	../bin/coff2noff $*.coff $@
	

