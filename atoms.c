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

#define true 1
#define false 0

#define TILE_DET_ON 7
#define TILE_DET_OFF 6
#define TILE_LASER_OFF 8
#define TILE_LASER_ON 9
#define TILE_LASER_REC 10

typedef int bool;

bool game_running = 1;

int * game_board;

int board_size_X = 5;
int board_size_Y = 5;

int board_pos_x = 0;
int board_pos_y = 0;

int atom_number = 0;
int atoms_left = 10;

int board_square_size;

int chosen_laser_bank;
int chosen_laser_number;

bool fired_laser = 0;

int shooting_laser_x;
int shooting_laser_y;

bool detector_hit = 0;

int detector_hit_x;
int detector_hit_y;

int cursor_pos_x = 0;
int cursor_pos_y = 0;

int current_difficulty = 0;

unsigned char far * titlescreen_location = 0xA0004B00L;
unsigned char far * tilemap_location = 0xA0009600L;

#define SCREEN_RES_X 320
#define SCREEN_RES_Y 240

#define HAS_ATOM        1
#define HAS_FLAG        2
#define HAS_QUESTION    4

void toggle_atom(int posX, int posY)
{
    *(game_board+posX+posY*board_size_X) ^= HAS_ATOM;
}

bool has_atom(int posX, int posY)
{
    if( (posX < 0 || posX > (board_size_X-1)) ||
        (posY < 0 || posY > (board_size_Y-1)) )
    {
        return 0;
    }

    return *(game_board+posX+posY*board_size_X) & HAS_ATOM;
}

void toggle_flag(int posX, int posY)
{
    if(*(game_board+posX+posY*board_size_X) & HAS_QUESTION)
    {
        *(game_board+posX+posY*board_size_X) ^= HAS_QUESTION;
    }

    *(game_board+posX+posY*board_size_X) ^= HAS_FLAG;
}

bool has_flag(int posX, int posY)
{
    if( (posX < 0 || posX > (board_size_X-1)) ||
        (posY < 0 || posY > (board_size_Y-1)) )
    {
        return 0;
    }

    return *(game_board+posX+posY*board_size_X) & HAS_FLAG;
}

void toggle_question(int posX, int posY)
{
    if(*(game_board+posX+posY*board_size_X) & HAS_FLAG)
    {
        *(game_board+posX+posY*board_size_X) ^= HAS_FLAG;
    }

    *(game_board+posX+posY*board_size_X) ^= HAS_QUESTION;
}

bool has_question(int posX, int posY)
{
    if( (posX < 0 || posX > (board_size_X-1)) ||
        (posY < 0 || posY > (board_size_Y-1)) )
    {
        return 0;
    }

    return *(game_board+posX+posY*board_size_X) & HAS_QUESTION;
}

void populate_board(int atomNumber)
{
    int i;

    bool searching = true;

    int x, y;

    for(i=0; i<atomNumber; i++)
    {
        do
        {
            x = rand()%(board_size_X-1);
            y = rand()%(board_size_Y-1);

            if(!has_atom(x, y))
            {
                toggle_atom(x, y);
                searching = false;
            }
        }
        while(searching);
    }
}

void draw_tile(int posX, int posY, int tileX, int tileY)
{
    copy_vmem_to_dbuffer(   
                            tilemap_location, 
                            board_pos_x + posX*16, board_pos_y + posY*16,
                            16*tileX, (16*(tileX+1)-1),
                            16*tileY, (16*(tileY+1)-1),
                            64
                        );
}

void victory_screen()
{
    unsigned char* color_pallette;
    unsigned char* color_temp;

    int x, y;

    fill_screen(0);

    flip_front_page();

    load_pallette("graphix/rainbow.plt", 256);

    for(x=0;x<320;x++)
    {
        for(y=0;y<240;y++)
        {
            set_pixel(x,y,(int)( ((float)(y)*(float)(x)) * (255.0 / (320.0*240.0) )  ));
        }
    }

    fill_rectangle(0,320,92,136,150);

    print_string(10,96,30,"GAME OVER",1);
    print_string(10,106,30,"THANK YOU FOR PLAYING",1);

    print_string(10,126,30,"PRESS ANY KEY TO RETURN TO DOS",1);

    flip_front_page();

    color_pallette = malloc(255*3);
    color_temp = malloc(1*3);

    while(!kbhit())
    {

        delay(50);  
        
        get_pallette(color_pallette,1,255); 
        get_pallette(color_temp,0,0);
        set_pallette(color_pallette,0,254);
        set_pallette(color_temp,255,255);
        
    }

    free(color_pallette);
    free(color_temp);
}

