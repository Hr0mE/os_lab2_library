#include "process.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#ifdef _WIN32
    #include <windows.h>
    #define sleep_ms(ms) Sleep(ms)
#else
    #include <unistd.h>
    #define sleep_ms(ms) usleep((ms) * 1000)
#endif

/* ================== визуальный профиль ================== */
#define CLR_FLOW   "\033[38;5;39m"   /* основной поток */
#define CLR_OK     "\033[38;5;82m"   /* успех */
#define CLR_WARN   "\033[38;5;214m"  /* предупреждение */
#define CLR_ERR    "\033[38;5;196m"  /* ошибка */
#define CLR_DIM    "\033[2m"         /* вторичный текст */
#define CLR_RESET  "\033[0m"

#define FLOW(msg, ...)  printf(CLR_FLOW msg CLR_RESET "\n", ##__VA_ARGS__)
#define OK(msg, ...)    printf(CLR_OK   msg CLR_RESET "\n", ##__VA_ARGS__)
#define WARN(msg, ...)  printf(CLR_WARN msg CLR_RESET "\n", ##__VA_ARGS__)
#define ERR(msg, ...)   fprintf(stderr, CLR_ERR msg CLR_RESET "\n", ##__VA_ARGS__)
#define DIM(msg, ...)   printf(CLR_DIM  msg CLR_RESET "\n", ##__VA_ARGS__)

/* ================== служебные проверки ================== */
/* Проверка валидности дескриптора процесса */
static int handle_invalid(process_handle_t h) {
#ifdef _WIN32
    return h == INVALID_HANDLE_VALUE;
#else
    return h < 0;
#endif
}

/* ================== путь к тестовому бинарю ================== */
static char TEST_PROGRAM_PATH[PATH_MAX];

/* ================== фаза 1 ================== */
/* Базовая проверка запуска процесса */
void phase_basic_execution(void) {
    FLOW("\n[ФАЗА-1] Базовая проверка запуска процесса");

    char* args[] = { NULL };
    process_handle_t h = start_background_process(TEST_PROGRAM_PATH, args);

    if (handle_invalid(h)) {
        ERR("[ФАЗА-1] Не удалось запустить процесс");
        return;
    }

    DIM("[ФАЗА-1] Процесс запущен, ожидание завершения");

    int exit_code;
    if (wait_for_process(h, &exit_code) == 0) {
        OK("[ФАЗА-1] Процесс завершился с кодом %d", exit_code);
    } else {
        ERR("[ФАЗА-1] Ошибка при ожидании завершения процесса");
    }

    close_process_handle(h);
}

/* ================== фаза 2 ================== */
/* Проверка запуска процесса с аргументами */
void phase_argument_execution(void) {
    FLOW("\n[ФАЗА-2] Запуск процесса с параметрами");

    char* args[] = { "--sleep", "1000", "--exit-code", "17", NULL };
    process_handle_t h = start_background_process(TEST_PROGRAM_PATH, args);

    if (handle_invalid(h)) {
        ERR("[ФАЗА-2] Не удалось запустить процесс");
        return;
    }

    DIM("[ФАЗА-2] Пользовательские аргументы переданы");

    int exit_code;
    if (wait_for_process(h, &exit_code) == 0) {
        FLOW("[ФАЗА-2] Процесс завершился с кодом %d", exit_code);
        if (exit_code == 17) {
            OK("[ФАЗА-2] Код завершения корректен");
        } else {
            WARN("[ФАЗА-2] Ожидался код 17, получен %d", exit_code);
        }
    }

    close_process_handle(h);
}

