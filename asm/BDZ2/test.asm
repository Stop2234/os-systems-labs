.data
test_header:    .asciz "=== SQUARE ROOT TESTS ===\n"
test1_msg:      .asciz "Test 1 - sqrt(4.0): "
test2_msg:      .asciz "Test 2 - sqrt(2.0): "
test3_msg:      .asciz "Test 3 - sqrt(9.0): "
test4_msg:      .asciz "Test 4 - sqrt(0.25): "
test5_msg:      .asciz "Test 5 - sqrt(1.0): "
passed_msg:     .asciz "PASSED\n"
failed_msg:     .asciz "FAILED - expected: "
space:          .asciz " "

# Тестовые данные
test1_val:      .double 4.0
test1_exp:      .double 2.0

test2_val:      .double 2.0
test2_exp:      .double 1.414213562

test3_val:      .double 9.0
test3_exp:      .double 3.0

test4_val:      .double 0.25
test4_exp:      .double 0.5

test5_val:      .double 1.0
test5_exp:      .double 1.0

epsilon_test:   .double 0.0005  # 0.05%

.text
.globl main

main:
    la a0, test_header
    li a7, 4
    ecall
    
    # Тест 1: sqrt(4.0)
    la a0, test1_msg
    li a7, 4
    ecall
    
    la t0, test1_val
    fld fa0, 0(t0)
    jal sqrt_heron
    
    la t0, test1_exp
    fld ft0, 0(t0)
    jal check_result
    beqz a0, test1_fail
    
    la a0, passed_msg
    li a7, 4
    ecall
    j test2

test1_fail:
    mv t1, a0
    la a0, failed_msg
    li a7, 4
    ecall
    fmv.d fa0, ft0
    jal print_double
    la a0, newline
    li a7, 4
    ecall
    j test2

test2:
    # Тест 2: sqrt(2.0)
    la a0, test2_msg
    li a7, 4
    ecall
    
    la t0, test2_val
    fld fa0, 0(t0)
    jal sqrt_heron
    
    la t0, test2_exp
    fld ft0, 0(t0)
    jal check_result
    beqz a0, test2_fail
    
    la a0, passed_msg
    li a7, 4
    ecall
    j test3

test2_fail:
    mv t1, a0
    la a0, failed_msg
    li a7, 4
    ecall
    fmv.d fa0, ft0
    jal print_double
    la a0, newline
    li a7, 4
    ecall
    j test3

test3:
    # Тест 3: sqrt(9.0)
    la a0, test3_msg
    li a7, 4
    ecall
    
    la t0, test3_val
    fld fa0, 0(t0)
    jal sqrt_heron
    
    la t0, test3_exp
    fld ft0, 0(t0)
    jal check_result
    beqz a0, test3_fail
    
    la a0, passed_msg
    li a7, 4
    ecall
    j test4

test3_fail:
    mv t1, a0
    la a0, failed_msg
    li a7, 4
    ecall
    fmv.d fa0, ft0
    jal print_double
    la a0, newline
    li a7, 4
    ecall
    j test4

test4:
    # Тест 4: sqrt(0.25)
    la a0, test4_msg
    li a7, 4
    ecall
    
    la t0, test4_val
    fld fa0, 0(t0)
    jal sqrt_heron
    
    la t0, test4_exp
    fld ft0, 0(t0)
    jal check_result
    beqz a0, test4_fail
    
    la a0, passed_msg
    li a7, 4
    ecall
    j test5

test4_fail:
    mv t1, a0
    la a0, failed_msg
    li a7, 4
    ecall
    fmv.d fa0, ft0
    jal print_double
    la a0, newline
    li a7, 4
    ecall
    j test5

test5:
    # Тест 5: sqrt(1.0)
    la a0, test5_msg
    li a7, 4
    ecall
    
    la t0, test5_val
    fld fa0, 0(t0)
    jal sqrt_heron
    
    la t0, test5_exp
    fld ft0, 0(t0)
    jal check_result
    beqz a0, test5_fail
    
    la a0, passed_msg
    li a7, 4
    ecall
    j end

test5_fail:
    mv t1, a0
    la a0, failed_msg
    li a7, 4
    ecall
    fmv.d fa0, ft0
    jal print_double
    la a0, newline
    li a7, 4
    ecall

end:
    # Завершение программы
    li a7, 10
    ecall

# ========== ПОДПРОГРАММА ПРОВЕРКИ РЕЗУЛЬТАТА ==========
# Вход: fa1 = вычисленное значение, ft0 = ожидаемое значение
# Выход: a0 = 1 (PASSED), 0 (FAILED)
check_result:
    addi sp, sp, -16
    fsd fs0, 8(sp)
    sw ra, 0(sp)
    
    la t0, epsilon_test
    fld ft1, 0(t0)
    
    # Вычисляем относительную ошибку: |actual - expected| / |expected|
    fsub.d ft2, fa1, ft0  
    fabs.d ft2, ft2       
    fabs.d ft3, ft0        
    fdiv.d ft2, ft2, ft3    
    
    # Сравниваем с epsilon
    fle.d t0, ft2, ft1     
    
    mv a0, t0
    
    lw ra, 0(sp)
    fld fs0, 8(sp)
    addi sp, sp, 16
    ret

# Подключаем функцию вычисления корня
.include "core.asm"