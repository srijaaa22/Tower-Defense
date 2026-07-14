#ifndef PATHFINDING_H
#define PATHFINDING_H
#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <unordered_map>
#include "Vec2.h"
#include "Grid.h"

std::vector<Vec2> bfs(Vec2 start, Vec2 goal, const Grid& grid) {
    int rows = grid.getRows();
    int cols = grid.getCols();

    auto toKey = [cols](int x, int y) { return y * cols + x; };

    std::vector<std::vector<bool>> vis(rows, std::vector<bool>(cols, false));
    std::unordered_map<int, int> cameFrom;
    std::queue<Vec2> q;

    int sx = int(start.x), sy = int(start.y);
    int gx = int(goal.x),  gy = int(goal.y);

    q.push(start);
    vis[sy][sx] = true;

    std::vector<int> dx = {0,-1,0,1};
    std::vector<int> dy= {-1,0,1,0};

    bool found = false;
    while (!q.empty()) {
        Vec2 node = q.front();
        q.pop();

        int nx = int(node.x), ny = int(node.y);
        if (nx == gx && ny == gy) { found = true; break; }

        for (int i = 0; i < 4; i++) {
            int nnx = nx + dx[i];
            int nny = ny + dy[i];

            if (nnx < 0 || nny < 0 || nnx >= cols || nny >= rows) continue;
            if (!grid.isWalkable(nnx, nny)) continue;
            if (vis[nny][nnx]) continue;

            vis[nny][nnx] = true;
            cameFrom[toKey(nnx, nny)] = toKey(nx, ny);
            q.push(Vec2(nnx, nny));
        }
    }

    if (!found) return {};

    std::vector<Vec2> path;
    int cx = gx, cy = gy;
    while (cx != sx || cy != sy) {
        path.push_back(Vec2(cx, cy));
        int key = cameFrom[toKey(cx, cy)];
        cx = key % cols;
        cy = key / cols;
    }
    std::reverse(path.begin(), path.end());
    return path;
}

#endif