void check_win()
{
    bool win = true;
    int i, j;

    for (i = 0; i < board_size_X; i++)
    {
        for (j = 0; j < board_size_Y; j++)
        {
            if(has_atom(i, j) && !has_flag(i, j))
            {
                win = false;
            }
        }
    }

    if(win == true)
    {
        victory_screen();
        game_running = false;
    }
}

void init_game()
{
    int i, j;

    srand(time(NULL));

    if(current_difficulty == 0)
    {
        board_size_X = 7;
        board_size_Y = 7;
        atom_number = 5;
    }
    else if(current_difficulty == 1)
    {
        board_size_X = 8;
        board_size_Y = 8;
        atom_number = 7;
    }
    else if(current_difficulty == 2)
    {
        board_size_X = 9;
        board_size_Y = 9;
        atom_number = 10;
    }
    else if(current_difficulty == 3)
    {
        board_size_X = 10;
        board_size_Y = 10;
        atom_number = 14;
    }

    atoms_left = atom_number;
    game_board = malloc(sizeof(int)*board_size_X*board_size_Y);

    for (i = 0; i < board_size_X; i++)
    {
        for (j = 0; j < board_size_Y; j++)
        {
            *(game_board + i + j*board_size_X) = 0;
        }
    }

    board_square_size = 16;

    board_pos_x = (SCREEN_RES_X/2)-(board_size_X*board_square_size/2);
    board_pos_y = (SCREEN_RES_Y/2)-(board_size_Y*board_square_size/2);

    load_pgm("graphix/TileMap.pgm", tilemap_location, 64, 208);
    load_pallette("graphix/TileMap.plt", 40);

    populate_board(atom_number);
}

void kill_game()
{
    set_graphics_mode(TEXT_MODE);
}

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

        default:
            return -1;
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

void draw_background()
{
    int x, y;

    fill_screen(0);

    for(x = -2; x <= board_size_X+1; x++)
    {
        for(y = -2; y <= board_size_Y+1; y++)
        {
            //DETECTORS//
            //DETECTORS UP
            if(y == -2 && x != -2 && x != board_size_X+1)
            {
                draw_tile(  x, y,
                            DIR_UP, TILE_DET_OFF);
            }

            //DETECTORS RIGHT
            if(x == board_size_X+1 && y != -2 && y != board_size_Y+1)
            {
                draw_tile(  x, y,
                            DIR_RIGHT, TILE_DET_OFF);
            }

             //DETECTORS DOWN
            if(y == board_size_Y+1 && x != -2 && x != board_size_X+1)
            {
                draw_tile(  x, y,
                            DIR_DOWN, TILE_DET_OFF);
            }

            //DETECTORS LEFT
            if(x == -2 && y != -2 && y != board_size_Y+1)
            {
                draw_tile(  x, y,
                            DIR_LEFT, TILE_DET_OFF);
            }
            //=====================
            //LASERS//
            //TOPLEFT
            if(y == -1 && x == -1)
            {
                draw_tile(  x, y,
                            DIR_UP, 11);
            }
            //TOPRIGHT
            if(y == -1 && x == board_size_X)
            {
                draw_tile(  x, y,
                            DIR_RIGHT, 11);
            }
            //DOWNRIGHT
            if(y == board_size_Y && x == board_size_X)
            {
                draw_tile(  x, y,
                            DIR_DOWN, 11);
            }
            //TOPRIGHT
            if(y == board_size_Y && x == -1)
            {
                draw_tile(  x, y,
                            DIR_LEFT, 11);
            }

            //LASER TOP
            if(y == -1 && (x >= 0 && x < board_size_X))
            {
                draw_tile(  x, y,
                            DIR_UP, TILE_LASER_OFF);
            }

            //LASERRIGHT
            if(x == board_size_X && (y >= 0 && y < board_size_Y))
            {
                draw_tile(  x, y,
                            DIR_RIGHT, TILE_LASER_OFF);
            }

             //LASER DOWN
            if(y == board_size_Y && (x >= 0 && x < board_size_X))
            {
                draw_tile(  x, y,
                            DIR_DOWN, TILE_LASER_OFF);
            }

            //LASER LEFT
            if(x == -1 && (y >= 0 && y < board_size_Y))
            {
                draw_tile(  x, y,
                            DIR_LEFT, TILE_LASER_OFF);
            }
            //=====================================
            //BOARD
            //CORNERS
            //TOPLEFT
            if(x == 0 && y == 0)
            {
                draw_tile(  x, y,
                            DIR_UP, 3);
            }
            //TOPRIGHT
            if(x == board_size_X-1 && y == 0)
            {
                draw_tile(  x, y,
                            DIR_RIGHT, 3);
            }
            //DOWNRIGHT
            if(x == board_size_X-1 && y == board_size_Y-1)
            {
                draw_tile(  x, y,
                            DIR_DOWN, 3);
            }
            //DOWNLEFT
            if(x == 0 && y == board_size_Y-1)
            {
                draw_tile(  x, y,
                            DIR_LEFT, 3);
            }
            //EDGES
            //TOP
            if(y == 0 && (x >= 1 && x < board_size_X-1))
            {
                draw_tile(  x, y,
                            DIR_UP, 1);
            }
            //RIGHT
            if(x == board_size_X-1 && (y >= 1 && y < board_size_Y-1))
            {
                draw_tile(  x, y,
                            DIR_RIGHT, 1);
            }
            //DOWN
            if(y == board_size_Y-1 && (x >= 1 && x < board_size_X-1))
            {
                draw_tile(  x, y,
                            DIR_DOWN, 1);
            }
            //LEFT
            if(x == 0 && (y >= 1 && y < board_size_Y-1))
            {
                draw_tile(  x, y,
                            DIR_LEFT, 1);
            }
            //MIDDLE
            if( x >= 1 && x <= board_size_X-2 &&
                y >= 1 && y <= board_size_Y-2)
            {
                draw_tile(  x, y,
                            0, 0);
            }
        }
    } 
}

