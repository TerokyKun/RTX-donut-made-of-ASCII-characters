#ifndef MAIN_H
#define MAIN_H

// Определения констант для этого модуля.
// Они имеют внутреннее связывание (static), поэтому каждая единица трансляции получит свою копию.
static constexpr float PI = 3.14159265358979323846f;
static constexpr char GRADIENT[] = ".,-~:;=!*#@$*";
static constexpr int STAT_WIDTH = 30;
static constexpr float cubeSize = 2.0f;  // Размер куба
static constexpr float K2 = 5.0f;  // Размер доната
static constexpr float scaleFactor = 0.7f; // Коэффициент масштабирования для уменьшения размера фигур

#endif // MAIN_H
