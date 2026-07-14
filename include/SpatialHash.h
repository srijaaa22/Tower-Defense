#ifndef SPATIALHASH_H
#define SPATIALHASH_H
#include <vector>
#include "Entity.h"

class SpatialHash {
private:
    int rows;
    int cols;
    float cellSize;
    std::vector<std::vector<Entity*>> cells;

    int toIndex(int x, int y) const {
        return y * cols + x;
    }

public:
    SpatialHash(float cellSize, int rows, int cols)
        : cellSize(cellSize), rows(rows), cols(cols), cells(rows * cols) {}

    void rebuild(std::vector<Entity*>& entities) {
        for(auto& cell : cells) cell.clear();
        for(auto* e : entities) {
            int cx = int(e->getPosition().x / cellSize);
            int cy = int(e->getPosition().y / cellSize);
            if(cx >= 0 && cy >= 0 && cx < cols && cy < rows)
                cells[toIndex(cx, cy)].push_back(e);
        }
    }

    void queryNearby(Vec2 pos, std::vector<Entity*>& result) const {
        result.clear();
        result.reserve(32);

        int cx = int(pos.x / cellSize);
        int cy = int(pos.y / cellSize);
        for(int dx = -1; dx <= 1; dx++) {
            for(int dy = -1; dy <= 1; dy++) {
                int nx = cx + dx;
                int ny = cy + dy;
                if(nx < 0 || ny < 0 || nx >= cols || ny >= rows) continue;
                for(auto* e : cells[toIndex(nx, ny)])
                    result.push_back(e);
            }
        }
    }
};
#endif