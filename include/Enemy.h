#ifndef ENEMY_H
#define ENEMY_H
#include <vector>
#include "Vec2.h"
#include "Entity.h"

class Enemy : public Entity{
    protected:
        Vec2 velocity;
        float speed;
        float health;
        std::vector<Vec2> path;
        int pathIndex = 0;
        bool active = true;
        bool pathAssigned = false;

    public:
        Enemy(Vec2 p, Vec2 s, Vec2 v, float sp, float h): Entity(p,s), velocity(v), speed(sp), health(h) {}
        
        void setPath(std::vector<Vec2> p){
            path = p;
            pathIndex = 0;
            active = true;
            pathAssigned = true;
        }
        
        virtual void update(float dt){
            if(path.empty() || pathIndex >= (int)path.size()){
                active = false;
                return;
            }

            Vec2 target = path[pathIndex];
            Vec2 dir = target + Vec2(-position.x, -position.y);
            float len = dir.length();
            
            if(len < speed * dt) pathIndex++;
            else {
                Vec2 norm = dir.normalized();
                position = position + norm * speed * dt;
            }
        }

        bool isActive() const { 
            return active; 
        }
        bool hasPath() const { 
            return pathAssigned; 
        }

        virtual void render(SDL_Renderer* renderer){
            SDL_SetRenderDrawColor(renderer, 240, 235, 220, 255);
            SDL_Rect outline{(int)position.x - 1, (int)position.y - 1, (int)size.x + 2, (int)size.y + 2};
            SDL_RenderFillRect(renderer, &outline);

            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_Rect r{(int)position.x, (int)position.y, (int)size.x, (int)size.y};
            SDL_RenderFillRect(renderer, &r); 
        }
};
#endif