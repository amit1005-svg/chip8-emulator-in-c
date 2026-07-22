#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL3/SDL.h>
#include "chip8.h"

int main(int argc, char *argv[])
{
    CHIP8_State *cpu = (CHIP8_State *)malloc(sizeof(CHIP8_State));
    init_chip8(cpu);
    srand(time(NULL));
    if (argc < 2)
    {
        printf("no rom file given.");
        return 1;
    }
    load_rom(cpu, argv[1]);
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
    {
        SDL_Log("Failed!");
        printf("SDL failed: %s\n", SDL_GetError());
        return -1;
    };
    SDL_Window *window;
    window = SDL_CreateWindow(argv[1], 640, 320, SDL_WINDOW_RESIZABLE);
    SDL_Renderer *renderer;
    renderer = SDL_CreateRenderer(window, NULL);
    if (renderer == NULL)
    {
        SDL_Log("not able to create a renderer");
    }
    SDL_Texture *texture;
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);
    SDL_AudioSpec spec;
    spec.freq = 44100;
    spec.format = SDL_AUDIO_S16;
    spec.channels = 1;
    SDL_AudioStream *stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
    SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(stream));
    SDL_Event event;
    const bool *keystate = SDL_GetKeyboardState(NULL);
    bool quit = false;
    while (!quit)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                quit = true;
            }
        }
        cpu->keypad[0x0] = keystate[SDL_SCANCODE_0];
        cpu->keypad[0x1] = keystate[SDL_SCANCODE_1];
        cpu->keypad[0x2] = keystate[SDL_SCANCODE_2];
        cpu->keypad[0x3] = keystate[SDL_SCANCODE_3];
        cpu->keypad[0x4] = keystate[SDL_SCANCODE_4];
        cpu->keypad[0x5] = keystate[SDL_SCANCODE_5];
        cpu->keypad[0x6] = keystate[SDL_SCANCODE_6];
        cpu->keypad[0x7] = keystate[SDL_SCANCODE_7];
        cpu->keypad[0x8] = keystate[SDL_SCANCODE_8];
        cpu->keypad[0x9] = keystate[SDL_SCANCODE_9];
        cpu->keypad[0xa] = keystate[SDL_SCANCODE_A];
        cpu->keypad[0xb] = keystate[SDL_SCANCODE_B];
        cpu->keypad[0xc] = keystate[SDL_SCANCODE_C];
        cpu->keypad[0xd] = keystate[SDL_SCANCODE_D];
        cpu->keypad[0xe] = keystate[SDL_SCANCODE_E];
        cpu->keypad[0xf] = keystate[SDL_SCANCODE_F];
        for (int i = 0; i < 10; i++)
        {
            emulate_cycle(cpu);
        }
        if (cpu->sound_timer > 0)
        {
            int16_t sample_buffer[735];
            for (int i = 0; i < 735; i++)
            {
                if ((i/50) % 2 == 0)
                    sample_buffer[i] = 3000;
                else
                    sample_buffer[i] = -3000;
            }
            SDL_PutAudioStreamData(stream, sample_buffer, sizeof(sample_buffer));
            cpu->sound_timer -= 1;
        }
        if (cpu->delay_timer > 0)
            cpu->delay_timer -= 1;
        uint32_t pixel_buffer[2048];
        for (int i = 0; i < 2048; i++)
        {
            if (cpu->display[i] == 1)
            {
                pixel_buffer[i] = 0xffffffff;
            }
            else
            {
                pixel_buffer[i] = 0x000000ff;
            }
        }
        SDL_UpdateTexture(texture, NULL, pixel_buffer, 64 * sizeof(uint32_t));
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    free(cpu);
    return 0;
}