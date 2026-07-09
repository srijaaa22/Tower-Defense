#include <iostream>
#include <SDL2/SDL.h>
#include <vector>
#include "Vec2.h"
#include "Enemy.h"
#include "Tower.h"
#include "Spatialhash.h"

bool overlaps(Entity& a, Entity& b) {
    return a.getPosition().x < b.getPosition().x + b.getSize().x &&
           a.getPosition().x + a.getSize().x > b.getPosition().x &&
           a.getPosition().y < b.getPosition().y + b.getSize().y &&
           a.getPosition().y + a.getSize().y > b.getPosition().y;
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_CreateWindowAndRenderer(800, 600, 0, &window, &renderer);
    SDL_SetWindowTitle(window, "Tower-Defense");

    SDL_Event event;
    bool running = true;
    std::vector<Tower> t_store;

    Enemy e{Vec2{0.0,0.0}, Vec2{16.0,32.0}, Vec2{0.05,0.0}, 1, 100};

    Uint32 curr = SDL_GetTicks();
    float acc = 0;
    const float ft = 16.67f;

    const int cs = 64;
    SpatialHash sh(cs);

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_MOUSEBUTTONDOWN){
                t_store.push_back(Tower(Vec2(event.button.x,event.button.y),Vec2(16.0,32.0),100,10,1));
            }
        }

        Uint32 newt = SDL_GetTicks();
        int dt = newt - curr;
        curr = newt;
        acc += dt;

        while (acc >= ft) {
            // game logic goes here later
            acc -= ft;
            e.update(ft);

            std::vector<Entity*> allEntities;
            allEntities.push_back(&e);

            for(int i=0; i<t_store.size(); i++){
                allEntities.push_back(&t_store[i]);
            }

            sh.rebuild(allEntities);

            std::vector<Entity*> nearby = sh.queryNearby(e.getPosition());
            for(int i=0; i<nearby.size(); i++){
                if(nearby[i] == &e) continue;
                if(overlaps(e, *nearby[i])) std::cout << "Overlaps ";
            }
        }

        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);
        e.render(renderer);
        for(int i=0; i<t_store.size(); i++){
            t_store[i].render(renderer);
        }
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}