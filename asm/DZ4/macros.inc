# Макро для ввода числа с проверкой
.macro INPUT_NUMBER
input_loop:
    # Вывод приглашения
    la a0, input_msg
    li a7, 4
    ecall
    
    # Ввод числа
    li a7, 7
    ecall
    
    # Проверка на положительность
    fmv.d ft0, fa0        # сохраняем число в ft0
    fld ft1, zero_const, t0  # загружаем 0.0
    flt.d t1, ft1, ft0    # если 0 < number, t1 = 1
    bnez t1, positive_number
    
    # Если отрицательное - ошибка
    la a0, error_msg
    li a7, 4
    ecall
    j input_loop
    
positive_number:
    # Сохраняем число в fs0
    fmv.d fs0, fa0
.end_macro

# Макро для ввода точности
.macro INPUT_PRECISION
    la a0, precision_msg
    li a7, 4
    ecall
    
    li a7, 7
    ecall
    fmv.d fs1, fa0
.end_macro

# Макро для вычисления квадратного корня
.macro CALCULATE_SQRT
    fmv.d fa0, fs0    # число
    fmv.d fa1, fs1    # точность
    jal calculate_sqrt
    fmv.d fs2, fa0    # результат
.end_macro

# Макро для вывода результата
.macro OUTPUT_RESULT
    la a0, result_msg
    li a7, 4
    ecall
    
    fmv.d fa0, fs2
    li a7, 3
    ecall
    
    la a0, newline
    li a7, 4
    ecall
.end_macro

.data
zero_const: .double 0.0