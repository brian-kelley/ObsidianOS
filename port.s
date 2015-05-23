.section .text

.global readport
.global writeport
.global passthru

readport:		//byte readport(dword portNum)
	movl $0, %edx
	movl 4(%esp), %dx
	in %dx, %al
	ret

writeport:		//void writeport(dword portNum, dword value)
	movl $0, %edx
	movl 8(%esp), %edx
	movl 4(%esp), %eax
	out %al, %dx
	ret

passthru:
	movl 4(%esp), %eax
	ret
