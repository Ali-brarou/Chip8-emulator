#include "chip-8.h"

#include <string.h> 
#include <time.h> 
#include <unistd.h> 

//the fonts

byte fonts[16 * 5] = 
{
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
}; 

//instuctions functions 

static void Clear_screen(Chip8* chip)
{
    memset(chip -> display, 0, 32 * sizeof(int64_t)); 
}

static void Jump(Chip8* chip, word add)
{
    chip -> PC = add; 
}

static void Jump_offset(Chip8* chip, word add)
{
    chip -> PC = add + chip -> registers[0]; 
}

static void Set_reg(Chip8* chip, byte R_n, byte R_v)
{
    chip -> registers[R_n] = R_v; 
}

static void Set_index_reg(Chip8* chip, word R_v)
{
    chip -> I = R_v; 
}

static void Add_reg(Chip8* chip, byte R_n, byte v)
{
    chip -> registers[R_n] += v; 
}

static void Skip_val_eq(Chip8* chip, byte R_n, byte v)
{
    if (chip -> registers[R_n] == v)
        chip -> PC += 2; 

}

static void Skip_val_not(Chip8* chip, byte R_n, byte v)
{
    if (chip -> registers[R_n] != v)
        chip -> PC += 2; 
}

static void Skip_reg_eq(Chip8* chip, byte R_x, byte R_y)
{
    if (chip -> registers[R_x] == chip -> registers[R_y])
        chip -> PC += 2; 
}

static void Skip_reg_not(Chip8* chip, byte R_x, byte R_y)
{
    if (chip -> registers[R_x] != chip -> registers[R_y])
        chip -> PC += 2; 
}

static void Skip_key_down(Chip8* chip, byte R_x)
{
    if ((chip -> keys >> chip -> registers[R_x]) & 1)
        chip -> PC += 2; 
}

static void Skip_key_up(Chip8* chip, byte R_x)
{
    if ( !((chip -> keys >> chip -> registers[R_x]) & 1))
        chip -> PC += 2; 
}

static void Subr_push(Chip8* chip, word addr)
{
    chip -> stack[chip -> SP++] = chip -> PC; 
    chip -> PC = addr; 
}

static void Subr_pop(Chip8* chip)
{
    chip -> PC = chip -> stack[--(chip -> SP)]; 
}

static void Random_reg(Chip8* chip, byte R_n, byte n)
{
    chip -> registers[R_n] = rand() &  n; 
}

static void Math(Chip8* chip, byte R_x, byte R_y, byte t)
{
    switch (t)
    {
        case 0x0: 

        chip -> registers[R_x] = chip -> registers[R_y]; 
        break; 

        case 0x1: 

        chip -> registers[R_x] |= chip -> registers[R_y]; 
        break; 

        case 0x2: 

        chip -> registers[R_x] &= chip -> registers[R_y]; 
        break; 

        case 0x3: 

        chip -> registers[R_x] ^= chip -> registers[R_y]; 
        break; 
        case 0x4: 
        {
        
        byte x = chip -> registers[R_x]; 

        chip -> registers[R_x] += chip -> registers[R_y]; 

        chip -> registers[0x0F] = 0; 
        if ((int)(x) + chip -> registers[R_y]
                > 0xFF)
            chip -> registers[0x0F] = 1; 

        }
        break; 
        case 0x5: 
        {

        byte x = chip -> registers[R_x]; 
        
        chip -> registers[R_x] -= chip -> registers[R_y]; 

        chip -> registers[0x0F] = x
            >= chip -> registers[R_y]; 

        }
        break; 

        case 0x6: 


        chip -> registers[0x0F] = chip -> registers[R_y] & 1; 
        chip -> registers[R_x] = chip -> registers[R_y] >> 1; 
        printf("math baby \n"); 

        break; 

        case 0x7:

        byte x = chip -> registers[R_x]; 
        
        chip -> registers[R_x] = chip -> registers[R_y]
                                - chip -> registers[R_x]; 

        chip -> registers[0x0F] = x 
            <= chip -> registers[R_y]; 
        break; 

        case 0xE:
        
        chip -> registers[0x0F] = chip -> registers[R_y] >> 7;
        chip -> registers[R_x] = chip -> registers[R_y] << 1; 


        break; 
    }
}