/* ================== фаза 3 ================== */
/* Мониторинг состояния выполняющегося процесса */
void phase_runtime_status(void) {
    FLOW("\n[ФАЗА-3] Мониторинг состояния процесса");

    char* args[] = { "--sleep", "3000", NULL };
    process_handle_t h = start_background_process(TEST_PROGRAM_PATH, args);

    if (handle_invalid(h)) {
        ERR("[ФАЗА-3] Не удалось запустить процесс");
        return;
    }

    for (int i = 0; i < 5; i++) {
        sleep_ms(800);
        int status = is_process_running(h);

        if (status == 1) {
            DIM("[ФАЗА-3] [%d] Процесс всё ещё выполняется", i + 1);
        } else if (status == 0) {
            OK("[ФАЗА-3] [%d] Процесс завершён", i + 1);
            break;
        } else {
            WARN("[ФАЗА-3] [%d] Ошибка проверки состояния процесса", i + 1);
        }
    }

    int exit_code;
    wait_for_process(h, &exit_code);
    FLOW("[ФАЗА-3] Финальный код завершения: %d", exit_code);

    close_process_handle(h);
}

/* ================== фаза 4 ================== */
/* Параллельный запуск нескольких процессов */
void phase_parallel_execution(void) {
    FLOW("\n[ФАЗА-4] Параллельное выполнение процессов");

    const int N = 3;
    process_handle_t h[N];

    char* a1[] = { "--sleep", "1000", "--exit-code", "1", NULL };
    char* a2[] = { "--sleep", "2000", "--exit-code", "2", NULL };
    char* a3[] = { "--sleep", "1500", "--exit-code", "3", NULL };

    DIM("[ФАЗА-4] Запуск %d рабочих процессов", N);

    h[0] = start_background_process(TEST_PROGRAM_PATH, a1);
    h[1] = start_background_process(TEST_PROGRAM_PATH, a2);
    h[2] = start_background_process(TEST_PROGRAM_PATH, a3);

    for (int i = 0; i < N; i++) {
        if (handle_invalid(h[i])) continue;

        int exit_code;
        if (wait_for_process(h[i], &exit_code) == 0) {
            OK("[ФАЗА-4] Процесс %d завершился с кодом %d", i + 1, exit_code);
        }
        close_process_handle(h[i]);
    }

    FLOW("[ФАЗА-4] Все процессы завершены");
}

/* ================== фаза 5 ================== */
/* Проверка обработки ошибок */
void phase_error_handling(void) {
    FLOW("\n[ФАЗА-5] Проверка обработки ошибок");

    char* args[] = { NULL };
    process_handle_t h = start_background_process("./unknown_programm", args);

#ifdef _WIN32
    if (h == INVALID_HANDLE_VALUE) {
        OK("[ФАЗА-5] Ошибка корректно обработана (Windows)");
    } else {
        WARN("[ФАЗА-5] Процесс не должен был запуститься");
        close_process_handle(h);
    }
#else
    if (h < 0) {
        ERR("[ФАЗА-5] Ошибка fork (неожиданно)");
    } else {
        DIM("[ФАЗА-5] POSIX: fork успешен, exec завершится ошибкой");
        sleep_ms(100);

        int exit_code;
        if (wait_for_process(h, &exit_code) == 0 && exit_code != 0) {
            OK("[ФАЗА-5] Дочерний процесс завершился с ошибкой (%d)", exit_code);
        } else {
            WARN("[ФАЗА-5] Неожиданный результат дочернего процесса");
        }
        close_process_handle(h);
    }
#endif
}

/* ================== точка входа ================== */
int main(int argc, char **argv) {
    FLOW("========================================");
    FLOW(" Диагностический комплекс управления процессами");
    FLOW("========================================");

    /* ----------------- разбор аргументов ----------------- */
    const char *bin_dir = NULL;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--bin-dir") == 0 && i + 1 < argc) {
            bin_dir = argv[++i];
        }
    }

    if (!bin_dir) {
        ERR("Не задан обязательный параметр --bin-dir");
        ERR("Пример: ./process_tests --bin-dir /path/to/build/bin");
        return 1;
    }

    snprintf(TEST_PROGRAM_PATH, sizeof(TEST_PROGRAM_PATH),
             "%s/test_program", bin_dir);

    FLOW("Используется test_program: %s", TEST_PROGRAM_PATH);

    /* ----------------- выполнение фаз ----------------- */
    phase_basic_execution();
    phase_argument_execution();
    phase_runtime_status();
    phase_parallel_execution();
    phase_error_handling();

    FLOW("\n========================================");
    OK(" Диагностическая последовательность завершена");
    FLOW("========================================");

    return 0;
}

