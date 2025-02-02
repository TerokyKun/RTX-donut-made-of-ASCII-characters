#include "main.h"
#include "donut.h"
#include "console.h" // Добавляем объявление setCursorPosition
#include <iostream>
#include <vector>
#include <cmath>
#include <sstream>
#include <windows.h>

using namespace std;

// Глобальные переменные
float R1 = 1.0f;
float R2 = 2.0f;



vector<string> createDonutStats(int totalWidth, int totalHeight, int rotation_mode) {
    vector<string> stats;
    stats.push_back("DONUT SYSTEM v1.0");
    stats.push_back("---------------");

    ostringstream oss;
    oss << "R1: " << R1; stats.push_back(oss.str());
    oss.str(""); oss.clear();
    
    oss << "R2: " << R2; stats.push_back(oss.str());
    oss.str(""); oss.clear();
    
    oss << "K2: " << K2; stats.push_back(oss.str());
    oss.str(""); oss.clear();

    oss << "Donut Area: " << (totalWidth - STAT_WIDTH) << "x" << totalHeight;
    stats.push_back(oss.str());
    oss.str(""); oss.clear();

    oss << "Rotation Mode: " << rotation_mode;
    stats.push_back(oss.str());

    stats.push_back("YUM! THIS IS");
    stats.push_back("a REAL DONUT!");
    stats.push_back("System overload...");

    return stats;
}

void renderDonut(float A, float B, int totalWidth, int totalHeight, const vector<string>& stats) {
    int donutWidth = totalWidth - STAT_WIDTH;
    if (donutWidth < 20) donutWidth = totalWidth;
    
    vector<char> buffer(totalWidth * totalHeight, ' ');
    vector<float> zbuffer(donutWidth * totalHeight, 0.0f);
    
    float K1 = donutWidth * K2 * 3 / (8 * (R1 + R2));

    for (float theta = 0; theta < 2 * PI; theta += 0.07f) {
        for (float phi = 0; phi < 2 * PI; phi += 0.02f) {
            float cosTheta = cos(theta), sinTheta = sin(theta);
            float cosPhi = cos(phi), sinPhi = sin(phi);
            float cosA = cos(A), sinA = sin(A);
            float cosB = cos(B), sinB = sin(B);

            float circleX = R2 + R1 * cosTheta;
            float circleY = R1 * sinTheta;

            float x = circleX * (cosB * cosPhi + sinA * sinB * sinPhi) - circleY * cosA * sinB;
            float y = circleX * (sinB * cosPhi - sinA * cosB * sinPhi) + circleY * cosA * cosB;
            float z = K2 + cosA * circleX * sinPhi + circleY * sinA;
            float ooz = 1.0f / z;

            int xp = static_cast<int>(donutWidth / 2 + K1 * ooz * x);
            int yp = static_cast<int>(totalHeight / 2 - (K1 / 2) * ooz * y);

            if (xp >= 0 && xp < donutWidth && yp >= 0 && yp < totalHeight) {
                int idx = xp + yp * donutWidth;
                float L = cosPhi * cosTheta * sinB - cosA * cosTheta * sinPhi - sinA * sinTheta +
                          cosB * (cosA * sinTheta - cosTheta * sinA * sinPhi);
                if (L > 0 && ooz > zbuffer[idx]) {
                    zbuffer[idx] = ooz;
                    int luminance_index = static_cast<int>(L * 8);
                    if (luminance_index < 0) luminance_index = 0;
                    if (luminance_index > 11) luminance_index = 11;
                    buffer[xp + yp * totalWidth] = GRADIENT[luminance_index];
                }
            }
        }
    }

    // Отображаем статистику
    int statStartCol = donutWidth + 1;
    for (size_t i = 0; i < stats.size() && i < static_cast<size_t>(totalHeight); i++) {
        const string &line = stats[i];
        for (size_t j = 0; j < line.size() && (statStartCol + j) < static_cast<size_t>(totalWidth); j++) {
            buffer[i * totalWidth + statStartCol + j] = line[j];
        }
    }

    setCursorPosition(0, 0);
    for (int i = 0; i < totalHeight; i++) {
        for (int j = 0; j < totalWidth; j++) {
            putchar(buffer[i * totalWidth + j]);
        }
        putchar('\n');
    }
}