void draw_foreground()
{

    int x, y;
    char buffer [10];

    copy_vmem_to_dbuffer(   tilemap_location, 
                            266, 
                            216, 
                            32,
                            47,
                            0,
                            15,
                            64);

    print_string(282, 220, 10, ":", 0);
    print_string(290, 220, 10,(char*) itoa(atoms_left, buffer, 10), 0);

    if(detector_hit)
    {
        if(detector_hit_y == -2)
        {
            draw_tile(detector_hit_x, detector_hit_y, DIR_UP, TILE_DET_ON);

            if(detector_hit_x >= 0 && detector_hit_x <= board_size_X-1)
            {
                draw_tile(detector_hit_x, detector_hit_y+1, DIR_UP, TILE_LASER_REC);

                if(detector_hit_x == 0)
                {
                    draw_tile(detector_hit_x, detector_hit_y+2, 0, 4);
                }
                else if (detector_hit_x == board_size_X-1)
                {
                    draw_tile(detector_hit_x, detector_hit_y+2, 1, 5);
                }
                else
                {
                    draw_tile(detector_hit_x, detector_hit_y+2, 0, 2);
                }
            }
        }
        else if(detector_hit_x == board_size_X+1)
        {
            draw_tile(detector_hit_x, detector_hit_y, DIR_RIGHT, TILE_DET_ON);

            if(detector_hit_y >= 0 && detector_hit_y <= board_size_Y-1)
            {
                draw_tile(detector_hit_x-1, detector_hit_y, DIR_RIGHT, TILE_LASER_REC);

                if(detector_hit_y == 0)
                {
                    draw_tile(detector_hit_x-2, detector_hit_y, 1, 4);
                }
                else if (detector_hit_y == board_size_Y-1)
                {
                    draw_tile(detector_hit_x-2, detector_hit_y, 2, 5);
                }
                else
                {
                    draw_tile(detector_hit_x-2, detector_hit_y, 1, 2);
                }
            }
        }
        else if(detector_hit_y == board_size_Y+1)
        {
            draw_tile(detector_hit_x, detector_hit_y, DIR_DOWN, TILE_DET_ON);

            if(detector_hit_x >= 0 && detector_hit_x <= board_size_X-1)
            {
                draw_tile(detector_hit_x, detector_hit_y-1, DIR_DOWN, TILE_LASER_REC);

                if(detector_hit_x == 0)
                {
                    draw_tile(detector_hit_x, detector_hit_y-2, 3, 5);
                }
                else if (detector_hit_x == board_size_X-1)
                {
                    draw_tile(detector_hit_x, detector_hit_y-2, 2, 4);
                }
                else
                {
                    draw_tile(detector_hit_x, detector_hit_y-2, 2, 2);
                }
            }
        }
        else if(detector_hit_x == -2)
        {
            draw_tile(detector_hit_x, detector_hit_y, DIR_LEFT, TILE_DET_ON);

            if(detector_hit_y >= 0 && detector_hit_y <= board_size_Y-1)
            {
                draw_tile(detector_hit_x+1, detector_hit_y, DIR_LEFT, TILE_LASER_REC);

                if(detector_hit_y == 0)
                {
                    draw_tile(detector_hit_x+2, detector_hit_y, 0, 5);
                }
                else if (detector_hit_y == board_size_Y-1)
                {
                    draw_tile(detector_hit_x+2, detector_hit_y, 3, 4);
                }
                else
                {
                    draw_tile(detector_hit_x+2, detector_hit_y, 3, 2);
                }
            }
        }
    }

    if(fired_laser)
    {
        if(shooting_laser_y == -1)
        {
            draw_tile(shooting_laser_x, shooting_laser_y, DIR_UP, TILE_LASER_ON);

            if(shooting_laser_x == 0)
            {
                if(detector_hit && detector_hit_x == -2 && detector_hit_y == 0)
                {
                    draw_tile(shooting_laser_x, shooting_laser_y+1, 0, 12);
                }
                else
                {
                    draw_tile(shooting_laser_x, shooting_laser_y+1, 0, 4);
                }
            }
            else if (shooting_laser_x == board_size_X-1)
            {
                if(detector_hit && detector_hit_x == board_size_X+1 && detector_hit_y == 0)
                {
                    draw_tile(shooting_laser_x, shooting_laser_y+1, 1, 12);
                }
                else
                {
                    draw_tile(shooting_laser_x, shooting_laser_y+1, 1, 5);
                }
            }
            else
            {
                draw_tile(shooting_laser_x, shooting_laser_y+1, 0, 2);  
            }
        }
        else if (shooting_laser_x == board_size_X)
        {
            draw_tile(shooting_laser_x, shooting_laser_y, DIR_RIGHT, TILE_LASER_ON);
        
            if(shooting_laser_y == 0)
            {
                if(detector_hit && detector_hit_y == -2 && detector_hit_x == board_size_X-1)
                {
                    draw_tile(shooting_laser_x-1, shooting_laser_y, 1, 12);
                }
                else
                {
                    draw_tile(shooting_laser_x-1, shooting_laser_y, 1, 4);  
                }
            }
            else if (shooting_laser_y == board_size_Y-1)
            {
                if(detector_hit && detector_hit_y == board_size_Y+1 && detector_hit_x == board_size_X-1)
                {
                    draw_tile(shooting_laser_x-1, shooting_laser_y, 2, 12);
                }
                else
                {
                    draw_tile(shooting_laser_x-1, shooting_laser_y, 2, 5);
                }
            }
            else
            {
                draw_tile(shooting_laser_x-1, shooting_laser_y, 1, 2);
            }
        }
        else if (shooting_laser_y == board_size_Y)
        {
            draw_tile(shooting_laser_x, shooting_laser_y, DIR_DOWN, TILE_LASER_ON);

            if(shooting_laser_x == 0)
            {
                if(detector_hit && detector_hit_y == board_size_Y-1 && detector_hit_x == -2)
                {
                    draw_tile(shooting_laser_x, shooting_laser_y-1, 3, 12);
                }
                else
                {
                    draw_tile(shooting_laser_x, shooting_laser_y-1, 3, 5);
                }
            }
            else if (shooting_laser_x == board_size_X-1)
            {
                if(detector_hit && detector_hit_y == board_size_Y-1 && detector_hit_x == board_size_X+1)
                {
                    draw_tile(shooting_laser_x, shooting_laser_y-1, 2, 12);
                }
                else
                {
                    draw_tile(shooting_laser_x, shooting_laser_y-1, 2, 4);
                }
            }
            else
            {
                draw_tile(shooting_laser_x, shooting_laser_y-1, 2, 2);
            }
        }
        else if (shooting_laser_x == -1)
        {
            draw_tile(shooting_laser_x, shooting_laser_y, DIR_LEFT, TILE_LASER_ON);

            if(shooting_laser_y == 0)
            {
                if(detector_hit && detector_hit_y == -2 && detector_hit_x == 0)
                {
                    draw_tile(shooting_laser_x+1, shooting_laser_y, 0, 12);
                }
                else
                {
                    draw_tile(shooting_laser_x+1, shooting_laser_y, 0, 5);
                }
            }
            else if (shooting_laser_y == board_size_Y-1)
            {
                if(detector_hit && detector_hit_y == board_size_Y+1 && detector_hit_x == 0)
                {
                    draw_tile(shooting_laser_x+1, shooting_laser_y, 3, 12);
                }
                else
                {
                    draw_tile(shooting_laser_x+1, shooting_laser_y, 3, 4);
                }
            }
            else
            {
                draw_tile(shooting_laser_x+1, shooting_laser_y, 3, 2);
            }
        }
    }

    for(x = 0; x < board_size_X; x++)
    {
        for(y = 0; y < board_size_Y; y++)
        {
            if(has_flag(x, y))
            {
                draw_tile(x, y, 2, 0);
            }

            if(has_question(x, y))
            {
                draw_tile(x, y, 3, 0);
            }
        }
    }

    draw_tile(cursor_pos_x, cursor_pos_y, 1, 0);

    flip_front_page();
}

