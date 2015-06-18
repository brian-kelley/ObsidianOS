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
	//Write to an unused port to delay and give the previous write time to have effect
	movl $80, %edx
	movl $0, %eax
	out %al, %dx
	ret

readportw:		//dword readport(dword portNum)
	movl $0, %edx
	movl 4(%esp), %edx
	inw %dx, %ax
	ret

writeportw:		//void writeport(dword portNum, dword value)
	movl 4(%esp), %edx
	movl 8(%esp), %eax
	outw %ax, %dx
	//Write to an unused port to delay and give the previous write time to have effect
	movl $80, %edx
	movl $0, %eax
	out %al, %dx
	ret

enableInterrupts:
	sti
	ret

disableInterrupts:
	cli
	ret
