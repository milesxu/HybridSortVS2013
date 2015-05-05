zmm0 TEXTEQU <ymm8>
zmm1 TEXTEQU <ymm9>
zmm2 TEXTEQU <ymm10>
zmm3 TEXTEQU <ymm11>
zmm4 TEXTEQU <ymm12>
zmm5 TEXTEQU <ymm13>
zmm6 TEXTEQU <ymm14>
zmm7 TEXTEQU <ymm15>

fullLoad MACRO src			;load 64 items into ymm0-7
    vmovdqa ymm0, ymmword ptr [src]				
    vmovdqa ymm1, ymmword ptr [src + 32]
    vmovdqa ymm2, ymmword ptr [src + 64]
    vmovdqa ymm3, ymmword ptr [src + 96]
    vmovdqa ymm4, ymmword ptr [src + 128]
    vmovdqa ymm5, ymmword ptr [src + 160]
    vmovdqa ymm6, ymmword ptr [src + 192]
    vmovdqa ymm7, ymmword ptr [src + 224]
ENDM

fullLoadWhile MACRO src, dst, num
    for i, <num>
        vmovdqa &dst&&i, ymmword ptr [src + i * 32]
    ENDM
ENDM

halfLoadZ MACRO src			;load 64 items into zmm0-7
    vmovdqa zmm0, ymmword ptr [src]				
    vmovdqa zmm1, ymmword ptr [src + 32]
    vmovdqa zmm2, ymmword ptr [src + 64]
    vmovdqa zmm3, ymmword ptr [src + 96]
ENDM

fullStore MACRO	dst		;store all 64 sorted item from ymm0-7 to memory
    vmovdqa ymmword ptr [dst],       ymm0
    vmovdqa ymmword ptr [dst + 32],  ymm1
    vmovdqa ymmword ptr [dst + 64],  ymm2
    vmovdqa ymmword ptr [dst + 96],  ymm3
    vmovdqa ymmword ptr [dst + 128], ymm4
    vmovdqa ymmword ptr [dst + 160], ymm5
    vmovdqa ymmword ptr [dst + 192], ymm6
    vmovdqa ymmword ptr [dst + 224], ymm7
ENDM

halfStoreRev MACRO dst		;store 4 rows in ymm8-12
    vmovdqa ymmword ptr [dst],       zmm0
    vmovdqa ymmword ptr [dst + 32],  zmm1
    vmovdqa ymmword ptr [dst + 64],  zmm2
    vmovdqa ymmword ptr [dst + 96],  zmm3
ENDM

compareDis1 MACRO			;(1,2),(3,4) compare, source ymm0-7, des ymm8-15
    vpminsd zmm0, ymm0, ymm1
    vpminsd zmm2, ymm2, ymm3
    vpminsd zmm4, ymm4, ymm5
    vpminsd zmm6, ymm6, ymm7
    vpmaxsd zmm1, ymm0, ymm1
    vpmaxsd zmm3, ymm2, ymm3
    vpmaxsd zmm5, ymm4, ymm5
    vpmaxsd zmm7, ymm6, ymm7
ENDM

compareDis1AND MACRO src, dst
    vpminsd &dst&mm0, &src&mm0, &src&mm1
    vpminsd &dst&mm2, &src&mm2, &src&mm3
    vpminsd &dst&mm4, &src&mm4, &src&mm5
    vpminsd &dst&mm6, &src&mm6, &src&mm7
    vpmaxsd &dst&mm1, &src&mm0, &src&mm1
    vpmaxsd &dst&mm3, &src&mm2, &src&mm3
    vpmaxsd &dst&mm5, &src&mm4, &src&mm5
    vpmaxsd &dst&mm7, &src&mm6, &src&mm7
ENDM

compareDis1Rev MACRO		;(8,9),(10,11) compare, source ymm8-15, des ymm0-7
    vpminsd ymm0, zmm0, zmm1
    vpminsd ymm2, zmm2, zmm3
    vpminsd ymm4, zmm4, zmm5
    vpminsd ymm6, zmm6, zmm7
    vpmaxsd ymm1, zmm0, zmm1
    vpmaxsd ymm3, zmm2, zmm3
    vpmaxsd ymm5, zmm4, zmm5
    vpmaxsd ymm7, zmm6, zmm7
ENDM

