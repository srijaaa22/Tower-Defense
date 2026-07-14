#include <iostream>
#include <chrono>
#include <cstdlib>
#include <ctime>
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
    SDL_CreateWindowAndRenderer(1200, 800, 0, &window, &renderer);
    SDL_SetWindowTitle(window, "Tower-Defense");
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    std::cout << "Right-click on the grid to set your base (goal), then left-click to place towers." << std::endl;

    Grid grid(50, 75);
    Vec2 goal(74, 49);
    bool goalSet = false;

    SDL_Event event;
    bool running = true;
    std::vector<Tower> t_store;

    srand(time(nullptr));
    std::vector<Enemy> enemies;
    const int enemyCount = 300;
    for(int i = 0; i < enemyCount; i++){
        int col = rand() % 75;
        int row = rand() % 50;
        enemies.push_back(Enemy(Vec2(col * 16, row * 16), Vec2(16.0, 16.0), Vec2(0.05, 0.0), 0.04, 100));
    }

    Uint32 curr = SDL_GetTicks();
    float acc = 0;
    const float ft = 16.67f;
    SpatialHash sh(16, 50, 75);

    enum class CollisionMode { Naive, HashGrid };
    CollisionMode mode = CollisionMode::HashGrid;

    float naiveTotal = 0.0f, gridTotal = 0.0f;
    int naiveFrames = 0, gridFrames = 0;
    volatile int collisionHits = 0;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_MOUSEBUTTONDOWN){
                int cellX = event.button.x / 16;
                int cellY = event.button.y / 16;

                if(event.button.button == SDL_BUTTON_RIGHT && !goalSet){
                    goal = Vec2(cellX, cellY);
                    goalSet = true;
                    for(auto& e : enemies){
                        auto path = bfs(Vec2(e.getPosition().x/16, e.getPosition().y/16), goal, grid);
                        for(auto& p : path) p = p * 16.0f;
                        e.setPath(path);
                    }
                }

                if(event.button.button == SDL_BUTTON_LEFT && goalSet){
                    t_store.push_back(Tower(Vec2(cellX * 16, cellY * 16), Vec2(16.0, 16.0), 100, 10, 1));
                    grid.setBlocked(cellX, cellY);
                    for(auto& e : enemies){
                        if(!e.isActive()) continue;
                        auto newPath = bfs(Vec2(e.getPosition().x/16, e.getPosition().y/16), goal, grid);
                        for(auto& p : newPath) p = p * 16.0f;
                        e.setPath(newPath);
                    }
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

            bool anyActive = false;
            for(auto& e : enemies){
                if(e.hasPath() && e.isActive()){
                    e.update(ft);
                    anyActive = true;
                }
            }
            if(goalSet && !anyActive && enemies[0].hasPath()) running = false;

            std::vector<Entity*> allEntities;
            for(auto& e : enemies) allEntities.push_back(&e);
            for(auto& t : t_store) allEntities.push_back(&t);

            auto tStart = std::chrono::high_resolution_clock::now();

            if(mode == CollisionMode::Naive){
                for(int i = 0; i < (int)allEntities.size(); i++){
                    for(int j = i+1; j < (int)allEntities.size(); j++){
                        if(overlaps(*allEntities[i], *allEntities[j])){
                            collisionHits++;
                        }
                    }
                }
                auto tEnd = std::chrono::high_resolution_clock::now();
                naiveTotal += std::chrono::duration<float, std::milli>(tEnd - tStart).count();
                naiveFrames++;
            } else {
                sh.rebuild(allEntities);
                std::vector<Entity*> nearby;
                for(auto& e : enemies){
                    if(!e.isActive()) continue;
                    sh.queryNearby(e.getPosition(), nearby);
                    for(int i = 0; i < (int)nearby.size(); i++){
                        if(nearby[i] == &e) continue;
                        if(overlaps(e, *nearby[i])){
                            collisionHits++;
                        }
                    }
                }
                auto tEnd = std::chrono::high_resolution_clock::now();
                gridTotal += std::chrono::duration<float, std::milli>(tEnd - tStart).count();
                gridFrames++;
            }
        }

        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
        for(int x = 0; x < 1200; x += 16)
            SDL_RenderDrawLine(renderer, x, 0, x, 800);
        for(int y = 0; y < 800; y += 16)
            SDL_RenderDrawLine(renderer, 0, y, 1200, y);

        if(goalSet){
            SDL_SetRenderDrawColor(renderer, 0, 180, 180, 100);
            SDL_Rect goalRect{(int)(goal.x * 16), (int)(goal.y * 16), 16, 16};
            SDL_RenderFillRect(renderer, &goalRect);
        }

        for(auto& e : enemies){
            if(e.isActive()) e.render(renderer);
        }
        for(auto& t : t_store) t.render(renderer);

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