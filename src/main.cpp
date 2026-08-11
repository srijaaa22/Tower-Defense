// Libraries
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <string>
#include <SDL2/SDL.h>
#include <vector>
#include <memory>
#include <fstream>
#include "Vec2.h"
#include "Enemy.h"
#include "Fast_Enemy.h"
#include "Tank_Enemy.h"
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
    std::cout << "Block all enemy paths to the base to win!" << std::endl;
    std::cout << "Enemy types: Red=Normal(2hp), Green=Fast(2hp), Purple=Tank(15hp)" << std::endl;

    Grid grid(50, 75);
    Vec2 goal(74, 49);
    bool goalSet = false;

    SDL_Event event;
        // Game state
    bool running = true;
    std::vector<Tower> t_store;
    // Enemy setup
    std::vector<std::unique_ptr<Enemy>> enemies;

    srand(time(nullptr));

    int baseHealth = 100;
    bool gameOver = false;
    bool gameWon = false;

    auto spawnEnemies = [&]() {
        enemies.clear();
                // Spawn 100 enemies
        std::vector<int> enemyTypes;

        for(int i = 0; i < 60; i++){
            enemyTypes.push_back(0);
        }  // Normal
        for(int i = 0; i < 30; i++){
            enemyTypes.push_back(1);
        }  // Fast
        for(int i = 0; i < 10; i++){
            enemyTypes.push_back(2);
        }  // Tank

        for(int i = 99; i > 0; i--){
            int j = rand() % (i + 1);
            std::swap(enemyTypes[i], enemyTypes[j]);
        }

        for(int i = 0; i < 100; i++){
            int cx, cy;

            if(enemyTypes[i] == 2){
                int side = rand() % 4;
                int distance = 5 + rand() % 6;

                if(side == 0){
                    cx = (int)goal.x - distance;
                    cy = (int)goal.y;
                }
                else if(side == 1){
                    cx = (int)goal.x + distance;
                    cy = (int)goal.y;
                }
                else if(side == 2){
                    cx = (int)goal.x;
                    cy = (int)goal.y - distance;
                }
                else{
                    cx = (int)goal.x;
                    cy = (int)goal.y + distance;
                }

                cx = std::max(0, std::min(74, cx));
                cy = std::max(0, std::min(49, cy));
            }
            else{
                cx = rand() % 75;
                cy = rand() % 50;
            }

            if(cx == (int)goal.x && cy == (int)goal.y){
                cx = std::max(0, cx - 1);
            }

            Vec2 pos(cx * 16, cy * 16);

            if(enemyTypes[i] == 0){
                enemies.push_back(
                    std::make_unique<Enemy>(
                        pos, Vec2(16.0, 16.0),
                        Vec2(0.05, 0.0), 0.04, 100
                    )
                );
            }
            else if(enemyTypes[i] == 1){
                enemies.push_back(std::make_unique<FastEnemy>(pos));
            }
            else{
                enemies.push_back(std::make_unique<TankEnemy>(pos));
            }
        }
                // Assign enemy paths
        if(goalSet){
            for(auto& e : enemies){
                auto path = bfs(Vec2(e->getPosition().x/16, e->getPosition().y/16), goal, grid);
                for(auto& p : path) p = p * 16.0f;
                e->setPath(path);
            }
        }
        std::cout << "Enemies spawned: 100 (60 normal, 30 fast, 10 tank)" << std::endl;
    };

    auto baseIsSealed = [&]() {
        int gx = (int)goal.x;
        int gy = (int)goal.y;

        const int dx[4] = {1, -1, 0, 0};
        const int dy[4] = {0, 0, 1, -1};

        for(int k = 0; k < 4; k++){
            int nx = gx + dx[k];
            int ny = gy + dy[k];

            if(nx >= 0 && nx < 75 && ny >= 0 && ny < 50){
                if(grid.isWalkable(nx, ny))
                    return false;
            }
        }

        return true;
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
                    spawnEnemies();
                }

                if(event.button.button == SDL_BUTTON_LEFT && goalSet){
                    if(cellX == (int)goal.x && cellY == (int)goal.y)
                        continue;

                    grid.setBlocked(cellX, cellY);
                    t_store.push_back(Tower(Vec2(cellX * 16, cellY * 16),
                                            Vec2(16.0, 16.0), 100, 10, 1));

                    bool anyActiveEnemy = false;
                    bool anyReachable = false;

                    for(auto& e : enemies){
                        if(!e->isActive()) continue;

                        anyActiveEnemy = true;

                        auto newPath = bfs(
                            Vec2(e->getPosition().x / 16, e->getPosition().y / 16),
                            goal,
                            grid
                        );

                        if(!newPath.empty())
                            anyReachable = true;

                        for(auto& p : newPath)
                            p = p * 16.0f;

                        e->setPath(newPath);
                    }

                    bool sealed = baseIsSealed();
                    bool noEnemyCanReach = anyActiveEnemy && !anyReachable;

                    if(sealed || noEnemyCanReach){
                        gameWon = true;
                        running = false;
                        std::cout << "You win! All paths to the base are blocked."
                                  << std::endl;
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

        if(goalSet && !gameOver && !gameWon){
            bool anyActive = false;
            bool anyHadPath = false;
            for(auto& e : enemies){
                if(e->hasPath()) anyHadPath = true;
                if(e->hasPath() && e->isActive()) anyActive = true;
            }
            if(anyHadPath && !anyActive){
                gameWon = true;
                running = false;
                std::cout << "You win! All enemies defeated." << std::endl;
            }
        }

        // Update enemies and check damage
        if(!gameWon && !gameOver) {
            while (acc >= ft) {
                acc -= ft;
                frameNum++;

                if(goalSet) totalScore++;

                for(auto& e : enemies){
                    if(e->hasPath() && e->isActive()) e->update(ft);
                }

                for(int i = 0; i < (int)enemies.size(); i++){
                    if(gameOver || gameWon) break;

                    auto& e = enemies[i];

                    if(e->reachedGoal()){
                        int damage = e->damageToBase();

                        baseHealth -= damage;
                        e->clearReachedGoal();

                        if(damage == 15){
                            std::cout << "Tank enemy reached base! -15 HP. HP: "
                                      << baseHealth << std::endl;
                        } else {
                            std::cout << "Enemy reached base! -2 HP. HP: "
                                      << baseHealth << std::endl;
                        }

                        if(baseHealth <= 0){
                            baseHealth = 0;
                            gameOver = true;
                            running = false;
                            std::cout << "Game Over! Base destroyed." << std::endl;
                        }
                    }
                }

                std::vector<Entity*> allEntities;
            for(auto& e : enemies) allEntities.push_back(e.get());
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
                    if(!e->isActive()) continue;
                    sh.queryNearby(e->getPosition(), nearby);
                    for(int i = 0; i < (int)nearby.size(); i++){
                        if(nearby[i] == e.get()) continue;
                        if(overlaps(*e, *nearby[i])) collisionHits++;
                    }
                }
                auto tEnd = std::chrono::high_resolution_clock::now();
                float ms = std::chrono::duration<float, std::milli>(tEnd - tStart).count();
                gridTotal += ms; gridFrames++;
                benchLog << frameNum << ",grid," << ms << "," << allEntities.size() << "\n";
                }
            }
        }

        // Render game
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

        std::string info = "Time: " + std::to_string(totalScore / 60) + "s" +
                           "  HP: " + std::to_string(baseHealth) +
                           "  Mode: " + (mode == CollisionMode::HashGrid ? "HashGrid" : "Naive");
        SDL_SetWindowTitle(window, info.c_str());

        for(auto& e : enemies){
            if(e->isActive()) e->render(renderer);
        }
        for(auto& t : t_store) t.render(renderer);

        SDL_RenderPresent(renderer);
    }

    if(gameOver){
        SDL_SetWindowTitle(window, ("GAME OVER - Time: " + std::to_string(totalScore/60) + "s").c_str());
        SDL_SetRenderDrawColor(renderer, 180, 0, 0, 180);
        SDL_Rect overlay{0, 0, 1200, 800};
        SDL_RenderFillRect(renderer, &overlay);
        SDL_RenderPresent(renderer);
        SDL_Delay(2000);
    } else if(gameWon){
        SDL_SetWindowTitle(window, ("YOU WIN! Time: " + std::to_string(totalScore/60) + "s").c_str());
        SDL_SetRenderDrawColor(renderer, 0, 180, 0, 180);
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
    f << "{\"score\":" << totalScore/60 << ",\"waves\":1}";
    f.close();
    benchLog.close();

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}