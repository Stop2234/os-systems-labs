.data
# Здесь только специфичные для main данные, если нужны

.text
.globl main

main:
    # Ввод размера массива
    jal input_array_size
    mv s0, a0  # Сохраняем размер N в s0
    
    # Ввод элементов массива A
    la a0, array_a
    mv a1, s0
    jal input_array
    
    # Вывод массива A
    la a0, msg_array_a
    li a7, 4
    ecall
    
    la a0, array_a
    mv a1, s0
    jal print_array
    
    # Обработка массива (вариант 28)
    la a0, array_a
    mv a1, s0
    jal process_array_28
    
    mv s1, a0  # Адрес массива B
    mv s2, a1  # Размер массива B
    
    # Вывод массива B
    la a0, msg_array_b
    li a7, 4
    ecall
    
    mv a0, s1
    mv a1, s2
    jal print_array
    
    # Завершение программы
    li a7, 10
    ecall

# Подключаем основные подпрограммы и данные
.include "core.asm"