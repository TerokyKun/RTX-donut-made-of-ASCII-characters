#ifndef DONUT_H
#define DONUT_H

#include <vector>
#include <string>

// Убираем внешние переменные, так как они уже определены в main.cpp
extern float R1;  // Радиус внутреннего круга
extern float R2;  // Радиус внешнего круга


// Убираем createStats и renderDonut параметры, так как они не требуют повторного объявления
std::vector<std::string> createDonutStats(int totalWidth, int totalHeight, int rotation_mode);
void renderDonut(float A, float B, int totalWidth, int totalHeight, const std::vector<std::string>& stats);

#endif // DONUT_H
