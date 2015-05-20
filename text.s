.section .text

.global getchar
getchar:
	movl $0x50, %eax
	ret
