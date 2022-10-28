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
	.ascii	"arm neon is defined\000"
	.align	2
.LC1:
	.ascii	"%d \000"
	.align	2
.LC2:
	.ascii	"\012\000"
	.section	.text.startup,"ax",%progbits
	.align	2
	.global	main
	.arch armv6
	.syntax unified
	.arm
	.fpu neon
	.type	main, %function
main:
	@ args = 0, pretend = 0, frame = 8192
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, lr}
	sub	sp, sp, #8192  @ sp = start of 8192-byte array
	mov	r0, sp         @ r0 = pointer to the beginning of 8192-byte array
	mov	r1, #0         @ r1 = 0  
	ldr	r2, .L8+16     @ r2 = 8188  TODO see if rationale has to do with any required vector operating size
	str	r1, [r0], #4   @ r1 = put the value at [r0 + 4] into r1
	bl	memset         @ memset uses r0 for address, r1 for value, and r2 for size.
	ldr	r0, .L8+20     @ r0 = address of "arm neon is defined\000"
	bl	puts           @ WOW! This calls the puts function to print to stdout!


	mov	r3, sp         @ r3 = stack pointer (beginning of array)
  @ This is where vector stuff begins.
	vldr	d18, .L8     @ load double-word at .L8's address (0x00000000,00000001) into d18
	vldr	d19, .L8+8   @ load double-word at .L8+8's address (0x00000002,00000003) into d19
  @ q9: index
  @ q3: incrementer of index

  @ i32 treats q3 (128 bits) as having 4 32-bit elements. The below is correct.
	vmov.i32	q3, #16  @ v4si   @ 0x00000010,00000010,00000010,00000010  @ incrementer
	vmov.i32	q15, #4  @ v4si   @ 0x00000004,00000004,00000004,00000004
	vmov.i32	q14, #8  @ v4si   @ 0x00000008,00000008,00000008,00000008
	vmov.i32	q13, #12 @ v4si   @ 0x0000000C,0000000C,0000000C,0000000C
.L2:
  @ a quadword vector is 16 bytes.
  @ q10 = 000k000k000k000k (k = i + 4)  (each digit is a byte unit)
	vadd.i32	q10, q9, q15  
  @ q8  = 000i000i000i000i + 000C000C000C000C
	vadd.i32	q8, q9, q13   
  @ copies the least significant half of each quadword vector element to each double-word vector element
  @ d22 = 0i0i0i0i
	vmovn.i32	d22, q9       
  @ d23 = 0k0k0k0k  (k = i + 4)
	vmovn.i32	d23, q10      @ d23 = result of q10 # whats' the purpose of this unused register?
  @ q12 = 0m0m0m0m  (m = i + 8)
	vadd.i32	q12, q9, q14  
	add	r2, sp, #8192       @ r2 = end address in array loop condition
	vmovn.i32	d20, q12      @ why unused?  
	vmovn.i32	d21, q8       @ why unused?
	vmovn.i16	d16, q11      @ what's even in q11? Autopopulated somehow?
	vmovn.i16	d17, q10      @ {indexs + 4s}

  @ This is the for-loop's compare-then-increment structure.
    @ r2: sp + 8192
    @ r3: original sp

  @ It looks like this is telling it to write 64 bytes at r3 to {d16-d17}.
	vst1.64	{d16-d17}, [r3:64]!  @ word = 4bytes, double = 8 bytes, so two doubles = 16... integers?
	cmp	r3, r2
	vqadd.u8	q9, q9, q3  @ increment the index by 16 each time in units of 4 bytes
	bne	.L2
	add	r5, sp, #8128
	ldr	r6, .L8+24
	sub	r4, sp, #1
	add	r5, r5, #63
.L3:
	ldrb	r1, [r4, #1]!	@ zero_extendqisi2
	mov	r0, r6
	bl	printf
	cmp	r4, r5
	bne	.L3
	ldr	r0, .L8+28
	bl	puts
	mov	r0, #0  // return value
	add	sp, sp, #8192
	@ sp needed
	pop	{r4, r5, r6, pc}
.L9:
	.align	3
.L8:
	.word	0
	.word	1
	.word	2
	.word	3
	.word	8188  @ becasue 
	.word	.LC0
	.word	.LC1
	.word	.LC2
	.size	main, .-main
	.ident	"GCC: (Raspbian 10.2.1-6+rpi1) 10.2.1 20210110"
	.section	.note.GNU-stack,"",%progbits
