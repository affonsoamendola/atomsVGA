#ifndef FF_BLAST
#define FF_BLASE


typedef struct HEADER_TYPE_ 
{
    long            riff;
    char            padding_0[4];
    long            wave;
    long            fmt;
    char            padding_1[6];
    unsigned int    channels;
    long            frequency;
    char            padding_2[6]; 
    unsigned int    bit_res;
    long            data;
    long            data_size;
} HEADER_TYPE;

typedef struct WAVE_DATA_
{
    unsigned int sound_length;
    unsigned int frequency;
    unsigned char* sample;
} WAVE_DATA;

char reset_and_check_dsp(unsigned int test);

unsigned int find_sb();
void init_sb();
void quit_sb();

void write_dsp(unsigned char value);

void playback(WAVE_DATA* wave);

void load_voice(WAVE_DATA* voice, const char* filename);
void unload_voice(WAVE_DATA* voice);


#endif /* FF_BLAST */