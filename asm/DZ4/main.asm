.include "macros.inc"
.include "sqrt.asm"    # Добавляем подключение файла с подпрограммой

.data
    input_msg:    .asciz "Enter positive number: "
    precision_msg: .asciz "Enter precision: "
    error_msg:    .asciz "Error: number must be positive!\n"
    result_msg:   .asciz "Square root: "
    newline:      .asciz "\n"
    
.text
.globl main

main:
    # Ввод и проверка числа
    INPUT_NUMBER
    
    # Ввод точности  
    INPUT_PRECISION
    
    # Вычисление квадратного корня
    CALCULATE_SQRT
    
    # Вывод результата
    OUTPUT_RESULT
    
    # Завершение программы
    li a7, 10
    ecall