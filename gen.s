.data
	globals__: .space 8
.text
_lb_print:
	li $v0, 1
	lw $a0, 4($sp)
	syscall
	li $v0, 11
	li $a0, 0x0a
	syscall
	jr $ra
_lb_main:
	li $a0, 0
	sw $a0, 4($sp)
	sw $ra, 0($sp)
	addiu $sp, $sp, -4
	li $a0, 42
	not $a0, $a0
	addiu $a0, $a0, 1
	sw $a0, 0($sp)
	addiu $sp, $sp, -4
	jal _lb_print
	addiu $sp, $sp, 4
	lw $ra, 4($sp)
	addiu $sp, $sp, 4
	li $a0, 2
	sw $a0, 8($sp)
_lp_begin_1:
	lw $a0, 4($sp)
	sw $a0, 0($sp)
	addiu $sp, $sp, -4
	li $a0, 11
	lw $t0, 4($sp)
	slt $a0, $t0, $a0
	addiu $sp, $sp, 4
	beqz $a0, _lp_end_1
	sw $ra, 0($sp)
	addiu $sp, $sp, -4
	lw $a0, 8($sp)
	sw $a0, 0($sp)
	addiu $sp, $sp, -4
	lw $a0, 12($sp)
	lw $t0, 4($sp)
	mul $a0, $t0, $a0
	addiu $sp, $sp, 4
	sw $a0, 0($sp)
	addiu $sp, $sp, -4
	jal _lb_print
	addiu $sp, $sp, 4
	lw $ra, 4($sp)
	addiu $sp, $sp, 4
_lp_begin_2:
	lw $a0, 8($sp)
	sw $a0, 0($sp)
	addiu $sp, $sp, -4
	li $a0, 11
	lw $t0, 4($sp)
	slt $a0, $t0, $a0
	addiu $sp, $sp, 4
	beqz $a0, _lp_end_2
	sw $ra, 0($sp)
	addiu $sp, $sp, -4
	lw $a0, 12($sp)
	not $a0, $a0
	addiu $a0, $a0, 1
	sw $a0, 0($sp)
	addiu $sp, $sp, -4
	jal _lb_print
	addiu $sp, $sp, 4
	lw $ra, 4($sp)
	addiu $sp, $sp, 4
	j _lp_end_2
	j _lp_begin_2
_lp_end_2:
	lw $a0, 4($sp)
	sw $a0, 0($sp)
	addiu $sp, $sp, -4
	li $a0, 1
	lw $t0, 4($sp)
	addu $a0, $t0, $a0
	addiu $sp, $sp, 4
	sw $a0, 4($sp)
	j _lp_begin_1
_lp_end_1:
	sw $ra, 0($sp)
	addiu $sp, $sp, -4
	li $a0, 56
	not $a0, $a0
	addiu $a0, $a0, 1
	sw $a0, 0($sp)
	addiu $sp, $sp, -4
	jal _lb_print
	addiu $sp, $sp, 4
	lw $ra, 4($sp)
	addiu $sp, $sp, 4
_lp_begin_3:
	lw $a0, 4($sp)
	sw $a0, 0($sp)
	addiu $sp, $sp, -4
	li $a0, 0
	lw $t0, 4($sp)
	slt $a0, $t0, $a0
	xori $a0, $a0, 1
	addiu $sp, $sp, 4
	beqz $a0, _lp_end_3
	lw $a0, 4($sp)
	sw $a0, 0($sp)
	addiu $sp, $sp, -4
	li $a0, 1
	lw $t0, 4($sp)
	subu $a0, $t0, $a0
	addiu $sp, $sp, 4
	sw $a0, 4($sp)
	lw $a0, 4($sp)
	sw $a0, 0($sp)
	addiu $sp, $sp, -4
	li $a0, 5
	lw $t0, 4($sp)
	slt $a0, $t0, $a0
	addiu $sp, $sp, 4
	beqz $a0, _if_false_1
	j _lp_begin_3
	j _if_end_1
_if_false_1:
_if_end_1:
	sw $ra, 0($sp)
	addiu $sp, $sp, -4
	lw $a0, 8($sp)
	sw $a0, 0($sp)
	addiu $sp, $sp, -4
	jal _lb_print
	addiu $sp, $sp, 4
	lw $ra, 4($sp)
	addiu $sp, $sp, 4
	j _lp_begin_3
_lp_end_3:
	jr $ra # should not reach here
main:
	la $t9, globals__
	addiu $sp, $sp, -8
	jal _lb_main
	addiu $sp, $sp, 8
	li $v0, 10
	syscall

