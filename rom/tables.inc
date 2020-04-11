;; Chip ID database

chipid_list:
	; Brand, model, page size, mbit, brand string, model string
	
	; Atmel
	db	0x1F, 0x35
	dh	128
	dw	131072		; 128KB
	dw	chipid_brand_atmel, chipid_model_AT29LV010A
	
	db	0x1F, 0xD5
	dh	128
	dw	131072		; 128KB
	dw	chipid_brand_atmel, chipid_model_AT29C010A
	
	db	0x1F, 0xDA
	dh	256
	dw	262144		; 256KB
	dw	chipid_brand_atmel, chipid_model_AT29C020
	
	db	0x1F, 0xBA
	dh	256
	dw	262144		; 256KB
	dw	chipid_brand_atmel, chipid_model_AT29BV020
	
	db	0x1F, 0xA4
	dh	256
	dw	524288		; 512KB
	dw	chipid_brand_atmel, chipid_model_AT29C040A

	db	0x1F, 0xC4
	dh	256
	dw	524288		; 512KB
	dw	chipid_brand_atmel, chipid_model_AT29xV040A
	
	; SST
	db	0xBF, 0x07
	dh	128
	dw	131072		; 128KB
	dw	chipid_brand_sst, chipid_model_29EE010
	
	db	0xBF, 0x08
	dh	128
	dw	131072		; 128KB
	dw	chipid_brand_sst, chipid_model_29xE010
	
	db	0xBF, 0x22
	dh	128
	dw	131072		; 128KB
	dw	chipid_brand_sst, chipid_model_29EE010A
	
	db	0xBF, 0x23
	dh	128
	dw	131072		; 128KB
	dw	chipid_brand_sst, chipid_model_29xE010A
	
	db	0xBF, 0x10
	dh	128
	dw	262144		; 256KB
	dw	chipid_brand_sst, chipid_model_29EE020
	
	db	0xBF, 0x12
	dh	128
	dw	262144		; 256KB
	dw	chipid_brand_sst, chipid_model_29xE020
	
	db	0xBF, 0x24
	dh	128
	dw	262144		; 256KB
	dw	chipid_brand_sst, chipid_model_29EE020A
	
	db	0xBF, 0x25
	dh	128
	dw	262144		; 256KB
	dw	chipid_brand_sst, chipid_model_29xE020A
	
	db	0xBF, 0x04
	dh	256
	dw	524288		; 512KB
	dw	chipid_brand_sst, chipid_model_28SF040
	
	; Winbond
	db	0xDA, 0xC1
	dh	128
	dw	131072		; 128KB
	dw	chipid_brand_winbond, chipid_model_W29EE01x

	db	0xDA, 0x45
	dh	128
	dw	262144		; 256KB
	dw	chipid_brand_winbond, chipid_model_W29C020
	
	db	0xDA, 0x46
	dh	256
	dw	524288		; 512KB
	dw	chipid_brand_winbond, chipid_model_W29C040
	
	; Cypress
	db	0x01, 0xc4
	dh	256
	dw	2097152		; 2MB
	dw	chipid_brand_cypress, chipid_model_S29Al016J
	dw	0
	
chipid_model_AT29LV010A:
.asciiz "AT29LV010A"
chipid_model_AT29C010A:
.asciiz "AT29C010A"
chipid_model_AT29xV040A:
.asciiz "AT29xV040A"
chipid_model_AT29C020:
.asciiz "AT29C020"
chipid_model_AT29BV020:
.asciiz "AT29BV020"
chipid_model_AT29C040A:
.asciiz "AT29C040A"
chipid_model_29EE010:
.asciiz "29EE010"
chipid_model_29xE010:
.asciiz "29xE010"
chipid_model_29EE010A:
.asciiz "29EE010A"
chipid_model_29XE010A:
.asciiz "29xE010A"
chipid_model_29EE020:
.asciiz "29EE020"
chipid_model_29xE020:
.asciiz "29xE020"
chipid_model_29EE020A:
.asciiz "29EE020A"
chipid_model_29xE020A:
.asciiz "29xE020A"
chipid_model_28SF040:
.asciiz "28SF040"
chipid_model_W29C020:
.asciiz "W29C020"
chipid_model_W29C040:
.asciiz "W29C040"
chipid_model_W29EE01x:
.asciiz "W29EE01x"
chipid_model_S29Al016J:
.asciiz "S29A1016J"

