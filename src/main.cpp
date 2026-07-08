#include <SDL2/SDL.h>
#include "Enemy.h"

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_CreateWindowAndRenderer(800, 600, 0, &window, &renderer);
    SDL_SetWindowTitle(window, "Tower Defense");

    SDL_Event event;
    bool running = true;

    Enemy e{Vec2{0.0,0.0}, Vec2{32.0,32.0}, Vec2{0.05,0.0}, 1, 100};

    Uint32 curr = SDL_GetTicks();
    float acc = 0;
    const float ft = 16.67f;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;
        }

        Uint32 newt = SDL_GetTicks();
        int dt = newt - curr;
        curr = newt;
        acc += dt;

        while (acc >= ft) {
            // game logic goes here later
            acc -= ft;
            e.update(ft);
        }

        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);
        e.render(renderer);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}