.data
test1_msg:      .asciz "Test 1 - Increasing sequence: "
test2_msg:      .asciz "Test 2 - Decreasing sequence: "
test3_msg:      .asciz "Test 3 - All equal: "
test4_msg:      .asciz "Test 4 - Mixed: "
passed_msg:     .asciz "PASSED\n"
failed_msg:     .asciz "FAILED\n"
newline:        .asciz "\n"

test1_a:        .word 1, 2, 3, 4, 5
test1_size:     .word 5
test1_expected: .word 1, 2, 3, 4, 5
test1_exp_size: .word 5

test2_a:        .word 5, 4, 3, 2, 1
test2_size:     .word 5
test2_expected: .word 5
test2_exp_size: .word 1

test3_a:        .word 2, 2, 2, 2, 2
test3_size:     .word 5
test3_expected: .word 2, 2, 2, 2, 2
test3_exp_size: .word 5

test4_a:        .word 1, 3, 2, 4, 5
test4_size:     .word 5
test4_expected: .word 1, 3
test4_exp_size: .word 2

.text
.globl main

main:
    # Тест 1: Возрастающая последовательность
    la a0, test1_msg
    li a7, 4
    ecall
    
    la a0, test1_a
    lw a1, test1_size
    jal process_array_28
    
    # Проверка результата
    la t0, test1_expected
    lw t1, test1_exp_size
    bne a1, t1, test1_fail
    
    li t2, 0
test1_check:
    bge t2, a1, test1_pass
    slli t3, t2, 2
    add t4, a0, t3
    lw t5, 0(t4)
    add t6, t0, t3
    lw t3, 0(t6)  # Используем t3 вместо t7
    bne t5, t3, test1_fail
    addi t2, t2, 1
    j test1_check

test1_pass:
    la a0, passed_msg
    li a7, 4
    ecall
    j test2

test1_fail:
    la a0, failed_msg
    li a7, 4
    ecall

test2:
    # Тест 2: Убывающая последовательность
    la a0, test2_msg
    li a7, 4
    ecall
    
    la a0, test2_a
    lw a1, test2_size
    jal process_array_28
    
    # Проверка результата
    la t0, test2_expected
    lw t1, test2_exp_size
    bne a1, t1, test2_fail
    
    li t2, 0
test2_check:
    bge t2, a1, test2_pass
    slli t3, t2, 2
    add t4, a0, t3
    lw t5, 0(t4)
    add t6, t0, t3
    lw t3, 0(t6)  # Используем t3 вместо t7
    bne t5, t3, test2_fail
    addi t2, t2, 1
    j test2_check

test2_pass:
    la a0, passed_msg
    li a7, 4
    ecall
    j test3

test2_fail:
    la a0, failed_msg
    li a7, 4
    ecall

test3:
    # Тест 3: Все элементы равны
    la a0, test3_msg
    li a7, 4
    ecall
    
    la a0, test3_a
    lw a1, test3_size
    jal process_array_28
    
    # Проверка результата
    la t0, test3_expected
    lw t1, test3_exp_size
    bne a1, t1, test3_fail
    
    li t2, 0
test3_check:
    bge t2, a1, test3_pass
    slli t3, t2, 2
    add t4, a0, t3
    lw t5, 0(t4)
    add t6, t0, t3
    lw t3, 0(t6)  # Используем t3 вместо t7
    bne t5, t3, test3_fail
    addi t2, t2, 1
    j test3_check

test3_pass:
    la a0, passed_msg
    li a7, 4
    ecall
    j test4

test3_fail:
    la a0, failed_msg
    li a7, 4
    ecall

test4:
    # Тест 4: Смешанная последовательность
    la a0, test4_msg
    li a7, 4
    ecall
    
    la a0, test4_a
    lw a1, test4_size
    jal process_array_28
    
    # Проверка результата
    la t0, test4_expected
    lw t1, test4_exp_size
    bne a1, t1, test4_fail
    
    li t2, 0
test4_check:
    bge t2, a1, test4_pass
    slli t3, t2, 2
    add t4, a0, t3
    lw t5, 0(t4)
    add t6, t0, t3
    lw t3, 0(t6)  # Используем t3 вместо t7
    bne t5, t3, test4_fail
    addi t2, t2, 1
    j test4_check

test4_pass:
    la a0, passed_msg
    li a7, 4
    ecall
    j end

test4_fail:
    la a0, failed_msg
    li a7, 4
    ecall

end:
    # Завершение программы
    li a7, 10
    ecall

# ========== ПОДПРОГРАММА ОБРАБОТКИ МАССИВА (ВАРИАНТ 28) ==========
process_array_28:
    addi sp, sp, -24
    sw ra, 20(sp)
    sw s0, 16(sp)
    sw s1, 12(sp)
    sw s2, 8(sp)
    sw s3, 4(sp)
    sw s4, 0(sp)
    
    mv s0, a0  # Адрес массива A
    mv s1, a1  # Размер массива A
    
    # Выделение памяти для массива B
    slli a0, s1, 2
    addi a0, a0, 4
    li a7, 9
    ecall
    mv s2, a0  # Адрес массива B
    
    li s3, 0   # Размер массива B
    
    # Если массив пустой - завершаем
    beqz s1, process_end
    
    # Первый элемент всегда включаем в B
    lw t0, 0(s0)
    sw t0, 0(s2)
    addi s3, s3, 1
    
    # Если в массиве только один элемент - завершаем
    li t0, 1
    beq s1, t0, process_end
    
    # Обрабатываем остальные элементы массива A
    li s4, 1
process_loop:
    bge s4, s1, process_end
    
    # Загрузка A[i]
    slli t0, s4, 2
    add t0, s0, t0
    lw t1, 0(t0)
    
    # Загрузка A[i-1]
    addi t0, s4, -1
    slli t0, t0, 2
    add t0, s0, t0
    lw t2, 0(t0)
    
    # Проверка условия неубывания: A[i] >= A[i-1]
    blt t1, t2, process_end
    
    # Условие выполняется - добавляем A[i] в B
    slli t0, s3, 2
    add t0, s2, t0
    sw t1, 0(t0)
    addi s3, s3, 1
    
    addi s4, s4, 1
    j process_loop

process_end:
    mv a0, s2
    mv a1, s3
    
    lw s4, 0(sp)
    lw s3, 4(sp)
    lw s2, 8(sp)
    lw s1, 12(sp)
    lw s0, 16(sp)
    lw ra, 20(sp)
    addi sp, sp, 24
    ret