.text
.globl main

main:
    # Ввод числа
    jal input_positive_double
    
    # Вычисление квадратного корня
    jal sqrt_heron
    
    # Вывод результата
    la a0, msg_result
    li a7, 4
    ecall
    
    fmv.d fa0, fa1
    jal print_double
    
    # Завершение программы
    li a7, 10
    ecall

# Подключаем основные подпрограммы
.include "core.asm"