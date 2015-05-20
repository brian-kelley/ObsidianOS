.section .text

.global _getchar
_getchar:
	movl $0x50, %eax
	ret
