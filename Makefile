#	Copyright Affonso Amendola 2019					#
#													#
#	Fofonso's Atoms VGA								#
#---------------------------------------------------#

GAMENAME = ATOMSVGA
CC = tcc
RM = rm -f

INCLUDE = include;D:\tc\include
LIB = D:\tc\lib
CCFLAGS = 

all: $(GAMENAME).exe

$(GAMENAME).exe:
	$(CC) "-I$(INCLUDE) -L$(LIB) -e$(GAMENAME) $(CCFLAGS)"  *.c 
	mkdir release
	mkdir ./release/graphix
	cp $(GAMENAME).EXE ./release/
	cp -r ./graphix ./release
	
run:
	dosbox -conf ~/.dosbox/tcc.conf -c "$(GAMENAME)"

clean:
	$(RM) *.OBJ *.EXE *.LOG *.BAT

cleanall:
	$(RM) *.OBJ *.EXE *.LOG *.BAT
	$(RM) -r release