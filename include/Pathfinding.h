#ifndef PATHFINDING_H
#define PATHFINDING_H
#include <vector>
#include <queue>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include "Vec2.h"
#include "Grid.h"

struct Vec2hash{
    size_t operator()(const Vec2 &k) const {
        return std::hash<float>()(k.x) ^ (std::hash<float>()(k.y) << 1);
    }
};

std::vector<Vec2>bfs(Vec2 start, Vec2 goal, const Grid &grid){
    std::queue<Vec2> q;
    std::unordered_set<Vec2, Vec2hash> vis;
    std::unordered_map<Vec2, Vec2, Vec2hash> cameFrom;

    q.push(start);
    vis.insert(start);

    while(!q.empty()){
        Vec2 node = q.front();  
        q.pop();
        if(node==goal) break;
        std::vector<int> dx = {-1, 0, 1, 0};
        std::vector<int> dy = {0, -1, 0, 1};
        for(int i=0; i<4; i++){
            Vec2 neighbour = node + Vec2(dx[i], dy[i]);
            if(grid.isWalkable(int(neighbour.x), int(neighbour.y)) && vis.find(neighbour) == vis.end()){
                vis.insert(neighbour);
                cameFrom[neighbour] = node;
                q.push(neighbour);
            }
        }
    }

    std::vector<Vec2> path;
    Vec2 step = goal;
    while(!(step == start)){
        path.push_back(step);
        step = cameFrom[step];
    }
    std::reverse(path.begin(), path.end());
    return path;
}

#endif