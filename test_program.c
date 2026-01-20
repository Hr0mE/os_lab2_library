#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <windows.h>
    #define sleep_ms(ms) Sleep(ms)
#else
    #include <unistd.h>
    #define sleep_ms(ms) usleep((ms) * 1000)
#endif

/* ================== визуальный профиль ================== */
#define CLR_FLOW   "\033[38;5;39m"   /* основной поток */
#define CLR_OK     "\033[38;5;82m"   /* успешные операции */
#define CLR_WARN   "\033[38;5;214m"  /* предупреждения */
#define CLR_ERR    "\033[38;5;196m"  /* ошибки */
#define CLR_DIM    "\033[2m"         /* вторичная информация */
#define CLR_RESET  "\033[0m"

#define FLOW(msg, ...)  printf(CLR_FLOW msg CLR_RESET "\n", ##__VA_ARGS__)
#define OK(msg, ...)    printf(CLR_OK   msg CLR_RESET "\n", ##__VA_ARGS__)
#define WARN(msg, ...)  printf(CLR_WARN msg CLR_RESET "\n", ##__VA_ARGS__)
#define ERR(msg, ...)   fprintf(stderr, CLR_ERR msg CLR_RESET "\n", ##__VA_ARGS__)
#define DIM(msg, ...)   printf(CLR_DIM  msg CLR_RESET "\n", ##__VA_ARGS__)

/* ================== точка входа ================== */
int main(int argc, char* argv[]) {

    FLOW(" ЗАПУСК ТЕСТОВОЙ ПРОГРАММЫ");
    FLOW("----------------------------------------");

    FLOW("Программа запущена с %d аргументами", argc);
    for (int i = 0; i < argc; i++) {
        DIM("  арг[%d] = %s", i, argv[i]);
    }

    /* ================== настройка параметров ================== */
    int exit_code = 0;         /* код завершения по умолчанию */
    int sleep_time = 2000;     /* время ожидания по умолчанию, мс */

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--exit-code") == 0 && i + 1 < argc) {
            exit_code = atoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "--sleep") == 0 && i + 1 < argc) {
            sleep_time = atoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "--error") == 0) {
            ERR("[DIAG] Симулируем сообщение об ошибке");
        }
    }

    FLOW("Программа будет ожидать %d мс и завершится с кодом %d", 
         sleep_time, exit_code);

    OK("Программа завершила выполнение");

    return exit_code;
}

