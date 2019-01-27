#!/usr/bin/python

#ARGUMENTS:
#python ppmtoega.py [filein (with extension, expecting a .ppm file, with no comments)] ...
#					[fileout (without extension, creates a .pgm file, with the EGA pallete values)

import sys

def read_uint8(currentLine, posInLine, separator, returnEndPosition):

	out = 0
	done = False
	middleOfNumber = False
	j = posInLine

	while(not(done)):
		currentChar = currentLine[j]
		if((currentChar != '0' or middleOfNumber == True) and currentChar != separator):
			out = out*10 + (int(currentChar))
			middleOfNumber = True	
		elif(currentChar == '0' or currentChar == separator):
			done = True
		j = j+1

	if(returnEndPosition == True):
		return out, j
	else:
		return out

def color_check(colorA, colorB):

	if(colorA[0] <= colorB[0] + error_margin and colorA[0] > colorB[0] - error_margin and
	   colorA[1] <= colorB[1] + error_margin and colorA[1] > colorB[1] - error_margin and
	   colorA[2] <= colorB[2] + error_margin and colorA[2] > colorB[2] - error_margin):
		return True
	else:
		return False

filename = sys.argv[1]
error_margin = 30

fileout = sys.argv[2]

f = open(filename,"r")
foutpgm = open(fileout + ".pgm", "w")

foutpgm.write("P2\n")

filetype = f.readline()[1]

if(filetype != '3'):
	print("UNKNOWN FORMAT")
	exit()

currentLine = f.readline()

sizeX = 0
sizeY = 0
i = 0

middleOfNumber = False
done = False

while(not(done)):
	currentChar = currentLine[i]

	if((currentChar != '0' or middleOfNumber == True) and currentChar != ' '):
		sizeX = sizeX*10 + (int(currentChar))
		middleOfNumber = True	
	elif(currentChar == ' '):
		done = True
	i = i+1

middleOfNumber = False
done = False
while(not(done)):
	currentChar = currentLine[i]

	if((currentChar != '0' or middleOfNumber == True) and currentChar != '\n'):
		sizeY = sizeY*10 + (int(currentChar))
		middleOfNumber = True	
	elif(currentChar == '\n'):
		done = True
	i = i+1

f.readline()

foutpgm.write(str(sizeX)+' '+ str(sizeY)+'\n')
foutpgm.write("255\n")

r = 0
g = 0
b = 0

i = 0
j = 0

middleOfNumber = False
done = False

storage = []

for y in range(sizeY):
	storage.append([])
	for x in range(sizeX):
		storage[y].append(0)

ega_pallette = [[0x00, 0x00, 0x00], [0x00, 0x00, 0xAA], [0x00, 0xAA, 0x00], [0x00, 0xAA, 0xAA], [0xAA, 0x00, 0x00], [0xAA, 0x00, 0xAA], [0xAA, 0x55, 0x00], [0xAA, 0xAA, 0xAA],
				[0x55, 0x55, 0x55], [0x55, 0x55, 0xFF], [0x55, 0xFF, 0x55], [0x55, 0xFF, 0xFF], [0xFF, 0x55, 0x55], [0xFF, 0x55, 0xFF], [0xFF, 0xFF, 0x55], [0xFF, 0xFF, 0xFF]]

for y in range(sizeY):
	for x in range(sizeX):

		currentLine = f.readline()
		r = read_uint8(currentLine, 0, '\n', False)

		currentLine = f.readline()
		g = read_uint8(currentLine, 0, '\n', False)

		currentLine = f.readline()
		b = read_uint8(currentLine, 0, '\n', False)

		color = [r,g,b]

		color_index = 0
		for c in ega_pallette:
			if(color_check(color, c)):
				storage[y][x] = color_index
			color_index += 1

for x in range(sizeX):
	for y in range(sizeY):
		foutpgm.write(str(storage[y][x]))
		foutpgm.write('\n')




	