void laser(int posX, int posY, int direction)
{
    bool going = true;

    int currentX = posX;
    int currentY = posY;

    int reflection_direction;

    while(going)
    {

        set_pixel(currentX+10, currentY+10,60);
        
        reflection_direction = check_collision(currentX, currentY, direction);
        
        if( reflection_direction != -1 )
        {
            laser(currentX, currentY, reflection_direction);
            going = false;
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
            detector_hit_x = currentX;
            detector_hit_y = currentY;

            detector_hit = true;

            going = false;
        }
    }
}

void shoot_laser(int posX, int posY, int direction)
{
    fired_laser = true;

    shooting_laser_x = posX;
    shooting_laser_y = posY;

    laser(posX, posY, direction);
}

int check_collision(int posX, int posY, int direction)
{
    switch(direction)
    {
        
        case DIR_UP:
            if(has_atom(posX-1, posY-1) && has_atom(posX+1, posY-1))
            {
                return DIR_DOWN;
            }

            if(has_atom(posX-1, posY-1))
            {
                return DIR_RIGHT;
            }

            if(has_atom(posX+1, posY-1))
            {
                return DIR_LEFT;
            }

            if(has_atom(posX, posY-1))
            {
                return DIR_DOWN;
            }
            return -1;
        
        case DIR_DOWN:
            if(has_atom(posX-1, posY+1) && has_atom(posX+1, posY+1))
            {
                return DIR_UP;
            }

            if(has_atom(posX-1, posY+1))
            {
                return DIR_RIGHT;
            }

            if(has_atom(posX+1, posY+1))
            {
                return DIR_LEFT;
            }

            if(has_atom(posX, posY+1))
            {
                return DIR_UP;
            }
            return -1;

         case DIR_RIGHT:
            if(has_atom(posX+1, posY-1) && has_atom(posX+1, posY+1))
            {
                return DIR_LEFT;
            }

            if(has_atom(posX+1, posY-1))
            {
                return DIR_DOWN;
            }

            if(has_atom(posX+1, posY+1))
            {
                return DIR_UP;
            }

            if(has_atom(posX+1, posY))
            {
                return DIR_LEFT;
            }
            return -1;

        case DIR_LEFT:
            if(has_atom(posX-1, posY-1) && has_atom(posX-1, posY+1))
            {
                return DIR_RIGHT;
            }

            if(has_atom(posX-1, posY-1))
            {
                return DIR_DOWN;
            }

            if(has_atom(posX-1, posY+1))
            {
                return DIR_UP;
            }

            if(has_atom(posX-1, posY))
            {
                return DIR_RIGHT;
            }
            return -1;

        default:
            return -1;
    }
}

