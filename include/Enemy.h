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
        bool active=true;

    public:
        Enemy(Vec2 p, Vec2 s, Vec2 v, float sp, float h): Entity(p,s), velocity(v), speed(sp), health(h) {}
        
        void setPath(std::vector<Vec2> p){
            path = p;
            pathIndex = 0;
        }
        
        virtual void update(float dt){
            if(path.empty() || pathIndex >= path.size()){
                active=false;
                return;
            }

            Vec2 target = path[pathIndex];
            Vec2 dir = target + Vec2(-position.x, -position.y);
            float len = dir.length();
            
            if(len < speed*dt) pathIndex++;
            else {
                Vec2 norm = dir.normalized();
                position = position + norm * speed * dt;
            }
        }
        bool isActive() const {
            return active; 
        }

        virtual void render(SDL_Renderer* renderer){
            SDL_Rect r{position.x, position.y, size.x, size.y};
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderFillRect(renderer, &r); 
        }
        
};
#endif