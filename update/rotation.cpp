#include "rotation.h"
#include <cmath>
#include <iostream>

using namespace std;

int getRotationMode() {
    int mode;
    // Запрос у пользователя выбора режима вращения
    cout << "Select rotation mode:\n";
    cout << "1 - Clockwise\n";
    cout << "2 - Counterclockwise\n";
    cout << "3 - Up and Down\n";
    cout << "4 - Left and Right\n";
    cin >> mode;

    // Если введен некорректный режим, выбираем по умолчанию 1
    if (mode < 1 || mode > 4) 
        mode = 1;

    return mode;
}

void updateAngles(float &A, float &B, int mode) {
    constexpr float PI = 3.14159265358979323846f;

    // Обновление углов на основе выбранного режима
    switch (mode) {
        case 1: A += 0.04f; B += 0.02f; break;  // По часовой стрелке
        case 2: A -= 0.04f; B -= 0.02f; break;  // Против часовой стрелки
        case 3: A += 0.04f; break;               // Вверх и вниз
        case 4: B += 0.04f; break;               // Влево и вправо
    }

    // Ограничиваем углы, чтобы они не превышали 2*PI
    if (A > 2 * PI) A -= 2 * PI;
    if (B > 2 * PI) B -= 2 * PI;
}
