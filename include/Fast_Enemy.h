#ifndef FASTENEMY_H
#define FASTENEMY_H

#include "Enemy.h"

class FastEnemy : public Enemy {
public:
    FastEnemy(Vec2 p)
        : Enemy(p, Vec2(8.0, 8.0), Vec2(0.05, 0.0), 0.08, 50) {}

    virtual void render(SDL_Renderer* renderer) override {
        SDL_SetRenderDrawColor(renderer, 30, 120, 30, 255);
        SDL_Rect outline{
            (int)position.x - 1, (int)position.y - 1,
            (int)size.x + 2, (int)size.y + 2
        };
        SDL_RenderFillRect(renderer, &outline);

        SDL_SetRenderDrawColor(renderer, 50, 220, 50, 255);
        SDL_Rect r{
            (int)position.x, (int)position.y,
            (int)size.x, (int)size.y
        };
        SDL_RenderFillRect(renderer, &r);
    }

    virtual int damageToBase() const override { return 2; }
};

#endif