chipid_brand_sst:
.asciiz	"SST "
chipid_brand_atmel:
.asciiz	"Atmel "
chipid_brand_winbond:
.asciiz	"Winbond "
chipid_brand_cypress:
.asciiz "Cypress "
chipid_unknown:
.asciiz "Unknown "
	
.align 4

colorbar_colors:
.align 0x4
	db	235	, 235	, 235	, 0x60	; 75% white
	db	191	, 191	, 0		, 0x60	; yellow
	db	0	, 191	, 191	, 0x60	; cyan
	db	0	, 191	, 0		, 0x60	; green
	db	191	, 0		, 191	, 0x60	; magenta
	db	191	, 0		, 0		, 0x60	; red
	db	0	, 0		, 191	, 0x60	; blue
	db	16	, 16	, 16	, 0x60	; black
	
	
plasma_colors:
	dw	0x00000000
	dw	0x00000408
	dw	0x00040811
	dw	0x0008111a
	dw	0x00081623
	dw	0x000d1f2c
	dw	0x00112335
	dw	0x0011273d
	dw	0x0016304a
	dw	0x001a3553
	dw	0x001f3d5b
	dw	0x001f4163
	dw	0x00234a6b
	dw	0x00274e73
	dw	0x0027537b
	dw	0x002c5b83
	dw	0x00305f8e
	dw	0x00356795
	dw	0x00356b9c
	dw	0x00396fa3
	dw	0x003d77aa
	dw	0x003d7bb1
	dw	0x004183b7
	dw	0x004687bd
	dw	0x00468ac6
	dw	0x004a92cb
	dw	0x004e95d0
	dw	0x00539cd5
	dw	0x0053a0da
	dw	0x0057a7df
	dw	0x005baae3
	dw	0x005bade7
	dw	0x005fb4ec
	dw	0x0063b7ef
	dw	0x0067bdf2
	dw	0x0067c0f5
	dw	0x006bc6f7
	dw	0x006fc8f9
	dw	0x006fcbfb
	dw	0x0073d0fc
	dw	0x0077d3fe
	dw	0x007bd8fe
	dw	0x007bdafe
	dw	0x007fdcfe
	dw	0x0083e1fe
	dw	0x0083e3fe
	dw	0x0087e7fd
	dw	0x008ae8fb
	dw	0x008eecf9
	dw	0x008eeef7
	dw	0x0092eff5
	dw	0x0095f2f2
	dw	0x0095f3ef
	dw	0x0099f6ec
	dw	0x009cf7e8
	dw	0x009cf8e5
	dw	0x00a0fadf
	dw	0x00a3fbda
	dw	0x00a7fcd5
	dw	0x00a7fdd0
	dw	0x00aafdcb
	dw	0x00adfec6
	dw	0x00adfec0
	dw	0x00b1feba
	dw	0x00b4ffb1
	dw	0x00b7feaa
	dw	0x00b7fea3
	dw	0x00bafe9c
	dw	0x00bdfd95
	dw	0x00bdfd8e
	dw	0x00c0fb87
	dw	0x00c3fb7f
	dw	0x00c3fa73
	dw	0x00c6f86b
	dw	0x00c8f763
	dw	0x00cbf55b
	dw	0x00cbf353
	dw	0x00cef14a
	dw	0x00d0ef41
	dw	0x00d0ee39
	dw	0x00d3ea2c
	dw	0x00d5e823
	dw	0x00d8e51a
	dw	0x00d8e311
	dw	0x00dae108
	dw	0x00dcdc00
	dw	0x00dcda08
	dw	0x00dfd511
	dw	0x00e1d31f
	dw	0x00e3ce27
	dw	0x00e3cb30
	dw	0x00e5c839
	dw	0x00e7c341
	dw	0x00e7c04a
	dw	0x00e8ba53
	dw	0x00eab75b
	dw	0x00eab467
	dw	0x00ecad6f
	dw	0x00eeaa77
	dw	0x00efa37f
	dw	0x00efa087
	dw	0x00f1998e
	dw	0x00f29595
	dw	0x00f2929c
	dw	0x00f38aa7
	dw	0x00f587ad
	dw	0x00f67fb4
	dw	0x00f67bba
	dw	0x00f773c0
	dw	0x00f86fc6
	dw	0x00f86bcb
	dw	0x00f963d0
	dw	0x00fa5fd8
	dw	0x00fb57dc
	dw	0x00fb53e1
	dw	0x00fb4ee5
	dw	0x00fc46e8
	dw	0x00fc41ec
	dw	0x00fd39ef
	dw	0x00fd35f2
	dw	0x00fe2cf6
	dw	0x00fe27f8
	dw	0x00fe23fa
	dw	0x00fe1afb
	dw	0x00fe16fd
	dw	0x00fe0dfe
	dw	0x00fe08fe
	dw	0x00fe04fe
	dw	0x00fe04fe
	dw	0x00fe08fe
	dw	0x00fe11fd
	dw	0x00fe16fc
	dw	0x00fe1ffb
	dw	0x00fe23f9
	dw	0x00fe27f7
	dw	0x00fe30f5
	dw	0x00fd35f1
	dw	0x00fd3dee
	dw	0x00fd41ea
	dw	0x00fc46e7
	dw	0x00fb4ee3
	dw	0x00fb53df
	dw	0x00fb5bda
	dw	0x00fa5fd5
	dw	0x00f967ce
	dw	0x00f96bc8
	dw	0x00f86fc3
	dw	0x00f777bd
	dw	0x00f77bb7
	dw	0x00f683b1
	dw	0x00f587aa
	dw	0x00f58aa3
	dw	0x00f39299
	dw	0x00f29592
	dw	0x00f19c8a
	dw	0x00f1a083
	dw	0x00efa77b
	dw	0x00eeaa73
	dw	0x00eead6b
	dw	0x00ecb463
	dw	0x00eab757
	dw	0x00e8bd4e
	dw	0x00e8c046
	dw	0x00e7c33d
	dw	0x00e5c835
	dw	0x00e5cb2c
	dw	0x00e3d023
	dw	0x00e1d31a
	dw	0x00dfd80d
	dw	0x00dfda04
	dw	0x00dcdc04
	dw	0x00dae10d
	dw	0x00dae316
	dw	0x00d8e71f
	dw	0x00d5e827
	dw	0x00d5ea30
	dw	0x00d3ee3d
	dw	0x00d0ef46
	dw	0x00cef24e
	dw	0x00cef357
	dw	0x00cbf65f
	dw	0x00c8f767
	dw	0x00c8f86f
	dw	0x00c6fa77
	dw	0x00c3fb83
	dw	0x00c0fc8a
	dw	0x00c0fd92
	dw	0x00bdfd99
	dw	0x00bafea0
	dw	0x00bafea7
	dw	0x00b7fead
	dw	0x00b4ffb4
	dw	0x00b1febd
	dw	0x00b1fec3
	dw	0x00adfec8
	dw	0x00aafdce
	dw	0x00aafdd3
	dw	0x00a7fbd8
	dw	0x00a3fbdc
	dw	0x00a3fae1
	dw	0x00a0f8e7
	dw	0x009cf7ea
	dw	0x0099f5ee
	dw	0x0099f3f1
	dw	0x0095f1f3
	dw	0x0092eff6
	dw	0x0092eef8
	dw	0x008eeafa
	dw	0x008ae8fc
	dw	0x0087e5fd
	dw	0x0087e3fe
	dw	0x0083e1fe
	dw	0x007fdcff
	dw	0x007fdafe
	dw	0x007bd5fe
	dw	0x0077d3fd
	dw	0x0073cefb
	dw	0x0073cbfa
	dw	0x006fc8f8
	dw	0x006bc3f6
	dw	0x006bc0f3
	dw	0x0067baf1
	dw	0x0063b7ee
	dw	0x0063b4ea
	dw	0x005fade5
	dw	0x005baae1
	dw	0x0057a3dc
	dw	0x0057a0d8
	dw	0x00539cd3
	dw	0x004e95ce
	dw	0x004e92c8
	dw	0x004a8ac3
	dw	0x004687ba
	dw	0x00417fb4
	dw	0x00417bad
	dw	0x003d77a7
	dw	0x00396fa0
	dw	0x00396b99
	dw	0x00356392
	dw	0x00305f8a
	dw	0x00305b7f
	dw	0x002c5377
	dw	0x00274e6f
	dw	0x00234667
	dw	0x0023415f
	dw	0x001f3957
	dw	0x001a354e
	dw	0x001a3046
	dw	0x00162739
	dw	0x00112330
	dw	0x000d1a27
	dw	0x000d161f
	dw	0x00081116
	dw	0x0004080d
	dw	0x00040404
	dw	0x00000404
	
