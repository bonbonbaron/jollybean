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
	.file	"test.c"
	.text
.Ltext0:
	.cfi_sections	.debug_frame
	.section	.rodata.str1.4,"aMS",%progbits,1
	.align	2
.LC1:
	.ascii	"0x%08x \000"
	.section	.text.startup,"ax",%progbits
	.align	2
	.global	main
	.arch armv6
	.syntax unified
	.arm
	.fpu neon
	.type	main, %function
main:
.LVL0:
.LFB11:
	.file 1 "test.c"
	.loc 1 7 33 view -0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 32
	@ frame_needed = 0, uses_anonymous_args = 0
	.loc 1 10 3 view .LVU1
	.loc 1 12 3 view .LVU2
	.loc 1 7 33 is_stmt 0 view .LVU3
	push	{r4, r5, r6, lr}
	.cfi_def_cfa_offset 16
	.cfi_offset 4, -16
	.cfi_offset 5, -12
	.cfi_offset 6, -8
	.cfi_offset 14, -4
	sub	sp, sp, #32
	.cfi_def_cfa_offset 48
	.loc 1 12 7 view .LVU4
	mov	r2, #32
	mov	r1, #0
.LVL1:
	.loc 1 12 7 view .LVU5
	mov	r0, sp
	bl	memset
.LVL2:
	.loc 1 13 3 is_stmt 1 view .LVU6
	.loc 1 14 3 view .LVU7
	.loc 1 15 3 view .LVU8
	mov	r3, sp
	.syntax divided
@ 15 "test.c" 1
	vmov.i8 q10, 0xf0
	vld1.32 {d0-d1}, [r2]!
	vand q1, q0, q10
	vst1.64 {d2-d3}, [r3]!
	vld1.32 {d0-d1}, [r2]!
	vand q1, q0, q10
	vst1.64 {d2-d3}, [r3]
@ 0 "" 2
.LVL3:
	.loc 1 26 3 view .LVU9
	.arm
	.syntax unified
.LBB2:
	.loc 1 26 8 view .LVU10
	.loc 1 26 19 view .LVU11
	.loc 1 27 5 is_stmt 0 view .LVU12
	ldr	r6, .L7
	mov	r4, sp
	add	r5, sp, #28
.LBE2:
	.loc 1 15 3 view .LVU13
	mov	r1, #0
	b	.L3
