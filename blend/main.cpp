#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <cmath>
#include <thread>
#include <conio.h>
#include <windows.h>
#include <algorithm>
#include <cctype>
#include <chrono>
#include <sstream>

using namespace std;

// --- Базовые константы (из don.cpp) ---
constexpr float R1 = 1.0f;
constexpr float R2 = 2.0f;
constexpr float K2 = 16.0f;
constexpr float PI = 3.14159265358979323846f;
constexpr int STAT_WIDTH = 30;

// --- Параметры рендера ---
constexpr int DEFAULT_WIDTH = 80;
constexpr int DEFAULT_HEIGHT = 40;
constexpr char GRADIENT[] = ".,-~:;=!*#$@";

// --- Структуры ---
struct Vertex { float x, y, z; };
struct Face   { int v1, v2, v3; };
struct Point2 { float x, y; };

// --- Глобальные переменные объекта ---
float A = 0.0f, B = 0.0f;       // углы вращения
bool running = true;
int rotation_mode = 1;
string objFileName;           // имя файла (для статистики)

// Для динамического масштабирования (уровень детализации зависит от размера консоли)
float dynamicScale = 0.6f;    // эмпирически подобрано для DEFAULT_WIDTH×DEFAULT_HEIGHT

// --- Функции работы с консолью ---
void setCursorPosition(int x, int y) {
    COORD pos = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void getConsoleSize(int &width, int &height) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if(GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    } else {
        width = DEFAULT_WIDTH; height = DEFAULT_HEIGHT;
    }
}

string removeQuotes(const string &str) {
    string s = str;
    s.erase(s.begin(), find_if(s.begin(), s.end(), [](unsigned char ch){ return !isspace(ch); }));
    s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char ch){ return !isspace(ch); }).base(), s.end());
    if(!s.empty() && s.front()=='"' && s.back()=='"')
        s = s.substr(1, s.size()-2);
    return s;
}

// --- Функция формирования блока статистики (выводим только имя файла) ---
vector<string> createStats(int totalWidth, int totalHeight, const string &filename) {
    vector<string> stats;
    stats.push_back("File: " + filename);
    return stats;
}

// --- Функция загрузки OBJ-файла ---
void loadOBJ(const string &filename, vector<Vertex> &vertices, vector<Face> &faces) {
    ifstream file(filename);
    if(!file) {
        cerr << "Error: Unable to open file " << filename << endl;
        return;
    }
    string line;
    while(getline(file, line)) {
        if(line.rfind("v ", 0)==0) {
            istringstream iss(line.substr(2));
            Vertex v; iss >> v.x >> v.y >> v.z;
            vertices.push_back(v);
        } else if(line.rfind("f ", 0)==0) {
            istringstream iss(line.substr(2));
            Face face; string token;
            if(iss >> token)
                face.v1 = stoi(token.substr(0, token.find('/'))) - 1;
            if(iss >> token)
                face.v2 = stoi(token.substr(0, token.find('/'))) - 1;
            if(iss >> token)
                face.v3 = stoi(token.substr(0, token.find('/'))) - 1;
            faces.push_back(face);
        }
    }
}

// --- Нормализация вершин ---
// Центрирует и масштабирует модель так, чтобы её максимальный размер ≈10 единиц.
void normalizeVertices(vector<Vertex>& vertices) {
    if(vertices.empty()) return;
    float minX = vertices[0].x, maxX = vertices[0].x;
    float minY = vertices[0].y, maxY = vertices[0].y;
    float minZ = vertices[0].z, maxZ = vertices[0].z;
    for(const auto &v : vertices) {
        minX = min(minX, v.x); maxX = max(maxX, v.x);
        minY = min(minY, v.y); maxY = max(maxY, v.y);
        minZ = min(minZ, v.z); maxZ = max(maxZ, v.z);
    }
    float centerX = (minX+maxX)/2.0f, centerY = (minY+maxY)/2.0f, centerZ = (minZ+maxZ)/2.0f;
    float scaleFactor = 10.0f / max({maxX-minX, maxY-minY, maxZ-minZ});
    for(auto &v : vertices) {
        v.x = (v.x - centerX)*scaleFactor;
        v.y = (v.y - centerY)*scaleFactor;
        v.z = (v.z - centerZ)*scaleFactor;
    }
}

// --- Вращение вершины ---
// Поворачиваем вершину вокруг осей X и Y (локальное вращение объекта)
Vertex rotateVertex(const Vertex &v, float ax, float ay) {
    float cosX = cos(ax), sinX = sin(ax);
    float cosY = cos(ay), sinY = sin(ay);
    float y = v.y * cosX - v.z * sinX;
    float z = v.y * sinX + v.z * cosX;
    float x = v.x * cosY + z * sinY;
    z = -v.x * sinY + z * cosY;
    return { x, y, z };
}

