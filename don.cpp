#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <thread>
#include <atomic>
#include <sstream>
#include "lib/window/window.h"

using namespace std;

// Параметры пончика
constexpr float R1 = 1.0f;
constexpr float R2 = 2.0f;
constexpr float K2 = 16.0f;
constexpr float PI = 3.14159265358979323846f;

// Градиент для отрисовки
constexpr char GRADIENT[] = ".,-~:;=!*#$@";

// Резервированная ширина для статистики (справа)
constexpr int STAT_WIDTH = 30;

atomic<bool> running(true);
int rotation_mode = 1;

// Функция получения размеров консоли


// Функция установки курсора в позицию (x, y)

// Функция формирования блока статической статистики
vector<string> createStats(int totalWidth, int totalHeight) {
    // Если окно слишком узкое, можно оставить пустой блок или формировать статистику по-другому
    vector<string> stats;
    stats.push_back("DONUT SYSTEM v1.0");
    stats.push_back("---------------");
    
    { ostringstream oss; oss << "R1: " << R1; stats.push_back(oss.str()); }
    { ostringstream oss; oss << "R2: " << R2; stats.push_back(oss.str()); }
    { ostringstream oss; oss << "K2: " << K2; stats.push_back(oss.str()); }
    
    { ostringstream oss; oss << "Donut Area: " << (totalWidth - STAT_WIDTH) << "x" << totalHeight; stats.push_back(oss.str()); }
    { ostringstream oss; oss << "Rotation Mode: " << rotation_mode; stats.push_back(oss.str()); }
    
    // Добавляем ещё один или два информационных сообщения
    stats.push_back("YUM! THIS IS");
    stats.push_back("a REAL DONUT!");
    stats.push_back("System overload...");
    
    return stats;
}

// Функция отрисовки кадра (передаём также блок статистики, сформированный заранее)
void renderFrame(float A, float B, int totalWidth, int totalHeight, const vector<string>& stats) {
    int donutWidth = totalWidth - STAT_WIDTH;
    if (donutWidth < 20)
        donutWidth = totalWidth;
    
    // Буфер для всего экрана
    vector<char> buffer(totalWidth * totalHeight, ' ');
    // Z-буфер для области пончика
    vector<float> zbuffer(donutWidth * totalHeight, 0.0f);
    
    // Вычисляем масштаб, чтобы пончик вписывался в выделенную область
    float K1 = donutWidth * K2 * 3 / (8 * (R1 + R2));
    
    // Отрисовка пончика (левый блок)
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
                    if (luminance_index < 0)
                        luminance_index = 0;
                    if (luminance_index > 11)
                        luminance_index = 11;
                    buffer[xp + yp * totalWidth] = GRADIENT[luminance_index];
                }
            }
        }
    }
    
    // Записываем статистику в правую область
    int statStartCol = donutWidth + 1;
    for (int i = 0; i < stats.size() && i < totalHeight; i++) {
        const string &line = stats[i];
        for (size_t j = 0; j < line.size() && (statStartCol + j) < (size_t)totalWidth; j++) {
            buffer[i * totalWidth + statStartCol + j] = line[j];
        }
    }
    
    // Возвращаем курсор в начало экрана и выводим буфер
    setCursorPosition(0, 0);
    for (int i = 0; i < totalHeight; i++) {
        for (int j = 0; j < totalWidth; j++) {
            putchar(buffer[i * totalWidth + j]);
        }
        putchar('\n');
    }
}

void getRotationMode() {
    cout << "Select rotation mode:\n";
    cout << "1 - Clockwise\n";
    cout << "2 - Counterclockwise\n";
    cout << "3 - Up and Down\n";
    cout << "4 - Left and Right\n";
    cin >> rotation_mode;
    if (rotation_mode < 1 || rotation_mode > 4)
        rotation_mode = 1;
}

void updateAngles(float &A, float &B) {
    switch (rotation_mode) {
        case 1: A += 0.04f; B += 0.02f; break;
        case 2: A -= 0.04f; B -= 0.02f; break;
        case 3: A += 0.04f; break;
        case 4: B += 0.04f; break;
    }
    if (A > 2 * PI) A -= 2 * PI;
    if (B > 2 * PI) B -= 2 * PI;
}

int main() {
    float A = 0.0f, B = 0.0f;
    getRotationMode();
    
    // В цикле можем обновлять динамические параметры, но блок статистики,
    // содержащий статичные элементы (например, заголовок), можно пересобирать только при изменениях размера окна.
    int totalWidth, totalHeight;
    getConsoleSize(&totalWidth, &totalHeight);
    vector<string> stats = createStats(totalWidth, totalHeight);
    
    while (running) {
        // Если размер консоли может изменяться, можно периодически обновлять stats.
        getConsoleSize(&totalWidth, &totalHeight);
        // if (newWidth != totalWidth || newHeight != totalHeight) {
        //     totalWidth = newWidth;
        //     totalHeight = newHeight;
        stats = createStats(totalWidth, totalHeight);
        // }
        
        renderFrame(A, B, totalWidth, totalHeight, stats);
        updateAngles(A, B);
        this_thread::sleep_for(chrono::milliseconds(10));
        // if (_kbhit() && _getch() == 13) // Выход по клавише Enter
        //     running = false;
    }
    
    return 0;
}
