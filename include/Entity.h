#ifndef ENTITY_H
#define ENTITY_H
#include "Vec2.h"
#include <SDL2/SDL.h>

class Entity{
    protected: 
        Vec2 position;
        Vec2 size;
        Entity(Vec2 p, Vec2 s) : position(p), size(s) {} 
    public:
        virtual void update(float dt) = 0;
        virtual void render(SDL_Renderer* renderer) = 0;
        virtual ~Entity() = default;
        Vec2 getPosition() const { 
            return position;
        }
        Vec2 getSize() const { 
            return size;
        }
};
#endif