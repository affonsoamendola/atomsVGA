/*  Copyright 2022 Affonso Amendola
    
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

#include "sound.h"




HEADER_TYPE header;

unsigned int sb_base_address;

unsigned int find_sb()
{
    unsigned int address = 0x220;

    if(reset_and_check_dsp(address))
    {
        sb_base_address = address;
    }

    address = 0x240;

    if(reset_and_check_dsp(address))
    {
        sb_base_address = address;
    }

}

void init_sb()
{
    find_sb();
}

char reset_and_check_dsp(unsigned int test)
{
    int success = 0;

    outp(test + 0x06, 1);
    delay(10);
    outp(test + 0x06, 0);
    delay(10);

    success = ((inp(test + 0xE) & 0x80) == 0x80 &&
               (inp(test + 0xA)         == 0xAA));

    if(success)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void write_dsp(unsigned char value)
{
    int counter = 0;

    while((inp(sb_base_address + 0xC) & 0x80) == 0x80)
    {
        count ++;
        if(count > 65534) break;
    }

    outp(sb_base_address + 0xC, value);
}

void playback(WAVE_DATA* wave)
{
    long linear_address;
    unsigned int page, offset;
    unsigned char time_constant;

    time_constant = (65536 - (256000000 / wave->frequency)) >> 8;

    write_dsp(0x40); 

    write_dsp(time_constant);

    linear_address = FP_SEG(wave->sample);
    linear_address = (linear_address << 4) + FP_OFF(wave->sample);

    page = linear_address >> 16;
    offset = linear_address & 0xFFFF;

    /*
      Note - this procedure only works with DMA channel 1
    */
    outp (0x0A, 5);              //Mask DMA channel 1
    outp (0x0C, 0);              //Clear byte pointer
    outp (0x0B, 0x49);           //Set mode
    /*
      The mode consists of the following:
      0x49 = binary 01 00 10 01
                    |  |  |  |
                    |  |  |  +- DMA channel 01
                    |  |  +---- Read operation (the DSP reads from memory)
                    |  +------- Single cycle mode
                    +---------- Block mode
    */

    outp (0x02, OffSet & 0xFF); //Write the offset to the DMA controller
    outp (0x02, OffSet >> 8);

    outp (0x83, Page);           //Write the page to the DMA controller

    outp (0x03, Wave->SoundLength & 0xFF);
    outp (0x03, Wave->SoundLength >> 8);

    outp (0x0A, 1);              //Unmask DMA channel

    write_dsp(0xD1);
    write_dsp(0x14);
    write_dsp(wave->sound_length & 0xFF);
    write_dsp(wave->sound_length >> 8);

}


int load_voice(WAVE_DATA* voice, const char* filename)
{
    FILE* wav_file;

    wav_file = fopen(filename, "rb");

    if(wav_file == NULL)
    {
        
    }

}