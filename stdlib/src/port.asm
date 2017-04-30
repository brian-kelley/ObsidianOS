section .text

global readport
global writeport
global readportw
global writeportw
global enableInterrupts
global disableInterrupts

readport:		;byte readport(dword portNum)
  mov edx, [esp + 4]
	in al, dx
	ret

writeport:	;void writeport(dword portNum, dword value)
  mov edx, [esp + 4]
  mov eax, [esp + 8]
  out dx, al
	ret

readportw:	;word readport(dword portNum)
  mov edx, [esp + 4]
  in ax, dx
	ret

writeportw:	;void writeport(dword portNum, dword value)
  mov edx, [esp + 4]
  mov eax, [esp + 8]
  out dx, ax
	ret

enableInterrupts:
	sti
	ret

disableInterrupts:
	cli
	ret

