.data
	globals__: .space 12
.text
_lb_print:
	li $v0, 1
	lw $a0, 4($sp)
	syscall
	li $v0, 11
	li $a0, 0x0a
	syscall
	jr $ra
	nop # add (0 declarations)
_lb_add:
	lw $a0, 4($sp)
	sw $a0, 0($sp)
	addiu $sp, $sp, -4
	lw $a0, 12($sp)
	lw $t0, 4($sp)
	addu $a0, $t0, $a0
	addiu $sp, $sp, 4
	jr $ra
	jr $ra # should not reach here
	nop # main (1 declarations)
_lb_main:
	sw $0, 4($sp)
	li $a0, 3
	sw $a0, 4($sp)
	sw $ra, 0($sp)
	addiu $sp, $sp, -4
	sw $ra, 0($sp)
	addiu $sp, $sp, -4
	li $a0, 2
	sw $a0, 0($sp)
	addiu $sp, $sp, -4
	lw $a0, 16($sp)
	sw $a0, 0($sp)
	addiu $sp, $sp, -4
	jal _lb_add
	addiu $sp, $sp, 8
	lw $ra, 4($sp)
	addiu $sp, $sp, 4
	sw $a0, 0($sp)
	addiu $sp, $sp, -4
	jal _lb_print
	addiu $sp, $sp, 4
	lw $ra, 4($sp)
	addiu $sp, $sp, 4
	jr $ra # should not reach here
main:
	la $t9, globals__
	sw $0, 4($t9)
	addiu $sp, $sp, -4
	jal _lb_main
	addiu $sp, $sp, 4
	li $v0, 10
	syscall

