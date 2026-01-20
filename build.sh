#!/bin/bash

# ================== визуальный профиль ==================
C_FLOW="\033[38;5;39m"     # основной поток
C_OK="\033[38;5;82m"       # успех
C_WARN="\033[38;5;214m"    # предупреждение
C_ERR="\033[38;5;196m"     # ошибка
C_DIM="\033[2m"            # вторичный текст
C_RESET="\033[0m"

flow() { printf "%b\n" "${C_FLOW}$1${C_RESET}"; }
ok()   { printf "%b\n" "${C_OK}$1${C_RESET}"; }
warn() { printf "%b\n" "${C_WARN}$1${C_RESET}"; }
err()  { printf "%b\n" "${C_ERR}$1${C_RESET}"; }

# ================== ориентация ==================
ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${ROOT_DIR}/build"

# ================== фаза инициализации ==================
flow ">> Инициилизируем рабочее окружение"

if [ ! -d "$BUILD_DIR" ]; then
  mkdir -p "$BUILD_DIR"
  warn ">> Рабочее окружение создано"
else
  flow ">> Рабочее окружение обнаружено"
fi

cd "$BUILD_DIR" || {
  err ">> Ошибка во время создания рабочего окружения"
  exit 1
}

# ================== фаза конфигурации ==================
flow ">> Калибруем toolchain"
flow "${C_DIM}Source matrix: ${ROOT_DIR}${C_RESET}"

cmake ..

if [ $? -ne 0 ]; then
  err ">> Калибровка Toolchain провалилась"
  exit 1
fi

ok ">> Toolchain готов"

# ================== фаза сборки ==================
flow ">> Собираем проект"

cmake --build .

if [ $? -ne 0 ]; then
  err ">> Сборка прервана"
  exit 1
fi

ok ">> Сборка завершена"

# ================== сводка ==================
echo
flow ">> Deployment notes"
printf "%b\n" "${C_DIM}Artifacts location:${C_RESET} ${BUILD_DIR}/bin"
echo
printf "%b\n" "${C_DIM}Test sequence:${C_RESET}"
printf "%b\n" "${C_DIM}  cd build/bin${C_RESET}"
printf "%b\n" "${C_DIM}  ./test_code${C_RESET}"
echo

read -r -p "Хотите запустить тестовый скрипт? [Д/н]: " answer

case "$answer" in
  ""|[ДдYy])
    flow "Запускаю тестовый скрипт..."
    ./bin/test_code --bin-dir "./bin"; read -p "Нажмите Enter для выхода"
    ;;
  [НнNn])
    exit 0
    ;;
  *)
    err "Некорректный ввод, закрываю программу"
    exit 1
    ;;
esac

