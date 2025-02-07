#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

void getConsoleSize(int* width, int* height){
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    *width = w.ws_row ;
    *height = w.ws_col;
    
    return;
}


void clearScreen(){

}

/**
 * Устанавливает позицию курсора в терминале.
 *
 * @param x Координата столбца (начинается с 1).
 * @param y Координата строки (начинается с 1).
 */
void setCursorPosition(int x, int y) {
    // // Проверка границ для координат (предполагается, что терминал имеет размер 80x24)
    // if (x < 1) x = 1;
    // if (y < 1) y = 1;
    // if (x > 80) x = 80;
    // if (y > 24) y = 24;

    // Перемещение курсора
    printf("\033[%d;%dH", y, x);
}