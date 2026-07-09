#ifndef TOWER_H
#define TOWER_H
#include <SDL2/SDL.h>
#include "Vec2.h"
#include "Entity.h"

class Tower: public Entity{
    protected:
        float range;
        float damage;
        float fire_rate;
    public:
        Tower(Vec2 p, Vec2 s, float r, float d, float fr): Entity(p,s), range(r), damage(d), fire_rate(fr) {}
        virtual void update(float dt){}
        virtual void render(SDL_Renderer* renderer){
            SDL_Rect r{position.x, position.y, size.x, size.y};
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
            SDL_RenderFillRect(renderer, &r);
        }
};
#endif