compareDis3_1Rev MACRO		;(8,11),(9,10) compare, source ymm8-15, des ymm0-7
    vpminsd ymm0, zmm0, zmm3
    vpminsd ymm1, zmm1, zmm2
    vpminsd ymm4, zmm4, zmm7
    vpminsd ymm5, zmm5, zmm6
    vpmaxsd ymm3, zmm0, zmm3
    vpmaxsd ymm2, zmm1, zmm2
    vpmaxsd ymm7, zmm4, zmm7
    vpmaxsd ymm6, zmm5, zmm6
ENDM

compareDis7_1 MACRO			;(0,7),(1,6) compare, source ymm0-7, des ymm8-15
    vpminsd zmm0, ymm0, ymm7
    vpminsd zmm1, ymm1, ymm6
    vpminsd zmm2, ymm2, ymm5
    vpminsd zmm3, ymm3, ymm4
    vpmaxsd zmm7, ymm0, ymm7
    vpmaxsd zmm6, ymm1, ymm6
    vpmaxsd zmm5, ymm2, ymm5
    vpmaxsd zmm4, ymm3, ymm4
ENDM

compareDis7_1Rev MACRO		;(8,15),(9,14) compare, source ymm8-15, des ymm0-7
    vpminsd ymm0, zmm0, zmm7
    vpminsd ymm1, zmm1, zmm6
    vpminsd ymm2, zmm2, zmm5
    vpminsd ymm3, zmm3, zmm4
    vpmaxsd ymm7, zmm0, zmm7
    vpmaxsd ymm6, zmm1, zmm6
    vpmaxsd ymm5, zmm2, zmm5
    vpmaxsd ymm4, zmm3, zmm4
ENDM

compareDis2 MACRO			;(0,2),(1,3) compare, source ymm0-7, des ymm8-15
    vpminsd zmm0, ymm0, ymm2
    vpminsd zmm1, ymm1, ymm3
    vpminsd zmm4, ymm4, ymm6
    vpminsd zmm5, ymm5, ymm7
    vpmaxsd zmm2, ymm0, ymm2
    vpmaxsd zmm3, ymm1, ymm3
    vpmaxsd zmm6, ymm4, ymm6
    vpmaxsd zmm7, ymm5, ymm7
ENDM

compareDis2Rev MACRO		;(8,10),(9,11) compare, source ymm8-15, des ymm0-7
    vpminsd ymm0, zmm0, zmm2
    vpminsd ymm1, zmm1, zmm3
    vpminsd ymm4, zmm4, zmm6
    vpminsd ymm5, zmm5, zmm7
    vpmaxsd ymm2, zmm0, zmm2
    vpmaxsd ymm3, zmm1, zmm3
    vpmaxsd ymm6, zmm4, zmm6
    vpmaxsd ymm7, zmm5, zmm7
ENDM

compareDis4 MACRO			;(0,4),(1,5) compare, source ymm0-7, des ymm8-15
    vpminsd zmm0, ymm0, ymm4
    vpminsd zmm1, ymm1, ymm5
    vpminsd zmm2, ymm2, ymm6
    vpminsd zmm3, ymm3, ymm7
    vpmaxsd zmm4, ymm0, ymm4
    vpmaxsd zmm5, ymm1, ymm5
    vpmaxsd zmm6, ymm2, ymm6
    vpmaxsd zmm7, ymm3, ymm7
ENDM

compareDis4Rev MACRO		;(8,12),(9,13) compare, source ymm8-15, des ymm0-7
    vpminsd ymm0, zmm0, zmm4
    vpminsd ymm1, zmm1, zmm5
    vpminsd ymm2, zmm2, zmm6
    vpminsd ymm3, zmm3, zmm7
    vpmaxsd ymm4, zmm0, zmm4
    vpmaxsd ymm5, zmm1, zmm5
    vpmaxsd ymm6, zmm2, zmm6
    vpmaxsd ymm7, zmm3, zmm7
ENDM

initialSort MACRO			;initial column sort, for unsorted lists
    compareDis1
    compareDis3_1Rev
    compareDis1
    compareDis7_1Rev
    compareDis2
    compareDis1Rev
ENDM

shuffle4_7 MACRO imm	;shuffle ymm4-7
    vpshufd ymm4, ymm4, imm
    vpshufd ymm5, ymm5, imm
    vpshufd ymm6, ymm6, imm
    vpshufd ymm7, ymm7, imm
