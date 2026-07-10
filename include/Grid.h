#ifndef GRID_H
#define GRID_H
#include <vector>

class Grid{
    protected:
        int row;
        int col;
        std::vector<std::vector<bool>> grid;
    public:
        Grid(int r, int c): row(r), col(c), grid(r, std::vector<bool>(c,true)){}
        bool isWalkable(int x, int y) const {
            if(grid[x][y]) return true;
            else return false;
        }
        void setBlocked(int x, int y){
            grid[x][y]=false;
        }
};
#endif