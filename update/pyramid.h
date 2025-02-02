#ifndef PYRAMID_H
#define PYRAMID_H

#include <vector>
#include <string>

// Создаёт статистический блок для отображения данных о пирамиде
std::vector<std::string> createPyramidStats(int totalWidth, int totalHeight, int rotation_mode);

// Рендерит пирамиду с углами поворота A и B, размером окна totalWidth x totalHeight,
// а также выводит статистику (справа)
void renderPyramid(float A, float B, int totalWidth, int totalHeight, const std::vector<std::string>& stats);

#endif // PYRAMID_H