ENDM

shuffle4_7Rev MACRO imm 	;shuffle ymm12-15
    vpshufd zmm4, zmm4, imm
    vpshufd zmm5, zmm5, imm
    vpshufd zmm6, zmm6, imm
    vpshufd zmm7, zmm7, imm
ENDM

shuffleEchg0_7 MACRO imm1, imm2 	;shuffle ymm0-7, des ymm8-15
    vpshufd zmm0, ymm0, imm1
    vpshufd zmm1, ymm1, imm1
    vpshufd zmm2, ymm2, imm1
    vpshufd zmm3, ymm3, imm1
    vpshufd zmm4, ymm4, imm2
    vpshufd zmm5, ymm5, imm2
    vpshufd zmm6, ymm6, imm2
    vpshufd zmm7, ymm7, imm2
ENDM

shuffleEchg0_7SameZ MACRO imm1, imm2 ;shuffle same like above within same zmms
    vpshufd zmm0, zmm0, imm1
    vpshufd zmm1, zmm1, imm1
    vpshufd zmm2, zmm2, imm1
    vpshufd zmm3, zmm3, imm1
    vpshufd zmm4, zmm4, imm2
    vpshufd zmm5, zmm5, imm2
    vpshufd zmm6, zmm6, imm2
    vpshufd zmm7, zmm7, imm2
ENDM

shuffleEchg0_7Rev MACRO imm1, imm2 	;shuffle ymm8-15, des ymm0-4
    vpshufd ymm0, zmm0, imm1
    vpshufd ymm1, zmm1, imm1
    vpshufd ymm2, zmm2, imm1
    vpshufd ymm3, zmm3, imm1
    vpshufd ymm4, zmm4, imm2
    vpshufd ymm5, zmm5, imm2
    vpshufd ymm6, zmm6, imm2
    vpshufd ymm7, zmm7, imm2
ENDM

blendDis4 MACRO imm1, imm2 ;blend (0,4),(1,5), des ymm8-15
    vpblendd zmm0, ymm0, ymm4, imm1
    vpblendd zmm1, ymm1, ymm5, imm1
    vpblendd zmm2, ymm2, ymm6, imm1
    vpblendd zmm3, ymm3, ymm7, imm1
    vpblendd zmm4, ymm0, ymm4, imm2
    vpblendd zmm5, ymm1, ymm5, imm2
    vpblendd zmm6, ymm2, ymm6, imm2
    vpblendd zmm7, ymm3, ymm7, imm2
ENDM

blendDis4Rev MACRO imm1, imm2 	;blend (8,12),(9,13), des ymm0-7
    vpblendd ymm0, zmm0, zmm4, imm1
    vpblendd ymm1, zmm1, zmm5, imm1
    vpblendd ymm2, zmm2, zmm6, imm1
    vpblendd ymm3, zmm3, zmm7, imm1
    vpblendd ymm4, zmm0, zmm4, imm2
    vpblendd ymm5, zmm1, zmm5, imm2
    vpblendd ymm6, zmm2, zmm6, imm2
    vpblendd ymm7, zmm3, zmm7, imm2
ENDM

blendDis7_1 MACRO imm1, imm2 ;blend (0,7),(1,6), des ymm8-15
    vpblendd zmm0, ymm0, ymm7, imm1
    vpblendd zmm1, ymm1, ymm6, imm1
    vpblendd zmm2, ymm2, ymm5, imm1
    vpblendd zmm3, ymm3, ymm4, imm1
    vpblendd zmm7, ymm0, ymm7, imm2
    vpblendd zmm6, ymm1, ymm6, imm2
    vpblendd zmm5, ymm2, ymm5, imm2
    vpblendd zmm4, ymm3, ymm4, imm2
ENDM

blendDis7_1Rev MACRO imm1, imm2	; blend (8,15),(9,14), des ymm0-7
    vpblendd ymm0, zmm0, zmm7, imm1
    vpblendd ymm1, zmm1, zmm6, imm1
    vpblendd ymm2, zmm2, zmm5, imm1
    vpblendd ymm3, zmm3, zmm4, imm1
    vpblendd ymm7, zmm0, zmm7, imm2
    vpblendd ymm6, zmm1, zmm6, imm2
    vpblendd ymm5, zmm2, zmm5, imm2
    vpblendd ymm4, zmm3, zmm4, imm2
