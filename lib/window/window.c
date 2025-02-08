#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

void getConsoleSize(int* width, int* height){
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    *width = w.ws_col * 0.3;
    *height = w.ws_row * 0.9;
    // printf("row: %d, col: %d\nwidth: %d, height: %d\n", w.ws_row, w.ws_col, *width, *height);
    
    return;
}


void clearScreen(){
    printf("\e[1;1H\e[2J");
}

/**
 * Устанавливает позицию курсора в терминале.
 *
 * @param x Координата столбца (начинается с 1).
 * @param y Координата строки (начинается с 1).
 */
void setCursorPosition(int x, int y) {

    clearScreen();
    // Перемещение курсора
    printf("\033[%d;%dH", y, x);
}