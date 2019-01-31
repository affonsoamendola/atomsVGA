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

all: ega
all: vga
	
ega:
	$(CC) "-I$(INCLUDE) -L$(LIB) -DEGA -e$(GAMENAME)EGA -l $(CCFLAGS)"  *.c libega.lib
	cp $(GAMENAME)EGA.EXE ./release/
	cp -r ./graphix ./release/

vga:
	$(CC) "-I$(INCLUDE) -L$(LIB) -DVGA -e$(GAMENAME)VGA -l $(CCFLAGS)"  *.c libVGA87.lib
	cp $(GAMENAME)VGA.EXE ./release/
	cp -r ./graphix ./release/
	
runega:
	dosbox -conf ~/.dosbox/tcc.conf -c "$(GAMENAME)EGA"

runvga:
	dosbox -conf ~/.dosbox/tcc.conf -c "$(GAMENAME)VGA"

clean:
	$(RM) *.OBJ *.EXE *.LOG *.BAT *.MAP

cleanall:
	$(RM) *.OBJ *.EXE *.LOG *.BAT *.MAP
	$(RM) release/$(GAMENAME).EXE
	$(RM) -r release/graphix