.LVL4:
.L6:
.LBB3:
	.loc 1 27 5 view .LVU14
	ldr	r1, [r4, #4]!
.L3:
	.loc 1 27 5 is_stmt 1 discriminator 3 view .LVU15
	mov	r0, r6
	bl	printf
.LVL5:
	.loc 1 26 26 discriminator 3 view .LVU16
	.loc 1 26 19 discriminator 3 view .LVU17
	.loc 1 26 3 is_stmt 0 discriminator 3 view .LVU18
	cmp	r4, r5
	bne	.L6
.LBE3:
	.loc 1 28 3 is_stmt 1 view .LVU19
	mov	r0, #10
	bl	putchar
.LVL6:
	.loc 1 31 3 view .LVU20
	.loc 1 32 1 is_stmt 0 view .LVU21
	mov	r0, #0
	add	sp, sp, #32
	.cfi_def_cfa_offset 16
	@ sp needed
	pop	{r4, r5, r6, pc}
.L8:
	.align	2
.L7:
	.word	.LC1
	.cfi_endproc
.LFE11:
	.size	main, .-main
	.text
.Letext0:
	.file 2 "<built-in>"
	.file 3 "/usr/include/stdio.h"
	.section	.debug_info,"",%progbits
.Ldebug_info0:
	.4byte	0x1a0
	.2byte	0x4
	.4byte	.Ldebug_abbrev0
	.byte	0x4
	.uleb128 0x1
	.4byte	.LASF18
	.byte	0xc
	.4byte	.LASF19
	.4byte	.LASF20
	.4byte	.Ldebug_ranges0+0x18
	.4byte	0
	.4byte	.Ldebug_line0
	.uleb128 0x2
	.byte	0x4
	.byte	0x7
	.4byte	.LASF0
	.uleb128 0x2
	.byte	0x1
	.byte	0x8
	.4byte	.LASF1
	.uleb128 0x2
	.byte	0x2
	.byte	0x7
	.4byte	.LASF2
	.uleb128 0x2
	.byte	0x4
	.byte	0x7
	.4byte	.LASF3
	.uleb128 0x2
	.byte	0x1
	.byte	0x6
	.4byte	.LASF4
	.uleb128 0x2
	.byte	0x2
	.byte	0x5
	.4byte	.LASF5
	.uleb128 0x3
	.byte	0x4
	.byte	0x5
	.ascii	"int\000"
	.uleb128 0x2
	.byte	0x8
	.byte	0x5
	.4byte	.LASF6
	.uleb128 0x2
	.byte	0x8
	.byte	0x7
	.4byte	.LASF7
	.uleb128 0x2
	.byte	0x4
	.byte	0x5
	.4byte	.LASF8
	.uleb128 0x4
	.byte	0x4
	.4byte	0x71
	.uleb128 0x2
	.byte	0x1
	.byte	0x8
	.4byte	.LASF9
	.uleb128 0x5
	.ascii	"U32\000"
	.byte	0x1
	.byte	0x5
	.byte	0x16
	.4byte	0x25
	.uleb128 0x6
	.4byte	.LASF21
	.byte	0x1
	.byte	0x7
	.byte	0x5
	.4byte	0x4f
	.4byte	.LFB11
	.4byte	.LFE11-.LFB11
	.uleb128 0x1
	.byte	0x9c
	.4byte	0x164
	.uleb128 0x7
	.4byte	.LASF10
	.byte	0x1
	.byte	0x7
	.byte	0xe
	.4byte	0x4f
	.4byte	.LLST0
	.4byte	.LVUS0
	.uleb128 0x7
	.4byte	.LASF11
	.byte	0x1
	.byte	0x7
	.byte	0x1b
	.4byte	0x164
	.4byte	.LLST1
	.4byte	.LVUS1
	.uleb128 0x8
	.ascii	"inc\000"
	.byte	0x1
	.byte	0xa
	.byte	0x7
	.4byte	0x16a
	.uleb128 0x9
	.4byte	.LASF22
	.byte	0x1
	.byte	0xc
	.byte	0x7
	.4byte	0x16a
	.uleb128 0x2
	.byte	0x91
	.sleb128 -48
	.uleb128 0xa
	.4byte	.LASF12
	.byte	0x1
	.byte	0xd
	.byte	0x8
	.4byte	0x17a
	.4byte	.LLST2
	.4byte	.LVUS2
	.uleb128 0xa
	.4byte	.LASF13
	.byte	0x1
	.byte	0xe
	.byte	0x8
	.4byte	0x17a
	.4byte	.LLST3
	.4byte	.LVUS3
	.uleb128 0xb
	.4byte	.Ldebug_ranges0+0
	.4byte	0x135
	.uleb128 0xc
	.ascii	"i\000"
	.byte	0x1
	.byte	0x1a
	.byte	0xc
	.4byte	0x4f
	.4byte	.LLST4
	.4byte	.LVUS4
	.uleb128 0xd
	.4byte	.LVL5
	.4byte	0x180
	.uleb128 0xe
	.uleb128 0x1
	.byte	0x50
	.uleb128 0x2
	.byte	0x76
	.sleb128 0
	.byte	0
	.byte	0
	.uleb128 0xf
	.4byte	.LVL2
	.4byte	0x18d
	.4byte	0x154
	.uleb128 0xe
	.uleb128 0x1
	.byte	0x50
	.uleb128 0x2
	.byte	0x91
	.sleb128 -56
	.uleb128 0xe
	.uleb128 0x1
	.byte	0x51
	.uleb128 0x1
	.byte	0x30
	.uleb128 0xe
	.uleb128 0x1
	.byte	0x52
	.uleb128 0x2
	.byte	0x8
	.byte	0x20
	.byte	0
	.uleb128 0xd
	.4byte	.LVL6
	.4byte	0x198
	.uleb128 0xe
	.uleb128 0x1
	.byte	0x50
	.uleb128 0x1
	.byte	0x3a
	.byte	0
	.byte	0
	.uleb128 0x4
	.byte	0x4
	.4byte	0x6b
	.uleb128 0x10
	.4byte	0x78
	.4byte	0x17a
	.uleb128 0x11
	.4byte	0x25
	.byte	0x7
	.byte	0
	.uleb128 0x4
	.byte	0x4
	.4byte	0x78
	.uleb128 0x12
	.4byte	.LASF23
	.4byte	.LASF23
	.byte	0x3
	.2byte	0x14c
	.byte	0xc
	.uleb128 0x13
	.4byte	.LASF14
	.4byte	.LASF16
	.byte	0x2
	.byte	0
	.uleb128 0x13
	.4byte	.LASF15
	.4byte	.LASF17
	.byte	0x2
	.byte	0
	.byte	0
	.section	.debug_abbrev,"",%progbits
.Ldebug_abbrev0:
	.uleb128 0x1
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x25
	.uleb128 0xe
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1b
	.uleb128 0xe
	.uleb128 0x55
	.uleb128 0x17
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x10
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0x2
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.byte	0
	.byte	0
	.uleb128 0x3
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x8
	.byte	0
	.byte	0
	.uleb128 0x4
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x5
	.uleb128 0x16
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x6
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x6
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x2117
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x7
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x17
	.uleb128 0x2137
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0x8
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x9
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0xa
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x17
	.uleb128 0x2137
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.byte	0x1
	.uleb128 0x55
	.uleb128 0x17
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xc
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x17
	.uleb128 0x2137
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0xd
	.uleb128 0x4109
	.byte	0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x31
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xe
	.uleb128 0x410a
	.byte	0
	.uleb128 0x2
	.uleb128 0x18
	.uleb128 0x2111
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0xf
	.uleb128 0x4109
	.byte	0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x10
	.uleb128 0x1
	.byte	0x1
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x11
	.uleb128 0x21
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2f
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x12
	.uleb128 0x2e
	.byte	0
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3c
	.uleb128 0x19
	.uleb128 0x6e
	.uleb128 0xe
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x39
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x13
	.uleb128 0x2e
	.byte	0
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3c
	.uleb128 0x19
	.uleb128 0x6e
	.uleb128 0xe
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_loc,"",%progbits
.Ldebug_loc0:
.LVUS0:
	.uleb128 0
	.uleb128 .LVU6
	.uleb128 .LVU6
	.uleb128 0
.LLST0:
	.4byte	.LVL0
	.4byte	.LVL2-1
	.2byte	0x1
	.byte	0x50
	.4byte	.LVL2-1
	.4byte	.LFE11
	.2byte	0x4
	.byte	0xf3
	.uleb128 0x1
	.byte	0x50
	.byte	0x9f
	.4byte	0
	.4byte	0
.LVUS1:
	.uleb128 0
	.uleb128 .LVU5
	.uleb128 .LVU5
	.uleb128 0
.LLST1:
	.4byte	.LVL0
	.4byte	.LVL1
	.2byte	0x1
	.byte	0x51
	.4byte	.LVL1
	.4byte	.LFE11
	.2byte	0x4
	.byte	0xf3
	.uleb128 0x1
	.byte	0x51
	.byte	0x9f
	.4byte	0
	.4byte	0
.LVUS2:
	.uleb128 .LVU7
	.uleb128 .LVU9
.LLST2:
	.4byte	.LVL2
	.4byte	.LVL3
	.2byte	0x6
	.byte	0xf2
	.4byte	.Ldebug_info0+198
	.sleb128 0
	.4byte	0
	.4byte	0
.LVUS3:
	.uleb128 .LVU8
	.uleb128 .LVU9
.LLST3:
	.4byte	.LVL2
	.4byte	.LVL3
	.2byte	0x3
	.byte	0x91
	.sleb128 -56
	.byte	0x9f
	.4byte	0
	.4byte	0
.LVUS4:
	.uleb128 .LVU11
	.uleb128 .LVU14
.LLST4:
	.4byte	.LVL3
	.4byte	.LVL4
	.2byte	0x2
	.byte	0x30
	.byte	0x9f
	.4byte	0
	.4byte	0
	.section	.debug_aranges,"",%progbits
	.4byte	0x1c
	.2byte	0x2
	.4byte	.Ldebug_info0
	.byte	0x4
	.byte	0
	.2byte	0
	.2byte	0
	.4byte	.LFB11
	.4byte	.LFE11-.LFB11
	.4byte	0
	.4byte	0
	.section	.debug_ranges,"",%progbits
.Ldebug_ranges0:
	.4byte	.LBB2
	.4byte	.LBE2
	.4byte	.LBB3
	.4byte	.LBE3
	.4byte	0
	.4byte	0
	.4byte	.LFB11
	.4byte	.LFE11
	.4byte	0
	.4byte	0
	.section	.debug_line,"",%progbits
.Ldebug_line0:
	.section	.debug_str,"MS",%progbits,1
.LASF16:
	.ascii	"__builtin_memset\000"
.LASF15:
	.ascii	"putchar\000"
.LASF0:
	.ascii	"unsigned int\000"
.LASF3:
	.ascii	"long unsigned int\000"
.LASF7:
	.ascii	"long long unsigned int\000"
.LASF20:
	.ascii	"/home/bonbonbaron/hack/jollybean/utils/img\000"
.LASF9:
	.ascii	"char\000"
.LASF13:
	.ascii	"resultP\000"
.LASF1:
	.ascii	"unsigned char\000"
.LASF21:
	.ascii	"main\000"
.LASF19:
	.ascii	"test.c\000"
.LASF22:
	.ascii	"result\000"
.LASF8:
	.ascii	"long int\000"
.LASF18:
	.ascii	"GNU C17 10.2.1 20210110 -mfpu=neon -mfloat-abi=hard"
	.ascii	" -mtls-dialect=gnu -marm -march=armv6+fp -g -Ofast "
	.ascii	"-O2\000"
.LASF17:
	.ascii	"__builtin_putchar\000"
.LASF6:
	.ascii	"long long int\000"
.LASF10:
	.ascii	"argc\000"
.LASF2:
	.ascii	"short unsigned int\000"
.LASF23:
	.ascii	"printf\000"
.LASF12:
	.ascii	"inputP\000"
.LASF5:
	.ascii	"short int\000"
.LASF14:
	.ascii	"memset\000"
.LASF4:
	.ascii	"signed char\000"
.LASF11:
	.ascii	"argv\000"
	.ident	"GCC: (Raspbian 10.2.1-6+rpi1) 10.2.1 20210110"
	.section	.note.GNU-stack,"",%progbits
