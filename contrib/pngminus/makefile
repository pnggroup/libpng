# Makefile for PngMinus (png2pnm and pnm2png)
# Linux / Unix

CC=cc -O -n32
LD=cc -O -n32
#CC=gcc -O
#LD=gcc -O
LB=ar
RM=rm
CP=cp

PNGPATH = /usr/local
PNGINC = $(PNGPATH)/include
PNGLIB = $(PNGPATH)/lib -lpng
# PNGLIB = $(PNGPATH)/libpng.a

ZPATH = /usr/local
ZINC = $(ZPATH)/include
ZLIB = $(ZPATH)/lib -lz
# ZLIB = $(ZPATH)/libz.a

CCFLAGS=-I$(PNGINC) -I$(ZINC)
LDFLAGS=-L$(PNGLIB) -L$(ZLIB) -lm
C=.c
O=.o
L=.a
E=

# dependencies

all: png2pnm$(E) pnm2png$(E)

png2pnm$(O): png2pnm$(C)
	$(CC) -c $(CCFLAGS) png2pnm$(C)

png2pnm$(E): png2pnm$(O)
	$(LD) -o png2pnm$(E) png2pnm$(O) $(LDFLAGS)

pnm2png$(O): pnm2png$(C)
	$(CC) -c $(CCFLAGS) pnm2png$(C)

pnm2png$(E): pnm2png$(O)
	$(LD) -o pnm2png$(E) pnm2png$(O) $(LDFLAGS)

clean:
	$(RM) png2pnm$(O)
	$(RM) pnm2png$(O)
	$(RM) png2pnm$(E)
	$(RM) pnm2png$(E)

# End of makefile for png2pnm / pnm2png