void handle_input()
{
    char input;

    fired_laser = false;
    detector_hit = false;

    input = getch();

    if(input == 'w' || input == 'W')
    {
        if(cursor_pos_x >= -1 && cursor_pos_x <= board_size_X)
        {
            if(cursor_pos_y > -1)
            {
                cursor_pos_y -= 1;
            }
        }
    }
    else if (input == 'd' || input == 'D')
    {
        if(cursor_pos_y >= -1 && cursor_pos_y <= board_size_Y)
        {
            if(cursor_pos_x < board_size_X)
            {
                cursor_pos_x += 1;
            }
        }
    }
    else if(input == 's' || input == 'S')
    {
        if(cursor_pos_x >= -1 && cursor_pos_x <= board_size_X)
        {
            if(cursor_pos_y < board_size_Y)
            {
                cursor_pos_y += 1;
            }
        }
    }
    else if (input == 'a' || input == 'A')
    {
        if(cursor_pos_y >= -1 && cursor_pos_y <= board_size_Y)
        {
            if(cursor_pos_x > -1)
            {
                cursor_pos_x -= 1;
            }
        }
    }
    else if(input == 'j' || input == 'J')
    {
        if(cursor_pos_y == -1 && cursor_pos_x >= 0 && cursor_pos_x <= board_size_X-1)
        {
            shoot_laser(cursor_pos_x, cursor_pos_y, DIR_DOWN);
        }
        else if(cursor_pos_x == board_size_X && cursor_pos_y >= 0 && cursor_pos_y <= board_size_Y-1)
        {
            shoot_laser(cursor_pos_x, cursor_pos_y, DIR_LEFT);
        }
        else if(cursor_pos_y == board_size_Y && cursor_pos_x >= 0 && cursor_pos_x <= board_size_X-1)
        {
            shoot_laser(cursor_pos_x, cursor_pos_y, DIR_UP);
        }
        else if(cursor_pos_x == -1 && cursor_pos_y >= 0 && cursor_pos_y <= board_size_Y-1)
        {
            shoot_laser(cursor_pos_x, cursor_pos_y, DIR_RIGHT);
        }
        else if(cursor_pos_x >= 0 && cursor_pos_x <= board_size_X-1 &&
                cursor_pos_y >= 0 && cursor_pos_y <= board_size_Y-1)
        {
            if(!has_question(cursor_pos_x, cursor_pos_y) && !has_flag(cursor_pos_x, cursor_pos_y))
            {
                toggle_question(cursor_pos_x, cursor_pos_y);
            }
            else if(has_question(cursor_pos_x, cursor_pos_y) && atoms_left > 0)
            {
                atoms_left -= 1;
                toggle_flag(cursor_pos_x, cursor_pos_y);
            }
            else if(has_flag(cursor_pos_x, cursor_pos_y))
            {
                atoms_left += 1; 
                toggle_flag(cursor_pos_x, cursor_pos_y);
            }
            else if(has_question(cursor_pos_x, cursor_pos_y))
            {
                toggle_question(cursor_pos_x, cursor_pos_y);
            }
        }
    }
}

