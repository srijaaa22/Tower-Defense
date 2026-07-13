#include <iostream>
#include <chrono>
#include <SDL2/SDL.h>
#include <vector>
#include "Vec2.h"
#include "Enemy.h"
#include "Tower.h"
#include "Spatialhash.h"
#include "Grid.h"
#include "Pathfinding.h"

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
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_CreateWindowAndRenderer(800, 600, 0, &window, &renderer);
    SDL_SetWindowTitle(window, "Tower-Defense");
    std::cout << "Right-click on the grid to set your base (goal), then left-click to place towers." << std::endl;

    Grid grid(19, 25);
    Vec2 start(0, 0);
    Vec2 goal(24, 17);
    bool goalSet = false;

    SDL_Event event;
    bool running = true;
    std::vector<Tower> t_store;

    Enemy e{Vec2{0.0,0.0}, Vec2{32.0,32.0}, Vec2{0.05,0.0}, 0.05, 100};

    Uint32 curr = SDL_GetTicks();
    float acc = 0;
    const float ft = 16.67f;
    const int cs = 64;
    SpatialHash sh(cs);

    enum class CollisionMode { Naive, HashGrid };
    CollisionMode mode = CollisionMode::HashGrid;

    float naiveTotal = 0.0f, gridTotal = 0.0f;
    int naiveFrames = 0, gridFrames = 0;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_MOUSEBUTTONDOWN){
                int cellX = event.button.x / 32;
                int cellY = event.button.y / 32;

                if(event.button.button == SDL_BUTTON_RIGHT && !goalSet){
                    goal = Vec2(cellX, cellY);
                    goalSet = true;
                    auto path = bfs(start, goal, grid);
                    for(auto& p : path) p = p * 32.0f;
                    e.setPath(path);
                }

                if(event.button.button == SDL_BUTTON_LEFT && goalSet){
                    t_store.push_back(Tower(Vec2(cellX * 32, cellY * 32), Vec2(32.0, 32.0), 100, 10, 1));
                    grid.setBlocked(cellX, cellY);
                    auto newPath = bfs(e.getPosition() * (1.0f/32.0f), goal, grid);
                    for(auto& p : newPath) p = p * 32.0f;
                    e.setPath(newPath);
                }
            }
            if (event.type == SDL_KEYDOWN){
                if(event.key.keysym.sym == SDLK_b){
                    if(mode == CollisionMode::HashGrid){
                        if(gridFrames > 0)
                            std::cout << "HashGrid avg: " << gridTotal/gridFrames << " ms/frame" << std::endl;
                        mode = CollisionMode::Naive;
                        std::cout << "Switched to: Naive" << std::endl;
                    } else {
                        if(naiveFrames > 0)
                            std::cout << "Naive avg: " << naiveTotal/naiveFrames << " ms/frame" << std::endl;
                        mode = CollisionMode::HashGrid;
                        std::cout << "Switched to: HashGrid" << std::endl;
                    }
                }
            }
        }

        Uint32 newt = SDL_GetTicks();
        int dt = newt - curr;
        curr = newt;
        acc += dt;

        while (acc >= ft) {
            acc -= ft;
            if(goalSet && e.isActive()){
                e.update(ft);
            }
            else if(!e.isActive()) running = false;

            if(goalSet){
                std::vector<Entity*> allEntities;
                allEntities.push_back(&e);
                for(int i = 0; i < (int)t_store.size(); i++){
                    allEntities.push_back(&t_store[i]);
                }

                auto tStart = std::chrono::high_resolution_clock::now();

                if(mode == CollisionMode::Naive){
                    for(int i = 0; i < (int)allEntities.size(); i++){
                        for(int j = i+1; j < (int)allEntities.size(); j++){
                            overlaps(*allEntities[i], *allEntities[j]);
                        }
                    }
                    auto tEnd = std::chrono::high_resolution_clock::now();
                    naiveTotal += std::chrono::duration<float, std::milli>(tEnd - tStart).count();
                    naiveFrames++;
                } else {
                    sh.rebuild(allEntities);
                    std::vector<Entity*> nearby = sh.queryNearby(e.getPosition());
                    for(int i = 0; i < (int)nearby.size(); i++){
                        if(nearby[i] == &e) continue;
                        overlaps(e, *nearby[i]);
                    }
                    auto tEnd = std::chrono::high_resolution_clock::now();
                    gridTotal += std::chrono::duration<float, std::milli>(tEnd - tStart).count();
                    gridFrames++;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);

        // grid lines
        SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
        for(int x = 0; x < 800; x += 32)
            SDL_RenderDrawLine(renderer, x, 0, x, 600);
        for(int y = 0; y < 600; y += 32)
            SDL_RenderDrawLine(renderer, 0, y, 800, y);

        // teal goal highlight
        if(goalSet){
            SDL_SetRenderDrawColor(renderer, 0, 90, 90, 10);
            SDL_Rect goalRect{(int)(goal.x * 32), (int)(goal.y * 32), 32, 32};
            SDL_RenderFillRect(renderer, &goalRect);
        }

        if(e.isActive()) e.render(renderer);
        for(int i = 0; i < (int)t_store.size(); i++){
            t_store[i].render(renderer);
        }
        SDL_RenderPresent(renderer);
    }

    std::cout << "\n--- Final Benchmark Results ---" << std::endl;
    if(gridFrames > 0)
        std::cout << "HashGrid avg: " << gridTotal/gridFrames << " ms/frame over " << gridFrames << " frames" << std::endl;
    if(naiveFrames > 0)
        std::cout << "Naive avg: " << naiveTotal/naiveFrames << " ms/frame over " << naiveFrames << " frames" << std::endl;

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}