// --- Функции для работы с векторами (для камеры) ---
Vertex subtract(const Vertex &a, const Vertex &b) {
    return { a.x - b.x, a.y - b.y, a.z - b.z };
}
float dot(const Vertex &a, const Vertex &b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}
Vertex cross(const Vertex &a, const Vertex &b) {
    return { a.y*b.z - a.z*b.y,
             a.z*b.x - a.x*b.z,
             a.x*b.y - a.y*b.x };
}
Vertex normalize(const Vertex &v) {
    float mag = sqrt(dot(v,v));
    return (mag == 0) ? Vertex{0,0,0} : Vertex{ v.x/mag, v.y/mag, v.z/mag };
}

// --- Параметры камеры ---
// Камера задаётся фиксированно: положение, точка взгляда и вектор up.
const Vertex camEye    = {20.0f, 20.0f, 20.0f};
const Vertex camLookAt = {0.0f, 0.0f, 0.0f};
const Vertex camUp     = {0.0f, 1.0f, 0.0f};

// Вычисляем базис камеры (единичные векторы: right, up, forward)
const Vertex camForward = normalize(subtract(camLookAt, camEye));  // направление взгляда
const Vertex camRight   = normalize(cross(camUp, camForward));
const Vertex camUpCorrect = cross(camForward, camRight);

// Преобразуем точку из мировых координат в координаты камеры.
Vertex transformToCamera(const Vertex &v) {
    Vertex p = subtract(v, camEye);
    return { dot(p, camRight), dot(p, camUpCorrect), dot(p, camForward) };
}

// --- Проекция ---
// После преобразования в координаты камеры выполняем перспективную проекцию.
// Используем фокусное расстояние f (например, равное половине ширины экрана).
Point2 projectVertex(const Vertex &v, int width, int height) {
    Vertex camP = transformToCamera(v);
    // Если точка находится за камерой (или очень близко к ней), не рисуем её.
    if(camP.z <= 0.1f) camP.z = 0.1f;
    float f = width / 2.0f;
    Point2 p;
    p.x = width / 2.0f + (camP.x * f / camP.z);
    p.y = height / 2.0f - (camP.y * f / camP.z);
    return p;
}

// --- Барицентрические координаты ---
bool barycentric(const Point2 &p, const Point2 &p0, const Point2 &p1, const Point2 &p2,
                 float &u, float &v, float &w) {
    float denom = (p1.y - p2.y)*(p0.x - p2.x) + (p2.x - p1.x)*(p0.y - p2.y);
    if(fabs(denom) < 1e-6f) return false;
    u = ((p1.y - p2.y)*(p.x - p2.x) + (p2.x - p1.x)*(p.y - p2.y)) / denom;
    v = ((p2.y - p0.y)*(p.x - p2.x) + (p0.x - p2.x)*(p.y - p2.y)) / denom;
    w = 1.0f - u - v;
    return (u>=0 && v>=0 && w>=0);
}

// --- Вычисление нормали ---
Vertex computeNormal(const Vertex &v0, const Vertex &v1, const Vertex &v2) {
    Vertex u = { v1.x - v0.x, v1.y - v0.y, v1.z - v0.z };
    Vertex v = { v2.x - v0.x, v2.y - v0.y, v2.z - v0.z };
    Vertex n = { u.y*v.z - u.z*v.y,
                 u.z*v.x - u.x*v.z,
                 u.x*v.y - u.y*v.x };
    float mag = sqrt(n.x*n.x + n.y*n.y + n.z*n.z);
    return (mag == 0 ? Vertex{0,0,0} : Vertex{ n.x/mag, n.y/mag, n.z/mag });
}

