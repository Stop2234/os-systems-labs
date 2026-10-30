.data
result_msg: .asciz "Maximum argument for 32-bit factorial: "
newline:    .asciz "\n"

.text
.globl main

main:

    li s0, 0           
    li s1, 0           
    
test_loop:
    mv a0, s0
    jal factorial
    
    li t0, -1
    beq a0, t0, overflow_found
    
    mv s1, s0
    addi s0, s0, 1
    j test_loop

overflow_found:
    la a0, result_msg
    li a7, 4
    ecall
    mv a0, s1
    li a7, 1
    ecall
    la a0, newline
    li a7, 4
    ecall
 
    li a7, 10
    ecall

factorial:
    beqz a0, base_case
    addi sp, sp, -12
    sw ra, 8(sp)
    sw a0, 4(sp)
    addi a0, a0, -1
    jal factorial
    li t0, -1
    beq a0, t0, recursive_overflow
    lw t1, 4(sp)
    
    mul t2, t1, a0
    
    div t3, t2, t1
    bne t3, a0, multiplication_overflow
    
    mv a0, t2
    j factorial_end

base_case:
    li a0, 1
    ret

recursive_overflow:
multiplication_overflow:
    li a0, -1

factorial_end:
    lw ra, 8(sp)
    addi sp, sp, 12
    ret