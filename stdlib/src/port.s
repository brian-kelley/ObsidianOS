.section .text

.global readport
.global writeport
.global readportw
.global writeportw
.global enableInterrupts
.global disableInterrupts

readport:		//byte readport(dword portNum)
	movl $0, %edx
	movl 4(%esp), %edx
	in %dx, %al
	ret

writeport:		//void writeport(dword portNum, dword value)
	movl 4(%esp), %edx
	movl 8(%esp), %eax
	out %al, %dx
	ret

readportw:		//word readport(dword portNum)
	movl $0, %edx
	movl 4(%esp), %edx
	inw %dx, %ax
	ret

writeportw:		//void writeport(dword portNum, dword value)
	movl 4(%esp), %edx
	movl 8(%esp), %eax
	outw %ax, %dx
	ret

enableInterrupts:
	sti
	ret

disableInterrupts:
	cli
	ret
