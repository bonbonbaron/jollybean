	.arch armv6
	.eabi_attribute 28, 1
	.eabi_attribute 20, 1
	.eabi_attribute 21, 1
	.eabi_attribute 23, 3
	.eabi_attribute 24, 1
	.eabi_attribute 25, 1
	.eabi_attribute 26, 2
	.eabi_attribute 30, 2
	.eabi_attribute 34, 1
	.eabi_attribute 18, 4
	.file	"data.c"
	.text
	.align	2
	.arch armv6
	.syntax unified
	.arm
	.fpu neon
	.type	preMapSet.part.0, %function
preMapSet.part.0:
	@ args = 4, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, lr}
	push	{lr}
	bl	__gnu_mcount_nc
	sub	ip, r1, #1
	and	r1, ip, #7
	asr	lr, ip, #3
	add	ip, r0, lr, lsl #1
	ldrb	r5, [r0, lr, lsl #1]	@ zero_extendqisi2
	ldrb	ip, [ip, #1]	@ zero_extendqisi2
	mvn	lr, #0
	bic	ip, ip, lr, lsl r1
	ldr	lr, [r0, #64]
	lsr	r1, ip, #1
	and	r1, r1, #85
	sub	ip, ip, r1
	and	r4, ip, #51
	lsr	ip, ip, #2
	and	ip, ip, #51
	add	ip, r4, ip
	add	ip, ip, ip, lsr #4
	and	r1, ip, #15
	ldr	ip, [lr, #-8]
	add	r1, r1, r5
	mla	lr, ip, r1, lr
	str	lr, [r2]
	ldrb	ip, [r0, #63]	@ zero_extendqisi2
	ldrb	r5, [r0, #62]	@ zero_extendqisi2
	lsr	r4, ip, #1
	and	r4, r4, #85
	sub	ip, ip, r4
	and	r2, ip, #51
	lsr	ip, ip, #2
	and	ip, ip, #51
	add	r2, r2, ip
	add	r2, r2, r2, lsr #4
	and	r2, r2, #15
	add	r2, r2, r5
	cmp	r1, r2
	bcc	.L6
	mov	r0, #0
	pop	{r4, r5, r6, pc}
.L6:
	ldr	r0, [r0, #64]
	cmp	r0, #0
	subne	r2, r2, r1
	ldrne	r0, [r0, #-8]
	addne	lr, lr, r0
	mulne	r0, r0, r2
	ldr	r2, [sp, #16]
	str	r0, [r2]
	mov	r0, #0
	str	lr, [r3]
	pop	{r4, r5, r6, pc}
	.size	preMapSet.part.0, .-preMapSet.part.0
	.align	2
	.global	jbAlloc
	.syntax unified
	.arm
	.fpu neon
	.type	jbAlloc, %function
jbAlloc:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, lr}
	push	{lr}
	bl	__gnu_mcount_nc
	subs	r4, r0, #0
	beq	.L9
	mul	r0, r1, r2
	bl	malloc
	mov	r3, r0
	cmp	r0, #0
	movne	r0, #0
	moveq	r0, #2
	str	r3, [r4]
	pop	{r4, pc}
.L9:
	mov	r0, #1
	pop	{r4, pc}
	.size	jbAlloc, .-jbAlloc
	.align	2
	.global	jbFree
	.syntax unified
	.arm
	.fpu neon
	.type	jbFree, %function
jbFree:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, lr}
	push	{lr}
	bl	__gnu_mcount_nc
	subs	r4, r0, #0
	popeq	{r4, pc}
	ldr	r0, [r4]
	bl	free
	mov	r3, #0
	str	r3, [r4]
	pop	{r4, pc}
	.size	jbFree, .-jbFree
	.align	2
	.global	arrayNew
	.syntax unified
	.arm
	.fpu neon
	.type	arrayNew, %function
arrayNew:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, lr}
	push	{lr}
	bl	__gnu_mcount_nc
	cmp	r0, #0
	cmpne	r1, #0
	mov	r6, r2
	mov	r5, r1
	moveq	r7, #1
	movne	r7, #0
	mov	r4, r0
	beq	.L19
	cmp	r2, #0
	bne	.L18
	mov	r0, r2
	str	r2, [r4]
	pop	{r4, r5, r6, r7, r8, pc}
.L18:
	mul	r8, r2, r1
	add	r0, r8, #8
	bl	malloc
	cmp	r0, #0
	beq	.L20
	str	r5, [r0]
	str	r6, [r0, #4]
	add	r0, r0, #8
	mov	r2, r8
	mov	r1, r7
	str	r0, [r4]
	bl	memset
	mov	r0, r7
	pop	{r4, r5, r6, r7, r8, pc}
.L19:
	mov	r0, #1
	pop	{r4, r5, r6, r7, r8, pc}
.L20:
	mov	r0, #2
	pop	{r4, r5, r6, r7, r8, pc}
	.size	arrayNew, .-arrayNew
	.align	2
	.global	arrayDel
	.syntax unified
	.arm
	.fpu neon
	.type	arrayDel, %function
arrayDel:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, lr}
	push	{lr}
	bl	__gnu_mcount_nc
	subs	r4, r0, #0
	popeq	{r4, pc}
	ldr	r0, [r4]
	cmp	r0, #0
	popeq	{r4, pc}
	sub	r0, r0, #8
	bl	free
	mov	r3, #0
	str	r3, [r4]
	pop	{r4, pc}
	.size	arrayDel, .-arrayDel
	.align	2
	.global	arrayGetNElems
	.syntax unified
	.arm
	.fpu neon
	.type	arrayGetNElems, %function
arrayGetNElems:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	push	{lr}
	bl	__gnu_mcount_nc
	cmp	r0, #0
	ldrne	r0, [r0, #-4]
	bx	lr
	.size	arrayGetNElems, .-arrayGetNElems
	.align	2
	.global	arrayGetElemSz
	.syntax unified
	.arm
	.fpu neon
	.type	arrayGetElemSz, %function
arrayGetElemSz:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	push	{lr}
	bl	__gnu_mcount_nc
	cmp	r0, #0
	ldrne	r0, [r0, #-8]
	bx	lr
	.size	arrayGetElemSz, .-arrayGetElemSz
	.align	2
	.global	arrayGetVoidElemPtr
	.syntax unified
	.arm
	.fpu neon
	.type	arrayGetVoidElemPtr, %function
arrayGetVoidElemPtr:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	push	{lr}
	bl	__gnu_mcount_nc
	cmp	r0, #0
	bxeq	lr
	cmp	r1, #0
	ldr	r3, [r0, #-4]
	blt	.L42
	cmp	r3, r1
	bls	.L43
	ldr	r3, [r0, #-8]
	mla	r0, r3, r1, r0
	bx	lr
.L42:
	ldr	r1, [r0, #-8]
	mla	r0, r1, r3, r0
	bx	lr
.L43:
	mov	r0, #0
	bx	lr
	.size	arrayGetVoidElemPtr, .-arrayGetVoidElemPtr
	.align	2
	.global	arraySetVoidElem
	.syntax unified
	.arm
	.fpu neon
	.type	arraySetVoidElem, %function
arraySetVoidElem:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, lr}
	push	{lr}
	bl	__gnu_mcount_nc
	cmp	r0, #0
	mov	r3, r1
	mov	r1, r2
	beq	.L46
	ldr	r2, [r0, #-8]
	mla	r0, r2, r3, r0
	bl	memcpy
	mov	r0, #0
	pop	{r4, pc}
.L46:
	mov	r0, #1
	pop	{r4, pc}
	.size	arraySetVoidElem, .-arraySetVoidElem
	.align	2
	.global	arrayIniPtrs
	.syntax unified
	.arm
	.fpu neon
	.type	arrayIniPtrs, %function
arrayIniPtrs:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	push	{lr}
	bl	__gnu_mcount_nc
	cmp	r0, #0
	str	r0, [r1]
	beq	.L50
	cmp	r3, #0
	ldr	r1, [r0, #-4]
	blt	.L57
	cmp	r1, r3
	movls	r0, #0
	ldrhi	r1, [r0, #-8]
	mlahi	r0, r1, r3, r0
.L50:
	str	r0, [r2]
	bx	lr
.L57:
	ldr	r3, [r0, #-8]
	mla	r0, r3, r1, r0
	str	r0, [r2]
	bx	lr
	.size	arrayIniPtrs, .-arrayIniPtrs
	.align	2
	.global	mapNew
	.syntax unified
	.arm
	.fpu neon
	.type	mapNew, %function
mapNew:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, r9, r10, lr}
	push	{lr}
	bl	__gnu_mcount_nc
	cmp	r2, #0
	cmpne	r1, #0
	moveq	r7, #1
	mov	r6, r0
	mov	r4, r1
	movne	r7, #0
	mov	r5, r2
	moveq	r7, #1
	beq	.L58
	cmp	r0, #0
	beq	.L65
	mov	r0, #68
	bl	malloc
	cmp	r0, #0
	mov	r8, r0
	str	r0, [r6]
	beq	.L62
	smulbb	r9, r4, r5
	add	r0, r9, #8
	bl	malloc
	subs	r3, r0, #0
	beq	.L62
	mov	r2, r9
	add	r0, r3, #8
	mov	r1, r7
	str	r4, [r3]
	str	r5, [r3, #4]
	str	r0, [r8, #64]
	bl	memset
	mov	r2, #64
	mov	r1, r7
	mov	r0, r8
	bl	memset
.L58:
	mov	r0, r7
	pop	{r4, r5, r6, r7, r8, r9, r10, pc}
.L65:
	mov	r7, #1
.L60:
	ldr	r0, [r6]
	ldr	r5, [r0, #64]
	cmp	r5, #0
	beq	.L63
	ldr	r3, [r5]
	cmp	r3, #0
	beq	.L63
	sub	r0, r3, #8
	bl	free
	mov	r3, #0
	str	r3, [r5]
	ldr	r0, [r6]
.L63:
	bl	free
	mov	r3, #0
	mov	r0, r7
	str	r3, [r6]
	pop	{r4, r5, r6, r7, r8, r9, r10, pc}
.L62:
	mov	r7, #2
	b	.L60
	.size	mapNew, .-mapNew
	.align	2
	.global	mapDel
	.syntax unified
	.arm
	.fpu neon
	.type	mapDel, %function
mapDel:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, lr}
	push	{lr}
	bl	__gnu_mcount_nc
	subs	r4, r0, #0
	popeq	{r4, r5, r6, pc}
	ldr	r5, [r4]
	cmp	r5, #0
	popeq	{r4, r5, r6, pc}
	ldr	r0, [r5, #64]
	cmp	r0, #0
	beq	.L77
	sub	r0, r0, #8
	bl	free
	mov	r3, #0
	str	r3, [r5, #64]
	ldr	r5, [r4]
.L77:
	mov	r0, r5
	bl	free
	mov	r3, #0
	str	r3, [r4]
	pop	{r4, r5, r6, pc}
	.size	mapDel, .-mapDel
	.align	2
	.global	mapGetIndex
	.syntax unified
	.arm
	.fpu neon
	.type	mapGetIndex, %function
mapGetIndex:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, lr}
	push	{lr}
	bl	__gnu_mcount_nc
	sub	r1, r1, #1
	uxtb	r3, r1
	and	ip, r3, #7
	lsr	r4, r3, #3
	add	r3, r0, r4, lsl #1
	ldrb	lr, [r3, #1]	@ zero_extendqisi2
	mov	r3, #1
	ands	ip, lr, r3, lsl ip
	bne	.L90
	mov	r0, #4
	pop	{r4, pc}
.L90:
	and	r1, r1, #7
	ldrb	ip, [r0, r4, lsl #1]	@ zero_extendqisi2
	lsl	r3, r3, r1
	sub	r3, r3, #1
	and	r3, r3, lr
	mov	r0, #0
	lsr	r1, r3, #1
	and	r1, r1, #85
	sub	r3, r3, r1
	and	r1, r3, #51
	lsr	r3, r3, #2
	and	r3, r3, #51
	add	r3, r1, r3
	add	r3, r3, r3, lsr #4
	and	r3, r3, #15
	add	r3, ip, r3
	strb	r3, [r2]
	pop	{r4, pc}
	.size	mapGetIndex, .-mapGetIndex
	.align	2
	.global	mapGet
	.syntax unified
	.arm
	.fpu neon
	.type	mapGet, %function
mapGet:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	push	{lr}
	bl	__gnu_mcount_nc
	sub	r1, r1, #1
	and	r3, r1, #7
	lsr	ip, r1, #3
	mov	r1, #1
	add	r2, r0, ip, lsl #1
	lsl	r1, r1, r3
	ldrb	r2, [r2, #1]	@ zero_extendqisi2
	ands	r3, r2, r1
	beq	.L93
	ldr	r3, [r0, #64]
	ldrb	r0, [r0, ip, lsl #1]	@ zero_extendqisi2
	sub	r1, r1, #1
	ldr	ip, [r3, #-8]
	and	r1, r1, r2
	.syntax divided
@ 204 "/home/bonbonbaron/hack/botox/src/data.c" 1
	vmov.8 d20[0], r1
	vcnt.i8 d20, d21
	vmov.u8 r1, d20[0]
@ 0 "" 2
	.arm
	.syntax unified
	add	r1, r0, r1
	mla	r0, ip, r1, r3
	bx	lr
.L93:
	mov	r0, r3
	bx	lr
	.size	mapGet, .-mapGet
	.align	2
	.global	mapSet
	.syntax unified
	.arm
	.fpu neon
	.type	mapSet, %function
mapSet:
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, lr}
	sub	sp, sp, #28
	push	{lr}
	bl	__gnu_mcount_nc
	mov	r3, #0
	subs	r4, r0, #0
	mov	r5, r1
	mov	r7, r2
	str	r3, [sp, #20]
	beq	.L102
	ldr	r3, [r4, #64]
	cmp	r3, #0
	cmpne	r1, #0
	bne	.L152
.L102:
	mov	r6, #1
.L94:
	mov	r0, r6
	add	sp, sp, #28
	@ sp needed
	pop	{r4, r5, r6, r7, pc}
.L152:
	add	r3, sp, #20
	str	r3, [sp]
	add	r2, sp, #12
	add	r3, sp, #16
	bl	preMapSet.part.0
	subs	r6, r0, #0
	bne	.L94
	ldr	r2, [sp, #20]
	ldr	r0, [sp, #12]
	cmp	r2, #0
	bne	.L153
.L96:
	ldr	r3, [r4, #64]
	sub	r5, r5, #1
	cmp	r3, #0
	moveq	r2, r3
	ldrne	r2, [r3, #-8]
	mov	r1, r7
	bl	memcpy
	asr	r1, r5, #3
	mov	ip, #1
	add	r0, r4, r1, lsl #1
	and	r5, r5, #7
	ldrb	r2, [r0, #1]	@ zero_extendqisi2
	uxtb	r3, r1
	orr	r5, r2, ip, lsl r5
	add	r2, r3, ip
	strb	r5, [r0, #1]
	uxtb	r2, r2
	cmp	r2, #32
	beq	.L94
	rsb	r0, r3, #30
	uxtb	r0, r0
	cmp	r0, #15
	bls	.L99
	add	r3, r3, #17
	add	r1, r1, ip
	uxtb	r2, r3
	add	r3, r4, r1, lsl ip
	vmov.i8	q8, #1  @ v16qi
	vld2.8	{d20-d23}, [r3]
	add	r1, r4, r1, lsl #1
	vadd.i8	q8, q8, q10
	vst1.8	{d16[0]}, [r1]
	add	r1, r3, #2
	vst1.8	{d16[1]}, [r1]
	add	r1, r3, #4
	vst1.8	{d16[2]}, [r1]
	add	r1, r3, #6
	vst1.8	{d16[3]}, [r1]
	add	r1, r3, #8
	vst1.8	{d16[4]}, [r1]
	add	r1, r3, #10
	vst1.8	{d16[5]}, [r1]
	add	r1, r3, #12
	vst1.8	{d16[6]}, [r1]
	add	r1, r3, #14
	vst1.8	{d16[7]}, [r1]
	add	r1, r3, #16
	vst1.8	{d17[0]}, [r1]
	add	r1, r3, #18
	vst1.8	{d17[1]}, [r1]
	add	r1, r3, #20
	vst1.8	{d17[2]}, [r1]
	add	r1, r3, #22
	vst1.8	{d17[3]}, [r1]
	add	r1, r3, #24
	vst1.8	{d17[4]}, [r1]
	add	r1, r3, #26
	vst1.8	{d17[5]}, [r1]
	add	r1, r3, #28
	add	r3, r3, #30
	vst1.8	{d17[6]}, [r1]
	vst1.8	{d17[7]}, [r3]
.L99:
	ldrb	r1, [r4, r2, lsl #1]	@ zero_extendqisi2
	add	r3, r2, #1
	uxtb	r3, r3
	add	r1, r1, #1
	cmp	r3, #32
	strb	r1, [r4, r2, lsl #1]
	beq	.L94
	ldrb	r1, [r4, r3, lsl #1]	@ zero_extendqisi2
	add	r1, r1, #1
	strb	r1, [r4, r3, lsl #1]
	add	r3, r2, #2
	uxtb	r3, r3
	cmp	r3, #32
	beq	.L94
	ldrb	r1, [r4, r3, lsl #1]	@ zero_extendqisi2
	add	r1, r1, #1
	strb	r1, [r4, r3, lsl #1]
	add	r3, r2, #3
	uxtb	r3, r3
	cmp	r3, #32
	beq	.L94
	ldrb	r1, [r4, r3, lsl #1]	@ zero_extendqisi2
	add	r1, r1, #1
	strb	r1, [r4, r3, lsl #1]
	add	r3, r2, #4
	uxtb	r3, r3
	cmp	r3, #32
	beq	.L94
	ldrb	r1, [r4, r3, lsl #1]	@ zero_extendqisi2
	add	r1, r1, #1
	strb	r1, [r4, r3, lsl #1]
	add	r3, r2, #5
	uxtb	r3, r3
	cmp	r3, #32
	beq	.L94
	ldrb	r1, [r4, r3, lsl #1]	@ zero_extendqisi2
	add	r1, r1, #1
	strb	r1, [r4, r3, lsl #1]
	add	r3, r2, #6
	uxtb	r3, r3
	cmp	r3, #32
	beq	.L94
	ldrb	r1, [r4, r3, lsl #1]	@ zero_extendqisi2
	add	r1, r1, #1
	strb	r1, [r4, r3, lsl #1]
	add	r3, r2, #7
	uxtb	r3, r3
	cmp	r3, #32
	beq	.L94
	ldrb	r1, [r4, r3, lsl #1]	@ zero_extendqisi2
	add	r1, r1, #1
	strb	r1, [r4, r3, lsl #1]
	add	r3, r2, #8
	uxtb	r3, r3
	cmp	r3, #32
	beq	.L94
	ldrb	r1, [r4, r3, lsl #1]	@ zero_extendqisi2
	add	r1, r1, #1
	strb	r1, [r4, r3, lsl #1]
	add	r3, r2, #9
	uxtb	r3, r3
	cmp	r3, #32
	beq	.L94
	ldrb	r1, [r4, r3, lsl #1]	@ zero_extendqisi2
	add	r1, r1, #1
	strb	r1, [r4, r3, lsl #1]
	add	r3, r2, #10
	uxtb	r3, r3
	cmp	r3, #32
	beq	.L94
	ldrb	r1, [r4, r3, lsl #1]	@ zero_extendqisi2
	add	r1, r1, #1
	strb	r1, [r4, r3, lsl #1]
	add	r3, r2, #11
	uxtb	r3, r3
	cmp	r3, #32
	beq	.L94
	ldrb	r1, [r4, r3, lsl #1]	@ zero_extendqisi2
	add	r1, r1, #1
	strb	r1, [r4, r3, lsl #1]
	add	r3, r2, #12
	uxtb	r3, r3
	cmp	r3, #32
	beq	.L94
	ldrb	r1, [r4, r3, lsl #1]	@ zero_extendqisi2
	add	r1, r1, #1
	strb	r1, [r4, r3, lsl #1]
	add	r3, r2, #13
	uxtb	r3, r3
	cmp	r3, #32
	beq	.L94
	ldrb	r1, [r4, r3, lsl #1]	@ zero_extendqisi2
	add	r1, r1, #1
	strb	r1, [r4, r3, lsl #1]
	add	r3, r2, #14
	uxtb	r3, r3
	cmp	r3, #32
	beq	.L94
	ldrb	r1, [r4, r3, lsl #1]	@ zero_extendqisi2
	add	r2, r2, #15
	uxtb	r2, r2
	add	r1, r1, #1
	cmp	r2, #32
	strb	r1, [r4, r3, lsl #1]
	ldrbne	r3, [r4, r2, lsl #1]	@ zero_extendqisi2
	addne	r3, r3, #1
	strbne	r3, [r4, r2, lsl #1]
	b	.L94
.L153:
	mov	r1, r0
	ldr	r0, [sp, #16]
	bl	memcpy
	ldr	r0, [sp, #12]
	b	.L96
	.size	mapSet, .-mapSet
	.align	2
	.global	mapRem
	.syntax unified
	.arm
	.fpu neon
	.type	mapRem, %function
mapRem:
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, lr}
	sub	sp, sp, #24
	push	{lr}
	bl	__gnu_mcount_nc
	mov	r3, #0
	subs	r4, r0, #0
	mov	r5, r1
	str	r3, [sp, #20]
	beq	.L161
	ldr	r3, [r4, #64]
	cmp	r3, #0
	cmpne	r1, #0
	bne	.L210
.L161:
	mov	r6, #1
.L154:
	mov	r0, r6
	add	sp, sp, #24
	@ sp needed
	pop	{r4, r5, r6, pc}
.L210:
	add	r3, sp, #20
	str	r3, [sp]
	add	r2, sp, #12
	add	r3, sp, #16
	bl	preMapSet.part.0
	subs	r6, r0, #0
	bne	.L154
	ldr	r2, [sp, #20]
	cmp	r2, #0
	bne	.L211
.L156:
	sub	r5, r5, #1
	mov	ip, #1
	asr	r1, r5, #3
	and	r5, r5, #7
	add	r0, r4, r1, lsl #1
	uxtb	r3, r1
	ldrb	r2, [r0, #1]	@ zero_extendqisi2
	bic	r5, r2, ip, lsl r5
	add	r2, r3, ip
	strb	r5, [r0, #1]
	uxtb	r2, r2
	cmp	r2, #32
	beq	.L154
	rsb	r0, r3, #30
	uxtb	r0, r0
	cmp	r0, #15
	bls	.L158
	add	r3, r3, #17
	add	r1, r1, ip
	uxtb	r2, r3
	add	r3, r4, r1, lsl ip
	vmov.i32	q8, #4294967295  @ v16qi
	vld2.8	{d20-d23}, [r3]
	add	r1, r4, r1, lsl #1
	vadd.i8	q8, q8, q10
	vst1.8	{d16[0]}, [r1]
	add	r1, r3, #2
	vst1.8	{d16[1]}, [r1]
	add	r1, r3, #4
	vst1.8	{d16[2]}, [r1]
	add	r1, r3, #6
	vst1.8	{d16[3]}, [r1]
	add	r1, r3, #8
	vst1.8	{d16[4]}, [r1]
	add	r1, r3, #10
	vst1.8	{d16[5]}, [r1]
	add	r1, r3, #12
	vst1.8	{d16[6]}, [r1]
	add	r1, r3, #14
	vst1.8	{d16[7]}, [r1]
	add	r1, r3, #16
	vst1.8	{d17[0]}, [r1]
	add	r1, r3, #18
	vst1.8	{d17[1]}, [r1]
	add	r1, r3, #20
	vst1.8	{d17[2]}, [r1]
	add	r1, r3, #22
	vst1.8	{d17[3]}, [r1]
	add	r1, r3, #24
	vst1.8	{d17[4]}, [r1]
	add	r1, r3, #26
	vst1.8	{d17[5]}, [r1]
	add	r1, r3, #28
	add	r3, r3, #30
	vst1.8	{d17[6]}, [r1]
	vst1.8	{d17[7]}, [r3]
.L158:
	ldrb	r1, [r4, r2, lsl #1]	@ zero_extendqisi2
	add	r3, r2, #1
	uxtb	r3, r3
	sub	r1, r1, #1
	cmp	r3, #32
	strb	r1, [r4, r2, lsl #1]
	beq	.L154
	ldrb	r1, [r4, r3, lsl #1]	@ zero_extendqisi2
	sub	r1, r1, #1
	strb	r1, [r4, r3, lsl #1]
	add	r3, r2, #2
	uxtb	r3, r3
	cmp	r3, #32
	beq	.L154
	ldrb	r1, [r4, r3, lsl #1]	@ zero_extendqisi2
	sub	r1, r1, #1
	strb	r1, [r4, r3, lsl #1]
	add	r3, r2, #3
	uxtb	r3, r3
	cmp	r3, #32
	beq	.L154
	ldrb	r1, [r4, r3, lsl #1]	@ zero_extendqisi2
	sub	r1, r1, #1
	strb	r1, [r4, r3, lsl #1]
	add	r3, r2, #4
	uxtb	r3, r3
	cmp	r3, #32
	beq	.L154
	ldrb	r1, [r4, r3, lsl #1]	@ zero_extendqisi2
	sub	r1, r1, #1
	strb	r1, [r4, r3, lsl #1]
	add	r3, r2, #5
	uxtb	r3, r3
	cmp	r3, #32
	beq	.L154
	ldrb	r1, [r4, r3, lsl #1]	@ zero_extendqisi2
	sub	r1, r1, #1
	strb	r1, [r4, r3, lsl #1]
	add	r3, r2, #6
	uxtb	r3, r3
	cmp	r3, #32
	beq	.L154
	ldrb	r1, [r4, r3, lsl #1]	@ zero_extendqisi2
	sub	r1, r1, #1
	strb	r1, [r4, r3, lsl #1]
	add	r3, r2, #7
	uxtb	r3, r3
	cmp	r3, #32
	beq	.L154
	ldrb	r1, [r4, r3, lsl #1]	@ zero_extendqisi2
	sub	r1, r1, #1
	strb	r1, [r4, r3, lsl #1]
	add	r3, r2, #8
	uxtb	r3, r3
	cmp	r3, #32
	beq	.L154
	ldrb	r1, [r4, r3, lsl #1]	@ zero_extendqisi2
	sub	r1, r1, #1
	strb	r1, [r4, r3, lsl #1]
	add	r3, r2, #9
	uxtb	r3, r3
	cmp	r3, #32
	beq	.L154
	ldrb	r1, [r4, r3, lsl #1]	@ zero_extendqisi2
	sub	r1, r1, #1
	strb	r1, [r4, r3, lsl #1]
	add	r3, r2, #10
	uxtb	r3, r3
	cmp	r3, #32
	beq	.L154
	ldrb	r1, [r4, r3, lsl #1]	@ zero_extendqisi2
	sub	r1, r1, #1
	strb	r1, [r4, r3, lsl #1]
	add	r3, r2, #11
	uxtb	r3, r3
	cmp	r3, #32
	beq	.L154
	ldrb	r1, [r4, r3, lsl #1]	@ zero_extendqisi2
	sub	r1, r1, #1
	strb	r1, [r4, r3, lsl #1]
	add	r3, r2, #12
	uxtb	r3, r3
	cmp	r3, #32
	beq	.L154
	ldrb	r1, [r4, r3, lsl #1]	@ zero_extendqisi2
	sub	r1, r1, #1
	strb	r1, [r4, r3, lsl #1]
	add	r3, r2, #13
	uxtb	r3, r3
	cmp	r3, #32
	beq	.L154
	ldrb	r1, [r4, r3, lsl #1]	@ zero_extendqisi2
	sub	r1, r1, #1
	strb	r1, [r4, r3, lsl #1]
	add	r3, r2, #14
	uxtb	r3, r3
	cmp	r3, #32
	beq	.L154
	ldrb	r1, [r4, r3, lsl #1]	@ zero_extendqisi2
	add	r2, r2, #15
	uxtb	r2, r2
	sub	r1, r1, #1
	cmp	r2, #32
	strb	r1, [r4, r3, lsl #1]
	ldrbne	r3, [r4, r2, lsl #1]	@ zero_extendqisi2
	subne	r3, r3, #1
	strbne	r3, [r4, r2, lsl #1]
	b	.L154
.L211:
	ldrd	r0, [sp, #12]
	bl	memcpy
	b	.L156
	.size	mapRem, .-mapRem
	.align	2
	.global	mapGetNestedMapP
	.syntax unified
	.arm
	.fpu neon
	.type	mapGetNestedMapP, %function
mapGetNestedMapP:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, lr}
	push	{lr}
	bl	__gnu_mcount_nc
	cmp	r1, #0
	cmpne	r0, #0
	moveq	ip, #1
	movne	ip, #0
	beq	.L214
	sub	r1, r1, #1
	lsr	r3, r1, #3
	and	r1, r1, #7
	add	r4, r0, r3, lsl #1
	ldrb	lr, [r0, r3, lsl #1]	@ zero_extendqisi2
	mov	r3, #1
	ldrb	r4, [r4, #1]	@ zero_extendqisi2
	lsl	r1, r3, r1
	tst	r1, r4
	beq	.L216
	ldr	r3, [r0, #64]
	sub	r1, r1, #1
	and	r1, r1, r4
	.syntax divided
@ 204 "/home/bonbonbaron/hack/botox/src/data.c" 1
	vmov.8 d20[0], r1
	vcnt.i8 d20, d21
	vmov.u8 r1, d20[0]
@ 0 "" 2
	.arm
	.syntax unified
	add	lr, lr, r1
	ldr	r1, [r3, #-8]
	mul	lr, r1, lr
	ldr	r3, [r3, lr]
	cmp	r3, #0
	beq	.L216
	mov	r0, ip
	str	r3, [r2]
	pop	{r4, pc}
.L216:
	mov	r0, #4
	pop	{r4, pc}
.L214:
	mov	r0, #1
	pop	{r4, pc}
	.size	mapGetNestedMapP, .-mapGetNestedMapP
	.align	2
	.global	mapGetNestedMapPElem
	.syntax unified
	.arm
	.fpu neon
	.type	mapGetNestedMapPElem, %function
mapGetNestedMapPElem:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, lr}
	push	{lr}
	bl	__gnu_mcount_nc
	cmp	r3, #0
	cmpne	r2, #0
	beq	.L222
	cmp	r1, #0
	cmpne	r0, #0
	beq	.L219
	sub	r1, r1, #1
	lsr	ip, r1, #3
	and	r1, r1, #7
	add	lr, r0, ip, lsl #1
	ldrb	ip, [r0, ip, lsl #1]	@ zero_extendqisi2
	ldrb	r4, [lr, #1]	@ zero_extendqisi2
	mov	lr, #1
	lsl	r1, lr, r1
	tst	r1, r4
	beq	.L219
	ldr	r0, [r0, #64]
	sub	r1, r1, #1
	and	r1, r1, r4
	.syntax divided
@ 204 "/home/bonbonbaron/hack/botox/src/data.c" 1
	vmov.8 d20[0], r1
	vcnt.i8 d20, d21
	vmov.u8 r1, d20[0]
@ 0 "" 2
	.arm
	.syntax unified
	add	ip, ip, r1
	ldr	r1, [r0, #-8]
	mul	ip, r1, ip
	ldr	r0, [r0, ip]
	cmp	r0, #0
	beq	.L219
	sub	r1, r2, #1
	lsr	r4, r1, #3
	and	r1, r1, #7
	lsl	r2, lr, r1
	add	r1, r0, r4, lsl lr
	ldrb	ip, [r1, #1]	@ zero_extendqisi2
	tst	r2, ip
	beq	.L230
	ldrb	r1, [r0, r4, lsl #1]	@ zero_extendqisi2
	ldr	r0, [r0, #64]
	sub	r2, r2, #1
	and	r2, r2, ip
	ldr	ip, [r0, #-8]
	.syntax divided
@ 204 "/home/bonbonbaron/hack/botox/src/data.c" 1
	vmov.8 d20[0], r2
	vcnt.i8 d20, d21
	vmov.u8 r2, d20[0]
@ 0 "" 2
	.arm
	.syntax unified
	add	r2, r1, r2
	mul	r2, ip, r2
	ldr	r2, [r0, r2]
	str	r2, [r3]
.L221:
	cmp	r2, #0
	beq	.L223
.L219:
	mov	r0, #0
	pop	{r4, pc}
.L222:
	mov	r0, #1
	pop	{r4, pc}
.L230:
	ldr	r2, [r3]
	b	.L221
.L223:
	mov	r0, #4
	pop	{r4, pc}
	.size	mapGetNestedMapPElem, .-mapGetNestedMapPElem
	.align	2
	.global	histoNew
	.syntax unified
	.arm
	.fpu neon
	.type	histoNew, %function
histoNew:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, lr}
	push	{lr}
	bl	__gnu_mcount_nc
	subs	r5, r0, #0
	mov	r4, r1
	beq	.L234
	cmp	r1, #0
	bne	.L233
	mov	r0, r1
	str	r1, [r5]
	pop	{r4, r5, r6, pc}
.L233:
	lsl	r6, r1, #2
	add	r0, r6, #8
	bl	malloc
	subs	r3, r0, #0
	beq	.L235
	add	r0, r3, #8
	mov	r1, #4
	str	r1, [r3]
	mov	r2, r6
	mov	r1, #0
	str	r4, [r3, #4]
	str	r0, [r5]
	bl	memset
	mov	r0, #0
	pop	{r4, r5, r6, pc}
.L234:
	mov	r0, #1
	pop	{r4, r5, r6, pc}
.L235:
	mov	r0, #2
	pop	{r4, r5, r6, pc}
	.size	histoNew, .-histoNew
	.align	2
	.global	histoDel
	.syntax unified
	.arm
	.fpu neon
	.type	histoDel, %function
histoDel:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, lr}
	push	{lr}
	bl	__gnu_mcount_nc
	subs	r4, r0, #0
	popeq	{r4, pc}
	ldr	r0, [r4]
	cmp	r0, #0
	popeq	{r4, pc}
	sub	r0, r0, #8
	bl	free
	mov	r3, #0
	str	r3, [r4]
	pop	{r4, pc}
	.size	histoDel, .-histoDel
	.section	.rodata.str1.4,"aMS",%progbits,1
	.align	2
.LC2:
	.ascii	"\003\003\013\000"
	.align	2
.LC0:
	.ascii	"\005\005\004\000"
	.align	2
.LC1:
	.ascii	"\002\003\007\000"
	.text
	.align	2
	.global	tinfl_decompress
	.syntax unified
	.arm
	.fpu neon
	.type	tinfl_decompress, %function
tinfl_decompress:
	@ args = 12, pretend = 0, frame = 224
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, r9, r10, fp, lr}
	vpush.64	{d8, d9, d10, d11, d12, d13}
	sub	sp, sp, #228
	push	{lr}
	bl	__gnu_mcount_nc
	mov	fp, r3
	ldr	r3, [r2]
	mov	r7, r2
	add	r3, r1, r3
	str	r3, [sp, #8]
	ldr	r3, [sp, #316]
	ldr	r2, [sp, #320]
	ldr	r4, [sp, #312]
	ldr	r3, [r3]
	ands	r2, r2, #4
	str	r2, [sp, #28]
	add	r2, r4, r3
	mov	r5, r1
	mov	r9, r0
	str	r2, [sp, #24]
	bne	.L481
	sub	r3, r3, #1
	sub	r2, r4, fp
	add	r6, r2, r3
	add	r3, r6, #1
	tst	r3, r6
	bne	.L246
.L245:
	cmp	r4, fp
	bcc	.L246
	ldr	r1, [r9, #56]
	ldm	r9, {r2, r8}
	str	r1, [sp]
	ldr	r1, [r9, #32]
	ldr	r10, [r9, #36]
	str	r1, [sp, #12]
	ldr	r1, [r9, #40]
	str	r1, [sp, #16]
	ldr	r1, [r9, #60]
	str	r1, [sp, #20]
	cmp	r2, #53
	ldrls	pc, [pc, r2, asl #2]
	b	.L482
.L251:
	.word	.L283
	.word	.L282
	.word	.L281
	.word	.L280
	.word	.L482
	.word	.L279
	.word	.L278
	.word	.L277
	.word	.L482
	.word	.L483
	.word	.L484
	.word	.L274
	.word	.L482
	.word	.L482
	.word	.L273
	.word	.L482
	.word	.L272
	.word	.L485
	.word	.L270
	.word	.L482
	.word	.L482
	.word	.L486
	.word	.L482
	.word	.L268
	.word	.L487
	.word	.L266
	.word	.L265
	.word	.L264
	.word	.L482
	.word	.L482
	.word	.L482
	.word	.L482
	.word	.L263
	.word	.L482
	.word	.L488
	.word	.L489
	.word	.L490
	.word	.L491
	.word	.L492
	.word	.L493
	.word	.L494
	.word	.L255
	.word	.L254
	.word	.L482
	.word	.L482
	.word	.L482
	.word	.L482
	.word	.L482
	.word	.L482
	.word	.L482
	.word	.L482
	.word	.L253
	.word	.L495
	.word	.L496
.L481:
	mvn	r6, #0
	mov	r3, #0
	b	.L245
.L482:
	mov	r6, #0
	mov	r5, r6
	mvn	r0, #0
.L249:
	ldr	r3, [sp]
	str	r8, [r9, #4]
	str	r3, [r9, #56]
	ldr	r3, [sp, #12]
	str	r10, [r9, #36]
	str	r3, [r9, #32]
	ldr	r3, [sp, #16]
	str	r3, [r9, #40]
	ldr	r3, [sp, #20]
	str	r3, [r9, #60]
	ldr	r3, [sp, #316]
	str	r5, [r7]
	str	r6, [r3]
	ldr	r3, [sp, #320]
	tst	r3, #9
	beq	.L244
	cmn	r0, #1
	beq	.L244
	ldr	r5, .L675+32
	ldr	r1, [r9, #28]
	cmp	r6, #0
	umull	r3, r5, r5, r6
	ldr	r3, .L675+36
	uxth	fp, r1
	lsr	r5, r5, #11
	lsr	r1, r1, #16
	mul	r5, r3, r5
	sub	r5, r6, r5
	beq	.L471
	cmp	r5, #7
	ldr	r8, .L675+40
	bls	.L635
.L475:
	sub	ip, r5, #8
	add	lr, r4, #16
	bic	r3, ip, #7
	add	lr, lr, r3
	add	r2, r4, #8
	lsr	r7, ip, #3
.L472:
	ldrb	r3, [r2, #-8]	@ zero_extendqisi2
	ldrb	r10, [r2, #-7]	@ zero_extendqisi2
	add	r2, r2, #8
	add	fp, r3, fp
	add	r10, r10, fp
	add	r3, fp, r10
	ldrb	fp, [r2, #-14]	@ zero_extendqisi2
	add	r10, fp, r10
	ldrb	fp, [r2, #-13]	@ zero_extendqisi2
	add	r3, r3, r10
	add	r10, fp, r10
	ldrb	fp, [r2, #-12]	@ zero_extendqisi2
	add	r3, r3, r10
	add	r10, fp, r10
	ldrb	fp, [r2, #-11]	@ zero_extendqisi2
	add	r3, r3, r10
	add	r10, fp, r10
	ldrb	fp, [r2, #-10]	@ zero_extendqisi2
	add	r3, r3, r10
	add	r10, fp, r10
	ldrb	fp, [r2, #-9]	@ zero_extendqisi2
	add	r3, r3, r10
	cmp	r2, lr
	add	fp, fp, r10
	add	r3, r3, fp
	add	r1, r1, r3
	bne	.L472
	add	r7, r7, #1
	bic	ip, ip, #7
	add	r4, r4, r7, lsl #3
	add	ip, ip, #8
.L476:
	cmp	ip, r5
	bcs	.L473
	ldrb	r3, [r4]	@ zero_extendqisi2
	add	fp, fp, r3
	add	r3, ip, #1
	cmp	r3, r5
	add	r1, r1, fp
	bcs	.L474
	ldrb	r3, [r4, #1]	@ zero_extendqisi2
	add	fp, fp, r3
	add	r3, ip, #2
	cmp	r5, r3
	add	r1, r1, fp
	bls	.L474
	ldrb	r3, [r4, #2]	@ zero_extendqisi2
	add	fp, fp, r3
	add	r3, ip, #3
	cmp	r5, r3
	add	r1, r1, fp
	bls	.L474
	ldrb	r3, [r4, #3]	@ zero_extendqisi2
	add	fp, fp, r3
	add	r3, ip, #4
	cmp	r3, r5
	add	r1, r1, fp
	bcs	.L474
	ldrb	r3, [r4, #4]	@ zero_extendqisi2
	add	fp, fp, r3
	add	r3, ip, #5
	cmp	r5, r3
	add	r1, r1, fp
	bls	.L474
	ldrb	r3, [r4, #5]	@ zero_extendqisi2
	add	fp, fp, r3
	add	r3, ip, #6
	cmp	r3, r5
	add	r1, r1, fp
	bcs	.L474
	ldrb	r3, [r4, #6]	@ zero_extendqisi2
	add	fp, fp, r3
	add	r3, ip, #7
	cmp	r5, r3
	add	r1, r1, fp
	ldrbhi	r3, [r4, #7]	@ zero_extendqisi2
	addhi	fp, fp, r3
	addhi	r1, r1, fp
.L474:
	sub	ip, r5, ip
	add	r4, r4, ip
.L473:
	umull	r3, r2, r8, fp
	umull	ip, r3, r8, r1
	lsr	r2, r2, #15
	lsr	r3, r3, #15
	rsb	ip, r2, r2, lsl #12
	sub	r6, r6, r5
	add	r2, r2, ip, lsl #4
	sub	fp, fp, r2
	rsb	r2, r3, r3, lsl #12
	cmp	r6, #0
	add	r3, r3, r2, lsl #4
	ldr	r5, .L675+36
	sub	r1, r1, r3
	bne	.L475
.L471:
	add	fp, fp, r1, lsl #16
	cmp	r0, #0
	str	fp, [r9, #28]
	bne	.L244
	ldr	r3, [sp, #320]
	tst	r3, #1
	beq	.L244
	ldr	r3, [r9, #16]
	cmp	fp, r3
	mvnne	r0, #1
.L244:
	add	sp, sp, #228
	@ sp needed
	vldm	sp!, {d8-d13}
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L635:
	mov	ip, #0
	b	.L476
.L253:
	ldr	r3, [sp, #8]
	cmp	r5, r3
	bcs	.L507
	mov	r2, r5
	mov	r0, r4
	ldrb	r3, [r2], #1	@ zero_extendqisi2
	str	r2, [sp, #4]
	ldr	r2, [sp]
	orr	r3, r2, r3, lsl r8
	str	r3, [sp]
.L321:
	add	r3, r8, #8
	cmp	r3, #7
	bhi	.L319
.L318:
	ldr	r2, [sp, #4]
	ldr	r1, [sp, #8]
	cmp	r2, r1
	movcs	r8, r3
	bcs	.L322
	ldr	r2, [sp, #4]
	ldr	ip, [sp]
	mov	r8, r3
	ldrb	r1, [r2], #1	@ zero_extendqisi2
	str	r2, [sp, #4]
	orr	r3, ip, r1, lsl r3
	str	r3, [sp]
	b	.L321
.L254:
	ldr	r3, [sp, #8]
	cmp	r5, r3
	bcs	.L536
	mov	r2, r5
	mov	r6, #0
	ldrb	r3, [r2], #1	@ zero_extendqisi2
	str	r2, [sp, #4]
.L466:
	ldr	r2, [r9, #16]
	add	r10, r10, #1
	cmp	r10, #3
	orr	r3, r3, r2, lsl #8
	str	r3, [r9, #16]
	bhi	.L646
.L469:
	cmp	r8, #0
	beq	.L460
	cmp	r8, #7
	subhi	r8, r8, #8
	bhi	.L462
	mov	r3, r8
.L461:
	ldr	r2, [sp, #4]
	ldr	r1, [sp, #8]
	mov	r8, r3
	cmp	r2, r1
	bcs	.L465
	ldrb	r1, [r2], #1	@ zero_extendqisi2
	ldr	r0, [sp]
	str	r2, [sp, #4]
	orr	r3, r0, r1, lsl r3
	str	r3, [sp]
.L464:
	add	r3, r8, #8
	cmp	r3, #7
	bls	.L461
.L462:
	ldr	r2, [sp]
	uxtb	r3, r2
	lsr	r2, r2, #8
	str	r2, [sp]
	b	.L466
.L263:
	ldr	r3, [sp, #8]
	cmp	r5, r3
	bcs	.L533
	mov	r2, r5
	mov	r6, #0
	ldrb	r3, [r2], #1	@ zero_extendqisi2
	str	r2, [sp, #4]
	ldr	r2, [sp]
	orr	r3, r2, r3, lsl r8
	str	r3, [sp]
.L457:
	add	r8, r8, #8
.L645:
	and	r3, r8, #7
	cmp	r3, r8
	bls	.L647
	ldr	r3, [sp, #4]
	ldr	r2, [sp, #8]
	cmp	r3, r2
	bcs	.L458
	ldr	r3, [sp, #4]
	ldr	r1, [sp]
	ldrb	r2, [r3], #1	@ zero_extendqisi2
	orr	r2, r1, r2, lsl r8
	strd	r2, [sp]
	b	.L457
.L268:
	ldr	r3, [sp, #8]
	cmp	r5, r3
	bcs	.L516
	mov	r2, r5
	str	r4, [sp, #32]
	ldrb	r3, [r2], #1	@ zero_extendqisi2
	str	r2, [sp, #4]
	ldr	r2, [sp]
	orr	r3, r2, r3, lsl r8
	str	r3, [sp]
.L399:
	add	r8, r8, #8
	cmp	r8, #14
	bhi	.L391
.L401:
	ldr	r3, [sp]
	lsl	r3, r3, #22
	lsr	r3, r3, #22
	add	r3, r3, #176
	lsl	r3, r3, #1
	ldrsh	r1, [r9, r3]
	cmp	r1, #0
	mov	r3, r1
	blt	.L393
	asrs	r2, r1, #9
	movne	r3, #1
	moveq	r3, #0
	cmp	r8, r2
	movcc	r3, #0
	andcs	r3, r3, #1
	cmp	r3, #0
	bne	.L394
.L395:
	ldr	r3, [sp, #4]
	ldr	r2, [sp, #8]
	cmp	r3, r2
	bcs	.L400
	ldr	r3, [sp, #4]
	ldr	r1, [sp]
	ldrb	r2, [r3], #1	@ zero_extendqisi2
	orr	r2, r1, r2, lsl r8
	strd	r2, [sp]
	b	.L399
.L272:
	ldr	r3, [sp, #8]
	cmp	r5, r3
	bcs	.L512
	mov	r2, r5
	str	r4, [sp, #32]
	ldrb	r3, [r2], #1	@ zero_extendqisi2
	str	r2, [sp, #4]
	ldr	r2, [sp]
	orr	r3, r2, r3, lsl r8
	str	r3, [sp]
.L372:
	add	r8, r8, #8
	cmp	r8, #14
	bhi	.L364
.L374:
	ldr	r3, [sp]
	lsl	r3, r3, #22
	lsr	r3, r3, #22
	add	r3, r3, #3664
	lsl	r3, r3, #1
	ldrsh	r1, [r9, r3]
	cmp	r1, #0
	mov	r3, r1
	blt	.L366
	asrs	r2, r1, #9
	movne	r3, #1
	moveq	r3, #0
	cmp	r8, r2
	movcc	r3, #0
	andcs	r3, r3, #1
	cmp	r3, #0
	bne	.L367
.L368:
	ldr	r3, [sp, #4]
	ldr	r2, [sp, #8]
	cmp	r3, r2
	bcs	.L373
	ldr	r3, [sp, #4]
	ldr	r1, [sp]
	ldrb	r2, [r3], #1	@ zero_extendqisi2
	orr	r2, r1, r2, lsl r8
	strd	r2, [sp]
	b	.L372
.L270:
	ldr	r3, [sp, #8]
	cmp	r5, r3
	bcs	.L513
	mov	r2, r5
	str	r4, [sp, #32]
	ldrb	r3, [r2], #1	@ zero_extendqisi2
	str	r2, [sp, #4]
	ldr	r2, [sp]
	orr	r3, r2, r3, lsl r8
	str	r3, [sp]
.L381:
	ldr	r3, [sp, #16]
	add	r8, r8, #8
	cmp	r3, r8
	bls	.L648
.L383:
	ldr	r3, [sp, #4]
	ldr	r2, [sp, #8]
	cmp	r3, r2
	bcs	.L382
	ldrb	r2, [r3], #1	@ zero_extendqisi2
	ldr	r1, [sp]
	orr	r2, r1, r2, lsl r8
	add	r8, r8, #8
	strd	r2, [sp]
	ldr	r3, [sp, #16]
	cmp	r3, r8
	bhi	.L383
.L648:
	ldr	r3, [sp, #12]
	sub	r3, r3, #16
.L379:
	ldr	r2, [sp, #12]
	ldr	r1, [sp, #16]
	cmp	r2, #16
	ldr	r2, .L675+44
	ldr	r0, [sp]
	sub	r8, r8, r1
	ldrb	r2, [r2, r3]	@ zero_extendqisi2
	mvn	r3, #0
	bic	r3, r0, r3, lsl r1
	add	r2, r2, r3
	lsr	r3, r0, r1
	str	r3, [sp]
	add	r3, r9, #10496
	add	r3, r3, #36
	add	r0, r3, r10
	addeq	r3, r9, r10
	addeq	r3, r3, #10496
	movne	r1, #0
	ldrbeq	r1, [r3, #35]	@ zero_extendqisi2
	add	r10, r10, r2
	bl	memset
.L350:
	ldr	r2, [r9, #48]
	ldr	r3, [r9, #44]
	str	r2, [sp, #36]
	add	r2, r3, r2
	cmp	r2, r10
	bhi	.L385
	beq	.L386
	ldr	r3, [sp, #4]
	sub	r5, r3, r5
	ldr	r3, [sp, #32]
	sub	r6, r3, r4
	b	.L269
.L273:
	ldr	r3, [sp, #8]
	cmp	r5, r3
	bcs	.L510
	mov	r2, r5
	str	r4, [sp, #32]
	ldrb	r3, [r2], #1	@ zero_extendqisi2
	str	r2, [sp, #4]
	ldr	r2, [sp]
	orr	r3, r2, r3, lsl r8
	str	r3, [sp]
.L342:
	add	r8, r8, #8
	cmp	r8, #2
	bhi	.L649
.L344:
	ldr	r3, [sp, #4]
	ldr	r2, [sp, #8]
	cmp	r3, r2
	bcs	.L343
	ldr	r3, [sp, #4]
	ldr	r1, [sp]
	ldrb	r2, [r3], #1	@ zero_extendqisi2
	orr	r2, r1, r2, lsl r8
	strd	r2, [sp]
	b	.L342
.L255:
	ldr	r3, [sp, #8]
	cmp	r5, r3
	bcs	.L535
	mov	r2, r5
	mov	r6, #0
	ldrb	r3, [r2], #1	@ zero_extendqisi2
	str	r2, [sp, #4]
	ldr	r2, [sp]
	orr	r3, r2, r3, lsl r8
	str	r3, [sp]
	b	.L464
.L274:
	ldr	r3, [sp, #8]
	cmp	r5, r3
	bcs	.L509
	ldr	r3, .L675+48
	mov	r1, r5
	str	r4, [sp, #32]
	ldrb	r2, [r3, r10]	@ zero_extendqisi2
	ldrb	r3, [r1], #1	@ zero_extendqisi2
	str	r1, [sp, #4]
	ldr	r1, [sp]
	orr	r3, r1, r3, lsl r8
	str	r3, [sp]
	mov	r3, r2
.L335:
	add	r8, r8, #8
.L644:
	cmp	r8, r3
	bcs	.L650
	ldr	r1, [sp, #4]
	ldr	r0, [sp, #8]
	cmp	r1, r0
	bcs	.L336
	ldrb	r0, [r1], #1	@ zero_extendqisi2
	ldr	ip, [sp]
	orr	r0, ip, r0, lsl r8
	add	r8, r8, #8
	strd	r0, [sp]
	b	.L644
.L265:
	ldr	r3, [sp, #8]
	cmp	r5, r3
	bcs	.L523
	mov	r2, r5
	str	r4, [sp, #32]
	ldrb	r3, [r2], #1	@ zero_extendqisi2
	str	r2, [sp, #4]
	ldr	r2, [sp]
	orr	r3, r2, r3, lsl r8
	str	r3, [sp]
.L430:
	add	r8, r8, #8
	cmp	r8, #14
	bhi	.L422
.L432:
	ldr	r3, [sp]
	lsl	r3, r3, #22
	lsr	r3, r3, #22
	add	r3, r3, #1920
	lsl	r3, r3, #1
	ldrsh	r1, [r9, r3]
	cmp	r1, #0
	mov	r3, r1
	blt	.L424
	asrs	r2, r1, #9
	movne	r3, #1
	moveq	r3, #0
	cmp	r8, r2
	movcc	r3, #0
	andcs	r3, r3, #1
	cmp	r3, #0
	bne	.L425
.L426:
	ldr	r3, [sp, #4]
	ldr	r2, [sp, #8]
	cmp	r3, r2
	bcs	.L431
	ldrb	r2, [r3], #1	@ zero_extendqisi2
	ldr	r1, [sp]
	orr	r2, r1, r2, lsl r8
	add	r8, r8, #8
	cmp	r8, #14
	strd	r2, [sp]
	bls	.L432
.L422:
	ldr	r3, [sp]
	lsl	r3, r3, #22
	lsr	r3, r3, #22
	add	r3, r3, #1920
	lsl	r3, r3, #1
	ldrsh	r1, [r9, r3]
	cmp	r1, #0
	mov	r3, r1
	blt	.L427
	asr	r2, r1, #9
.L425:
	lsl	r3, r1, #23
	lsr	r3, r3, #23
.L433:
	ldr	r1, [sp]
	sub	r8, r8, r2
	lsr	r1, r1, r2
	ldr	r2, .L675+52
	str	r1, [sp]
	add	r3, r2, r3, lsl #2
	ldr	r2, [r3, #296]
	ldr	r3, [r3, #424]
	cmp	r2, #0
	str	r2, [sp, #16]
	str	r3, [sp, #12]
	beq	.L435
	cmp	r8, r2
	bcc	.L440
.L436:
	ldr	r2, [sp, #16]
	ldr	r1, [sp]
	mvn	r3, #0
	ldr	r0, [sp, #12]
	bic	r3, r1, r3, lsl r2
	add	r3, r0, r3
	str	r3, [sp, #12]
	sub	r8, r8, r2
	lsr	r3, r1, r2
	str	r3, [sp]
.L435:
	ldr	r3, [sp, #32]
	ldr	r2, [sp, #12]
	ldr	r1, [sp, #28]
	sub	r3, r3, fp
	cmp	r1, #0
	cmpne	r2, r3
	movhi	r0, #1
	movls	r0, #0
	str	r3, [sp, #36]
	bhi	.L651
	sub	r2, r3, r2
	and	r3, r2, r6
	add	ip, fp, r3
	str	r3, [sp, #48]
	ldr	r3, [sp, #32]
	ldr	r2, [sp, #24]
	cmp	r3, ip
	addcs	r3, r10, r3
	addcc	r3, r10, ip
	cmp	r2, r3
	bcc	.L442
	ldr	r3, [sp, #48]
	ldr	r1, [sp, #32]
	add	r3, r3, #48
	add	r3, fp, r3
	add	r2, r1, #48
	cmp	ip, r2
	cmpcc	r1, r3
	sub	r2, r10, #24
	movcs	r3, #1
	movcc	r3, #0
	cmn	r2, #-2147483624
	movhi	r2, #0
	movls	r2, #1
	tst	r2, r3
	beq	.L445
	ldr	r2, .L675+56
	sub	r3, r10, #3
	cmp	r3, #44
	umull	r3, r2, r2, r3
	lsr	r3, r2, #1
	str	r3, [sp, #52]
	add	r3, r3, #1
	str	r3, [sp, #44]
	bls	.L526
	vdup.32	q14, r1
	mov	r2, r1
	lsr	r1, r3, #4
	str	r8, [sp, #40]
	str	r4, [sp, #312]
	mov	r3, ip
	mov	r8, r0
	mov	r4, r1
	vldr	d16, .L675
	vldr	d17, .L675+8
	vdup.32	q15, r10
	vldr	d18, .L675+16
	vldr	d19, .L675+24
	vdup.32	q13, ip
	vadd.i32	q15, q15, q9
	vadd.i32	q14, q14, q8
	vadd.i32	q13, q13, q8
	vmov.i32	q2, #48  @ v4si
	vmov.i32	q5, #36  @ v4si
	vmov.i32	q4, #4294967248  @ v4si
	vmov.i32	q0, #4294967260  @ v4si
	vmov.i32	q3, #39  @ v4si
	vmov.i32	q1, #4294967257  @ v4si
	str	ip, [sp, #20]
.L447:
	add	r8, r8, #1
	cmp	r4, r8
	mov	ip, r3
	mov	r0, r2
	vld1.8	{q12}, [ip]!
	vmov	q8, q14  @ v4si
	vld1.8	{q11}, [ip]
	vmov	q9, q15  @ v4si
	add	lr, r3, #32
	add	r1, r2, #32
	vld1.8	{q10}, [lr]
	vmov	q6, q13  @ v4si
	vst1.8	{q12}, [r0]!
	add	r3, r3, #48
	vst1.8	{q11}, [r0]
	add	r2, r2, #48
	vst1.8	{q10}, [r1]
	vadd.i32	q11, q8, q5
	vadd.i32	q10, q9, q0
	vadd.i32	q14, q14, q2
	vadd.i32	q13, q13, q2
	vadd.i32	q15, q15, q4
	vadd.i32	q6, q6, q3
	vadd.i32	q8, q8, q3
	vadd.i32	q9, q9, q1
	bhi	.L447
	ldr	r3, [sp, #44]
	ldr	ip, [sp, #20]
	bic	r0, r3, #15
	ldr	r1, [sp, #32]
	sub	r2, r0, r0, lsl #2
	cmp	r0, r3
	add	r3, r0, r0, lsl #1
	add	ip, ip, r3
	add	r3, r1, r3
	add	r1, r2, r10
	ldr	r8, [sp, #40]
	add	r2, sp, #20
	str	r1, [sp, #40]
	vst1.32	{d13[1]}, [r2]
	ldr	r4, [sp, #312]
	add	r2, sp, #56
	lsl	r10, r0, #1
	vst1.32	{d17[1]}, [r2]
	vmov.32	r1, d19[1]
	vmov.32	lr, d23[1]
	vmov.32	r2, d21[1]
	beq	.L527
	ldr	r1, [sp, #52]
	sub	r2, r1, r0
	cmp	r2, #6
	ldr	r2, [sp, #44]
	sub	r2, r2, r0
	str	r2, [sp, #44]
	bls	.L449
.L446:
	ldr	r1, [sp, #32]
	add	r0, r10, r0
	add	r10, r1, r0
	ldr	r1, [sp, #40]
	ldr	r2, [sp, #48]
	str	r1, [sp, #80]
	ldr	lr, [sp, #44]
	sub	r1, r1, #3
	str	r1, [sp, #84]
	add	r2, r2, r0
	add	r1, r3, #3
	str	r3, [sp, #72]
	str	r1, [sp, #76]
	add	r0, fp, r2
	bic	r2, lr, #7
	vldr	d16, [sp, #80]
	mov	r1, r0
	vmov.i32	d20, #0x15  @ v2si
	vld1.8	{d23}, [r1]!
	add	r0, r0, #16
	vld1.8	{d22}, [r1]
	vmov.i32	d18, #0x12  @ v2si
	mov	r1, r10
	vmov.i32	d17, #0xffffffee  @ v2si
	vmov.i32	d19, #0xffffffeb  @ v2si
	vadd.i32	d19, d16, d19
	vadd.i32	d16, d16, d17
	vldr	d17, [sp, #72]
	vadd.i32	d21, d17, d20
	vadd.i32	d17, d17, d18
	vld1.8	{d18}, [r0]
	vst1.8	{d23}, [r1]!
	add	r0, r10, #16
	vst1.8	{d22}, [r1]
	vst1.8	{d18}, [r0]
	ldr	r0, [sp, #40]
	add	r1, r2, r2, lsl #1
	cmp	lr, r2
	add	lr, ip, #3
	sub	r2, r2, r2, lsl #2
	str	lr, [sp, #68]
	str	ip, [sp, #64]
	add	r2, r0, r2
	vldr	d18, [sp, #64]
	add	r3, r3, r1
	add	ip, ip, r1
	vadd.i32	d18, d18, d20
	str	r2, [sp, #40]
	vmov.32	r1, d19[1]
	add	r2, sp, #20
	mov	r10, r1
	vst1.32	{d18[1]}, [r2]
	vmov.32	lr, d17[1]
	add	r2, sp, #32
	vst1.32	{d21[1]}, [r2]
	vmov.32	r2, d16[1]
	beq	.L452
.L449:
	ldrb	r2, [ip]	@ zero_extendqisi2
	ldr	r0, [sp, #40]
	add	r1, ip, #3
	strb	r2, [r3]
	ldrb	r2, [ip, #1]	@ zero_extendqisi2
	str	r1, [sp, #20]
	add	lr, r3, #3
	strb	r2, [r3, #1]
	ldrb	r2, [ip, #2]	@ zero_extendqisi2
	strb	r2, [r3, #2]
	sub	r2, r0, #3
	cmp	r2, #2
	mov	r1, r2
	ble	.L528
	ldrb	r1, [ip, #3]	@ zero_extendqisi2
	sub	r10, r0, #6
	cmp	r10, #2
	strb	r1, [r3, #3]
	ldrb	r1, [ip, #4]	@ zero_extendqisi2
	strb	r1, [r3, #4]
	ldrb	r1, [ip, #5]	@ zero_extendqisi2
	strb	r1, [r3, #5]
	add	r1, r3, #6
	str	r1, [sp, #32]
	add	r1, ip, #6
	str	r1, [sp, #20]
	mov	r1, r10
	bls	.L452
	ldrb	r2, [ip, #6]	@ zero_extendqisi2
	sub	r1, r0, #9
	cmp	r1, #2
	strb	r2, [r3, #6]
	ldrb	r2, [ip, #7]	@ zero_extendqisi2
	mov	lr, r0
	str	r1, [sp, #40]
	strb	r2, [r3, #7]
	ldrb	r2, [ip, #8]	@ zero_extendqisi2
	add	r0, r3, #9
	strb	r2, [r3, #8]
	add	r2, ip, #9
	str	r2, [sp, #20]
	bls	.L529
	ldrb	r2, [ip, #9]	@ zero_extendqisi2
	add	r1, ip, #12
	mov	r10, lr
	strb	r2, [r3, #9]
	ldrb	r2, [ip, #10]	@ zero_extendqisi2
	str	r1, [sp, #20]
	strb	r2, [r3, #10]
	ldrb	r2, [ip, #11]	@ zero_extendqisi2
	strb	r2, [r3, #11]
	sub	r2, lr, #12
	cmp	r2, #2
	add	lr, r3, #12
	mov	r1, r2
	bls	.L530
	ldrb	r1, [ip, #12]	@ zero_extendqisi2
	mov	r0, r10
	sub	r10, r10, #15
	strb	r1, [r3, #12]
	ldrb	r1, [ip, #13]	@ zero_extendqisi2
	cmp	r10, #2
	strb	r1, [r3, #13]
	ldrb	r1, [ip, #14]	@ zero_extendqisi2
	strb	r1, [r3, #14]
	add	r1, r3, #15
	str	r1, [sp, #32]
	add	r1, ip, #15
	str	r1, [sp, #20]
	mov	r1, r10
	bls	.L452
	ldrb	r2, [ip, #15]	@ zero_extendqisi2
	add	r1, ip, #18
	str	r1, [sp, #20]
	strb	r2, [r3, #15]
	ldrb	r2, [ip, #16]	@ zero_extendqisi2
	add	lr, r3, #18
	strb	r2, [r3, #16]
	ldrb	r2, [ip, #17]	@ zero_extendqisi2
	strb	r2, [r3, #17]
	sub	r2, r0, #18
	cmp	r2, #2
	mov	r1, r2
	bls	.L531
	ldrb	r1, [ip, #18]	@ zero_extendqisi2
	sub	r10, r0, #21
	add	r3, r3, #21
	strb	r1, [r3, #-3]
	ldrb	r1, [ip, #19]	@ zero_extendqisi2
	strb	r1, [r3, #-2]
	ldrb	r1, [ip, #20]	@ zero_extendqisi2
	strb	r1, [r3, #-1]
	mov	r1, r10
	str	r3, [sp, #32]
	add	r3, ip, #21
	str	r3, [sp, #20]
.L452:
	cmp	r1, #0
	ble	.L532
	ldr	r3, [sp, #20]
	cmp	r1, #2
	ldr	r1, [sp, #32]
	ldrb	r3, [r3]	@ zero_extendqisi2
	strb	r3, [r1]
	ldreq	r3, [sp, #20]
	ldrbeq	r3, [r3, #1]	@ zero_extendqisi2
	strbeq	r3, [r1, #1]
	add	r3, lr, r2
	str	r3, [sp, #32]
	ldr	r3, [sp, #36]
	str	r3, [sp, #20]
	b	.L388
.L676:
	.align	3
.L675:
	.word	0
	.word	3
	.word	6
	.word	9
	.word	0
	.word	-3
	.word	-6
	.word	-9
	.word	1584310703
	.word	5552
	.word	-2146992015
	.word	.LC2
	.word	.LC0
	.word	.LANCHOR0
	.word	-1431655765
.L264:
	ldr	r3, [sp, #8]
	cmp	r5, r3
	bcs	.L524
	mov	r2, r5
	str	r4, [sp, #32]
	ldrb	r3, [r2], #1	@ zero_extendqisi2
	str	r2, [sp, #4]
	ldr	r2, [sp]
	orr	r3, r2, r3, lsl r8
	str	r3, [sp]
.L438:
	ldr	r3, [sp, #16]
	add	r8, r8, #8
	cmp	r3, r8
	bls	.L436
.L440:
	ldr	r3, [sp, #4]
	ldr	r2, [sp, #8]
	cmp	r3, r2
	bcs	.L439
	ldrb	r2, [r3], #1	@ zero_extendqisi2
	ldr	r1, [sp]
	orr	r2, r1, r2, lsl r8
	add	r8, r8, #8
	strd	r2, [sp]
	ldr	r3, [sp, #16]
	cmp	r3, r8
	bhi	.L440
	b	.L436
.L280:
	ldr	r3, [sp, #8]
	cmp	r5, r3
	bcs	.L501
	mov	r2, r5
	str	r4, [sp, #32]
	ldrb	r3, [r2], #1	@ zero_extendqisi2
	str	r2, [sp, #4]
	ldr	r2, [sp]
	orr	r3, r2, r3, lsl r8
	str	r3, [sp]
.L298:
	add	r8, r8, #8
.L642:
	cmp	r8, #2
	bls	.L284
	ldr	r2, [sp]
	sub	r8, r8, #3
	and	r3, r2, #7
	str	r3, [r9, #20]
	lsr	r3, r3, #1
	lsr	r2, r2, #3
	cmp	r3, #0
	str	r2, [sp]
	str	r3, [r9, #24]
	beq	.L652
	cmp	r3, #3
	beq	.L653
	cmp	r3, #1
	movne	r3, #5
	movne	r2, r3
	movne	r10, #0
	bne	.L644
	add	r3, r9, #44
	vldr	d16, .L677
	mov	r2, #32
	vst1.32	{d16}, [r3]
	mov	r1, #5
	add	r0, r9, #3552
	bl	memset
	mov	r2, #144
	mov	r1, #8
	add	r0, r9, #64
	bl	memset
	mov	r2, #112
	mov	r1, #9
	add	r0, r9, #208
	bl	memset
	mov	r2, #24
	mov	r1, #7
	add	r0, r9, #320
	bl	memset
	ldr	r3, .L677+8
	str	r3, [r9, #344]	@ unaligned
	str	r3, [r9, #348]	@ unaligned
	ldr	r3, [r9, #24]
	str	r3, [sp, #36]
.L331:
	cmp	r3, #0
	bge	.L654
.L388:
	ldrd	r2, [sp, #4]
	sub	r3, r3, r2
	cmp	r3, #3
	ble	.L406
	ldr	r3, [sp, #24]
	ldr	r2, [sp, #32]
	sub	r3, r3, r2
	cmp	r3, #1
	ble	.L389
	str	r6, [sp, #36]
	str	r4, [sp, #312]
	ldr	r0, .L677+12
	ldr	ip, [sp]
	ldr	r4, [sp, #4]
	mov	r6, r2
	b	.L390
.L410:
	tst	r3, #256
	lsr	ip, ip, r1
	sub	r8, r8, r1
	mov	r10, r3
	bne	.L518
	cmp	r8, #14
	ldrhls	r2, [r4], #2	@ unaligned
	orrls	ip, ip, r2, lsl r8
	and	r2, ip, r0
	add	r2, r2, #176
	addls	r8, r8, #16
	lsl	r2, r2, #1
	ldrsh	r2, [r9, r2]
	cmp	r2, #0
	asrge	lr, r2, #9
	blt	.L655
.L413:
	tst	r2, #256
	lsr	ip, ip, lr
	sub	r8, r8, lr
	strb	r3, [r6]
	bne	.L656
	ldr	r3, [sp, #8]
	strb	r2, [r6, #1]
	sub	r3, r3, r4
	cmp	r3, #3
	add	r6, r6, #2
	ble	.L657
	ldr	r3, [sp, #24]
	sub	r3, r3, r6
	cmp	r3, #1
	ble	.L658
.L390:
	cmp	r8, #14
	ldrhls	r3, [r4], #2	@ unaligned
	orrls	ip, ip, r3, lsl r8
	and	r3, ip, r0
	add	r3, r3, #176
	addls	r8, r8, #16
	lsl	r3, r3, #1
	ldrsh	r3, [r9, r3]
	cmp	r3, #0
	asrge	r1, r3, #9
	bge	.L410
	mov	r1, #10
	mov	r2, r3
.L409:
	lsr	r3, ip, r1
	and	r3, r3, #1
	sub	r3, r3, r2
	add	r3, r3, #1184
	add	r3, r3, #15
	add	r1, r1, #1
	lsl	r3, r3, #1
	ldrsh	r2, [r9, r3]
	cmp	r2, #0
	blt	.L409
	mov	r3, r2
	b	.L410
.L652:
	and	r3, r8, #7
	cmp	r3, r8
	bls	.L659
.L305:
	ldr	r3, [sp, #4]
	ldr	r2, [sp, #8]
	cmp	r3, r2
	bcs	.L304
	ldrb	r2, [r3], #1	@ zero_extendqisi2
	ldr	r1, [sp]
	orr	r2, r1, r2, lsl r8
	strd	r2, [sp]
.L303:
	add	r8, r8, #8
	and	r3, r8, #7
	cmp	r3, r8
	bhi	.L305
.L659:
	ldr	r2, [sp]
	bic	r8, r8, #7
	mov	r10, #0
	lsr	r3, r2, r3
	str	r3, [sp]
.L316:
	cmp	r8, #0
	beq	.L306
	cmp	r8, #7
	subhi	r8, r8, #8
	bhi	.L308
	mov	r3, r8
	b	.L307
.L282:
	ldr	r2, [sp, #8]
	cmp	r5, r2
	bcs	.L285
	mov	r2, r5
	ldrb	r1, [r2], #1	@ zero_extendqisi2
	str	r2, [sp, #4]
	str	r1, [r9, #8]
.L287:
	ldr	r0, [sp, #8]
	cmp	r2, r0
	bcs	.L288
	ldrb	ip, [r2], #1	@ zero_extendqisi2
	lsl	r0, r1, #8
	str	r2, [sp, #4]
	str	ip, [r9, #12]
.L289:
	ldr	r2, .L677+16
	add	r0, r0, ip
	and	ip, ip, #32
	umull	lr, r2, r2, r0
	sub	lr, r0, r2
	add	r2, r2, lr, lsr #1
	lsr	r2, r2, #4
	rsb	r2, r2, r2, lsl #5
	sub	r2, r0, r2
	orrs	r2, r2, ip
	beq	.L292
.L291:
	ldr	r2, [sp, #28]
	cmp	r2, #0
	moveq	r10, #1
	bne	.L640
.L477:
	lsr	r2, r1, #4
	add	r2, r2, #8
	mov	r1, #1
	lsl	r1, r1, r2
	cmp	r1, #32768
	bls	.L478
.L640:
	ldr	r3, [sp, #4]
	mov	r10, #1
	sub	r5, r3, r5
.L260:
	mov	r3, #36
	mov	r6, #0
	mvn	r0, #0
	str	r3, [r9]
	b	.L249
.L283:
	ldr	r0, [sp, #320]
	mov	r1, #1
	ands	r8, r0, r1
	vmov.i32	d16, #0  @ v2si
	add	r0, r9, #8
	vst1.32	{d16}, [r0]
	str	r1, [r9, #28]
	str	r1, [r9, #16]
	beq	.L497
	ldr	r1, [sp, #8]
	cmp	r5, r1
	bcc	.L660
	mov	r8, r2
	mov	r10, r2
	str	r2, [sp]
	str	r2, [sp, #16]
	str	r2, [sp, #12]
.L285:
	ldr	r2, [sp, #320]
	ands	r2, r2, #2
	beq	.L661
	mov	r0, #1
	mov	r6, #0
	mov	r5, r6
	str	r0, [r9]
	b	.L249
.L266:
	ldr	r3, [sp, #8]
	cmp	r5, r3
	bcs	.L522
	mov	r2, r5
	str	r4, [sp, #32]
	ldrb	r3, [r2], #1	@ zero_extendqisi2
	str	r2, [sp, #4]
	ldr	r2, [sp]
	orr	r3, r2, r3, lsl r8
	str	r3, [sp]
.L419:
	ldr	r3, [sp, #16]
	add	r8, r8, #8
	cmp	r3, r8
	bls	.L662
.L421:
	ldr	r3, [sp, #4]
	ldr	r2, [sp, #8]
	cmp	r3, r2
	bcs	.L420
	ldr	r3, [sp, #4]
	ldr	r1, [sp]
	ldrb	r2, [r3], #1	@ zero_extendqisi2
	orr	r2, r1, r2, lsl r8
	strd	r2, [sp]
	b	.L419
.L278:
	ldr	r3, [sp, #8]
	cmp	r5, r3
	bcs	.L504
	mov	r2, r5
	str	r4, [sp, #32]
	ldrb	r3, [r2], #1	@ zero_extendqisi2
	str	r2, [sp, #4]
	ldr	r2, [sp]
	orr	r3, r2, r3, lsl r8
	str	r3, [sp]
.L310:
	add	r3, r8, #8
	cmp	r3, #7
	bhi	.L308
.L307:
	ldr	r2, [sp, #4]
	ldr	r1, [sp, #8]
	mov	r8, r3
	cmp	r2, r1
	bcs	.L311
	ldrb	r1, [r2], #1	@ zero_extendqisi2
	ldr	r0, [sp]
	str	r2, [sp, #4]
	orr	r3, r0, r1, lsl r3
	str	r3, [sp]
	add	r3, r8, #8
	cmp	r3, #7
	bls	.L307
.L308:
	ldr	r2, [sp]
	add	r3, r9, r10
	add	r3, r3, #10496
	strb	r2, [r3, #32]
	lsr	r3, r2, #8
	str	r3, [sp]
	b	.L312
.L678:
	.align	3
.L677:
	.word	288
	.word	32
	.word	134744072
	.word	1023
	.word	138547333
	.word	138547332
	.word	.LC0
	.word	.LC1
	.word	.LANCHOR0
.L277:
	ldr	r3, [sp, #8]
	cmp	r5, r3
	bcs	.L505
	mov	r1, r5
	add	r3, r9, r10
	ldrb	r2, [r1], #1	@ zero_extendqisi2
	add	r3, r3, #10496
	str	r4, [sp, #32]
	str	r1, [sp, #4]
	strb	r2, [r3, #32]
.L312:
	add	r10, r10, #1
	cmp	r10, #3
	bls	.L316
	add	r2, r9, #10496
	ldrh	r3, [r2, #34]
	ldrh	r10, [r2, #32]
	mvn	r3, r3
	uxth	r3, r3
	cmp	r10, r3
	beq	.L317
	ldr	r3, [sp, #4]
	sub	r5, r3, r5
	ldr	r3, [sp, #32]
	sub	r6, r3, r4
.L257:
	mov	r3, #39
	mvn	r0, #0
	str	r3, [r9]
	b	.L249
.L663:
	mov	r3, r0
	ldr	r2, [sp, #12]
	sub	r10, r10, #1
	strb	r2, [r3], #1
	str	r3, [sp, #32]
.L317:
	cmp	r10, #0
	cmpne	r8, #0
	beq	.L325
	cmp	r8, #7
	bls	.L506
	ldr	r0, [sp, #32]
	sub	r8, r8, #8
.L319:
	ldr	r3, [sp]
	uxtb	r2, r3
	lsr	r3, r3, #8
	str	r2, [sp, #12]
	str	r3, [sp]
.L252:
	ldr	r3, [sp, #24]
	cmp	r0, r3
	bcc	.L663
	ldr	r3, [sp, #4]
	sub	r6, r0, r4
	sub	r5, r3, r5
	mov	r3, #52
	mov	r0, #2
	str	r3, [r9]
	b	.L249
.L281:
	ldr	r2, [sp, #8]
	cmp	r5, r2
	bcs	.L499
	mov	r2, r5
	ldr	r1, [r9, #8]
	ldrb	ip, [r2], #1	@ zero_extendqisi2
	lsl	r0, r1, #8
	str	r2, [sp, #4]
	str	ip, [r9, #12]
	b	.L289
.L279:
	ldr	r3, [sp, #8]
	cmp	r5, r3
	bcs	.L502
	mov	r2, r5
	str	r4, [sp, #32]
	ldrb	r3, [r2], #1	@ zero_extendqisi2
	str	r2, [sp, #4]
	ldr	r2, [sp]
	orr	r3, r2, r3, lsl r8
	str	r3, [sp]
	b	.L303
.L489:
	mov	r6, #0
	mov	r5, r6
.L261:
	mov	r3, #35
	mvn	r0, #0
	str	r3, [r9]
	b	.L249
.L494:
	mov	r6, #0
	mov	r5, r6
.L256:
	mov	r3, #40
	mvn	r0, #0
	str	r3, [r9]
	b	.L249
.L493:
	mov	r6, #0
	mov	r5, r6
	b	.L257
.L492:
	str	r4, [sp, #32]
	str	r5, [sp, #4]
.L258:
	ldr	r3, [sp, #4]
	ldr	r2, [sp, #8]
	cmp	r3, r2
	bcc	.L664
	sub	r5, r3, r5
	ldr	r3, [sp, #320]
	tst	r3, #2
	ldr	r3, [sp, #32]
	sub	r6, r3, r4
	beq	.L256
	mov	r3, #38
	mov	r0, #1
	str	r3, [r9]
	b	.L249
.L491:
	mov	r6, #0
	mov	r5, r6
.L259:
	mov	r3, #37
	mvn	r0, #0
	str	r3, [r9]
	b	.L249
.L486:
	mov	r6, #0
	mov	r5, r6
.L269:
	mov	r3, #21
	mvn	r0, #0
	str	r3, [r9]
	b	.L249
.L484:
	mov	r6, #0
	mov	r5, r6
.L275:
	mov	r3, #10
	mvn	r0, #0
	str	r3, [r9]
	b	.L249
.L496:
	ldr	r2, [sp, #24]
	mov	r3, r4
	cmp	r3, r2
	str	r5, [sp, #4]
	bcs	.L444
.L665:
	mov	r1, r3
	ldr	r2, [sp, #20]
	ldr	r3, [sp, #12]
	sub	r3, r2, r3
	and	r3, r3, r6
	add	r2, r2, #1
	ldrb	r3, [fp, r3]	@ zero_extendqisi2
	str	r2, [sp, #36]
	strb	r3, [r1], #1
	str	r1, [sp, #32]
.L442:
	cmp	r10, #0
	beq	.L525
	ldr	r3, [sp, #36]
	ldr	r2, [sp, #24]
	str	r3, [sp, #20]
	ldr	r3, [sp, #32]
	sub	r10, r10, #1
	cmp	r3, r2
	bcc	.L665
.L444:
	ldr	r2, [sp, #4]
	sub	r6, r3, r4
	mov	r3, #53
	sub	r5, r2, r5
	mov	r0, #2
	str	r3, [r9]
	b	.L249
.L487:
	mov	r3, r4
	str	r4, [sp, #32]
	str	r5, [sp, #4]
.L267:
	ldr	r2, [sp, #24]
	cmp	r3, r2
	bcs	.L405
	strb	r10, [r3], #1
	str	r3, [sp, #32]
	b	.L388
.L483:
	str	r4, [sp, #32]
	str	r5, [sp, #4]
.L276:
	ldr	r3, [sp, #32]
	ldr	r2, [sp, #24]
	cmp	r3, r2
	bcc	.L258
	ldr	r3, [sp, #4]
	mov	r0, #2
	sub	r5, r3, r5
	ldr	r3, [sp, #32]
	sub	r6, r3, r4
	mov	r3, #9
	str	r3, [r9]
	b	.L249
.L495:
	mov	r0, r4
	str	r5, [sp, #4]
	b	.L252
.L488:
	mov	r6, #0
	mov	r5, r6
.L262:
	mov	r3, #34
	mov	r0, #0
	str	r3, [r9]
	b	.L249
.L490:
	mov	r5, #0
	b	.L260
.L485:
	mov	r6, #0
	mov	r5, r6
.L271:
	mov	r3, #17
	mvn	r0, #0
	str	r3, [r9]
	b	.L249
.L664:
	sub	r2, r2, r3
	ldr	r0, [sp, #32]
	mov	r1, r3
	ldr	r3, [sp, #24]
	sub	r3, r3, r0
	cmp	r2, r3
	movcs	r2, r3
	cmp	r2, r10
	movcs	r2, r10
	add	r3, r1, r2
	str	r3, [sp, #4]
	add	r3, r0, r2
	sub	r10, r10, r2
	str	r3, [sp, #32]
	bl	memcpy
.L325:
	cmp	r10, #0
	bne	.L276
.L328:
	ldr	r3, [r9, #20]
	tst	r3, #1
	beq	.L642
	ldr	r3, [sp, #320]
	tst	r3, #1
	ldr	r3, [sp, #32]
	sub	r6, r3, r4
	bne	.L645
.L646:
	ldr	r3, [sp, #4]
	sub	r5, r3, r5
	b	.L262
.L246:
	ldr	r2, [sp, #316]
	mov	r3, #0
	mvn	r0, #2
	str	r3, [r2]
	str	r3, [r7]
	b	.L244
.L499:
	str	r5, [sp, #4]
.L288:
	ldr	r2, [sp, #320]
	ands	r2, r2, #2
	bne	.L290
	ldr	r1, [r9, #8]
	str	r2, [r9, #12]
	ldr	r0, .L677+20
	lsl	r2, r1, #13
	add	r2, r2, r1, lsl #8
	lsl	r2, r2, #5
	add	r2, r2, r1, lsl #8
	add	r2, r2, r2, lsl #15
	lsl	r2, r2, #5
	add	r2, r2, r1, lsl #8
	rsb	r2, r2, #0
	cmp	r2, r0
	bhi	.L291
.L292:
	ldr	r2, [sp, #28]
	and	r10, r1, #15
	subs	r10, r10, #8
	movne	r10, #1
	cmp	r2, #0
	beq	.L477
.L293:
	cmp	r10, #0
	streq	r4, [sp, #32]
	beq	.L642
	b	.L640
.L497:
	mov	r10, r8
	str	r4, [sp, #32]
	str	r5, [sp, #4]
	str	r8, [sp]
	str	r8, [sp, #16]
	str	r8, [sp, #12]
.L284:
	ldr	r3, [sp, #4]
	ldr	r2, [sp, #8]
	cmp	r3, r2
	bcs	.L299
	ldrb	r2, [r3], #1	@ zero_extendqisi2
	ldr	r1, [sp]
	orr	r2, r1, r2, lsl r8
	strd	r2, [sp]
	b	.L298
.L501:
	str	r4, [sp, #32]
	str	r5, [sp, #4]
.L299:
	ldr	r3, [sp, #320]
	tst	r3, #2
	beq	.L298
	ldr	r3, [sp, #4]
	mov	r0, #1
	sub	r5, r3, r5
	ldr	r3, [sp, #32]
	sub	r6, r3, r4
	mov	r3, #3
	str	r3, [r9]
	b	.L249
.L502:
	str	r4, [sp, #32]
	str	r5, [sp, #4]
.L304:
	ldr	r3, [sp, #320]
	tst	r3, #2
	beq	.L303
	ldr	r3, [sp, #4]
	mov	r0, #1
	sub	r5, r3, r5
	ldr	r3, [sp, #32]
	sub	r6, r3, r4
	mov	r3, #5
	str	r3, [r9]
	b	.L249
.L650:
	ldr	ip, [sp]
	sub	r8, r8, r3
	mvn	r3, #0
	ldr	r0, .L677+32
	bic	r1, ip, r3, lsl r2
	lsr	r3, ip, r2
	str	r3, [sp]
	ldr	r3, [r0, r10, lsl #2]
	add	r2, r9, r10, lsl #2
	add	r10, r10, #1
	add	r3, r3, r1
	cmp	r10, #2
	str	r0, [sp, #36]
	str	r3, [r2, #44]
	bhi	.L338
	ldr	r3, .L677+24
	ldrb	r2, [r3, r10]	@ zero_extendqisi2
	mov	r3, r2
	b	.L644
.L524:
	str	r4, [sp, #32]
	str	r5, [sp, #4]
.L439:
	ldr	r3, [sp, #320]
	tst	r3, #2
	beq	.L438
	ldr	r3, [sp, #4]
	mov	r0, #1
	sub	r5, r3, r5
	ldr	r3, [sp, #32]
	sub	r6, r3, r4
	mov	r3, #27
	str	r3, [r9]
	b	.L249
.L509:
	str	r4, [sp, #32]
	str	r5, [sp, #4]
.L336:
	ldr	r3, [sp, #320]
	tst	r3, #2
	bne	.L334
	ldr	r3, .L677+24
	ldrb	r2, [r3, r10]	@ zero_extendqisi2
	mov	r3, r2
	b	.L335
.L504:
	str	r4, [sp, #32]
	str	r5, [sp, #4]
.L311:
	ldr	r3, [sp, #320]
	tst	r3, #2
	beq	.L310
	ldr	r3, [sp, #4]
	mov	r0, #1
	sub	r5, r3, r5
	ldr	r3, [sp, #32]
	sub	r6, r3, r4
	mov	r3, #6
	str	r3, [r9]
	b	.L249
.L513:
	str	r4, [sp, #32]
	str	r5, [sp, #4]
.L382:
	ldr	r3, [sp, #320]
	tst	r3, #2
	beq	.L381
	ldr	r3, [sp, #4]
	mov	r0, #1
	sub	r5, r3, r5
	ldr	r3, [sp, #32]
	sub	r6, r3, r4
	mov	r3, #18
	str	r3, [r9]
	b	.L249
.L533:
	mov	r6, #0
	str	r5, [sp, #4]
.L458:
	ldr	r3, [sp, #320]
	tst	r3, #2
	beq	.L457
	ldr	r3, [sp, #4]
	mov	r0, #1
	sub	r5, r3, r5
	mov	r3, #32
	str	r3, [r9]
	b	.L249
.L535:
	mov	r6, #0
	str	r5, [sp, #4]
.L465:
	ldr	r3, [sp, #320]
	tst	r3, #2
	beq	.L464
	ldr	r3, [sp, #4]
	mov	r0, #1
	sub	r5, r3, r5
	mov	r3, #41
	str	r3, [r9]
	b	.L249
.L523:
	str	r4, [sp, #32]
	str	r5, [sp, #4]
.L431:
	ldr	r3, [sp, #320]
	tst	r3, #2
	beq	.L430
	ldr	r3, [sp, #4]
	mov	r0, #1
	sub	r5, r3, r5
	ldr	r3, [sp, #32]
	sub	r6, r3, r4
	mov	r3, #26
	str	r3, [r9]
	b	.L249
.L522:
	str	r4, [sp, #32]
	str	r5, [sp, #4]
.L420:
	ldr	r3, [sp, #320]
	tst	r3, #2
	beq	.L419
	ldr	r3, [sp, #4]
	mov	r0, #1
	sub	r5, r3, r5
	ldr	r3, [sp, #32]
	sub	r6, r3, r4
	mov	r3, #25
	str	r3, [r9]
	b	.L249
.L507:
	mov	r0, r4
	str	r5, [sp, #4]
.L322:
	ldr	r3, [sp, #320]
	tst	r3, #2
	beq	.L321
	ldr	r3, [sp, #4]
	sub	r6, r0, r4
	sub	r5, r3, r5
	mov	r3, #51
	mov	r0, #1
	str	r3, [r9]
	b	.L249
.L510:
	str	r4, [sp, #32]
	str	r5, [sp, #4]
.L343:
	ldr	r3, [sp, #320]
	tst	r3, #2
	beq	.L342
	ldr	r3, [sp, #4]
	mov	r0, #1
	sub	r5, r3, r5
	ldr	r3, [sp, #32]
	sub	r6, r3, r4
	mov	r3, #14
	str	r3, [r9]
	b	.L249
.L512:
	str	r4, [sp, #32]
	str	r5, [sp, #4]
.L373:
	ldr	r3, [sp, #320]
	tst	r3, #2
	beq	.L372
	ldr	r3, [sp, #4]
	mov	r0, #1
	sub	r5, r3, r5
	ldr	r3, [sp, #32]
	sub	r6, r3, r4
	mov	r3, #16
	str	r3, [r9]
	b	.L249
.L536:
	mov	r6, #0
	str	r5, [sp, #4]
.L468:
	ldr	r3, [sp, #320]
	ands	r3, r3, #2
	beq	.L466
	ldr	r3, [sp, #4]
	mov	r0, #1
	sub	r5, r3, r5
	mov	r3, #42
	str	r3, [r9]
	b	.L249
.L661:
	ldr	r1, [sp, #28]
	str	r2, [r9, #8]
	cmp	r1, #0
	str	r2, [r9, #12]
	str	r5, [sp, #4]
	bne	.L640
	mov	r10, #1
	mov	r2, #8
.L478:
	lsrs	r3, r3, r2
	orreq	r10, r10, #1
	b	.L293
.L505:
	str	r4, [sp, #32]
	str	r5, [sp, #4]
.L315:
	ldr	r3, [sp, #320]
	ands	r2, r3, #2
	addeq	r3, r9, r10
	addeq	r3, r3, #10496
	strbeq	r2, [r3, #32]
	beq	.L312
	ldr	r3, [sp, #4]
	mov	r0, #1
	sub	r5, r3, r5
	ldr	r3, [sp, #32]
	sub	r6, r3, r4
	mov	r3, #7
	str	r3, [r9]
	b	.L249
.L516:
	str	r4, [sp, #32]
	str	r5, [sp, #4]
.L400:
	ldr	r3, [sp, #320]
	tst	r3, #2
	beq	.L399
	ldr	r3, [sp, #4]
	mov	r0, #1
	sub	r5, r3, r5
	ldr	r3, [sp, #32]
	sub	r6, r3, r4
	mov	r3, #23
	str	r3, [r9]
	b	.L249
.L658:
	str	r4, [sp, #4]
	str	r6, [sp, #32]
	ldr	r4, [sp, #312]
	ldr	r6, [sp, #36]
	str	ip, [sp]
.L389:
	cmp	r8, #14
	bhi	.L391
.L392:
	ldr	r3, [sp, #4]
	mov	r2, r3
	ldrb	r1, [r3, #1]	@ zero_extendqisi2
	ldrb	r3, [r2], #2	@ zero_extendqisi2
	str	r2, [sp, #4]
	lsl	r3, r3, r8
	add	r2, r8, #8
	add	r8, r8, #16
	orr	r3, r3, r1, lsl r2
	ldr	r2, [sp]
	orr	r3, r2, r3
	str	r3, [sp]
.L391:
	ldr	r3, [sp]
	lsl	r3, r3, #22
	lsr	r3, r3, #22
	add	r3, r3, #176
	lsl	r3, r3, #1
	ldrsh	r1, [r9, r3]
	cmp	r1, #0
	mov	r3, r1
	blt	.L396
	asr	r2, r1, #9
.L394:
	lsl	r3, r1, #23
	lsr	r3, r3, #23
.L402:
	mov	r10, r3
	cmp	r3, #255
	ldr	r3, [sp]
	sub	r8, r8, r2
	lsr	r3, r3, r2
	str	r3, [sp]
	bgt	.L404
	ldr	r3, [sp, #32]
	b	.L267
.L385:
	cmp	r8, #14
	bhi	.L364
	ldrd	r2, [sp, #4]
	sub	r3, r3, r2
	cmp	r3, #1
	ble	.L374
	ldr	r3, [sp, #4]
	mov	r2, r3
	ldrb	r1, [r3, #1]	@ zero_extendqisi2
	ldrb	r3, [r2], #2	@ zero_extendqisi2
	str	r2, [sp, #4]
	lsl	r3, r3, r8
	add	r2, r8, #8
	add	r8, r8, #16
	orr	r3, r3, r1, lsl r2
	ldr	r2, [sp]
	orr	r3, r2, r3
	str	r3, [sp]
.L364:
	ldr	r3, [sp]
	lsl	r3, r3, #22
	lsr	r3, r3, #22
	add	r3, r3, #3664
	lsl	r3, r3, #1
	ldrsh	r1, [r9, r3]
	cmp	r1, #0
	mov	r3, r1
	blt	.L369
	asr	r2, r1, #9
.L367:
	lsl	r3, r1, #23
	lsr	r3, r3, #23
.L375:
	cmp	r3, #15
	str	r3, [sp, #12]
	ldr	r3, [sp]
	sub	r8, r8, r2
	lsr	r3, r3, r2
	str	r3, [sp]
	ble	.L666
	ldr	r3, [sp, #12]
	cmp	r3, #16
	cmpeq	r10, #0
	beq	.L667
	ldr	r2, .L677+28
	sub	r3, r3, #16
	ldrb	r2, [r2, r3]	@ zero_extendqisi2
	cmp	r8, r2
	str	r2, [sp, #16]
	bcs	.L379
	b	.L383
.L649:
	ldr	r2, [r9, #52]
	ldr	r3, .L677+32
	str	r3, [sp, #36]
.L340:
	ldr	r3, [sp, #36]
	ldr	r0, [sp]
	add	r3, r3, r10
	and	r1, r0, #7
	ldrb	r3, [r3, #16]	@ zero_extendqisi2
	sub	r8, r8, #3
	add	r10, r10, #1
	add	r3, r9, r3
	add	r3, r3, #7040
	strb	r1, [r3]
	lsr	r3, r0, #3
	str	r3, [sp]
.L339:
	cmp	r10, r2
	bcc	.L345
	mov	r3, #19
	str	r3, [r9, #52]
	ldr	r3, [r9, #24]
	str	r3, [sp, #36]
	b	.L331
.L647:
	ldr	r2, [sp]
	bic	r8, r8, #7
	mov	r10, #0
	lsr	r3, r2, r3
	str	r3, [sp]
	b	.L469
.L662:
	mov	r2, r3
.L417:
	ldr	r1, [sp]
	mvn	r3, #0
	sub	r8, r8, r2
	bic	r3, r1, r3, lsl r2
	add	r10, r10, r3
	lsr	r3, r1, r2
	str	r3, [sp]
.L416:
	cmp	r8, #14
	bhi	.L422
	ldrd	r2, [sp, #4]
	sub	r3, r3, r2
	cmp	r3, #1
	ble	.L432
	ldrb	r1, [r2, #1]	@ zero_extendqisi2
	ldrb	r3, [r2], #2	@ zero_extendqisi2
	str	r2, [sp, #4]
	lsl	r3, r3, r8
	add	r2, r8, #8
	add	r8, r8, #16
	orr	r3, r3, r1, lsl r2
	ldr	r2, [sp]
	orr	r3, r2, r3
	str	r3, [sp]
	b	.L422
.L405:
	ldr	r3, [sp, #4]
	mov	r0, #2
	sub	r5, r3, r5
	ldr	r3, [sp, #32]
	sub	r6, r3, r4
	mov	r3, #24
	str	r3, [r9]
	b	.L249
.L290:
	ldr	r3, [sp, #4]
	mov	r6, #0
	sub	r5, r3, r5
	mov	r3, #2
	mov	r0, #1
	str	r3, [r9]
	b	.L249
.L424:
	cmp	r8, #10
	bls	.L426
	mov	r0, #10
	mov	ip, r1
	b	.L428
.L668:
	cmp	r8, r1
	bcc	.L426
.L428:
	ldr	r2, [sp]
	mov	r1, r0
	add	r1, r1, #2
	lsr	r2, r2, r0
	and	r2, r2, #1
	sub	r2, r2, ip
	add	r2, r2, #2928
	add	r2, r2, #15
	add	r0, r0, #1
	lsl	r2, r2, #1
	ldrsh	ip, [r9, r2]
	cmp	ip, #0
	blt	.L668
.L427:
	ldr	r1, [sp]
	mov	r2, #10
	mov	r0, r3
.L434:
	lsr	r3, r1, r2
	and	r3, r3, #1
	sub	r3, r3, r0
	add	r3, r3, #2928
	add	r3, r3, #15
	add	r2, r2, #1
	lsl	r3, r3, #1
	ldrsh	r0, [r9, r3]
	cmp	r0, #0
	blt	.L434
	mov	r3, r0
	b	.L433
.L518:
	mov	r3, r6
	str	r4, [sp, #4]
	ldr	r6, [sp, #36]
	ldr	r4, [sp, #312]
	str	ip, [sp]
	str	r3, [sp, #32]
.L404:
	lsl	r10, r10, #23
	lsr	r10, r10, #23
	cmp	r10, #256
	beq	.L328
	sub	r10, r10, #256
	sub	r10, r10, #1
	ldr	r3, .L677+32
	add	r3, r3, r10, lsl #2
	ldr	r2, [r3, #40]
	ldr	r10, [r3, #168]
	cmp	r2, #0
	str	r2, [sp, #16]
	beq	.L416
	cmp	r8, r2
	bcs	.L417
	b	.L421
.L306:
	ldrd	r2, [sp, #4]
	cmp	r3, r2
	bls	.L315
	mov	r3, r2
	add	r2, r9, r10
	ldrb	r1, [r3], #1	@ zero_extendqisi2
	add	r2, r2, #10496
	str	r3, [sp, #4]
	strb	r1, [r2, #32]
	b	.L312
.L660:
	mov	r0, r5
	mov	r10, r2
	ldrb	r1, [r0], #1	@ zero_extendqisi2
	mov	r8, r2
	str	r2, [sp]
	str	r2, [sp, #16]
	str	r2, [sp, #12]
	str	r0, [sp, #4]
	mov	r2, r0
	str	r1, [r9, #8]
	b	.L287
.L655:
	mov	lr, #10
	mov	r1, r2
.L412:
	lsr	r2, ip, lr
	and	r2, r2, #1
	sub	r2, r2, r1
	add	r2, r2, #1184
	add	r2, r2, #15
	add	lr, lr, #1
	lsl	r2, r2, #1
	ldrsh	r1, [r9, r2]
	cmp	r1, #0
	blt	.L412
	mov	r2, r1
	b	.L413
.L651:
	ldr	r3, [sp, #4]
	sub	r5, r3, r5
	ldr	r3, [sp, #32]
	sub	r6, r3, r4
	ldr	r3, [sp, #36]
	str	r3, [sp, #20]
	b	.L259
.L532:
	ldr	r3, [sp, #36]
	str	r3, [sp, #20]
	b	.L388
.L393:
	cmp	r8, #10
	bls	.L395
	mov	r0, #10
	mov	ip, r1
	b	.L397
.L669:
	cmp	r8, r1
	bcc	.L395
.L397:
	ldr	r2, [sp]
	mov	r1, r0
	add	r1, r1, #2
	lsr	r2, r2, r0
	and	r2, r2, #1
	sub	r2, r2, ip
	add	r2, r2, #1184
	add	r2, r2, #15
	add	r0, r0, #1
	lsl	r2, r2, #1
	ldrsh	ip, [r9, r2]
	cmp	ip, #0
	blt	.L669
.L396:
	ldr	r1, [sp]
	mov	r2, #10
	mov	r0, r3
.L403:
	lsr	r3, r1, r2
	and	r3, r3, #1
	sub	r3, r3, r0
	add	r3, r3, #1184
	add	r3, r3, #15
	add	r2, r2, #1
	lsl	r3, r3, #1
	ldrsh	r0, [r9, r3]
	cmp	r0, #0
	blt	.L403
	mov	r3, r0
	b	.L402
.L338:
	mov	r2, #288
	mov	r1, #0
	add	r0, r9, #7040
	bl	memset
	mov	r10, #0
	ldr	r2, [r9, #52]
	b	.L339
.L334:
	ldr	r3, [sp, #4]
	mov	r0, #1
	sub	r5, r3, r5
	ldr	r3, [sp, #32]
	sub	r6, r3, r4
	mov	r3, #11
	str	r3, [r9]
	b	.L249
.L653:
	ldr	r3, [sp, #4]
	sub	r5, r3, r5
	ldr	r3, [sp, #32]
	sub	r6, r3, r4
	b	.L275
.L366:
	cmp	r8, #10
	bls	.L368
	mov	r0, #10
	mov	ip, r1
	b	.L370
.L670:
	cmp	r8, r1
	bcc	.L368
.L370:
	ldr	r2, [sp]
	mov	r1, r0
	add	r1, r1, #2
	lsr	r2, r2, r0
	and	r2, r2, #1
	sub	r2, r2, ip
	add	r2, r2, #4672
	add	r2, r2, #15
	add	r0, r0, #1
	lsl	r2, r2, #1
	ldrsh	ip, [r9, r2]
	cmp	ip, #0
	blt	.L670
.L369:
	ldr	r1, [sp]
	mov	r2, #10
	mov	r0, r3
.L376:
	lsr	r3, r1, r2
	and	r3, r3, #1
	sub	r3, r3, r0
	add	r3, r3, #4672
	add	r3, r3, #15
	add	r2, r2, #1
	lsl	r3, r3, #1
	ldrsh	r0, [r9, r3]
	cmp	r0, #0
	blt	.L376
	mov	r3, r0
	b	.L375
.L525:
	ldr	r3, [sp, #36]
	mvn	r10, #0
	str	r3, [sp, #20]
	b	.L388
.L460:
	ldrd	r2, [sp, #4]
	cmp	r3, r2
	bls	.L468
	ldrb	r3, [r2], #1	@ zero_extendqisi2
	str	r2, [sp, #4]
	b	.L466
.L666:
	add	r3, r9, r10
	ldr	r2, [sp, #12]
	add	r3, r3, #10496
	add	r10, r10, #1
	strb	r2, [r3, #36]
	b	.L350
.L654:
	mov	r2, #64
	mov	r1, #0
	add	r0, sp, #88
	bl	memset
	ldr	r2, [sp, #36]
	mov	r3, #3488
	mov	r1, #0
	mul	r3, r3, r2
	mov	r2, #2048
	str	r3, [sp, #44]
	add	r3, r3, #352
	add	r0, r9, r3
	str	r0, [sp, #52]
	bl	memset
	ldr	r3, [sp, #44]
	mov	r2, #1152
	add	r0, r3, #2400
	add	r0, r9, r0
	mov	r1, #0
	bl	memset
	ldr	r2, [r9, #24]
	str	r2, [sp, #40]
	add	r3, r9, r2, lsl #2
	ldr	r2, [r3, #44]
	cmp	r2, #0
	str	r2, [sp, #48]
	beq	.L346
	ldr	r3, [sp, #44]
	add	r0, r9, #63
	add	r0, r0, r2
	add	r0, r0, r3
	add	r3, r3, #63
	add	r3, r9, r3
.L347:
	ldrb	r2, [r3, #1]!	@ zero_extendqisi2
	add	r1, sp, #224
	cmp	r3, r0
	add	r2, r1, r2, lsl #2
	ldr	r1, [r2, #-136]
	add	r1, r1, #1
	str	r1, [r2, #-136]
	bne	.L347
.L346:
	ldrd	r2, [sp, #92]
	ldr	r0, [sp, #100]
	vmov.i32	d16, #0  @ v2si
	lsl	r1, r2, #1
	add	r2, r3, r2
	add	r3, r3, r1
	add	r2, r0, r2
	lsl	r3, r3, #1
	add	r0, r0, r3
	str	r1, [sp, #160]
	ldr	r1, [sp, #104]
	str	r3, [sp, #164]
	lsl	r3, r0, #1
	str	r3, [sp, #168]
	add	r3, r1, r3
	add	r1, r1, r2
	lsl	r2, r3, #1
	ldr	r3, [sp, #108]
	str	r2, [sp, #172]
	add	r2, r3, r2
	add	r3, r3, r1
	ldr	r1, [sp, #112]
	lsl	r2, r2, #1
	str	r2, [sp, #176]
	add	r2, r1, r2
	add	r3, r1, r3
	ldr	r1, [sp, #116]
	lsl	r2, r2, #1
	str	r2, [sp, #180]
	add	r2, r1, r2
	add	r3, r1, r3
	ldr	r1, [sp, #120]
	lsl	r2, r2, #1
	str	r2, [sp, #184]
	add	r2, r1, r2
	add	r3, r1, r3
	ldr	r1, [sp, #124]
	lsl	r2, r2, #1
	str	r2, [sp, #188]
	add	r2, r1, r2
	add	r3, r1, r3
	ldr	r1, [sp, #128]
	lsl	r2, r2, #1
	str	r2, [sp, #192]
	add	r2, r1, r2
	add	r3, r1, r3
	ldr	r1, [sp, #132]
	lsl	r2, r2, #1
	str	r2, [sp, #196]
	add	r2, r1, r2
	add	r3, r1, r3
	ldr	r1, [sp, #136]
	lsl	r2, r2, #1
	str	r2, [sp, #200]
	add	r2, r1, r2
	add	r3, r1, r3
	ldr	r1, [sp, #140]
	lsl	r2, r2, #1
	str	r2, [sp, #204]
	add	r2, r1, r2
	add	r3, r1, r3
	ldr	r1, [sp, #144]
	lsl	r2, r2, #1
	str	r2, [sp, #208]
	add	r2, r1, r2
	add	r1, r1, r3
	ldr	r3, [sp, #148]
	lsl	r2, r2, #1
	str	r2, [sp, #212]
	add	r2, r3, r2
	add	r3, r3, r1
	lsl	r2, r2, #1
	cmp	r2, #65536
	cmpne	r3, #1
	movhi	r3, #1
	movls	r3, #0
	vstr	d16, [sp, #152]
	str	r2, [sp, #216]
	bhi	.L671
	ldr	r2, [sp, #48]
	cmp	r2, #0
	beq	.L363
	ldr	r2, [sp, #44]
	mov	r1, r3
	add	r2, r2, #63
	add	r2, r9, r2
	str	r2, [sp, #44]
	ldr	r2, [sp, #36]
	mov	r3, #1744
	str	r10, [sp, #60]
	mul	r2, r3, r2
	str	r4, [sp, #312]
	mvn	ip, #0
	mov	r10, r8
	mov	r4, r1
	str	r2, [sp, #36]
	b	.L362
.L353:
	ldr	r3, [sp, #48]
	add	r4, r4, #1
	cmp	r3, r4
	beq	.L672
.L362:
	ldr	r3, [sp, #44]
	ldrb	r2, [r3, #1]!	@ zero_extendqisi2
	cmp	r2, #0
	mov	lr, r2
	str	r3, [sp, #44]
	beq	.L353
	add	r3, sp, #224
	add	r8, r3, r2, lsl #2
	mov	r3, #0
	ldr	r1, [r8, #-72]
	str	r2, [sp, #56]
	add	r0, r1, #1
	str	r0, [r8, #-72]
	mov	r0, r2
.L354:
	and	r8, r1, #1
	subs	r0, r0, #1
	orr	r3, r8, r3, lsl #1
	lsr	r1, r1, #1
	bne	.L354
	cmp	r2, #10
	bls	.L673
	lsl	r0, r3, #22
	ldr	r1, [sp, #36]
	lsr	r0, r0, #22
	add	r0, r0, r1
	lsr	r3, r3, #9
	add	r0, r9, r0, lsl #1
	add	r0, r0, #352
	ldrsh	r1, [r0]
	cmp	r1, #0
	moveq	r1, ip
	strheq	ip, [r0]	@ movhi
	subeq	ip, ip, #2
	cmp	r2, #11
	beq	.L358
	ldr	r0, [sp, #36]
.L361:
	lsr	r3, r3, #1
	and	r2, r3, #1
	sub	r2, r1, r2
	mvn	r2, r2
	add	r2, r2, r0
	sub	lr, lr, #1
	add	r2, r9, r2, lsl #1
	add	r2, r2, #2400
	mov	r1, ip
	ldrsh	r8, [r2]
	cmp	r8, #0
	strheq	ip, [r2]	@ movhi
	movne	r1, r8
	subeq	ip, ip, #2
	cmp	lr, #11
	bne	.L361
.L358:
	lsr	r3, r3, #1
	ldr	r2, [sp, #36]
	and	r3, r3, #1
	sub	r3, r1, r3
	sub	r3, r2, r3
	add	r3, r3, #1184
	add	r3, r3, #15
	lsl	r3, r3, #1
	strh	r4, [r9, r3]	@ movhi
	b	.L353
.L673:
	orr	r1, r4, r2, lsl #9
	cmp	r3, #1024
	sxth	r1, r1
	bcs	.L353
	mov	r0, #1
	lsl	r2, r0, r2
.L356:
	ldr	lr, [sp, #52]
	lsl	r0, r3, #1
	add	r3, r3, r2
	cmp	r3, #1024
	strh	r1, [lr, r0]	@ movhi
	bcc	.L356
	b	.L353
.L667:
	ldr	r3, [sp, #4]
	mov	r10, #0
	sub	r5, r3, r5
	ldr	r3, [sp, #32]
	sub	r6, r3, r4
	mov	r3, #16
	str	r3, [sp, #12]
	b	.L271
.L445:
	mov	r3, r1
	add	r0, ip, #3
.L451:
	ldrb	r2, [r0, #-3]	@ zero_extendqisi2
	mov	ip, r0
	mov	lr, r3
	strb	r2, [r3]
	ldrb	r2, [r0, #-2]	@ zero_extendqisi2
	add	r3, r3, #3
	add	r0, r0, #3
	strb	r2, [r3, #-2]
	ldrb	r2, [r0, #-4]	@ zero_extendqisi2
	strb	r2, [r3, #-1]
	mov	r2, r10
	sub	r10, r10, #3
	cmp	r10, #2
	mov	r1, r10
	bgt	.L451
	str	ip, [sp, #20]
	str	r3, [sp, #32]
	b	.L452
.L345:
	cmp	r8, #2
	bhi	.L340
	b	.L344
.L657:
	str	r4, [sp, #4]
	str	r6, [sp, #32]
	ldr	r4, [sp, #312]
	ldr	r6, [sp, #36]
	str	ip, [sp]
.L406:
	cmp	r8, #14
	bhi	.L391
	cmp	r3, #1
	ble	.L401
	b	.L392
.L656:
	mov	r3, r6
	add	r3, r3, #1
	str	r4, [sp, #4]
	ldr	r6, [sp, #36]
	ldr	r4, [sp, #312]
	mov	r10, r2
	str	ip, [sp]
	str	r3, [sp, #32]
	b	.L404
.L672:
	mov	r8, r10
	ldr	r4, [sp, #312]
	ldr	r10, [sp, #60]
.L363:
	ldr	r3, [sp, #40]
	cmp	r3, #2
	beq	.L674
.L351:
	ldr	r3, [sp, #40]
	sub	r3, r3, #1
	str	r3, [sp, #36]
	str	r3, [r9, #24]
	b	.L331
.L671:
	ldr	r3, [sp, #4]
	sub	r5, r3, r5
	ldr	r3, [sp, #32]
	sub	r6, r3, r4
	b	.L261
.L506:
	ldr	r0, [sp, #32]
	mov	r3, r8
	b	.L318
.L526:
	str	r10, [sp, #40]
	mov	r3, r1
	mov	r10, r0
	b	.L446
.L528:
	mov	r10, r2
	str	lr, [sp, #32]
	ldr	r2, [sp, #40]
	mov	lr, r3
	b	.L452
.L527:
	ldr	r3, [sp, #56]
	mov	r10, r1
	str	r3, [sp, #32]
	b	.L452
.L530:
	mov	r10, r2
	str	lr, [sp, #32]
	ldr	r2, [sp, #40]
	mov	lr, r0
	b	.L452
.L529:
	ldr	lr, [sp, #32]
	mov	r2, r10
	str	r0, [sp, #32]
	mov	r10, r1
	b	.L452
.L674:
	mov	r10, #0
	b	.L350
.L531:
	mov	r3, lr
	mov	r2, r10
	ldr	lr, [sp, #32]
	mov	r10, r1
	str	r3, [sp, #32]
	b	.L452
.L386:
	add	r1, r9, #10496
	add	r1, r1, #36
	mov	r2, r3
	add	r0, r9, #64
	str	r1, [sp, #44]
	str	r3, [sp, #40]
	bl	memcpy
	ldrd	r2, [sp, #36]
	ldr	r1, [sp, #44]
	add	r0, r9, #3552
	add	r1, r1, r3
	bl	memcpy
	ldr	r3, [r9, #24]
	str	r3, [sp, #40]
	b	.L351
	.size	tinfl_decompress, .-tinfl_decompress
	.align	2
	.global	tinfl_decompress_mem_to_heap
	.syntax unified
	.arm
	.fpu neon
	.type	tinfl_decompress_mem_to_heap, %function
tinfl_decompress_mem_to_heap:
	@ args = 0, pretend = 0, frame = 11008
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, r9, r10, fp, lr}
	sub	sp, sp, #11008
	sub	sp, sp, #20
	push	{lr}
	bl	__gnu_mcount_nc
	mov	ip, #0
	bic	r3, r3, #6
	mov	r9, r1
	str	r1, [sp, #24]
	mov	r7, r2
	orr	r8, r3, #4
	mov	r5, ip
	mov	r1, r0
	mov	r6, ip
	mov	r10, ip
	mov	r4, ip
	add	fp, sp, #28
	str	r0, [sp, #20]
	str	ip, [r2]
	str	ip, [sp, #32]
	str	ip, [sp, #28]
	b	.L684
.L680:
	cmp	r2, #0
	ldr	r2, [sp, #24]
	ldr	r4, [r7]
	add	r6, r6, r2
	ldr	r2, [sp, #28]
	add	r4, r4, r2
	str	r4, [r7]
	beq	.L679
	cmp	r5, #128
	movcc	r5, #128
	mov	r1, r5
	bl	realloc
	sub	r1, r9, r6
	subs	r2, r0, #0
	sub	r0, r5, r4
	beq	.L689
	ldr	r3, [sp, #20]
	str	r1, [sp, #24]
	add	r4, r2, r4
	mov	r10, r2
	add	r1, r3, r6
	str	r0, [sp, #28]
.L684:
	mov	r3, r10
	add	r2, sp, #24
	add	r0, sp, #32
	str	r4, [sp]
	str	r8, [sp, #8]
	str	fp, [sp, #4]
	bl	tinfl_decompress
	lsl	r5, r5, #1
	mov	r2, r0
	sub	r1, r2, #1
	clz	r1, r1
	lsr	r1, r1, #5
	orrs	r3, r1, r2, lsr #31
	mov	r0, r10
	beq	.L680
	bl	free
	mov	r3, #0
	str	r3, [r7]
.L681:
	mov	r10, #0
.L679:
	mov	r0, r10
	add	sp, sp, #11008
	add	sp, sp, #20
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L689:
	mov	r0, r10
	str	r2, [sp, #20]
	bl	free
	ldr	r2, [sp, #20]
	str	r2, [r7]
	b	.L681
	.size	tinfl_decompress_mem_to_heap, .-tinfl_decompress_mem_to_heap
	.align	2
	.global	botoxInflate
	.syntax unified
	.arm
	.fpu neon
	.type	botoxInflate, %function
botoxInflate:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, lr}
	push	{lr}
	bl	__gnu_mcount_nc
	subs	r4, r0, #0
	beq	.L693
	ldr	r5, [r4, #8]
	cmp	r5, #0
	beq	.L695
.L693:
	mov	r0, #0
	pop	{r4, r5, r6, pc}
.L695:
	ldr	r6, [r4, #4]
	mov	r0, r6
	bl	malloc
	cmp	r0, #0
	str	r0, [r4, #8]
	beq	.L694
	mov	r2, r4
	mov	r3, #1
	ldr	r1, [r2], #4
	ldr	r0, [r4, #12]
	bl	tinfl_decompress_mem_to_heap
	ldr	r3, [r4, #4]
	cmp	r6, r3
	str	r0, [r4, #8]
	beq	.L693
	bl	free
	mov	r0, #8
	str	r5, [r4, #8]
	pop	{r4, r5, r6, pc}
.L694:
	mov	r0, #2
	pop	{r4, r5, r6, pc}
	.size	botoxInflate, .-botoxInflate
	.align	2
	.global	botoxDeflate
	.syntax unified
	.arm
	.fpu neon
	.type	botoxDeflate, %function
botoxDeflate:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, lr}
	push	{lr}
	bl	__gnu_mcount_nc
	cmp	r0, #0
	popeq	{r4, pc}
	ldr	r4, [r0]
	cmp	r4, #0
	popeq	{r4, pc}
	ldr	r0, [r4, #8]
	cmp	r0, #0
	popeq	{r4, pc}
	bl	free
	mov	r3, #0
	str	r3, [r4, #8]
	pop	{r4, pc}
	.size	botoxDeflate, .-botoxDeflate
	.align	2
	.global	frayNew
	.syntax unified
	.arm
	.fpu neon
	.type	frayNew, %function
frayNew:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, lr}
	push	{lr}
	bl	__gnu_mcount_nc
	cmp	r0, #0
	cmpne	r2, #0
	moveq	r4, #1
	movne	r4, #0
	cmp	r1, #0
	orreq	r4, r4, #1
	cmp	r4, #0
	mov	r5, r2
	mov	r7, r0
	mov	r6, r1
	beq	.L711
	mov	r0, #1
	pop	{r4, r5, r6, r7, r8, pc}
.L711:
	mul	r8, r2, r1
	add	r0, r8, #24
	bl	malloc
	subs	r3, r0, #0
	beq	.L710
	add	r0, r3, #20
	mov	r2, r8
	mov	r1, r4
	str	r6, [r3, #12]
	str	r4, [r3]
	str	r4, [r3, #4]
	str	r4, [r3, #8]
	str	r5, [r3, #16]
	str	r0, [r7]
	bl	memset
	mov	r0, r4
	pop	{r4, r5, r6, r7, r8, pc}
.L710:
	mov	r0, #2
	pop	{r4, r5, r6, r7, r8, pc}
	.size	frayNew, .-frayNew
	.align	2
	.global	frayDel
	.syntax unified
	.arm
	.fpu neon
	.type	frayDel, %function
frayDel:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, lr}
	push	{lr}
	bl	__gnu_mcount_nc
	subs	r4, r0, #0
	popeq	{r4, pc}
	ldr	r0, [r4]
	cmp	r0, #0
	popeq	{r4, pc}
	sub	r0, r0, #20
	bl	free
	mov	r3, #0
	str	r3, [r4]
	pop	{r4, pc}
	.size	frayDel, .-frayDel
	.align	2
	.global	frayClr
	.syntax unified
	.arm
	.fpu neon
	.type	frayClr, %function
frayClr:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, lr}
	push	{lr}
	bl	__gnu_mcount_nc
	subs	r3, r0, #0
	moveq	r2, r3
	ldmdbne	r3, {r1, r2}
	mov	r0, r3
	mulne	r2, r1, r2
	mov	r1, #0
	bl	memset
	mov	r2, #0
	str	r2, [r0, #-12]
	pop	{r4, pc}
	.size	frayClr, .-frayClr
	.align	2
	.global	frayGetFirstInactiveIdx
	.syntax unified
	.arm
	.fpu neon
	.type	frayGetFirstInactiveIdx, %function
frayGetFirstInactiveIdx:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	push	{lr}
	bl	__gnu_mcount_nc
	ldr	r0, [r0, #-20]
	bx	lr
	.size	frayGetFirstInactiveIdx, .-frayGetFirstInactiveIdx
	.align	2
	.global	frayGetFirstInactiveIdxP
	.syntax unified
	.arm
	.fpu neon
	.type	frayGetFirstInactiveIdxP, %function
frayGetFirstInactiveIdxP:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	push	{lr}
	bl	__gnu_mcount_nc
	sub	r0, r0, #20
	bx	lr
	.size	frayGetFirstInactiveIdxP, .-frayGetFirstInactiveIdxP
	.align	2
	.global	frayGetNPausedP
	.syntax unified
	.arm
	.fpu neon
	.type	frayGetNPausedP, %function
frayGetNPausedP:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	push	{lr}
	bl	__gnu_mcount_nc
	sub	r0, r0, #16
	bx	lr
	.size	frayGetNPausedP, .-frayGetNPausedP
	.align	2
	.global	frayGetNPaused
	.syntax unified
	.arm
	.fpu neon
	.type	frayGetNPaused, %function
frayGetNPaused:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	push	{lr}
	bl	__gnu_mcount_nc
	ldr	r0, [r0, #-16]
	bx	lr
	.size	frayGetNPaused, .-frayGetNPaused
	.align	2
	.global	frayGetFirstPausedIdx
	.syntax unified
	.arm
	.fpu neon
	.type	frayGetFirstPausedIdx, %function
frayGetFirstPausedIdx:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	push	{lr}
	bl	__gnu_mcount_nc
	ldr	r2, [r0, #-20]
	ldr	r0, [r0, #-16]
	sub	r0, r2, r0
	bx	lr
	.size	frayGetFirstPausedIdx, .-frayGetFirstPausedIdx
	.align	2
	.global	frayGetFirstEmptyIdxP
	.syntax unified
	.arm
	.fpu neon
	.type	frayGetFirstEmptyIdxP, %function
frayGetFirstEmptyIdxP:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	push	{lr}
	bl	__gnu_mcount_nc
	sub	r0, r0, #12
	bx	lr
	.size	frayGetFirstEmptyIdxP, .-frayGetFirstEmptyIdxP
	.align	2
	.global	frayElemIsActive
	.syntax unified
	.arm
	.fpu neon
	.type	frayElemIsActive, %function
frayElemIsActive:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	push	{lr}
	bl	__gnu_mcount_nc
	ldr	r0, [r0, #-20]
	cmp	r1, r0
	movcs	r0, #0
	movcc	r0, #1
	bx	lr
	.size	frayElemIsActive, .-frayElemIsActive
	.align	2
	.global	frayAdd
	.syntax unified
	.arm
	.fpu neon
	.type	frayAdd, %function
frayAdd:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, lr}
	push	{lr}
	bl	__gnu_mcount_nc
	mov	r4, r0
	ldr	r0, [r0, #-12]
	ldr	r3, [r4, #-4]
	mov	r5, r2
	cmp	r0, r3
	bcs	.L732
	ldr	r2, [r4, #-8]
	add	r3, r0, #1
	str	r3, [r4, #-12]
	mla	r0, r2, r0, r4
	bl	memcpy
	cmp	r5, #0
	beq	.L733
	ldr	r3, [r4, #-12]
	mov	r0, #0
	sub	r3, r3, #1
	str	r3, [r5]
	pop	{r4, r5, r6, pc}
.L732:
	mov	r0, #13
	pop	{r4, r5, r6, pc}
.L733:
	mov	r0, r5
	pop	{r4, r5, r6, pc}
	.size	frayAdd, .-frayAdd
	.align	2
	.global	frayPause
	.syntax unified
	.arm
	.fpu neon
	.type	frayPause, %function
frayPause:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, lr}
	push	{lr}
	bl	__gnu_mcount_nc
	ldr	r6, [r0, #-20]
	mov	r3, r0
	cmp	r6, r1
	ldr	r2, [r3, #-16]
	mov	r0, r1
	bls	.L735
	sub	r1, r6, r2
	cmp	r0, r1
	pophi	{r4, r5, r6, r7, r8, pc}
	ldr	r1, [r3, #-4]
	cmp	r2, r1
	popcs	{r4, r5, r6, r7, r8, pc}
	add	r2, r2, #1
	sub	r6, r6, r2
	str	r2, [r3, #-16]
	b	.L738
.L735:
	ldr	r1, [r3, #-4]
	cmp	r2, r1
	popcs	{r4, r5, r6, r7, r8, pc}
	add	r2, r2, #1
	str	r2, [r3, #-16]
	add	r2, r6, #1
	str	r2, [r3, #-20]
.L738:
	ldr	r8, [r3, #-8]
	mov	r2, r8
	mla	r5, r8, r0, r3
	mla	r7, r1, r8, r3
	mla	r4, r8, r6, r3
	mov	r1, r5
	mov	r0, r7
	bl	memcpy
	mov	r2, r8
	mov	r1, r4
	mov	r0, r5
	bl	memcpy
	mov	r2, r8
	mov	r1, r7
	mov	r0, r4
	bl	memcpy
	mov	r0, r6
	pop	{r4, r5, r6, r7, r8, pc}
	.size	frayPause, .-frayPause
	.align	2
	.global	frayUnpause
	.syntax unified
	.arm
	.fpu neon
	.type	frayUnpause, %function
frayUnpause:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, lr}
	push	{lr}
	bl	__gnu_mcount_nc
	ldr	r2, [r0, #-20]
	mov	r3, r0
	cmp	r1, r2
	mov	r0, r1
	popcs	{r4, r5, r6, r7, r8, pc}
	ldr	r1, [r3, #-16]
	sub	r6, r2, r1
	cmp	r0, r6
	popls	{r4, r5, r6, r7, r8, pc}
	cmp	r2, r1
	popls	{r4, r5, r6, r7, r8, pc}
	ldr	r7, [r3, #-8]
	ldr	r8, [r3, #-4]
	sub	r1, r1, #1
	mla	r4, r7, r0, r3
	mla	r8, r8, r7, r3
	mla	r5, r7, r6, r3
	str	r1, [r3, #-16]
	mov	r2, r7
	mov	r1, r4
	mov	r0, r8
	bl	memcpy
	mov	r2, r7
	mov	r1, r5
	mov	r0, r4
	bl	memcpy
	mov	r2, r7
	mov	r1, r8
	mov	r0, r5
	bl	memcpy
	mov	r0, r6
	pop	{r4, r5, r6, r7, r8, pc}
	.size	frayUnpause, .-frayUnpause
	.align	2
	.global	frayPauseAll
	.syntax unified
	.arm
	.fpu neon
	.type	frayPauseAll, %function
frayPauseAll:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	push	{lr}
	bl	__gnu_mcount_nc
	ldr	r3, [r0, #-20]
	str	r3, [r0, #-16]
	bx	lr
	.size	frayPauseAll, .-frayPauseAll
	.align	2
	.global	frayUnpauseAll
	.syntax unified
	.arm
	.fpu neon
	.type	frayUnpauseAll, %function
frayUnpauseAll:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	push	{lr}
	bl	__gnu_mcount_nc
	mov	r3, #0
	str	r3, [r0, #-16]
	bx	lr
	.size	frayUnpauseAll, .-frayUnpauseAll
	.align	2
	.global	frayActivate
	.syntax unified
	.arm
	.fpu neon
	.type	frayActivate, %function
frayActivate:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r3, r4, r5, r6, r7, r8, r9, r10, fp, lr}
	push	{lr}
	bl	__gnu_mcount_nc
	ldr	r8, [r0, #-20]
	mov	r4, r0
	cmp	r1, r8
	mov	r5, r1
	bcs	.L756
	mov	r0, r5
	pop	{r3, r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L756:
	ldr	r6, [r0, #-8]
	ldr	fp, [r0, #-16]
	ldr	r7, [r0, #-4]
	cmp	fp, #0
	mla	r10, r6, r1, r0
	mla	r7, r7, r6, r0
	mla	r9, r6, r8, r0
	bne	.L754
	mov	r5, r8
	add	r3, r8, #1
	str	r3, [r0, #-20]
.L755:
	mov	r2, r6
	mov	r1, r10
	mov	r0, r7
	bl	memcpy
	mov	r2, r6
	mov	r1, r9
	mov	r0, r10
	bl	memcpy
	mov	r2, r6
	mov	r1, r7
	mov	r0, r9
	bl	memcpy
	mov	r0, r5
	pop	{r3, r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L754:
	mov	r2, r6
	mov	r1, r10
	mov	r0, r7
	bl	memcpy
	mov	r2, r6
	mov	r1, r9
	mov	r0, r10
	bl	memcpy
	mov	r2, r6
	mov	r1, r7
	mov	r0, r9
	bl	memcpy
	ldr	r3, [r4, #-20]
	ldrd	r6, [r4, #-8]
	sub	r5, r3, fp
	add	r3, r3, #1
	mla	r10, r6, r8, r4
	mla	r7, r7, r6, r4
	mla	r9, r6, r5, r4
	str	r3, [r4, #-20]
	b	.L755
	.size	frayActivate, .-frayActivate
	.align	2
	.global	frayDeactivate
	.syntax unified
	.arm
	.fpu neon
	.type	frayDeactivate, %function
frayDeactivate:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, r9, r10, lr}
	push	{lr}
	bl	__gnu_mcount_nc
	ldr	r3, [r0, #-20]
	mov	r4, r0
	cmp	r1, r3
	mov	r5, r1
	bcc	.L762
	mov	r0, r5
	pop	{r4, r5, r6, r7, r8, r9, r10, pc}
.L762:
	ldr	r6, [r0, #-8]
	ldr	r1, [r0, #-16]
	ldr	r7, [r0, #-4]
	cmp	r1, #0
	mla	r8, r6, r5, r0
	mla	r7, r7, r6, r0
	sub	r5, r3, #1
	bne	.L759
.L761:
	str	r5, [r4, #-20]
	mla	r4, r6, r5, r4
	mov	r2, r6
	mov	r1, r8
	mov	r0, r7
	bl	memcpy
	mov	r2, r6
	mov	r1, r4
	mov	r0, r8
	bl	memcpy
	mov	r2, r6
	mov	r1, r7
	mov	r0, r4
	bl	memcpy
	mov	r0, r5
	pop	{r4, r5, r6, r7, r8, r9, r10, pc}
.L759:
	sub	r5, r5, r1
	mov	r2, r6
	mla	r9, r6, r5, r0
	mov	r1, r8
	mov	r0, r7
	bl	memcpy
	mov	r2, r6
	mov	r1, r9
	mov	r0, r8
	bl	memcpy
	mov	r1, r7
	mov	r2, r6
	mov	r0, r9
	bl	memcpy
	ldrd	r6, [r4, #-8]
	ldr	r1, [r4, #-20]
	mla	r8, r6, r5, r4
	mla	r7, r7, r6, r4
	sub	r5, r1, #1
	b	.L761
	.size	frayDeactivate, .-frayDeactivate
	.align	2
	.global	mailboxWrite
	.syntax unified
	.arm
	.fpu neon
	.type	mailboxWrite, %function
mailboxWrite:
	@ args = 4, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	str	lr, [sp, #-4]!
	sub	sp, sp, #12
	push	{lr}
	bl	__gnu_mcount_nc
	mov	ip, r0
	ldrb	lr, [sp, #16]	@ zero_extendqisi2
	orr	r1, r1, r2, lsl #8
	ldr	r0, [r0, #-12]
	orr	r1, r1, r3, lsl #16
	ldr	r3, [ip, #-4]
	orr	r1, r1, lr, lsl #24
	cmp	r0, r3
	movcs	r0, #13
	str	r1, [sp, #4]
	bcs	.L763
	ldr	r2, [ip, #-8]
	add	r3, r0, #1
	add	r1, sp, #4
	mla	r0, r2, r0, ip
	str	r3, [ip, #-12]
	bl	memcpy
	mov	r0, #0
.L763:
	add	sp, sp, #12
	@ sp needed
	ldr	pc, [sp], #4
	.size	mailboxWrite, .-mailboxWrite
	.align	2
	.global	mailboxForward
	.syntax unified
	.arm
	.fpu neon
	.type	mailboxForward, %function
mailboxForward:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, lr}
	push	{lr}
	bl	__gnu_mcount_nc
	ldr	r3, [r0, #-12]
	ldr	r2, [r0, #-4]
	cmp	r3, r2
	bcs	.L768
	ldr	r2, [r0, #-8]
	add	ip, r3, #1
	str	ip, [r0, #-12]
	mla	r0, r2, r3, r0
	bl	memcpy
	mov	r0, #0
	pop	{r4, pc}
.L768:
	mov	r0, #13
	pop	{r4, pc}
	.size	mailboxForward, .-mailboxForward
	.section	.rodata
	.align	3
	.set	.LANCHOR0,. + 0
	.type	s_min_table_sizes.5, %object
	.size	s_min_table_sizes.5, 12
s_min_table_sizes.5:
	.word	257
	.word	1
	.word	4
	.space	4
	.type	s_length_dezigzag.4, %object
	.size	s_length_dezigzag.4, 19
s_length_dezigzag.4:
	.ascii	"\020\021\022\000\010\007\011\006\012\005\013\004\014"
	.ascii	"\003\015\002\016\001\017"
	.space	5
	.type	s_length_extra.3, %object
	.size	s_length_extra.3, 124
s_length_extra.3:
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	1
	.word	1
	.word	1
	.word	1
	.word	2
	.word	2
	.word	2
	.word	2
	.word	3
	.word	3
	.word	3
	.word	3
	.word	4
	.word	4
	.word	4
	.word	4
	.word	5
	.word	5
	.word	5
	.word	5
	.word	0
	.word	0
	.word	0
	.space	4
	.type	s_length_base.2, %object
	.size	s_length_base.2, 124
s_length_base.2:
	.word	3
	.word	4
	.word	5
	.word	6
	.word	7
	.word	8
	.word	9
	.word	10
	.word	11
	.word	13
	.word	15
	.word	17
	.word	19
	.word	23
	.word	27
	.word	31
	.word	35
	.word	43
	.word	51
	.word	59
	.word	67
	.word	83
	.word	99
	.word	115
	.word	131
	.word	163
	.word	195
	.word	227
	.word	258
	.word	0
	.word	0
	.space	4
	.type	s_dist_extra.1, %object
	.size	s_dist_extra.1, 128
s_dist_extra.1:
	.word	0
	.word	0
	.word	0
	.word	0
	.word	1
	.word	1
	.word	2
	.word	2
	.word	3
	.word	3
	.word	4
	.word	4
	.word	5
	.word	5
	.word	6
	.word	6
	.word	7
	.word	7
	.word	8
	.word	8
	.word	9
	.word	9
	.word	10
	.word	10
	.word	11
	.word	11
	.word	12
	.word	12
	.word	13
	.word	13
	.space	8
	.type	s_dist_base.0, %object
	.size	s_dist_base.0, 128
s_dist_base.0:
	.word	1
	.word	2
	.word	3
	.word	4
	.word	5
	.word	7
	.word	9
	.word	13
	.word	17
	.word	25
	.word	33
	.word	49
	.word	65
	.word	97
	.word	129
	.word	193
	.word	257
	.word	385
	.word	513
	.word	769
	.word	1025
	.word	1537
	.word	2049
	.word	3073
	.word	4097
	.word	6145
	.word	8193
	.word	12289
	.word	16385
	.word	24577
	.word	0
	.word	0
	.ident	"GCC: (Raspbian 10.2.1-6+rpi1) 10.2.1 20210110"
	.section	.note.GNU-stack,"",%progbits
