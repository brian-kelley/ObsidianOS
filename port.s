.section .text

.global readport
.global writeport

readport:		//byte readport(dword portNum)
	movl $0, %edx
	movl 4(%esp), %edx
	in %dx, %al
	ret

writeport:		//void writeport(dword portNum, dword value)
	movl $0, %edx
	movl 4(%esp), %edx
	movl 8(%esp), %eax
	out %al, %dx
	ret
