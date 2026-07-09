#ifndef SPATIALHASH_H
#define SPATIALHASH_H
#include <vector>
#include <unordered_map>
#include "Entity.h"

struct CellKey{
    int x;
    int y;
    bool operator==(const CellKey &other) const {
        return x==other.x && y==other.y;
    }
};

struct CellKeyhash{
    size_t operator()(const CellKey &k) const {
        return std::hash<int>()(k.x) ^ (std::hash<int>()(k.y) << 1);
    }
};

class SpatialHash{
    private:
        float cellSize;
        std::unordered_map <CellKey, std::vector<Entity*>, CellKeyhash> grid;
    public:
        SpatialHash(float c) : cellSize(c) {}

        CellKey cellOf(Vec2 pos){
            return CellKey{int(pos.x/cellSize), int(pos.y/cellSize)};
        }

        void insert(Entity* e){
            CellKey c = cellOf(e->getPosition());
            grid[c].push_back(e);
        }

        std::vector<Entity*> queryNearby(Vec2 pos){
            CellKey c = cellOf(pos);
            std::vector<Entity*> res;

            for(int dx=-1; dx<=1; dx++){
                for(int dy=-1; dy<=1; dy++){
                    if(grid.find(CellKey{c.x+dx, c.y+dy})!=grid.end()){
                        auto& cell = grid[CellKey{c.x+dx, c.y+dy}];
                        res.insert(res.end(), cell.begin(), cell.end());
                    }
                }
            }
            return res;
        }

        void rebuild(std::vector<Entity*> &entities){
            grid.clear();
            for(auto it : entities){
                insert(it);
            }
        }
};
#endif