void game_loop()
{
    draw_background();
    draw_foreground();

    handle_input();
    check_win();
}

void splash_screen()
{
    load_pgm("graphix/ttloff.pgm", titlescreen_location, 320, 240);
    load_pallette("graphix/ttloff.plt", 5);

    flip_front_page();
    delay(2000);

    load_pgm("graphix/ttlon.pgm", titlescreen_location, 320, 240);
    load_pallette("graphix/ttlon.plt", 11);

    flip_front_page();
    delay(1500);

    load_pgm("graphix/ttlon2.pgm", titlescreen_location, 320, 240);
    load_pallette("graphix/ttlon2.plt", 14);

    flip_front_page();
    delay(10);

    load_pgm("graphix/ttlon.pgm", titlescreen_location, 320, 240);
    load_pallette("graphix/ttlon.plt", 11);

    flip_front_page();

    delay(1500);

    fill_screen(0);
    flip_front_page();
}

void print_string_centralized(int y, int color, char *string, int strlen, int transparent)
{
    print_string(SCREEN_RES_X/2-strlen*4, y, color, string, transparent);
}

void credits()
{
    fill_screen(0);

    load_pgm("graphix/handsome.pgm", tilemap_location, 100, 100);
    load_pallette("graphix/handsome.plt", 143);

    print_string_centralized(30, 30, "This fine piece of software was made", 36, 0);
    print_string_centralized(40, 30, "possible by the following individuals:", 38, 0);

    copy_vmem_to_dbuffer(   tilemap_location, 
                            SCREEN_RES_X/3-50-26, 
                            SCREEN_RES_Y/2-50, 
                            0, 
                            99,
                            0,
                            99,
                            100);

    print_string(SCREEN_RES_X/3-64-26, 175, 10, "Affonso Amendola", 0);
    print_string(SCREEN_RES_X/3-64-26, 185, 30, " Handsome Devil ", 0);
    print_string(SCREEN_RES_X/3-64-26, 195, 30, " Programmer/Art ", 0);

    print_string(SCREEN_RES_X*2/3-64+26, 175, 10, "Rafael  Flauzino", 0);
    print_string(SCREEN_RES_X*2/3-64+26, 185, 30, " Bearded Master ", 0);
    print_string(SCREEN_RES_X*2/3-64+26, 195, 30, "  Sound/ Music  ", 0);

    print_string_centralized(230, 40, "Press any key to return to the menu", 35, 0);

    flip_front_page();

    getch();

    fill_screen(0);
    flip_front_page();

    load_pallette("graphix/menu.plt", 10);
    load_pgm("graphix/menu.pgm", tilemap_location, 320, 240);
}