ENDM

permuteDis4S MACRO imm  			;permute to get left or right 4 columns
    vperm2i128 zmm0, ymm0, ymm4, imm
    vperm2i128 zmm1, ymm1, ymm5, imm
    vperm2i128 zmm2, ymm2, ymm6, imm
    vperm2i128 zmm3, ymm3, ymm7, imm
ENDM

permute4_7Rev MACRO imm 		;permute ymm12-15
    vperm2i128 zmm4, zmm4, zmm4, imm
    vperm2i128 zmm5, zmm5, zmm5, imm
    vperm2i128 zmm6, zmm6, zmm6, imm
    vperm2i128 zmm7, zmm7, zmm7, imm
ENDM

halfPermute MACRO imm
    vperm2i128 zmm0, ymm0, ymm1, imm
    vperm2i128 zmm1, ymm2, ymm3, imm
    vperm2i128 zmm2, ymm4, ymm6, imm
    vperm2i128 zmm3, ymm6, ymm7, imm
ENDM

columnPermute MACRO imm1, imm2
    vperm2i128 ymm0, zmm0, ymm0, imm1
    vperm2i128 ymm2, zmm1, ymm2, imm1
    vperm2i128 ymm4, zmm2, ymm4, imm1
    vperm2i128 ymm6, zmm3, ymm6, imm1
    vperm2i128 ymm1, zmm0, ymm1, imm2
    vperm2i128 ymm3, zmm1, ymm3, imm2
    vperm2i128 ymm5, zmm2, ymm5, imm2
    vperm2i128 ymm7, zmm3, ymm7, imm2
ENDM

permuteDis4 MACRO imm1, imm2   ;permute (0,4),(1,5), des ymm8-15
    vperm2i128 zmm0, ymm0, ymm4, imm1
    vperm2i128 zmm1, ymm1, ymm5, imm1
    vperm2i128 zmm2, ymm2, ymm6, imm1
    vperm2i128 zmm3, ymm3, ymm7, imm1
    vperm2i128 zmm4, ymm0, ymm4, imm2
    vperm2i128 zmm5, ymm1, ymm5, imm2
    vperm2i128 zmm6, ymm2, ymm6, imm2
    vperm2i128 zmm7, ymm3, ymm7, imm2
ENDM

permuteDis4Rev MACRO imm1, imm2  ;permute (8,12),(9,13), dst ymm0-7
    vperm2i128 ymm0, zmm0, zmm4, imm1	
    vperm2i128 ymm1, zmm1, zmm5, imm1
    vperm2i128 ymm2, zmm2, zmm6, imm1
    vperm2i128 ymm3, zmm3, zmm7, imm1
    vperm2i128 ymm4, zmm0, zmm4, imm2
    vperm2i128 ymm5, zmm1, zmm5, imm2
    vperm2i128 ymm6, zmm2, zmm6, imm2
    vperm2i128 ymm7, zmm3, zmm7, imm2
ENDM

invertPermute47 MACRO src1, src2 	;invert items in ymm4-7
    vpermd ymm4, src1, ymm4
    vpermd ymm5, src2, ymm5
    vpermd ymm6, src1, ymm6
    vpermd ymm7, src2, ymm7
ENDM

transportation MACRO dst		;transport 4 sorted columns into rows
    vpshufd  zmm1, zmm1, 177
    vpshufd  zmm3, zmm3, 177
    vpblendd zmm6, zmm2, zmm3, 170
    vpblendd zmm7, zmm2, zmm3, 85
    vpblendd zmm5, zmm0, zmm1, 85
    vpblendd zmm4, zmm0, zmm1, 170
    vpshufd  zmm6, zmm6, 78
    vpshufd  zmm7, zmm7, 78
    vpblendd zmm0, zmm4, zmm6, 204
    vpblendd zmm1, zmm5, zmm7, 204
    vpblendd zmm2, zmm4, zmm6, 51
    vpblendd zmm3, zmm5, zmm7, 51
    vpshufd  zmm1, zmm1, 177
    vpshufd  zmm2, zmm2, 78
    vpshufd  zmm3, zmm3, 27
    halfStoreRev dst