// --- Рендеринг ---
// Для каждого треугольника объекта выполняем его вращение, затем преобразование в координаты камеры и проекцию.
// После этого выполняется растеризация с Z-буфером, а в правой части экрана накладывается блок статистики.
void renderFaces(const vector<Vertex>& vertices, const vector<Face>& faces,
                 int width, int height, const vector<string> &stats) {
    vector<char> screen(width * height, ' ');
    vector<float> zbuffer(width * height, -1e9f);
    // Направление света (фиксированное)
    Vertex light = {0, -1, 0};
    for(const auto &face : faces) {
        Vertex v0 = rotateVertex(vertices[face.v1], A, B);
        Vertex v1 = rotateVertex(vertices[face.v2], A, B);
        Vertex v2 = rotateVertex(vertices[face.v3], A, B);
        Vertex normal = computeNormal(v0, v1, v2);
        float shade = max(0.0f, min(1.0f, dot(normal, light)));
        int gradIndex = static_cast<int>(shade * (sizeof(GRADIENT)-2));
        char fillChar = GRADIENT[gradIndex];
        Point2 p0 = projectVertex(v0, width, height);
        Point2 p1 = projectVertex(v1, width, height);
        Point2 p2 = projectVertex(v2, width, height);
        int minX = max(0, static_cast<int>(min({p0.x, p1.x, p2.x})));
        int maxX = min(width-1, static_cast<int>(max({p0.x, p1.x, p2.x})));
        int minY = max(0, static_cast<int>(min({p0.y, p1.y, p2.y})));
        int maxY = min(height-1, static_cast<int>(max({p0.y, p1.y, p2.y})));
        for (int y = minY; y <= maxY; y++) {
            for (int x = minX; x <= maxX; x++) {
                Point2 p = { x + 0.5f, y + 0.5f };
                float u, v, w;
                if(barycentric(p, p0, p1, p2, u, v, w)) {
                    float z = u*v0.z + v*v1.z + w*v2.z;
                    int idx = y * width + x;
                    if(z > zbuffer[idx]) {
                        zbuffer[idx] = z;
                        screen[idx] = fillChar;
                    }
                }
            }
        }
    }
    // Накладываем блок статистики (справа)
    int statStart = width - STAT_WIDTH;
    for (int i = 0; i < stats.size() && i < height; i++) {
        for (int j = 0; j < stats[i].size() && (statStart+j) < width; j++) {
            screen[i * width + statStart + j] = stats[i][j];
        }
    }
    setCursorPosition(0, 0);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++)
            putchar(screen[y * width + x]);
        putchar('\n');
    }
}

// --- Выбор и обновление режима вращения ---
void getRotationMode() {
    cout << "Select rotation mode:\n";
    cout << "1 - Clockwise (A += 0.04, B += 0.02)\n";
    cout << "2 - Counterclockwise (A -= 0.04, B -= 0.02)\n";
    cout << "3 - Up and Down (A += 0.04)\n";
    cout << "4 - Left and Right (B += 0.04)\n";
    cin >> rotation_mode;
    if(rotation_mode < 1 || rotation_mode > 4)
        rotation_mode = 1;
}

void updateAngles(float &A, float &B) {
    switch(rotation_mode) {
        case 1: A += 0.04f; B += 0.02f; break;
        case 2: A -= 0.04f; B -= 0.02f; break;
        case 3: A += 0.04f; break;
        case 4: B += 0.04f; break;
    }
    if(A > 2*PI) A -= 2*PI;
    if(B > 2*PI) B -= 2*PI;
}

// --- Основной цикл рендеринга ---
// Вычисляем коэффициент масштабирования так, чтобы модель полностью попадала в кадр вне зависимости от размеров консоли.
void renderLoop(const vector<Vertex>& vertices, const vector<Face>& faces) {
    int width, height;
    while(running) {
        getConsoleSize(width, height);
        float scaleFactor = min(static_cast<float>(width)/DEFAULT_WIDTH,
                                static_cast<float>(height)/DEFAULT_HEIGHT);
        dynamicScale = 0.9f * scaleFactor;
        vector<string> stats = createStats(width, height, objFileName);
        renderFaces(vertices, faces, width, height, stats);
        updateAngles(A, B);
        this_thread::sleep_for(chrono::milliseconds(30));
        if(_kbhit() && _getch() == 13)
            running = false;
    }
}

int main() {
    cin.sync();
    cout << "Enter the path to the .obj file: ";
    string filePath;
    getline(cin, filePath);
    filePath = removeQuotes(filePath);
    replace(filePath.begin(), filePath.end(), '\\', '/');
    if(filePath.size() < 4 || 
       (filePath.substr(filePath.size()-4) != ".obj" && filePath.substr(filePath.size()-4) != ".OBJ")) {
        cerr << "Error: Please provide a file with a .obj extension." << endl;
        cout << "Press Enter to exit...";
        cin.ignore(); cin.get();
        return 1;
    }
    objFileName = filePath;
    vector<Vertex> vertices;
    vector<Face> faces;
    loadOBJ(filePath, vertices, faces);
    if(vertices.empty() || faces.empty()){
        cerr << "Error: Model could not be loaded (no vertices or faces)!" << endl;
        cout << "Press Enter to exit...";
        cin.ignore(); cin.get();
        return 1;
    }
    normalizeVertices(vertices);
    getRotationMode();
    cin.ignore();
    cout << "Rendering model with chosen rotation mode...\n(Press Enter to exit)\n";
    renderLoop(vertices, faces);
    cout << "Press Enter to exit...";
    cin.ignore(); cin.get();
    return 0;
}
