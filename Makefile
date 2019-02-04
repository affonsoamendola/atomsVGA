#	Copyright Affonso Amendola 2019					#
#													#
#	Fofonso's Atoms VGA								#
#---------------------------------------------------#

GAMENAME = ATOMS
CC = tcc
RM = rm -f

INCLUDE = include;D:\tc\include
LIB = D:\tc\lib;lib
CCFLAGS = 

all: vga
	
ega:
	$(CC) "-I$(INCLUDE) -L$(LIB) -DEGA -e$(GAMENAME)EGA -l $(CCFLAGS)"  *.c fflibega.lib

vga:
	$(CC) "-I$(INCLUDE) -L$(LIB) -DVGA -e$(GAMENAME)VGA -l $(CCFLAGS)"  *.c fflibvga.lib
	
runega:
	dosbox -conf ~/.dosbox/tcc.conf -c "$(GAMENAME)EGA"

runvga:
	dosbox -conf ~/.dosbox/tcc.conf -c "$(GAMENAME)VGA"

clean:
	$(RM) *.OBJ *.EXE *.LOG *.BAT *.MAP
