# ========== БИБЛИОТЕКА МАКРОСОВ ==========

# Макрос для вывода строки
.macro print_str(%str)
    la a0, %str
    li a7, 4
    ecall
.end_macro

# Макрос для вывода целого числа
.macro print_int(%reg)
    mv a0, %reg
    li a7, 1
    ecall
.end_macro

# Макрос для ввода целого числа
.macro read_int
    li a7, 5
    ecall
.end_macro

# Макрос для выделения памяти
.macro allocate_memory(%size)
    li a7, 9
    mv a0, %size
    ecall
.end_macro