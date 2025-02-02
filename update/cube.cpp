#include "main.h"
#include "console.h"
#include "cube.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <sstream>
#include <windows.h>

using namespace std;





// Создание блока статистики для отображения справа (для куба)
vector<string> createCubeStats(int totalWidth, int totalHeight, int rotation_mode) {
    vector<string> stats;
    stats.push_back("CUBE SYSTEM v1.0");
    stats.push_back("----------------");
    stats.push_back("Cube size: 0.7");
    stats.push_back("K2: " + to_string(K2));
    stats.push_back("Shading: Flat (Top lit)");
    stats.push_back("3D projection");
    return stats;
}

// Функция рендеринга куба с чёткими гранями, z-буфером и плоским освещением.
// angleX, angleY - углы поворота куба.
// totalWidth, totalHeight - размеры консольного окна (в символах).
// stats - статистика, отображаемая справа.
void renderCube(float angleX, float angleY, int totalWidth, int totalHeight, const vector<string>& stats) {
    // Вычисляем ширину области для куба (без статистики)
    int cubeWidth = totalWidth - STAT_WIDTH;
    if (cubeWidth < 20) cubeWidth = totalWidth;

    // Буфер для символов и z-буфер для области куба
    vector<char> buffer(totalWidth * totalHeight, ' ');
    vector<float> zbuffer(cubeWidth * totalHeight, 0.0f);

    // Коэффициент перспективного масштабирования (аналог K1)
    float K1 = cubeWidth * K2 * 3 / 8;

    // Предвычисляем синусы и косинусы для поворотов
    float cosX = cos(angleX), sinX = sin(angleX);
    float cosY = cos(angleY), sinY = sin(angleY);

    // Источник света направлен сверху (чтобы верхняя грань была самой яркой)
    float Lx = 0.0f, Ly = 1.0f, Lz = 0.0f;

    // Функция для обработки одной грани куба (face от 0 до 5)
    auto processFace = [&](int face) {
        float step = 0.05f; // шаг параметризации грани
        // Задаём базовую нормаль для грани (до поворота)
        float baseNx = 0, baseNy = 0, baseNz = 0;
        switch(face) {
            case 0: // front face (z = 1)
                baseNx = 0; baseNy = 0; baseNz = 1;
                break;
            case 1: // back face (z = -1)
                baseNx = 0; baseNy = 0; baseNz = -1;
                break;
            case 2: // right face (x = 1)
                baseNx = 1; baseNy = 0; baseNz = 0;
                break;
            case 3: // left face (x = -1)
                baseNx = -1; baseNy = 0; baseNz = 0;
                break;
            case 4: // top face (y = 1)
                baseNx = 0; baseNy = 1; baseNz = 0;
                break;
            case 5: // bottom face (y = -1)
                baseNx = 0; baseNy = -1; baseNz = 0;
                break;
        }
        // Поворот нормали грани (сначала вокруг Y, затем вокруг X)
        float tNx = baseNx * cosY - baseNz * sinY;
        float tNz = baseNx * sinY + baseNz * cosY;
        float nx = tNx;
        float nz = tNz;
        float ny = baseNy;
        float tny = ny * cosX - nz * sinX;
        float tnz = ny * sinX + nz * cosX;
        ny = tny; nz = tnz;
        // Вычисляем яркость для всей грани (плоское освещение)
        float brightness = nx * Lx + ny * Ly + nz * Lz;
        if (brightness < 0) brightness = 0;
        int luminance_index = static_cast<int>(brightness * 11);
        if (luminance_index < 0) luminance_index = 0;
        if (luminance_index > 11) luminance_index = 11;
        char faceChar = GRADIENT[luminance_index];

        // При отрисовке границ будем подставлять другие символы:
        // для горизонтальных краёв – '_' (верх) и '-' (низ),
        // для вертикальных – '|', а для углов – специальные символы.
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

        // Проходим по параметрам u и v для текущей грани
        for (float u = -1.0f; u <= 1.0f; u += step) {
            for (float v = -1.0f; v <= 1.0f; v += step) {
                // Вычисляем 3D координаты точки на грани с учётом масштабирования
                float x, y, z;
                switch(face) {
                    case 0: // front face (z = 1)
                        x = u * scaleFactor;
                        y = v * scaleFactor;
                        z = 1.0f * scaleFactor;
                        break;
                    case 1: // back face (z = -1)
                        x = u * scaleFactor;
                        y = v * scaleFactor;
                        z = -1.0f * scaleFactor;
                        break;
                    case 2: // right face (x = 1)
                        x = 1.0f * scaleFactor;
                        y = u * scaleFactor;
                        z = v * scaleFactor;
                        break;
                    case 3: // left face (x = -1)
                        x = -1.0f * scaleFactor;
                        y = u * scaleFactor;
                        z = v * scaleFactor;
                        break;
                    case 4: // top face (y = 1)
                        x = u * scaleFactor;
                        y = 1.0f * scaleFactor;
                        z = v * scaleFactor;
                        break;
                    case 5: // bottom face (y = -1)
                        x = u * scaleFactor;
                        y = -1.0f * scaleFactor;
                        z = v * scaleFactor;
                        break;
                    default:
                        x = y = z = 0;
                }
                // Поворот точки вокруг оси Y
                float tx = x * cosY - z * sinY;
                float tz = x * sinY + z * cosY;
                x = tx;  z = tz;
                // Поворот точки вокруг оси X
                float ty = y * cosX - z * sinX;
                float tz2 = y * sinX + z * cosX;
                y = ty;  z = tz2;

                // Перспективная проекция
                float zOffset = z + K2;
                if (zOffset <= 0) continue;
                float ooz = 1.0f / zOffset;
                int xp = static_cast<int>(cubeWidth / 2 + K1 * ooz * x);
                int yp = static_cast<int>(totalHeight / 2 - (K1 / 2) * ooz * y);

                // Определяем, использовать ли символ рамки или внутренний символ грани.
                // Если точка близка к одному из краёв параметризации, считаем её краевой.
                bool isEdge = (fabs(u + 1.0f) < (step * 1.1f)) ||
                              (fabs(u - 1.0f) < (step * 1.1f)) ||
                              (fabs(v + 1.0f) < (step * 1.1f)) ||
                              (fabs(v - 1.0f) < (step * 1.1f));
                char pixel = isEdge ? getBorderChar(u, v) : faceChar;

                // Если точка попадает в область куба, обновляем z-буфер и буфер символов
                if (xp >= 0 && xp < cubeWidth && yp >= 0 && yp < totalHeight) {
                    int idx = xp + yp * cubeWidth;
                    if (ooz > zbuffer[idx]) {
                        zbuffer[idx] = ooz;
                        buffer[xp + yp * totalWidth] = pixel;
                    }
                }
            }
        }
    };

    // Обрабатываем все 6 граней куба
    for (int face = 0; face < 6; face++) {
        processFace(face);
    }

    // Отображаем блок статистики справа
    int statStartCol = cubeWidth + 1;
    for (size_t i = 0; i < stats.size() && i < static_cast<size_t>(totalHeight); i++) {
        const string &line = stats[i];
        for (size_t j = 0; j < line.size() && (statStartCol + j) < static_cast<size_t>(totalWidth); j++) {
            buffer[i * totalWidth + statStartCol + j] = line[j];
        }
    }

    // Выводим буфер на экран
    setCursorPosition(0, 0);
    for (int i = 0; i < totalHeight; i++) {
        for (int j = 0; j < totalWidth; j++) {
            putchar(buffer[i * totalWidth + j]);
        }
        putchar('\n');
    }
}