void main_menu()
{
    int cursor_position = 0;

    int selected_color = 10;
    int not_selected_color = 30;

    char input;

    load_pallette("graphix/menu.plt", 10);
    load_pgm("graphix/menu.pgm", tilemap_location, 320, 240);

    while(game_running)
    {
        fill_screen(0);

        copy_vmem_to_dbuffer_latched (  tilemap_location,
                                        80*240,
                                        0);

        if(cursor_position == 0)
        {
            print_string_centralized(120, selected_color, "NEW GAME", 8, 0);
        }
        else
        {
            print_string_centralized(120, not_selected_color, "NEW GAME", 8, 0);
        }

        if(cursor_position == 1)
        {
            print_string_centralized(140, selected_color, "DIFFICULTY", 10, 0);
        }
        else
        {
            print_string_centralized(140, not_selected_color, "DIFFICULTY", 10, 0);
        }

        if (current_difficulty == 0)
        {
            print_string_centralized(150, not_selected_color, "EASY", 4, 0);
        }
        else if (current_difficulty == 1)
        {
            print_string_centralized(150, not_selected_color, "MEDIUM", 6, 0);
        }   
        else if (current_difficulty == 2)
        {
            print_string_centralized(150, not_selected_color, "HARD", 4, 0);
        }
        else if (current_difficulty == 3)
        {
            print_string_centralized(150, not_selected_color, "IMPOSSIBLE", 10, 0);
        }

        if(cursor_position == 2)
        {
            print_string_centralized(170, selected_color, "HOW TO PLAY", 11, 0);
        }
        else
        {
            print_string_centralized(170, not_selected_color, "HOW TO PLAY", 11, 0);
        }

        if(cursor_position == 3)
        {
            print_string_centralized(180, selected_color, "CREDITS", 7, 0);
        }
        else
        {
            print_string_centralized(180, not_selected_color, "CREDITS", 7, 0);
        }

        if(cursor_position == 4)
        {
            print_string_centralized(200, selected_color, "QUIT", 4, 0);
        }
        else
        {
            print_string_centralized(200, not_selected_color, "QUIT", 4, 0);
        }

        print_string_centralized(230, 40, "By Affonso Amendola, 2019", 24, 0);

        flip_front_page();
        
        input = getch();

        if(input == 'j' || input == 'J')
        {
            if(cursor_position == 0)
            {
                break;
            }
            else if(cursor_position == 1)
            {
                if(current_difficulty + 1 > 3)
                {
                    current_difficulty = 0;
                }
                else
                {
                    current_difficulty += 1;
                }
            }
            else if(cursor_position == 3)
            {
                credits();
            }
            else if(cursor_position == 4)
            {
                game_running = false;
                break;
            }
        }
        else if(input == 'w' || input == 'W')
        {
            if(cursor_position - 1 < 0)
            {
                cursor_position = 4;
            }
            else
            {
                cursor_position -= 1;
            }
        }
        else if(input == 's' || input == 'S')
        {
            if(cursor_position + 1 > 4)
            {
                cursor_position = 0;
            }
            else
            {
                cursor_position += 1;
            }
        }
    }
}

