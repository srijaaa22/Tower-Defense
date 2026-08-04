#include <iostream>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <string>
#include <SDL2/SDL.h>
#include <vector>
#include <fstream>
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
    std::cout << "Note: Tower placement is rejected if it completely blocks all enemy paths." << std::endl;

    Grid grid(50, 75);
    Vec2 goal(74, 49);
    bool goalSet = false;

    SDL_Event event;
    bool running = true;
    std::vector<Tower> t_store;
    std::vector<Enemy> enemies;

    srand(time(nullptr));

    const int maxWaves = 3;
    int waveCounts[] = {50, 150, 300};
    int currentWave = 0;
    bool waveInProgress = false;
    Uint32 waveTimer = 0;
    const Uint32 wavePause = 3000;
    int baseHealth = 100;
    bool gameOver = false;

    Vec2 rejectedCell(0, 0);
    Uint32 rejectTimer = 0;
    bool showReject = false;

    auto spawnWave = [&](int wave) {
        enemies.clear();
        int count = waveCounts[wave - 1];
        for(int i = 0; i < count; i++){
            int cx = rand() % 75;
            int cy = rand() % 50;
            enemies.push_back(Enemy(Vec2(cx * 16, cy * 16), Vec2(16.0, 16.0), Vec2(0.05, 0.0), 0.04, 100));
        }
        if(goalSet){
            for(auto& e : enemies){
                auto path = bfs(Vec2(e.getPosition().x/16, e.getPosition().y/16), goal, grid);
                for(auto& p : path) p = p * 16.0f;
                e.setPath(path);
            }
        }
        waveInProgress = true;
        std::cout << "Wave " << wave << " started! Enemies: " << count << std::endl;
    };

    Uint32 curr = SDL_GetTicks();
    float acc = 0;
    const float ft = 16.67f;
    SpatialHash sh(16, 50, 75);

    enum class CollisionMode { Naive, HashGrid };
    CollisionMode mode = CollisionMode::HashGrid;

    float naiveTotal = 0.0f, gridTotal = 0.0f;
    int naiveFrames = 0, gridFrames = 0;
    volatile int collisionHits = 0;
    int frameNum = 0;
    int totalScore = 0;

    std::ofstream benchLog("bench.csv");

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_MOUSEBUTTONDOWN){
                int cellX = event.button.x / 16;
                int cellY = event.button.y / 16;

                if(event.button.button == SDL_BUTTON_RIGHT && !goalSet){
                    goal = Vec2(cellX, cellY);
                    goalSet = true;
                    currentWave = 1;
                    spawnWave(currentWave);
                }

                if(event.button.button == SDL_BUTTON_LEFT && goalSet){
                    grid.setBlocked(cellX, cellY);

                    bool pathExists = false;
                    auto testPath = bfs(Vec2(0, 0), goal, grid);
                    if(!testPath.empty()) pathExists = true;

                    if(!pathExists){
                        for(auto& e : enemies){
                            if(!e.isActive()) continue;
                            auto ep = bfs(Vec2(e.getPosition().x/16, e.getPosition().y/16), goal, grid);
                            if(!ep.empty()){ pathExists = true; break; }
                        }
                    }

                    if(!pathExists){
                        grid.setWalkable(cellX, cellY);
                        rejectedCell = Vec2(cellX * 16, cellY * 16);
                        rejectTimer = SDL_GetTicks();
                        showReject = true;
                        std::cout << "Tower placement rejected, would block all paths!" << std::endl;
                    } else {
                        t_store.push_back(Tower(Vec2(cellX * 16, cellY * 16), Vec2(16.0, 16.0), 100, 10, 1));
                        for(auto& e : enemies){
                            if(!e.isActive()) continue;
                            auto newPath = bfs(Vec2(e.getPosition().x/16, e.getPosition().y/16), goal, grid);
                            for(auto& p : newPath) p = p * 16.0f;
                            e.setPath(newPath);
                        }
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

        if(showReject && SDL_GetTicks() - rejectTimer >= 500)
            showReject = false;

        if(goalSet && waveInProgress && !gameOver){
            bool anyActive = false;
            bool anyHadPath = false;
            for(auto& e : enemies){
                if(e.hasPath()) anyHadPath = true;
                if(e.hasPath() && e.isActive()) anyActive = true;
            }

            if(anyHadPath && !anyActive){
                waveInProgress = false;
                std::cout << "Wave " << currentWave << " complete!" << std::endl;

                if(currentWave >= maxWaves){
                    std::cout << "You win! All waves complete." << std::endl;
                    running = false;
                } else {
                    waveTimer = SDL_GetTicks();
                    std::cout << "Next wave in 3 seconds..." << std::endl;
                }
            }
        }

        if(goalSet && !waveInProgress && currentWave > 0 && currentWave < maxWaves && !gameOver){
            if(SDL_GetTicks() - waveTimer >= wavePause){
                currentWave++;
                spawnWave(currentWave);
            }
        }

        while (acc >= ft) {
            acc -= ft;
            frameNum++;

            if(goalSet && waveInProgress && !gameOver) totalScore++;

            for(auto& e : enemies){
                if(e.hasPath() && e.isActive()) e.update(ft);
            }

            for(auto& e : enemies){
                if(gameOver) break;
                if(e.reachedGoal()){
                    baseHealth -= 5;
                    e.clearReachedGoal();
                    if(baseHealth <= 0){
                        baseHealth = 0;
                        gameOver = true;
                        running = false;
                        std::cout << "Game Over! Base destroyed." << std::endl;
                    } else {
                        std::cout << "Enemy reached base! HP: " << baseHealth << std::endl;
                    }
                }
            }

            std::vector<Entity*> allEntities;
            for(auto& e : enemies) allEntities.push_back(&e);
            for(auto& t : t_store) allEntities.push_back(&t);

            auto tStart = std::chrono::high_resolution_clock::now();

            if(mode == CollisionMode::Naive){
                for(int i = 0; i < (int)allEntities.size(); i++){
                    for(int j = i+1; j < (int)allEntities.size(); j++){
                        if(overlaps(*allEntities[i], *allEntities[j])) collisionHits++;
                    }
                }
                auto tEnd = std::chrono::high_resolution_clock::now();
                float ms = std::chrono::duration<float, std::milli>(tEnd - tStart).count();
                naiveTotal += ms; naiveFrames++;
                benchLog << frameNum << ",naive," << ms << "," << allEntities.size() << "\n";
            } else {
                sh.rebuild(allEntities);
                std::vector<Entity*> nearby;
                for(auto& e : enemies){
                    if(!e.isActive()) continue;
                    sh.queryNearby(e.getPosition(), nearby);
                    for(int i = 0; i < (int)nearby.size(); i++){
                        if(nearby[i] == &e) continue;
                        if(overlaps(e, *nearby[i])) collisionHits++;
                    }
                }
                auto tEnd = std::chrono::high_resolution_clock::now();
                float ms = std::chrono::duration<float, std::milli>(tEnd - tStart).count();
                gridTotal += ms; gridFrames++;
                benchLog << frameNum << ",grid," << ms << "," << allEntities.size() << "\n";
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

        if(showReject){
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 200);
            SDL_Rect rejectRect{(int)rejectedCell.x, (int)rejectedCell.y, 16, 16};
            SDL_RenderFillRect(renderer, &rejectRect);
        }

        std::string waveInfo = "Wave: " + std::to_string(currentWave) + "/" +
                               std::to_string(maxWaves) +
                               "  Time: " + std::to_string(totalScore / 60) + "s" +
                               "  HP: " + std::to_string(baseHealth);
        SDL_SetWindowTitle(window, waveInfo.c_str());

        for(auto& e : enemies){
            if(e.isActive()) e.render(renderer);
        }
        for(auto& t : t_store) t.render(renderer);

        SDL_RenderPresent(renderer);
    }

    if(gameOver){
        SDL_SetWindowTitle(window, ("GAME OVER - Time Survived: " + std::to_string(totalScore/60) + "s").c_str());
        SDL_SetRenderDrawColor(renderer, 180, 0, 0, 180);
        SDL_Rect overlay{0, 0, 1200, 800};
        SDL_RenderFillRect(renderer, &overlay);
        SDL_RenderPresent(renderer);
        SDL_Delay(2000);
    }

    std::cout << "\n--- Final Benchmark Results ---" << std::endl;
    if(gridFrames > 0)
        std::cout << "HashGrid avg: " << gridTotal/gridFrames << " ms/frame over " << gridFrames << " frames" << std::endl;
    if(naiveFrames > 0)
        std::cout << "Naive avg: " << naiveTotal/naiveFrames << " ms/frame over " << naiveFrames << " frames" << std::endl;

    std::ofstream f("stats.json");
    f << "{\"score\":" << totalScore/60 << ",\"waves\":" << currentWave << "}";
    f.close();
    benchLog.close();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}