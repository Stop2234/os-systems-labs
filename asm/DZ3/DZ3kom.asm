.data
result_msg: .asciz "Maximum argument for 32-bit factorial: "
newline:    .asciz "\n"

.text
.globl main

main:
    # Инициализация
    li s0, 0           # Текущий аргумент
    li s1, 0           # Максимальный успешный аргумент
    
test_loop:
    # Вызываем factorial(s0)
    mv a0, s0
    jal factorial
    
    # Проверяем результат
    li t0, -1
    beq a0, t0, overflow_found
    
    # Факториал вычислен успешно, сохраняем аргумент
    mv s1, s0
    
    # Увеличиваем аргумент для следующей проверки
    addi s0, s0, 1
    j test_loop

overflow_found:
    # Выводим сообщение
    la a0, result_msg
    li a7, 4
    ecall
    
    # Выводим результат (максимальный успешный аргумент)
    mv a0, s1
    li a7, 1
    ecall
    
    # Выводим перевод строки
    la a0, newline
    li a7, 4
    ecall
    
    # Завершаем программу
    li a7, 10
    ecall

# Рекурсивная подпрограмма вычисления факториала
# Вход: a0 - аргумент n
# Выход: a0 - факториал n или -1 при переполнении
factorial:
    # Базовый случай: факториал 0 = 1
    beqz a0, base_case
    
    # Сохраняем регистры в стеке
    addi sp, sp, -12
    sw ra, 8(sp)
    sw a0, 4(sp)
    
    # Рекурсивный вызов: factorial(n-1)
    addi a0, a0, -1
    jal factorial
    
    # Проверяем на переполнение в рекурсивном вызове
    li t0, -1
    beq a0, t0, recursive_overflow
    
    # Восстанавливаем n
    lw t1, 4(sp)
    
    # Умножаем: n * factorial(n-1)
    mul t2, t1, a0ds    
    
    # Проверяем на переполнение
    # Если результат деления не равен factorial(n-1), то переполнение
    div t3, t2, t1
    bne t3, a0, multiplication_overflow
    
    # Результат корректен
    mv a0, t2
    j factorial_end

base_case:
    li a0, 1
    ret

recursive_overflow:
multiplication_overflow:
    li a0, -1

factorial_end:
    # Восстанавливаем регистры и возвращаемся
    lw ra, 8(sp)
    addi sp, sp, 12
    ret