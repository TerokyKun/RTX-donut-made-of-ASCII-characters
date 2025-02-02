#ifndef CUBE_H
#define CUBE_H

#include <vector>
#include <string>


std::vector<std::string> createCubeStats(int totalWidth, int totalHeight, int rotation_mode);
void renderCube(float A, float B, int totalWidth, int totalHeight, const std::vector<std::string>& stats);


#endif // CUBE_H
