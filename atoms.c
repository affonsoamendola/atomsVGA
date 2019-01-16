/*  Copyright 2019 Affonso Amendola
    
    This file is part of AtomsVGA.

    AtomsVGA is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AtomsVGA is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AtomsVGA.  If not, see <https://www.gnu.org/licenses/>
*/

#include <bios.h>
#include <dos.h>
#include <memory.h>
#include <math.h>
#include <time.h>

#include <conio.h>
#include <io.h>

#include <VGA.H>

#define DIR_UP 0
#define DIR_RIGHT 1
#define DIR_DOWN 2
#define DIR_LEFT 3

int game_running = 1;

int * game_board;

int board_size_X = 9;
int board_size_Y = 9;

int chosen_laser_bank;
int chosen_laser_number;

#define SCREEN_RES_X 320
#define SCREEN_RES_Y 240

int reverse_direction(int direction)
{
    switch(direction)
    {
        case DIR_UP:
            return DIR_DOWN;
        
        case DIR_RIGHT:
            return DIR_LEFT;
        
        case DIR_DOWN:
            return DIR_UP;
        
        case DIR_LEFT:
            return DIR_RIGHT;
    }
}

int rotate_direction(int direction, int counter_clock_wise)
{
    //counter_clock_wise supposed to be a boolean, 0 if clockwise, 1 if ccw.

    int rotated_direction;

    rotated_direction = direction + 1 + (counter_clock_wise * (-2));

    if(rotated_direction == -1)
    {
        rotated_direction = 3;
    }

    if(rotated_direction == 4)
    {
        rotated_direction = 0;
    }

    return rotated_direction;
}

void draw_foreground()
{

}

int has_atom(int posX, int posY)
{
}

void draw_board()
{
    int squareSize;

    int i, j;

    int xBoard = 0;
    int yBoard = 0;

    xBoard = (SCREEN_RES_X/2)-(board_size_X*squareSize/2);
    yBoard = (SCREEN_RES_Y/2)-(board_size_Y*squareSize/2);

    squareSize = (SCREEN_RES_Y-40)/board_size_Y;

    for(i = 0; i < board_size_X; i++)
    {
        for(j = 0; j < board_size_Y; j++)
        {
            fill_rectangle( xBoard + i*squareSize, xBoard + (i+1)*squareSize, 
                            yBoard + j*squareSize, yBoard + (j+1)*squareSize, 
                            14);
        }
    } 
}

void create_board(int sizeX, int sizeY, int atomNumber)
{
    game_board = malloc(sizeof(int)*sizeX*sizeY);
}

void shoot_laser(int posX, int posY, int direction)
{
    int going = 1;

    int currentX = posX;
    int currentY = posY;

    int collision_direction;

    while(going)
    {

        reflection_direction = check_collision(currentX, currentY, direction);
        if( reflection_direction != -1 )
        {
            shoot_laser(currentX, currentY, reflection_direction);
            break;
        }

        switch(direction)
        {
            case DIR_UP:
                currentY-=1;
            break;
            case DIR_DOWN:
                currentY+=1;
            break;
            case DIR_RIGHT:
                currentX+=1;
            break;
            case DIR_LEFT:
                currentX-=1;
            break;
        }

        if( (currentX < -1 || currentX > board_size_X) || 
            (currentY < -1 || currentY > board_size_Y) )
        {
            light_detector(currentX, currentY);
            going = 0;
        }
    }
}

int check_collision(int posX, int posY, int direction)
{
    switch(direction)
        case DIR_UP:
            if((has_atom(posX-1, posY-1) && has_atom(posX+1, posY-1)) || has_atom(posX, posY-1))
            {
                return reverse_direction(DIR_UP);
            }
            return -1;
        break;

        case DIR_DOWN:
            if((has_atom(posX-1, posY+1) && has_atom(posX+1, posY+1)) || has_atom(posX, posY+1))
            {
                return reverse_direction(DIR_DOWN);
            }
            return -1;
        break;

         case DIR_RIGHT:
            if((has_atom(posX+1, posY-1) && has_atom(posX+1, posY+1)) || has_atom(posX+1, posY))
            {
                return reverse_direction(DIR_RIGHT);
            }
            return -1;
        break;

        case DIR_LEFT:
            if((has_atom(posX-1, posY-1) && has_atom(posX-1, posY+1)) || has_atom(posX-1, posY))
            {
                return reverse_direction(DIR_LEFT);
            }
            return -1;
        break;

        default:
            return -1;
        break
}

void handle_input()
{
    char input;

    int laser_pos_x;
    int laser_pos_y;
    
    input = getch();

    if(input == 'F' || input == 'f')
    {
        do
        {
            input = getch();
        }
        while(input < 48 || input > 52);

        chosen_laser_bank = (int)input - 48;

        do
        {
            input = getch();
        }
        while(input < 48 || input > 57);

        chosen_laser_number = (int)input - 48;

        if(chosen_laser_bank == 0 || chosen_laser_bank == 2)
        {
            laser_pos_x = chosen_laser_number;

            if(chosen_laser_bank == 0)
            {
                laser_pos_y = -1;
            }
            else
            {
                laser_pos_y = board_size_Y;
            }
        }
        else
        {
            laser_pos_y = chosen_laser_number;

            if(chosen_laser_bank == 3)
            {
                laser_pos_x = -1;
            }
            else
            {
                laser_pos_x = board_size_X;
            }
        }

        shoot_laser(laser_pos_x, laser_pos_y, reverse_direction(chosen_laser_bank));
    }
}

void game_loop()
{
    handle_input();
}

int main()
{
    while(game_running == 1)
    {
        game_loop();
    }
    return 0;
}