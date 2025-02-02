#include <iostream>
// #include "main.h"
#include <vector>
#include <chrono>
#include <thread>
#include <windows.h>
#include <conio.h>
#include <atomic>
#include "rotation.h"
#include "console.h"
#include "donut.h"
#include "cube.h"
#include "pyramid.h"
// #include "sphere.h"
// #include "pyramid.h" // Создайте аналогичные файлы для других фигур
// #include "cone.h"
// #include "tetrahedron.h"
// #include "tesseract.h"  // Тесеракт

using namespace std;

atomic<bool> running(true);



int main() {
    // Выбор фигуры
    int figure_choice;
    cout << "Choose a shape to render:\n";
    cout << "1 - Donut\n";
    cout << "2 - Cube\n";
    cout << "3 - Pyramid\n";
    // cout << "5 - Cone\n";
    // cout << "6 - Tetrahedron\n";
    // cout << "7 - Tesseract\n";
    // cout << "Enter your choice (1-7): ";
    cin >> figure_choice;

    // Выбор режима вращения
    float A = 0.0f, B = 0.0f;
    int rotation_mode = getRotationMode();


    
    // Получаем текущий размер консоли и создаём статистику для выбранной фигуры
    int totalWidth, totalHeight;
    getConsoleSize(totalWidth, totalHeight);
    std::vector<std::string> stats;
    switch (figure_choice) {
        case 1: stats = createDonutStats(totalWidth, totalHeight, rotation_mode); break;
        case 2: stats = createCubeStats(totalWidth, totalHeight, rotation_mode); break;
        case 3: stats = createPyramidStats(totalWidth, totalHeight, rotation_mode); break;
        default: stats = createDonutStats(totalWidth, totalHeight, rotation_mode); break;
    }
 // Главный цикл рендеринга
    while (running) {
        int newWidth, newHeight;
        getConsoleSize(newWidth, newHeight);
        if (newWidth != totalWidth || newHeight != totalHeight) {
            totalWidth = newWidth;
            totalHeight = newHeight;
            // Пересоздаём статистику для выбранной фигуры
            switch (figure_choice) {
                case 1: stats = createDonutStats(totalWidth, totalHeight, rotation_mode); break;
                case 2: stats = createCubeStats(totalWidth, totalHeight, rotation_mode); break;
                case 3: stats = createPyramidStats(totalWidth, totalHeight, rotation_mode); break;
                default: stats = createDonutStats(totalWidth, totalHeight, rotation_mode); break;
            }
        }

        // Рендеринг выбранной фигуры
        switch (figure_choice) {
            case 1: renderDonut(A, B, totalWidth, totalHeight, stats); break;
            case 2: renderCube(A, B, totalWidth, totalHeight, stats); break;
            case 3: renderPyramid(A, B, totalWidth, totalHeight, stats); break;
            default: renderDonut(A, B, totalWidth, totalHeight, stats); break;
        }

        updateAngles(A, B, rotation_mode);
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        if (_kbhit() && _getch() == 13)  // Выход по клавише Enter
            running = false;
    }
    return 0;
}