plasma_sin1:
	dh	0
	dh	12
	dh	18
	dh	30
	dh	36
	dh	47
	dh	53
	dh	65
	dh	71
	dh	82
	dh	88
	dh	99
	dh	104
	dh	115
	dh	120
	dh	131
	dh	136
	dh	146
	dh	150
	dh	160
	dh	165
	dh	173
	dh	178
	dh	186
	dh	190
	dh	198
	dh	202
	dh	209
	dh	212
	dh	219
	dh	222
	dh	227
	dh	230
	dh	235
	dh	237
	dh	242
	dh	243
	dh	247
	dh	248
	dh	251
	dh	252
	dh	254
	dh	255
	dh	256
	dh	256
	dh	256
	dh	256
	dh	255
	dh	255
	dh	253
	dh	252
	dh	250
	dh	248
	
plasma_sin2:
	db	0
	db	9
	db	13
	db	21
	db	24
	db	29
	db	30
	db	32
	db	32
	db	30
	db	28
	db	22
	db	19
	db	11
	db	7
	db	-2
	db	-7
	db	-15
	db	-19
	db	-25
	db	-28
	db	-31
	db	-32
	db	-32
	db	-30
	db	-27
	db	-24
	db	-17
	db	-13
	db	-4
	db	0
	db	9
	db	13
	db	21
	db	24
	db	29
	db	30
	db	32
	db	32
	db	30
	
plasma_sin3:
	db	0
	db	9
	db	13
	db	22
	db	26
	db	34
	db	38
	db	44
	db	48
	db	53
	db	55
	db	59
	db	61
	db	63
	db	64
	db	64
	db	64
	db	62
	db	61
	db	58
	db	55
	db	50
	db	48
	db	41
	db	38
	db	30
	db	26
	db	18
	db	13
	db	4
	db	0
	db	-9
	db	-13
	db	-22
	db	-26
	db	-34
	db	-38
	db	-44
	db	-48
	db	-53
	db	-55
	db	-59
	db	-61
	db	-63
	db	-64
	db	-64
	db	-64
	db	-62
	db	-61
	db	-58
	db	-55
	db	-50
	db	-48
	db	-41
	db	-38
	db	-30
	db	-26
	db	-18
	db	-13
	db	-4
	
.align 0x4