static void Extra(Chip8* chip, byte R_n, byte t)
{
    switch (t)
    {
        case 0x07:
        
        chip -> registers[R_n] = chip -> DT; 

        break; 
        case 0x15: 

        chip -> DT = chip -> registers[R_n]; 

        break; 
        case 0x18:

        chip -> ST = chip -> registers[R_n]; 

        break; 
    
        case 0x1E: 

        chip -> I += chip -> registers[R_n]; 
 
        break; 
        case 0x29: 

        chip -> I = FONT_ADD + 5 * (chip -> registers[R_n]); 

        break; 
        case 0x33: 
        
        chip -> mem[(chip -> I)] = (chip -> registers[R_n] / 100) % 10; 
        chip -> mem[(chip -> I) + 1] = (chip -> registers[R_n] / 10) % 10; 
        chip -> mem[(chip -> I) + 2] = chip -> registers[R_n] % 10; 

        break; 
        case 0x55: 
        {
            for (int R_i = 0; R_i <= R_n; R_i++)
            {
                chip -> mem[R_i + chip -> I] = chip -> registers[R_i]; 
            }
        }
        break; 
        case 0x65: 
        {
            for (int R_i = 0; R_i <= R_n; R_i++)
            {
                chip -> registers[R_i] = chip -> mem[R_i + chip -> I];
            }
        }
        break; 
        case 0x0A:
        {
            while (chip -> keys == 0)
            {
                usleep(1000); 
            }
            
            for (byte key = 0x0; key < 0x10; key++)
            {
                if ((chip -> keys >> key) & 1)
                {
                    chip -> registers[R_n] = key; 
                    break; 
                }
            }
            chip -> registers[R_n] = 0x0; 
        }
        break; 
    }
}

static void Draw(Chip8* chip, byte R_x, byte R_y, byte n)
{
    byte p_x = chip -> registers[R_x] & 63; 
    byte p_y = chip -> registers[R_y] & 31; 

    chip -> registers[0x0F] = 0; 

    for (byte i = 0; i < n; i++) 
    {
        byte Y = p_y + i; 
        if (Y >= 32)
            return; 
        byte sprite = chip -> mem[(chip -> I) + i]; 

        for (int j = 0; j < 8; j++)
        {
            byte X = p_x + j; 
            if (X >= 64)
                continue; 
            

            int64_t pixel = (sprite >> (8 - j - 1)) & 1; 

            if ((chip -> display[Y] >> X & 1) && pixel)
                chip -> registers[0x0F] = 1; 
            
            chip -> display[Y] ^= pixel << X;

        }
    }
}

Chip8 Init_chip()
{
    srand(time(NULL)); 

    Chip8 newChip = 
    {
        .SP = 0, 
        .I = 0, 
        .PC = START_ADD, 
        .DT = 0, 
        .ST = 0,
        .keys = 0, 
    }; 

    memset(newChip.mem, 0, MEM_SIZE * sizeof(byte));
    memset(newChip.display, 0, 32 * sizeof(int64_t));
    memset(newChip.stack, 0, 16 * sizeof(word)); 
    memset(newChip.registers, 0, 16 * sizeof(byte)); 
    
    memcpy(newChip.mem + FONT_ADD, fonts, 16 * 5 * sizeof(byte));

    return newChip; 
}