ENDM

leftTransport MACRO	dst		;transport left four columns
    permuteDis4S 32
    transportation dst
ENDM

leftPartialTransport MACRO dst
    halfPermute 32
    halfStoreRev dst
ENDM

rightTransport MACRO dst		;transport right four columns
    permuteDis4S 49
    transportation dst
ENDM

rightPartialTransport MACRO dst
    halfPermute 49
    halfStoreRev dst
ENDM

leftLoad MACRO src
    halfLoadZ src
    columnPermute 48, 49
ENDM

rightLoad MACRO src
    halfLoadZ src
    columnPermute 2, 18
ENDM

leftHalfGather MACRO  ;load sorted 32 items to left four columns
    vpxor	   zmm6, zmm6, zmm6
    vpxor 	   zmm7, zmm7, zmm7
    vpcmpeqd   zmm6, zmm6, zmm5
    vpcmpeqd   zmm7, zmm7, zmm5
    vpgatherdd ymm0, [rax + zmm4 * 8],       zmm6
    vpcmpeqd   zmm6, zmm6, zmm5
    vpgatherdd ymm1, [rax + zmm4 * 8 + 4],   zmm7
    vpcmpeqd   zmm7, zmm7, zmm5
    vpgatherdd ymm2, [rax + zmm4 * 8 + 8],   zmm6
    vpcmpeqd   zmm6, zmm6, zmm5
    vpgatherdd ymm3, [rax + zmm4 * 8 + 12],  zmm7
    vpcmpeqd   zmm7, zmm7, zmm5
    vpgatherdd ymm4, [rax + zmm4 * 8 + 16],  zmm6
    vpcmpeqd   zmm6, zmm6, zmm5
    vpgatherdd ymm5, [rax + zmm4 * 8 + 20],  zmm7
    vpcmpeqd   zmm7, zmm7, zmm5
    vpgatherdd ymm6, [rax + zmm4 * 8 + 24],  zmm6
    vpgatherdd ymm7, [rax + zmm4 * 8 + 28],  zmm7
ENDM

rightHalfGather MACRO  ;load sorted 32 items to right four columns
    vpxor	   zmm6, zmm6, zmm6
    vpxor 	   zmm7, zmm7, zmm7
    vpor       zmm6, zmm6, zmm5
    vpor   	   zmm7, zmm7, zmm5
    vpgatherdd ymm0, [rax + zmm4 * 8],       zmm6
    vpor       zmm6, zmm6, zmm5
    vpgatherdd ymm1, [rax + zmm4 * 8 + 4],   zmm7
    vpor       zmm7, zmm7, zmm5
    vpgatherdd ymm2, [rax + zmm4 * 8 + 8],   zmm6
    vpor       zmm6, zmm6, zmm5
    vpgatherdd ymm3, [rax + zmm4 * 8 + 12],  zmm7
    vpor       zmm7, zmm7, zmm5
    vpgatherdd ymm4, [rax + zmm4 * 8 + 16],  zmm6
    vpor       zmm6, zmm6, zmm5
    vpgatherdd ymm5, [rax + zmm4 * 8 + 20],  zmm7
    vpor       zmm7, zmm7, zmm5
    vpgatherdd ymm6, [rax + zmm4 * 8 + 24],  zmm6
    vpgatherdd ymm7, [rax + zmm4 * 8 + 28],  zmm7
ENDM

end3Sort MACRO ;last three step in each stage
    compareDis4
    compareDis2Rev
    compareDis1AND y, z
ENDM

sort8_32 MACRO			;sort 4 columns
    shuffle4_7 177
    compareDis7_1
    blendDis7_1Rev 170, 85
    shuffle4_7 177
    end3Sort 		;each 16 items sorted

    shuffle4_7Rev 27
    compareDis7_1Rev
    blendDis4 170, 85
    shuffle4_7Rev 177
    compareDis4Rev
    blendDis7_1 204, 51
    shuffleEchg0_7Rev 216, 114
    end3Sort		;each 32 items sorted
ENDM

sort32_64 MACRO  ;sort all 8 columns
    permute4_7Rev 1
    shuffle4_7Rev 27
    compareDis7_1Rev
    permuteDis4 32, 49
    blendDis4Rev 102, 153
    shuffle4_7 78
    compareDis4
    blendDis4Rev 170, 85
    shuffle4_7 27
    compareDis4
    blendDis7_1Rev 102, 153
    shuffleEchg0_7 120, 210
    compareDis4Rev
    compareDis2
    compareDis1Rev
