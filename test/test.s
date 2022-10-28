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
	.align	2
	.global	switchGet
	.arch armv6
	.syntax unified
	.arm
	.fpu neon
	.type	switchGet, %function
switchGet:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	push	{lr}
	bl	__gnu_mcount_nc
	sub	r0, r0, #1
	uxtb	r0, r0
	cmp	r0, #255
	ldrne	r3, .L5
	ldrne	r0, [r3, r0, lsl #2]
	ldreq	r0, .L5+4
	bx	lr
.L6:
	.align	2
.L5:
	.word	.LANCHOR1
	.word	.LANCHOR0
	.size	switchGet, .-switchGet
	.section	.rodata.str1.4,"aMS",%progbits,1
	.align	2
.LC0:
	.ascii	"%d: %d\012\000"
	.align	2
.LC1:
	.ascii	"switchGet got %d\012\000"
	.section	.text.startup,"ax",%progbits
	.align	2
	.global	main
	.syntax unified
	.arm
	.fpu neon
	.type	main, %function
main:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, lr}
	sub	sp, sp, #12
	push	{lr}
	bl	__gnu_mcount_nc
	mov	r1, #4
	mov	r2, #255
	mov	r0, sp
	bl	mapNew
	mov	r1, #1
	str	r1, [sp, #4]
	cmp	r0, #0
	bne	.L10
.L8:
	uxtb	r1, r1
	ldr	r0, [sp]
	add	r2, sp, #4
	bl	mapSet
	ldr	r1, [sp, #4]
	add	r1, r1, #1
	str	r1, [sp, #4]
	cmp	r1, #255
	cmple	r0, #0
	beq	.L8
.L10:
	ldr	r0, [sp]
	mov	r1, #1
	bl	mapGet
	ldr	r6, .L18
	ldr	r7, .L18+4
	ldr	r5, .L18+8
	mov	r4, #2
.L9:
	orr	r1, r4, #1
	ldr	r0, [sp]
	uxtb	r1, r1
	bl	mapGet
	mul	r3, r6, r4
	cmp	r3, #42
	bhi	.L11
	ldr	r2, [r0]
	mov	r1, r4
	mov	r0, r7
	bl	printf
.L11:
	add	r4, r4, #1
	cmp	r4, r5
	bne	.L9
	ldr	r3, .L18+12
	ldr	r6, .L18+16
	ldr	r5, .L18+20
	ldr	r7, .L18+24
	mov	r4, #1
	b	.L13
.L17:
	ldr	r3, [r7, r3, lsl #2]
.L13:
	ldr	r1, [r3]
	add	r4, r4, #1
	mov	r0, r6
	bl	printf
	orr	r3, r4, #1
	sub	r3, r3, #1
	cmp	r4, r5
	uxtb	r3, r3
	bne	.L17
	mov	r0, #0
	add	sp, sp, #12
	@ sp needed
	pop	{r4, r5, r6, r7, pc}
.L19:
	.align	2
.L18:
	.word	-926212351
	.word	.LC0
	.word	500000000
	.word	.LANCHOR0+28
	.word	.LC1
	.word	1000000
	.word	.LANCHOR1
	.size	main, .-main
	.global	globalArray
	.section	.rodata
	.align	2
	.set	.LANCHOR1,. + 0
	.type	CSWTCH.7, %object
	.size	CSWTCH.7, 1020
CSWTCH.7:
	.word	globalArray+28
	.word	globalArray+56
	.word	globalArray+84
	.word	globalArray+112
	.word	globalArray+140
	.word	globalArray+168
	.word	globalArray+196
	.word	globalArray+224
	.word	globalArray+252
	.word	globalArray+280
	.word	globalArray+308
	.word	globalArray+336
	.word	globalArray+364
	.word	globalArray+392
	.word	globalArray+420
	.word	globalArray+448
	.word	globalArray+476
	.word	globalArray+504
	.word	globalArray+532
	.word	globalArray+560
	.word	globalArray+588
	.word	globalArray+616
	.word	globalArray+644
	.word	globalArray+672
	.word	globalArray+700
	.word	globalArray+728
	.word	globalArray+756
	.word	globalArray+784
	.word	globalArray+812
	.word	globalArray+840
	.word	globalArray+868
	.word	globalArray+896
	.word	globalArray+924
	.word	globalArray+952
	.word	globalArray+980
	.word	globalArray+1008
	.word	globalArray+16
	.word	globalArray+44
	.word	globalArray+72
	.word	globalArray+100
	.word	globalArray+128
	.word	globalArray+156
	.word	globalArray+184
	.word	globalArray+212
	.word	globalArray+240
	.word	globalArray+268
	.word	globalArray+296
	.word	globalArray+324
	.word	globalArray+352
	.word	globalArray+380
	.word	globalArray+408
	.word	globalArray+436
	.word	globalArray+464
	.word	globalArray+492
	.word	globalArray+520
	.word	globalArray+548
	.word	globalArray+576
	.word	globalArray+604
	.word	globalArray+632
	.word	globalArray+660
	.word	globalArray+688
	.word	globalArray+716
	.word	globalArray+744
	.word	globalArray+772
	.word	globalArray+800
	.word	globalArray+828
	.word	globalArray+856
	.word	globalArray+884
	.word	globalArray+912
	.word	globalArray+940
	.word	globalArray+968
	.word	globalArray+996
	.word	globalArray+4
	.word	globalArray+32
	.word	globalArray+60
	.word	globalArray+88
	.word	globalArray+116
	.word	globalArray+144
	.word	globalArray+172
	.word	globalArray+200
	.word	globalArray+228
	.word	globalArray+256
	.word	globalArray+284
	.word	globalArray+312
	.word	globalArray+340
	.word	globalArray+368
	.word	globalArray+396
	.word	globalArray+424
	.word	globalArray+452
	.word	globalArray+480
	.word	globalArray+508
	.word	globalArray+536
	.word	globalArray+564
	.word	globalArray+592
	.word	globalArray+620
	.word	globalArray+648
	.word	globalArray+676
	.word	globalArray+704
	.word	globalArray+732
	.word	globalArray+760
	.word	globalArray+788
	.word	globalArray+816
	.word	globalArray+844
	.word	globalArray+872
	.word	globalArray+900
	.word	globalArray+928
	.word	globalArray+956
	.word	globalArray+984
	.word	globalArray+1012
	.word	globalArray+20
	.word	globalArray+48
	.word	globalArray+76
	.word	globalArray+104
	.word	globalArray+132
	.word	globalArray+160
	.word	globalArray+188
	.word	globalArray+216
	.word	globalArray+244
	.word	globalArray+272
	.word	globalArray+300
	.word	globalArray+328
	.word	globalArray+356
	.word	globalArray+384
	.word	globalArray+412
	.word	globalArray+440
	.word	globalArray+468
	.word	globalArray+496
	.word	globalArray+524
	.word	globalArray+552
	.word	globalArray+580
	.word	globalArray+608
	.word	globalArray+636
	.word	globalArray+664
	.word	globalArray+692
	.word	globalArray+720
	.word	globalArray+748
	.word	globalArray+776
	.word	globalArray+804
	.word	globalArray+832
	.word	globalArray+860
	.word	globalArray+888
	.word	globalArray+916
	.word	globalArray+944
	.word	globalArray+972
	.word	globalArray+1000
	.word	globalArray+8
	.word	globalArray+36
	.word	globalArray+64
	.word	globalArray+92
	.word	globalArray+120
	.word	globalArray+148
	.word	globalArray+176
	.word	globalArray+204
	.word	globalArray+232
	.word	globalArray+260
	.word	globalArray+288
	.word	globalArray+316
	.word	globalArray+344
	.word	globalArray+372
	.word	globalArray+400
	.word	globalArray+428
	.word	globalArray+456
	.word	globalArray+484
	.word	globalArray+512
	.word	globalArray+540
	.word	globalArray+568
	.word	globalArray+596
	.word	globalArray+624
	.word	globalArray+652
	.word	globalArray+680
	.word	globalArray+708
	.word	globalArray+736
	.word	globalArray+764
	.word	globalArray+792
	.word	globalArray+820
	.word	globalArray+848
	.word	globalArray+876
	.word	globalArray+904
	.word	globalArray+932
	.word	globalArray+960
	.word	globalArray+988
	.word	globalArray+1016
	.word	globalArray+24
	.word	globalArray+52
	.word	globalArray+80
	.word	globalArray+108
	.word	globalArray+136
	.word	globalArray+164
	.word	globalArray+192
	.word	globalArray+220
	.word	globalArray+248
	.word	globalArray+276
	.word	globalArray+304
	.word	globalArray+332
	.word	globalArray+360
	.word	globalArray+388
	.word	globalArray+416
	.word	globalArray+444
	.word	globalArray+472
	.word	globalArray+500
	.word	globalArray+528
	.word	globalArray+556
	.word	globalArray+584
	.word	globalArray+612
	.word	globalArray+640
	.word	globalArray+668
	.word	globalArray+696
	.word	globalArray+724
	.word	globalArray+752
	.word	globalArray+780
	.word	globalArray+808
	.word	globalArray+836
	.word	globalArray+864
	.word	globalArray+892
	.word	globalArray+920
	.word	globalArray+948
	.word	globalArray+976
	.word	globalArray+1004
	.word	globalArray+12
	.word	globalArray+40
	.word	globalArray+68
	.word	globalArray+96
	.word	globalArray+124
	.word	globalArray+152
	.word	globalArray+180
	.word	globalArray+208
	.word	globalArray+236
	.word	globalArray+264
	.word	globalArray+292
	.word	globalArray+320
	.word	globalArray+348
	.word	globalArray+376
	.word	globalArray+404
	.word	globalArray+432
	.word	globalArray+460
	.word	globalArray+488
	.word	globalArray+516
	.word	globalArray+544
	.word	globalArray+572
	.word	globalArray+600
	.word	globalArray+628
	.word	globalArray+656
	.word	globalArray+684
	.word	globalArray+712
	.word	globalArray+740
	.word	globalArray+768
	.word	globalArray+796
	.word	globalArray+824
	.word	globalArray+852
	.word	globalArray+880
	.word	globalArray+908
	.word	globalArray+936
	.word	globalArray+964
	.word	globalArray+992
	.word	globalArray
	.data
	.align	3
	.set	.LANCHOR0,. + 0
	.type	globalArray, %object
	.size	globalArray, 1020
globalArray:
	.word	1
	.word	2
	.word	3
	.word	4
	.word	5
	.word	6
	.word	7
	.word	8
	.word	9
	.word	10
	.word	11
	.word	12
	.word	13
	.word	14
	.word	15
	.word	16
	.word	17
	.word	18
	.word	19
	.word	20
	.word	21
	.word	22
	.word	23
	.word	24
	.word	25
	.word	26
	.word	27
	.word	28
	.word	29
	.word	30
	.word	31
	.word	32
	.word	33
	.word	34
	.word	35
	.word	36
	.word	37
	.word	38
	.word	39
	.word	40
	.word	41
	.word	42
	.word	43
	.word	44
	.word	45
	.word	46
	.word	47
	.word	48
	.word	49
	.word	50
	.word	51
	.word	52
	.word	53
	.word	54
	.word	55
	.word	56
	.word	57
	.word	58
	.word	59
	.word	60
	.word	61
	.word	62
	.word	63
	.word	64
	.word	65
	.word	66
	.word	67
	.word	68
	.word	69
	.word	70
	.word	71
	.word	72
	.word	73
	.word	74
	.word	75
	.word	76
	.word	77
	.word	78
	.word	79
	.word	80
	.word	81
	.word	82
	.word	83
	.word	84
	.word	85
	.word	86
	.word	87
	.word	88
	.word	89
	.word	90
	.word	91
	.word	92
	.word	93
	.word	94
	.word	95
	.word	96
	.word	97
	.word	98
	.word	99
	.word	100
	.word	101
	.word	102
	.word	103
	.word	104
	.word	105
	.word	106
	.word	107
	.word	108
	.word	109
	.word	110
	.word	111
	.word	112
	.word	113
	.word	114
	.word	115
	.word	116
	.word	117
	.word	118
	.word	119
	.word	120
	.word	121
	.word	122
	.word	123
	.word	124
	.word	125
	.word	126
	.word	127
	.word	128
	.word	129
	.word	130
	.word	131
	.word	132
	.word	133
	.word	134
	.word	135
	.word	136
	.word	137
	.word	138
	.word	139
	.word	140
	.word	141
	.word	142
	.word	143
	.word	144
	.word	145
	.word	146
	.word	147
	.word	148
	.word	149
	.word	150
	.word	151
	.word	152
	.word	153
	.word	154
	.word	155
	.word	156
	.word	157
	.word	158
	.word	159
	.word	160
	.word	161
	.word	162
	.word	163
	.word	164
	.word	165
	.word	166
	.word	167
	.word	168
	.word	169
	.word	170
	.word	171
	.word	172
	.word	173
	.word	174
	.word	175
	.word	176
	.word	177
	.word	178
	.word	179
	.word	180
	.word	181
	.word	182
	.word	183
	.word	184
	.word	185
	.word	186
	.word	187
	.word	188
	.word	189
	.word	190
	.word	191
	.word	192
	.word	193
	.word	194
	.word	195
	.word	196
	.word	197
	.word	198
	.word	199
	.word	200
	.word	201
	.word	202
	.word	203
	.word	204
	.word	205
	.word	206
	.word	207
	.word	208
	.word	209
	.word	210
	.word	211
	.word	212
	.word	213
	.word	214
	.word	215
	.word	216
	.word	217
	.word	218
	.word	219
	.word	220
	.word	221
	.word	222
	.word	223
	.word	224
	.word	225
	.word	226
	.word	227
	.word	228
	.word	229
	.word	230
	.word	231
	.word	232
	.word	233
	.word	234
	.word	235
	.word	236
	.word	237
	.word	238
	.word	239
	.word	240
	.word	241
	.word	242
	.word	243
	.word	244
	.word	245
	.word	246
	.word	247
	.word	248
	.word	249
	.word	250
	.word	251
	.word	252
	.word	253
	.word	254
	.word	255
	.ident	"GCC: (Raspbian 10.2.1-6+rpi1) 10.2.1 20210110"
	.section	.note.GNU-stack,"",%progbits
