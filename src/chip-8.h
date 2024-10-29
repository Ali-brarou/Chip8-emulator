#ifndef CHIP_8_H
#define CHIP_8_H

#include "globals.h"

#define MEM_SIZE 4096
#define FONT_ADD 0x050
#define START_ADD 0x200

typedef struct Chip8_s
{
    byte mem[MEM_SIZE]; 

    int64_t display[32]; 

    word stack[16];
    byte SP; 

    word PC; 
    word I; 

    byte registers[16]; 

    byte DT; 
    byte ST;  

    word keys; // each bit represent a key from 0x0 to 0xF 
} Chip8; 

Chip8 Init_chip(); 

int Chip_load_rom(Chip8* chip, char* filename); 

void Chip_step(Chip8* chip); 

void Chip_tick(Chip8* chip); // this should be running at 60hz 


#endif
