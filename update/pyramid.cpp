#include "main.h"
#include "console.h"
#include "pyramid.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <sstream>
#include <windows.h>

using namespace std;





// Создание блока статистики для пирамиды
vector<string> createPyramidStats(int totalWidth, int totalHeight, int rotation_mode) {
    vector<string> stats;
    stats.push_back("PYRAMID SYSTEM v1.0");
    stats.push_back("------------------");
    stats.push_back("Pyramid size: 0.7");
    stats.push_back("K2: " + to_string(K2));
    stats.push_back("Shading: Flat");
    stats.push_back("3D projection");
    return stats;
}

// Вспомогательная функция для поворота точки (x,y,z) вокруг осей Y, затем X
void rotatePoint(float &x, float &y, float &z, float cosX, float sinX, float cosY, float sinY) {
    // Поворот вокруг Y
    float tx = x * cosY - z * sinY;
    float tz = x * sinY + z * cosY;
    x = tx; z = tz;
    // Поворот вокруг X
    float ty = y * cosX - z * sinX;
    float tz2 = y * sinX + z * cosX;
    y = ty; z = tz2;
}

// Вспомогательная функция: векторное произведение двух векторов
void crossProduct(float ax, float ay, float az,
                  float bx, float by, float bz,
                  float &rx, float &ry, float &rz) {
    rx = ay * bz - az * by;
    ry = az * bx - ax * bz;
    rz = ax * by - ay * bx;
}

// Нормализация вектора
void normalize(float &x, float &y, float &z) {
    float mag = sqrt(x * x + y * y + z * z);
    if(mag != 0) {
        x /= mag;
        y /= mag;
        z /= mag;
    }
}