void Chip_step(Chip8* chip)
{  
    word opcode; 
    opcode = (chip -> mem[chip -> PC] << 8 | chip -> mem[chip -> PC + 1]); 
    chip->PC += 2; 

    switch (opcode & 0xF000)
    {
        case 0x0000:
        {
            byte t = opcode & 0x0F; 
            if (t == 0x0E)
            {
                Subr_pop(chip); 
            }
            else if (t == 0x00)
            {
                Clear_screen(chip); 
            }    
        }
        break; 
        case 0x1000: 
        {
            word addr = opcode & 0x0FFF; 
            Jump(chip, addr); 
        }
        break; 
        case 0x2000: 
        {
            word addr = opcode & 0x0FFF; 
            Subr_push(chip, addr); 
        }
        break; 
        case 0x3000: 
        {
            byte R_n = (opcode >> 8) & 0x0F; 
            byte v = opcode & 0x0FF; 
            Skip_val_eq(chip, R_n, v); 
        }
        break; 
        case 0x4000: 
        {
            byte R_n = (opcode >> 8) & 0x0F; 
            byte v = opcode & 0x0FF; 
            Skip_val_not(chip, R_n, v); 
        }
        break; 
        case 0x5000: 
        {
            byte R_x = (opcode >> 8) & 0x0F; 
            byte R_y = (opcode >> 4) & 0x0F; 
            Skip_reg_eq(chip, R_x, R_y); 
        }
        break; 
        case 0x6000: 
        {
            byte R_v = opcode & 0x00FF; 
            byte R_n = (opcode >> 8) & 0x000F; 
            Set_reg(chip, R_n, R_v); 
        }
        break; 
        case 0x7000: 
        {
            byte R_n = (opcode >> 8) & 0x000F; 
            byte v = opcode & 0x00FF;  
            Add_reg(chip, R_n, v); 
        }
        break; 
        case 0x8000: 
        {
            byte R_x = (opcode >> 8) & 0x000F; 
            byte R_y = (opcode >> 4) & 0x000F; 
            byte t = opcode & 0x00F;
            Math(chip, R_x, R_y, t); 
        }
        break;
        case 0x9000: 
        {
            byte R_x = (opcode >> 8) & 0x0F; 
            byte R_y = (opcode >> 4) & 0x0F; 
            Skip_reg_not(chip, R_x, R_y); 
        }
        break; 
        case 0xA000:
            Set_index_reg(chip, opcode & 0x0FFF); 
        break; 
        case 0xB000: 
        {
            word addr = opcode & 0x0FFF; 
            Jump_offset(chip, addr);
        }
        break; 
        case 0xC000:
        {
            byte R_n = (opcode >> 8) & 0x00F; 
            byte n = opcode & 0x0FF;
            Random_reg(chip, R_n, n); 
        }
        break; 
        case 0xD000: 
        {
            byte R_x = (opcode >> 8) & 0x00F; 
            byte R_y = (opcode >> 4) & 0x00F; 
            byte n = opcode & 0x00F; 
            Draw(chip, R_x, R_y, n); 
        }
        break; 
        case 0xE000: 
        {
            byte R_n = (opcode >> 8) & 0x00F; 
            if ((opcode & 0x0FF) == 0x9E)
            {
                Skip_key_down(chip, R_n); 
            }
            else if ((opcode & 0x0FF) == 0xA1) 
            {
                Skip_key_up(chip, R_n); 
            }
        }
        break; 
        case 0xF000: 
        {
            byte R_n = (opcode >> 8) & 0x00F; 
            byte t = opcode & 0x0FF; 
            Extra(chip, R_n, t); 
        }
        break; 
    }


    //decoding it  
}

int Chip_load_rom(Chip8* chip, char* filename)
{
    FILE* fptr = fopen(filename, "rb"); 
    
    if (fptr == NULL)
        return -1; 

    fseek(fptr, 0, SEEK_END); 
    long fsize = ftell(fptr); 
    byte* file = malloc(fsize + 1); 

    fseek(fptr, 0, SEEK_SET); 


    int succ = fread(file, 1, fsize, fptr); 
    if (!succ)
        return -1; 

    memcpy(chip -> mem + START_ADD, file, fsize); 
    
    if (file)
        free(file); 

    return 0; 
}

void Chip_tick(Chip8* chip)
{
    if (chip -> DT)
        chip -> DT--; 

    if (chip -> ST)
        chip -> ST--; 
}
