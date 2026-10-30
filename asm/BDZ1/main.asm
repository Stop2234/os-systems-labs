.include "macros.asm"

.data
msg_n:          .asciz "Enter array size N (1-10): "
msg_invalid:    .asciz "Invalid size! Must be 1-10.\n"
msg_elements:   .asciz "Enter array elements:\n"
msg_array_a:    .asciz "Array A: "
msg_array_b:    .asciz "Array B (non-decreasing sequence): "
newline:        .asciz "\n"
space:          .asciz " "

array_a:        .word 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

.text
.globl main

main:
    # Ввод размера массива
    jal input_array_size
    mv s0, a0
    
    # Ввод элементов массива A
    la a0, array_a
    mv a1, s0
    jal input_array
    
    # Вывод массива A с макросом
    print_str(msg_array_a)
    la a0, array_a
    mv a1, s0
    jal print_array
    
    # Обработка массива
    la a0, array_a
    mv a1, s0
    jal process_array_28
    mv s1, a0
    mv s2, a1
    
    # Вывод массива B с макросом
    print_str(msg_array_b)
    mv a0, s1
    mv a1, s2
    jal print_array
    
    li a7, 10
    ecall

# Подключаем подпрограммы (без данных)
.include "core.asm"