// Функция рендеринга пирамиды
// Пирамиду будем строить из 5 граней:
//   - Лицевая (базовая) грань (квадрат) – face 0
//   - 4 боковые треугольные грани – face 1,2,3,4
void renderPyramid(float angleX, float angleY, int totalWidth, int totalHeight, const vector<string>& stats) {
    // Область для отрисовки фигуры (без статистики)
    int renderWidth = totalWidth - STAT_WIDTH;
    if (renderWidth < 20) renderWidth = totalWidth;

    vector<char> buffer(totalWidth * totalHeight, ' ');
    vector<float> zbuffer(renderWidth * totalHeight, 0.0f);

    // Коэффициент перспективного масштабирования (аналог K1)
    float K1 = renderWidth * K2 * 3 / 8;

    float cosX = cos(angleX), sinX = sin(angleX);
    float cosY = cos(angleY), sinY = sin(angleY);

    // Вектор источника света (например, сверху)
    float Lx = 0.0f, Ly = 1.0f, Lz = 0.0f;

    // =====================================
    // Обработка базовой (нижней) квадратной грани (face 0)
    // =====================================
    {
        float step = 0.05f;
        // Базовая грань – квадрат в плоскости y = -1:
        // Вершины: v0=(-1,-1,-1), v1=(1,-1,-1), v2=(1,-1,1), v3=(-1,-1,1)
        // Для вычисления нормали поворачиваем несколько вершин
        float v0[3] = { -1 * scaleFactor, -1 * scaleFactor, -1 * scaleFactor };
        float v1[3] = {  1 * scaleFactor, -1 * scaleFactor, -1 * scaleFactor };
        float v3[3] = { -1 * scaleFactor, -1 * scaleFactor,  1 * scaleFactor };
        rotatePoint(v0[0], v0[1], v0[2], cosX, sinX, cosY, sinY);
        rotatePoint(v1[0], v1[1], v1[2], cosX, sinX, cosY, sinY);
        rotatePoint(v3[0], v3[1], v3[2], cosX, sinX, cosY, sinY);
        float edge1[3] = { v1[0] - v0[0], v1[1] - v0[1], v1[2] - v0[2] };
        float edge2[3] = { v3[0] - v0[0], v3[1] - v0[1], v3[2] - v0[2] };
        float nx, ny, nz;
        crossProduct(edge1[0], edge1[1], edge1[2],
                     edge2[0], edge2[1], edge2[2],
                     nx, ny, nz);
        normalize(nx, ny, nz);
        float brightness = nx * Lx + ny * Ly + nz * Lz;
        if (brightness < 0) brightness = 0;
        int luminance_index = static_cast<int>(brightness * 11);
        if (luminance_index < 0) luminance_index = 0;
        if (luminance_index > 11) luminance_index = 11;
        char faceChar = GRADIENT[luminance_index];

        // Функция для выбора символа рамки для базовой грани
        auto getBorderChar = [&](float u, float v) -> char {
            const float tol = step / 2;
            bool left  = (fabs(u + 1.0f) < tol);
            bool right = (fabs(u - 1.0f) < tol);
            bool top   = (fabs(v - 1.0f) < tol);
            bool bot   = (fabs(v + 1.0f) < tol);
            if (left && top)      return '/';
            if (right && top)     return '\\';
            if (left && bot)      return '\\';
            if (right && bot)     return '/';
            if (top)              return '_';
            if (bot)              return '-';
            if (left || right)    return '|';
            return faceChar;
        };

        // Параметризация: u, v ∈ [–1, 1]
        for (float u = -1.0f; u <= 1.0f; u += step) {
            for (float v = -1.0f; v <= 1.0f; v += step) {
                float x = u * scaleFactor;
                float y = -1.0f * scaleFactor;
                float z = v * scaleFactor;
                rotatePoint(x, y, z, cosX, sinX, cosY, sinY);
                float zOffset = z + K2;
                if (zOffset <= 0) continue;
                float ooz = 1.0f / zOffset;
                int xp = static_cast<int>(renderWidth / 2 + K1 * ooz * x);
                int yp = static_cast<int>(totalHeight / 2 - (K1 / 2) * ooz * y);
                bool isEdge = (fabs(u + 1.0f) < (step * 1.1f)) ||
                              (fabs(u - 1.0f) < (step * 1.1f)) ||
                              (fabs(v + 1.0f) < (step * 1.1f)) ||
                              (fabs(v - 1.0f) < (step * 1.1f));
                char pixel = isEdge ? getBorderChar(u, v) : faceChar;
                if (xp >= 0 && xp < renderWidth && yp >= 0 && yp < totalHeight) {
                    int idx = xp + yp * renderWidth;
                    if (ooz > zbuffer[idx]) {
                        zbuffer[idx] = ooz;
                        buffer[xp + yp * totalWidth] = pixel;
                    }
                }
            }
        }
    }

    // =====================================
    // Обработка боковых (треугольных) граней (face 1..4)
    // =====================================
    auto processTriangleFace = [&](int face) {
        // Задаём вершины для текущей боковой грани
        // Используем следующие определения:
        // Face 1 (front): v0 = (-1,-1,-1), v1 = (1,-1,-1), v2 = (0,1,0)
        // Face 2 (right): v0 = (1,-1,-1), v1 = (1,-1,1), v2 = (0,1,0)
        // Face 3 (back):  v0 = (1,-1,1),  v1 = (-1,-1,1), v2 = (0,1,0)
        // Face 4 (left):  v0 = (-1,-1,1), v1 = (-1,-1,-1), v2 = (0,1,0)
        float v0[3], v1[3], v2[3];
        switch(face) {
            case 1:
                v0[0] = -1; v0[1] = -1; v0[2] = -1;
                v1[0] =  1; v1[1] = -1; v1[2] = -1;
                v2[0] =  0; v2[1] =  1; v2[2] =  0;
                break;
            case 2:
                v0[0] =  1; v0[1] = -1; v0[2] = -1;
                v1[0] =  1; v1[1] = -1; v1[2] =  1;
                v2[0] =  0; v2[1] =  1; v2[2] =  0;
                break;
            case 3:
                v0[0] =  1; v0[1] = -1; v0[2] =  1;
                v1[0] = -1; v1[1] = -1; v1[2] =  1;
                v2[0] =  0; v2[1] =  1; v2[2] =  0;
                break;
            case 4:
                v0[0] = -1; v0[1] = -1; v0[2] =  1;
                v1[0] = -1; v1[1] = -1; v1[2] = -1;
                v2[0] =  0; v2[1] =  1; v2[2] =  0;
                break;
        }
        // Применяем масштабирование
        for (int i = 0; i < 3; i++) {
            v0[i] *= scaleFactor;
            v1[i] *= scaleFactor;
            v2[i] *= scaleFactor;
        }
        // Вычисляем плоскую нормаль грани (на основе повернутых вершин)
        float rv0[3] = { v0[0], v0[1], v0[2] };
        float rv1[3] = { v1[0], v1[1], v1[2] };
        float rv2[3] = { v2[0], v2[1], v2[2] };
        rotatePoint(rv0[0], rv0[1], rv0[2], cosX, sinX, cosY, sinY);
        rotatePoint(rv1[0], rv1[1], rv1[2], cosX, sinX, cosY, sinY);
        rotatePoint(rv2[0], rv2[1], rv2[2], cosX, sinX, cosY, sinY);
        float edge1[3] = { rv1[0]-rv0[0], rv1[1]-rv0[1], rv1[2]-rv0[2] };
        float edge2[3] = { rv2[0]-rv0[0], rv2[1]-rv0[1], rv2[2]-rv0[2] };
        float nx, ny, nz;
        crossProduct(edge1[0], edge1[1], edge1[2],
                     edge2[0], edge2[1], edge2[2],
                     nx, ny, nz);
        normalize(nx, ny, nz);
        float brightness = nx * Lx + ny * Ly + nz * Lz;
        if (brightness < 0) brightness = 0;
        int luminance_index = static_cast<int>(brightness * 11);
        if (luminance_index < 0) luminance_index = 0;
        if (luminance_index > 11) luminance_index = 11;
        char faceChar = GRADIENT[luminance_index];

        // Функция для выбора символа рамки в параметризации треугольника
        auto getBorderChar = [&](float u, float v) -> char {
            const float tol = 0.05f / 2;
            bool border = (fabs(u) < tol) || (fabs(v) < tol) || (fabs(u + v - 1.0f) < tol);
            if (!border) return faceChar;
            // Выбор символа рамки по краю параметризации
            if (fabs(u) < tol && fabs(v) < tol) return '/';
            if (fabs(u) < tol) return '|';
            if (fabs(v) < tol) return '_';
            if (fabs(u + v - 1.0f) < tol) return '\\';
            return faceChar;
        };

        // Параметризация треугольника с использованием барицентрических координат:
        // u от 0 до 1, v от 0 до 1-u. Точка = (1 - u - v)*v0 + u*v1 + v*v2.
        for (float u = 0.0f; u <= 1.0f; u += 0.05f) {
            for (float v = 0.0f; v <= 1.0f - u; v += 0.05f) {
                float x = (1 - u - v) * v0[0] + u * v1[0] + v * v2[0];
                float y = (1 - u - v) * v0[1] + u * v1[1] + v * v2[1];
                float z = (1 - u - v) * v0[2] + u * v1[2] + v * v2[2];
                rotatePoint(x, y, z, cosX, sinX, cosY, sinY);
                float zOffset = z + K2;
                if (zOffset <= 0) continue;
                float ooz = 1.0f / zOffset;
                int xp = static_cast<int>(renderWidth / 2 + K1 * ooz * x);
                int yp = static_cast<int>(totalHeight / 2 - (K1 / 2) * ooz * y);
                bool isEdge = (fabs(u) < 0.06f) || (fabs(v) < 0.06f) || (fabs(u + v - 1.0f) < 0.06f);
                char pixel = isEdge ? getBorderChar(u, v) : faceChar;
                if (xp >= 0 && xp < renderWidth && yp >= 0 && yp < totalHeight) {
                    int idx = xp + yp * renderWidth;
                    if (ooz > zbuffer[idx]) {
                        zbuffer[idx] = ooz;
                        buffer[xp + yp * totalWidth] = pixel;
                    }
                }
            }
        }
    };

    // Обрабатываем все боковые грани (face 1..4)
    for (int face = 1; face <= 4; face++) {
        processTriangleFace(face);
    }

    // Вывод блока статистики справа
    int statStartCol = renderWidth + 1;
    for (size_t i = 0; i < stats.size() && i < static_cast<size_t>(totalHeight); i++) {
        const string &line = stats[i];
        for (size_t j = 0; j < line.size() && (statStartCol + j) < static_cast<size_t>(totalWidth); j++) {
            buffer[i * totalWidth + statStartCol + j] = line[j];
        }
    }

    // Вывод финального буфера в консоль
    setCursorPosition(0, 0);
    for (int i = 0; i < totalHeight; i++) {
        for (int j = 0; j < totalWidth; j++) {
            putchar(buffer[i * totalWidth + j]);
        }
        putchar('\n');
    }
}