ENDM

sort32_64p MACRO
    vmovdqa zmm0, ymmword ptr [invertp]
    vmovdqa zmm1, ymmword ptr [invertp]
    invertPermute47 zmm0, zmm1
    compareDis7_1
    permuteDis4Rev 32, 49
    blendDis4 102, 153
    shuffle4_7Rev 78
    compareDis4Rev
    blendDis4 170, 85
    shuffle4_7Rev 27
    compareDis4Rev
    blendDis7_1 102, 153
    shuffleEchg0_7SameZ 120, 210
    compareDis4Rev
    compareDis2
    compareDis1Rev
ENDM

.data
align 16
invertp dword 7, 6, 5, 4, 3, 2, 1, 0
lindex  dword 0, 4, 8, 12, 0, 0, 0, 0
rindex  dword 0, 0, 0, 0, 0, 4, 8, 12
masknum dword 0, 0, 0, 0, 80000000h, 80000000h, 80000000h, 80000000h
masklf  dword 80000000h, 80000000h, 80000000h, 80000000h, 0, 0, 0, 0

.code

;rcx input address, rdx output address, r8 total length which is already 
;divided by 64
align 16
AVXBitonicSort PROC
    cmp r8,  1
    je  OnlyOne
Sort:	
    ;fullLoad rcx
    fullLoadWhile rcx, ymm, <0, 1, 2, 3, 4, 5, 6, 7>
    initialSort
    sort8_32
    sort32_64
    leftPartialTransport rdx
    add rdx, 128
    rightPartialTransport rdx    
    add rdx, 128
    add rcx, 256
    dec r8
    jnz Sort
    jmp ABTerminal
OnlyOne:
    fullLoad rcx
    initialSort
    sort8_32
    sort32_64
    leftTransport rdx
    add rdx, 128
    rightTransport rdx
ABTerminal:
    ret
AVXBitonicSort ENDP

;rcx input address, rdx output address, r8 length of one list, r9 length of 
;the other list
align 16
AVXMergeSort PROC uses r12		
    ; shl     r8,   2  	;byte length of input 1
    ; shl     r9,   2	;byte length of input 2 which follow input 1
    mov     r10,  rdx
    add 	r10,  r8
    add   	r10,  r9
    sub 	r10,  128	;end address of output
    mov     rbx,  rcx	;head address of input 1
    add     rcx,  r8	;head address of input 2
    mov     r8,   rcx	;end address of input 1
    add 	r9,   rcx	;end address of input 2
    ; add 	r9,   r8
    ; vmovdqa zmm5, ymmword ptr [masknum]
    ; leftHalfGather
    rightLoad rcx
    add 	  rcx,  128
    leftLoad  rbx
    add 	  rbx,  128
Merging:
    sort32_64p
    ; fullStore rdx
    ; jmp 	  Terminal
    leftPartialTransport rdx
    add 	  rdx,  128
    cmp 	  rdx,  r10
    je 		  Terminal
    cmp 	  rbx,  r8
    je 		  SecondLoad
    mov 	  r11d, sdword ptr [rbx]
    cmp 	  rcx,  r9
    je 		  FirstLoad
    mov 	  r12d, sdword ptr [rcx]
    cmp 	  r11d, r12d
    jg  	  SecondLoad
FirstLoad:
    leftLoad  rbx
    add 	  rbx,  128
    jmp 	  Merging
SecondLoad:
    leftLoad  rcx
    add 	  rcx,  128
    jmp 	  Merging
Terminal:
    rightPartialTransport rdx
    ret
AVXMergeSort ENDP

;rcx input address, rdx output address, r8 length of one list, r9 length of 
;the other list, the last time in loop
align 16
AVXMergeSortEnd PROC uses r12		
    ; shl     r8,   2  	;byte length of input 1
    ; shl     r9,   2	;byte length of input 2 which follow input 1
    mov     r10,  rdx
    add 	r10,  r8
    add   	r10,  r9
    sub 	r10,  128	;end address of output
    mov     rbx,  rcx	;head address of input 1
    add     rcx,  r8	;head address of input 2
    mov     r8,   rcx	;end address of input 1
    add 	r9,   rcx	;end address of input 2
    rightLoad rcx
    add 	  rcx,  128
    leftLoad  rbx
    add 	  rbx,  128
