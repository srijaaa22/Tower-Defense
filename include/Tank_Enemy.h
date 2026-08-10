#ifndef TANKENEMY_H
#define TANKENEMY_H

#include "Enemy.h"

class TankEnemy : public Enemy {
public:
    TankEnemy(Vec2 p)
        : Enemy(p, Vec2(24.0, 24.0), Vec2(0.05, 0.0), 0.02, 300) {}

    virtual void render(SDL_Renderer* renderer) override {
        SDL_SetRenderDrawColor(renderer, 120, 0, 160, 255);
        SDL_Rect outline{
            (int)position.x - 1, (int)position.y - 1,
            (int)size.x + 2, (int)size.y + 2
        };
        SDL_RenderFillRect(renderer, &outline);

        SDL_SetRenderDrawColor(renderer, 180, 50, 220, 255);
        SDL_Rect r{
            (int)position.x, (int)position.y,
            (int)size.x, (int)size.y
        };
        SDL_RenderFillRect(renderer, &r);
    }

    virtual int damageToBase() const override { return 15; }
};

#endif