void print_order_info()
{
    textbackground(BLACK);
    cprintf("    ");
    textbackground(RED);textcolor(YELLOW);
    cprintf("                           FOFONSO'S ATOMS VGA                          ");textbackground(BLACK);cprintf("        ");textbackground(RED);
    textcolor(WHITE);cprintf("                 THE HOTTEST NEW PUZZLE GAME FOR VGA PCS                ");textbackground(BLACK);cprintf("        ");textbackground(RED);
    cprintf("   __________________________________________________________________   ");textbackground(BLACK);cprintf("        ");textbackground(RED);
    cprintf("                 Discover the secrets of the ");textcolor(MAGENTA);cprintf("ATOMIC");textcolor(WHITE);cprintf(" with                ");textbackground(BLACK);cprintf("        ");textbackground(RED);
    cprintf("                 the power of ");textcolor(BLUE);cprintf("SCIENCE");textcolor(WHITE);cprintf(", and help discover                ");textbackground(BLACK);cprintf("        ");textbackground(RED);
    cprintf("                the location of some very ");textcolor(BROWN);cprintf("PESKY");textcolor(WHITE);cprintf(" atoms, in               ");textbackground(BLACK);cprintf("        ");textbackground(RED);
    cprintf("             this reimagining of a classic no one remembers.            ");textbackground(BLACK);cprintf("        ");textbackground(RED);
    cprintf("                                                                        ");textbackground(BLACK);cprintf("        ");textbackground(RED);             
    cprintf("                           Hey guys, ");textcolor(GREEN);cprintf("I'M BACK!");textcolor(WHITE);cprintf("                          ");textbackground(BLACK);cprintf("        ");textbackground(RED);
    cprintf("                    And I bring thee a new (old) ");textcolor(CYAN);cprintf("GAME");textcolor(WHITE);cprintf(",                  ");textbackground(BLACK);cprintf("        ");textbackground(RED);
    cprintf("        Here's the story this time: I found an old shareware disk       ");textbackground(BLACK);cprintf("        ");textbackground(RED);
    cprintf("        with a bunch of demos, (classic stuff, love those disks),       ");textbackground(BLACK);cprintf("        ");textbackground(RED);
    cprintf("       and in there I found this little Win 3.11 game called ");textcolor(MAGENTA);cprintf("ATOMS");textcolor(WHITE);cprintf(",     ");textbackground(BLACK);cprintf("        ");textbackground(RED);
    cprintf("                    made by a guy called ");textcolor(YELLOW);cprintf("Mike McNamee");textcolor(WHITE);cprintf(".                  ");textbackground(BLACK);cprintf("        ");textbackground(RED);
    cprintf("       It's a really simple game and I enjoyed it alot! Enjoyed it      ");textbackground(BLACK);cprintf("        ");textbackground(RED);
    cprintf("       enough to try my hand at cloning it, and ");textcolor(BLUE);cprintf("OF COURSE");textcolor(WHITE);cprintf(" my clone      ");textbackground(BLACK);cprintf("        ");textbackground(RED);
    cprintf("   is for VGA enabled DOS PCs, because those are the stuff of dreams.   ");textbackground(BLACK);cprintf("        ");textbackground(RED);
    cprintf("                          Hope you enjoyed it!                          ");textbackground(BLACK);cprintf("        ");textbackground(RED);
    cprintf("   __________________________________________________________________   ");textbackground(BLACK);cprintf("        ");textbackground(RED);
    cprintf("               ");textcolor(YELLOW);cprintf("Affonso Amendola");textcolor(WHITE);cprintf(", affonso.gino.neto@usp.br               ");textbackground(BLACK);cprintf("        ");textbackground(RED);
    cprintf("             http://www.github.com/affonsoamendola/atomsvga             ");textbackground(BLACK);cprintf("        ");textbackground(RED);
    cprintf("           8:17 AM, 18 January 2019, Sao Paulo, Brazil, Earth           ");textbackground(BLACK);cprintf("        ");textbackground(RED);
    cprintf("                                                                        ");textbackground(BLACK);cprintf("        ");textbackground(RED);
    cprintf("                       ");textcolor(GREEN);cprintf("Be Excellent to each other.                      ");                                                           
}

int main()
{
    set_graphics_mode(GRAPHICS_MODEX);
    
    splash_screen();

    main_menu();
    
    init_game();

    while(game_running == true)
    {
        game_loop();
    }

    kill_game();
    print_order_info();
    return 0;
}