Merging:
    sort32_64p
    leftTransport rdx
    add 	  rdx,  128
    cmp 	  rdx,  r10
    je 		  Terminal
    cmp 	  rbx,  r8
    je 		  SecondLoad
    mov 	  r11d, sdword ptr [rbx]
    cmp 	  rcx,  r9
    je 		  FirstLoad
    mov 	  r12d, sdword ptr [rcx]
    cmp 	  r11d, r12d
    jg  	  SecondLoad
FirstLoad:
    leftLoad  rbx
    add 	  rbx,  128
    jmp 	  Merging
SecondLoad:
    leftLoad  rcx
    add 	  rcx,  128
    jmp 	  Merging
Terminal:
    rightTransport rdx
    ret
AVXMergeSortEnd ENDP

;rcx input address, rdx output address, r8 length of copy bytes
align 16
OddCopy PROC
    mov rax,  r8
    shr rax,  6
CopyLoop:
    fullLoad  rcx
    fullStore rdx
    add rcx,  256
    add rdx,  256
    dec rax
    jnz CopyLoop
    ret
OddCopy ENDP

;rcx input address, rdx output address, r8 total length
align 16
CoreSortStage1 PROC
    sub   rsp, 28h
    shr   r8,  6  		;divide by 64
    mov   r12, rcx
    mov   r13, rdx
    mov   r14, r8
    lzcnt rax, r8
    mov   rbx, 64
    sub   rbx, rax
    test  rbx, 1
    jz    FirstStep
    mov   r13, rcx
    mov   r12, rdx
    mov   rdx, rcx 		;if need input address equal output address
FirstStep:
    call  AVXBitonicSort
    add   rsp, 28h
    cmp   r14, 1
    je    CTerminal
    mov   r15, r14
    shl   r15, 8		;total length in byte
    mov   r10, 256		;unit length
    mov   r11, r14
    xor   rax, rax
    cmp   r14, 2
    jne   CMergeLoop
    mov   eax, 1
CMergeLoop:	
    mov   r9,  r10
    cmp   r11, 2
    ja    CNormalMerge
    push  rax
    mov   rax, r10
    shl   rax, 1
    mul   r14
    mov   rcx, r15
    sub   rcx, rax
    pop   rax
    cmp   r11, 1
    je    OnlyCopy    
    sub   rcx, r10
    mov   r9,  rcx
CNormalMerge:
    mov   rcx, r13
    mov   rdx, r12
    mov   r8,  r10
    mov   rbx, r8
    add   rbx, r9
    ; shr   r8,  2
    ; shr   r9,  2
    push  rax
    push  rbx
    push  r10
    push  r11
    sub   rsp, 28h
    mov   dword ptr [rsp + 20h], eax
    call  AVXMergeSort
    add   rsp, 28h
    pop   r11
    pop   r10
    pop   rbx
    pop   rax
    sub   r11, 2
    jmp   CLoopJudge
OnlyCopy:
    mov   r8,  rcx
    mov   rbx, r8
    mov   rcx, r13
    mov   rdx, r12
    shr   r8,  2
    push  rax
    push  rbx
    push  r10
    push  r11
    sub   rsp, 28h
    call  OddCopy
    add   rsp, 28h
    pop   r11
    pop   r10
    pop   rbx
    pop   rax
    sub   r11, 1
CLoopJudge:
    inc   r14
    add   r12, rbx
    add   r13, rbx
    cmp   r11, 0
    je    AddressExchange
    jmp   CMergeLoop
AddressExchange:
    cmp   r14, 1
    je    CTerminal
    sub   r12, r15
    sub   r13, r15
    mov   r11, r14
    mov   r14, r12
    mov   r12, r13
    mov   r13, r14
    xor   r14, r14
    shl   r10, 1
    ; shl   rbx, 1
    cmp   r11, 2
    jne   CMergeLoop
    mov   eax, 1
    jmp   CMergeLoop
CTerminal:
    ret
CoreSortStage1 ENDP

align 16
CoreSortStage2 PROC
CoreSortStage2 ENDP

end
