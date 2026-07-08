#ifndef ENEMY_H
#define ENEMY_H
#include "Vec2.h"
#include "Entity.h"

class Enemy : public Entity{
    protected:
        Vec2 velocity;
        float speed;
        float health;
    public:
        Enemy(Vec2 p, Vec2 s, Vec2 v, float sp, float h): Entity(p,s), velocity(v), speed(sp), health(h) {}
        virtual void update(float dt){
            position = position + velocity*dt;
        }
        virtual void render(SDL_Renderer* renderer){
            SDL_Rect r{position.x, position.y, size.x, size.y};
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderFillRect(renderer, &r); 
        }  
};
#endif