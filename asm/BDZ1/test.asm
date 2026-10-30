.data
test1_msg:      .asciz "Test 1 - Increasing sequence: "
test2_msg:      .asciz "Test 2 - Decreasing sequence: "
test3_msg:      .asciz "Test 3 - All equal: "
test4_msg:      .asciz "Test 4 - Mixed: "
passed_msg:     .asciz "PASSED\n"
failed_msg:     .asciz "FAILED\n"

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
    # Тест 1
    la a0, test1_msg
    li a7, 4
    ecall
    la a0, test1_a
    lw a1, test1_size
    jal process_array_28
    # ... проверка как раньше

# Подключаем ВСЁ что нужно
.include "macros.asm"
.include "core.asm"