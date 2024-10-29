#include <raylib.h> 
#include <unistd.h> 
#include <pthread.h> 

#include "chip-8.h"

#define screenWidth 1000
#define screenHeight 800

#define chipSpeed 2000 

void Draw_display(Chip8* chip); 

void Handle_input(Chip8* chip); 

void Play_sound(Chip8* chip);

void* updating_thread(void* chip)
{
    while (!WindowShouldClose())
    {
        Chip_step(chip); 
        usleep(chipSpeed);
    }
    return NULL; 
}

int main(int argc, char* argv[])
{
    if (argc != 2)
        return -1; 

    Chip8 chip8 = Init_chip(); 

    int fail = Chip_load_rom(&chip8, argv[1]); 

    if (fail)
        return -1; 

    InitWindow(screenWidth, screenHeight, "CHIP 8"); 
    SetTargetFPS(60); 

    pthread_t thread_id; 
    pthread_create(&thread_id, NULL, updating_thread, &chip8); 

    while (!WindowShouldClose())
    {
        Handle_input(&chip8); 

        Chip_tick(&chip8); 

        Play_sound(&chip8); 
        
        BeginDrawing(); 

        ClearBackground(BLACK); 

        Draw_display(&chip8); 

        EndDrawing(); 
    }

    CloseWindow();

    return 0; 
}

void Draw_display(Chip8* chip)
{  
    float blockWidth = screenWidth / 64; 
    float blockHeight = screenHeight / 32; 
    for (int y = 0; y < 32; y++)
    {
        for (int x = 0; x < 64; x++)
        {
            
            if ((chip -> display[y] >> x) & 1)
                DrawRectangle(blockWidth * x, blockHeight * y, blockWidth, blockHeight , WHITE); 
        }
    }
}

void Handle_input(Chip8* chip)
{
    int key_mapping[16] = {KEY_X, KEY_ONE, KEY_TWO, KEY_THREE, 
                        KEY_Q, KEY_W, KEY_E, KEY_A, 
                        KEY_S, KEY_D, KEY_Z, KEY_C,
                        KEY_FOUR, KEY_R, KEY_F, KEY_V}; 

    word keys = 0; 
    for (int i = 0; i < 16; i++)
    {
        keys |= IsKeyDown(key_mapping[i]) << i; 
    }

    chip -> keys = keys; 
}

void Play_sound(Chip8* chip)
{
    if (chip -> ST != 0)
        printf("s\n"); 
}
