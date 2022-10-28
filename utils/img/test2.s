	.arch armv6
	.eabi_attribute 28, 1
	.eabi_attribute 23, 1
	.eabi_attribute 24, 1
	.eabi_attribute 25, 1
	.eabi_attribute 26, 2
	.eabi_attribute 30, 2
	.eabi_attribute 34, 1
	.eabi_attribute 18, 4
	.file	"test.c"
	.text
	.section	.rodata.str1.4,"aMS",%progbits,1
	.align	2
.LC0:
	.section	.text.startup,"ax",%progbits
	.align	2
	.global	square16
	.arch armv6
	.syntax unified
	.arm
	.fpu neon
	.type	square16, %function
square16:
  push {lr}  
  vldr d19, [r0]   
  vldr d20, [r0,8]   
  vmov.u8 d21, 0xf0
  vand.u8 d19, d21
  vand.u8 d20, d21
  vsri.i8 d17, d19, #4
  vsri.i8 d18, d20, #4
  vst1.64 {d17-d18}, [r1]  // write to [r5]

  pop {pc}  // pops link register value back into the program counter
LC1:
	.size	square16, .-square16
	.ident	"GCC: (Raspbian 10.2.1-6+rpi1) 10.2.1 20210110"
	.section	.note.GNU-stack,"",%progbits
