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
            if(x<0 || y<0 || x>=col || y>=row) return false;
            if(grid[y][x]) return true;
            else return false;
        }
        void setBlocked(int x, int y){
            grid[y][x]=false;
        }
        int getRows() const { return row; }
        int getCols() const { return col; }
};
#endif