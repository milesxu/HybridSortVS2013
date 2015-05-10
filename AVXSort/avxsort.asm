wmm0 TEXTEQU <ymm8>
wmm1 TEXTEQU <ymm9>
wmm2 TEXTEQU <ymm10>
wmm3 TEXTEQU <ymm11>
wmm4 TEXTEQU <ymm12>
wmm5 TEXTEQU <ymm13>
wmm6 TEXTEQU <ymm14>
wmm7 TEXTEQU <ymm15>

; load data into ymm registers. src is a register has the base address,
; dst is ymm or wmm, namely ymm0-7 or ymm8-15, num is a subset of [0,7]
dataLoad MACRO src, dst, num
    for i, <num>
        vmovdqa &dst&&i, ymmword ptr [src + i * 32]
    ENDM
ENDM

; very similar with above macro, store data from ymm register to memory
dataStore MACRO src, dst, num
    for i, <num>
        vmovdqa ymmword ptr [dst + i * 32], &src&&i
    ENDM
ENDM

fullLoadY MACRO src
    dataLoad src, ymm, <0, 1, 2, 3, 4, 5, 6, 7>
ENDM

hadlfLoadW MACRO src
    dataLoad src, wmm, <0, 1, 2, 3>
ENDM

fullStoreY MACRO dst		;store all 64 sorted item from ymm0-7 to memory
    dataStore ymm, dst, <0, 1, 2, 3, 4, 5, 6, 7>
ENDM

halfStoreW MACRO dst		;store 4 rows in ymm8-12
    dataStore wmm, dst, <0, 1, 2, 3>
ENDM

;in order compare two ymm registers that the distant of them is dis
compareInOder MACRO src, dst, dis
    local j, k
    ; j TEXTEQU <>
    ; for i, <0, 1, 2, 3, 4, 5, 6, 7>
    ;     if ((i + dis) LE 7) ;and (@instr(, j, %i) EQ 0)
    ;         vpminsd &dst&&i, &src&&i, @catstr(src, %(i + dis))
    ;         ; j catstr j, %i, %(i + dis)
    ;         j catstr j, %(i + dis)
    ;     endif
    ; endm
    ; j TEXTEQU <>
    ; for i, <0, 1, 2, 3, 4, 5, 6, 7>
    ;     if (i GE dis) ;and (@instr(, j, %(i - dis)) EQ 0)
    ;         vpmaxsd &dst&&i, @catstr(src, %(i - dis)), &src&&i
    ;         ; j catstr j, %(i - dis), %i
    ;         j catstr j, %i
    ;     endif
    ; endm
    j = dis
    k = 0
    for i, <0, 1, 2, 3, 4, 5, 6, 7>
    	if i GE k and i LT j
    		echo ---------------success two!
    	endif
    	if i EQ j
    		k = j + dis
    		j = k + dis
    	endif
    endm
    j = 0
    while j LT 8
        echo ----------------success!
        ;vpminsd &dst&&j, &src&&j, &src&&j;@catstr(src, %(j+1))
        vpminsd @catstr(dst, %j), @catstr(src, %j), @catstr(src, %(j+1))
        j = j + 1
    endm
    ; k = j + dis
    ; for i, <0, 1, 2, 3, 4, 5, 6>
    ; 	j catstr <src>, %i
    ; 	k TEXTEQU %(i + 1)
    ; 	vpminsd &dst&&i, &src&&i, j
    ; endm
ENDM

;compare two ymm registers that the sum of their inices are equal to dis
compareSqueeze MACRO src, dst, dis
    local j
    j TEXTEQU <>
    for i, <0, 1, 2, 3, 4, 5, 6, 7>
        if not @instr(, j, %(dis - i))
            vpminsd &dst&&i, &src&&i, @catstr(src, %(dis - i))
            j catstr j, %(dis - i)
        endif
    endm
ENDM

compareDis1 MACRO			;(1,2),(3,4) compare, source ymm0-7, des ymm8-15
    compareInOder ymm, wmm, 1
    ; for i, <0, 2, 4, 6>
    ;     vpminsd wmm&i, ymm&i, @catstr(ymm, %(i + 1))
    ; ENDM
    ; for i, <1, 3, 5, 7>
    ;     vpmaxsd wmm&i, @catstr(ymm, %(i - 1)), ymm&i
    ; ENDM
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
    vpminsd ymm0, wmm0, wmm1
    vpminsd ymm2, wmm2, wmm3
    vpminsd ymm4, wmm4, wmm5
    vpminsd ymm6, wmm6, wmm7
    vpmaxsd ymm1, wmm0, wmm1
    vpmaxsd ymm3, wmm2, wmm3
    vpmaxsd ymm5, wmm4, wmm5
    vpmaxsd ymm7, wmm6, wmm7
ENDM

compareDis3_1Rev MACRO		;(8,11),(9,10) compare, source ymm8-15, des ymm0-7
    vpminsd ymm0, wmm0, wmm3
    vpminsd ymm1, wmm1, wmm2
    vpminsd ymm4, wmm4, wmm7
    vpminsd ymm5, wmm5, wmm6
    vpmaxsd ymm3, wmm0, wmm3
    vpmaxsd ymm2, wmm1, wmm2
    vpmaxsd ymm7, wmm4, wmm7
    vpmaxsd ymm6, wmm5, wmm6
ENDM

compareDis7_1 MACRO			;(0,7),(1,6) compare, source ymm0-7, des ymm8-15
    vpminsd wmm0, ymm0, ymm7
    vpminsd wmm1, ymm1, ymm6
    vpminsd wmm2, ymm2, ymm5
    vpminsd wmm3, ymm3, ymm4
    vpmaxsd wmm7, ymm0, ymm7
    vpmaxsd wmm6, ymm1, ymm6
    vpmaxsd wmm5, ymm2, ymm5
    vpmaxsd wmm4, ymm3, ymm4
ENDM

compareDis7_1Rev MACRO		;(8,15),(9,14) compare, source ymm8-15, des ymm0-7
    vpminsd ymm0, wmm0, wmm7
    vpminsd ymm1, wmm1, wmm6
    vpminsd ymm2, wmm2, wmm5
    vpminsd ymm3, wmm3, wmm4
    vpmaxsd ymm7, wmm0, wmm7
    vpmaxsd ymm6, wmm1, wmm6
    vpmaxsd ymm5, wmm2, wmm5
    vpmaxsd ymm4, wmm3, wmm4
ENDM

compareDis2 MACRO			;(0,2),(1,3) compare, source ymm0-7, des ymm8-15
    vpminsd wmm0, ymm0, ymm2
    vpminsd wmm1, ymm1, ymm3
    vpminsd wmm4, ymm4, ymm6
    vpminsd wmm5, ymm5, ymm7
    vpmaxsd wmm2, ymm0, ymm2
    vpmaxsd wmm3, ymm1, ymm3
    vpmaxsd wmm6, ymm4, ymm6
    vpmaxsd wmm7, ymm5, ymm7
ENDM

compareDis2Rev MACRO		;(8,10),(9,11) compare, source ymm8-15, des ymm0-7
    vpminsd ymm0, wmm0, wmm2
    vpminsd ymm1, wmm1, wmm3
    vpminsd ymm4, wmm4, wmm6
    vpminsd ymm5, wmm5, wmm7
    vpmaxsd ymm2, wmm0, wmm2
    vpmaxsd ymm3, wmm1, wmm3
    vpmaxsd ymm6, wmm4, wmm6
    vpmaxsd ymm7, wmm5, wmm7
    ; compareInOder wmm, ymm, 2
ENDM

compareDis4 MACRO			;(0,4),(1,5) compare, source ymm0-7, des ymm8-15
    vpminsd wmm0, ymm0, ymm4
    vpminsd wmm1, ymm1, ymm5
    vpminsd wmm2, ymm2, ymm6
    vpminsd wmm3, ymm3, ymm7
    vpmaxsd wmm4, ymm0, ymm4
    vpmaxsd wmm5, ymm1, ymm5
    vpmaxsd wmm6, ymm2, ymm6
    vpmaxsd wmm7, ymm3, ymm7
ENDM

compareDis4Rev MACRO		;(8,12),(9,13) compare, source ymm8-15, des ymm0-7
    vpminsd ymm0, wmm0, wmm4
    vpminsd ymm1, wmm1, wmm5
    vpminsd ymm2, wmm2, wmm6
    vpminsd ymm3, wmm3, wmm7
    vpmaxsd ymm4, wmm0, wmm4
    vpmaxsd ymm5, wmm1, wmm5
    vpmaxsd ymm6, wmm2, wmm6
    vpmaxsd ymm7, wmm3, wmm7
ENDM

initialSort MACRO			;initial column sort, for unsorted lists
    compareDis1
    ; compareDis3_1Rev
    ; compareDis1
    ; compareDis7_1Rev
    ; compareDis2
    ; compareDis1Rev
ENDM

shuffle4_7 MACRO imm	;shuffle ymm4-7
    vpshufd ymm4, ymm4, imm
    vpshufd ymm5, ymm5, imm
    vpshufd ymm6, ymm6, imm
    vpshufd ymm7, ymm7, imm
ENDM

shuffle4_7Rev MACRO imm 	;shuffle ymm12-15
    vpshufd wmm4, wmm4, imm
    vpshufd wmm5, wmm5, imm
    vpshufd wmm6, wmm6, imm
    vpshufd wmm7, wmm7, imm
ENDM

shuffleEchg0_7 MACRO imm1, imm2 	;shuffle ymm0-7, des ymm8-15
    vpshufd wmm0, ymm0, imm1
    vpshufd wmm1, ymm1, imm1
    vpshufd wmm2, ymm2, imm1
    vpshufd wmm3, ymm3, imm1
    vpshufd wmm4, ymm4, imm2
    vpshufd wmm5, ymm5, imm2
    vpshufd wmm6, ymm6, imm2
    vpshufd wmm7, ymm7, imm2
ENDM

shuffleEchg0_7SameZ MACRO imm1, imm2 ;shuffle same like above within same wmms
    vpshufd wmm0, wmm0, imm1
    vpshufd wmm1, wmm1, imm1
    vpshufd wmm2, wmm2, imm1
    vpshufd wmm3, wmm3, imm1
    vpshufd wmm4, wmm4, imm2
    vpshufd wmm5, wmm5, imm2
    vpshufd wmm6, wmm6, imm2
    vpshufd wmm7, wmm7, imm2
ENDM

shuffleEchg0_7Rev MACRO imm1, imm2 	;shuffle ymm8-15, des ymm0-4
    vpshufd ymm0, wmm0, imm1
    vpshufd ymm1, wmm1, imm1
    vpshufd ymm2, wmm2, imm1
    vpshufd ymm3, wmm3, imm1
    vpshufd ymm4, wmm4, imm2
    vpshufd ymm5, wmm5, imm2
    vpshufd ymm6, wmm6, imm2
    vpshufd ymm7, wmm7, imm2
ENDM

blendDis4 MACRO imm1, imm2 ;blend (0,4),(1,5), des ymm8-15
    vpblendd wmm0, ymm0, ymm4, imm1
    vpblendd wmm1, ymm1, ymm5, imm1
    vpblendd wmm2, ymm2, ymm6, imm1
    vpblendd wmm3, ymm3, ymm7, imm1
    vpblendd wmm4, ymm0, ymm4, imm2
    vpblendd wmm5, ymm1, ymm5, imm2
    vpblendd wmm6, ymm2, ymm6, imm2
    vpblendd wmm7, ymm3, ymm7, imm2
ENDM

blendDis4Rev MACRO imm1, imm2 	;blend (8,12),(9,13), des ymm0-7
    vpblendd ymm0, wmm0, wmm4, imm1
    vpblendd ymm1, wmm1, wmm5, imm1
    vpblendd ymm2, wmm2, wmm6, imm1
    vpblendd ymm3, wmm3, wmm7, imm1
    vpblendd ymm4, wmm0, wmm4, imm2
    vpblendd ymm5, wmm1, wmm5, imm2
    vpblendd ymm6, wmm2, wmm6, imm2
    vpblendd ymm7, wmm3, wmm7, imm2
ENDM

blendDis7_1 MACRO imm1, imm2 ;blend (0,7),(1,6), des ymm8-15
    vpblendd wmm0, ymm0, ymm7, imm1
    vpblendd wmm1, ymm1, ymm6, imm1
    vpblendd wmm2, ymm2, ymm5, imm1
    vpblendd wmm3, ymm3, ymm4, imm1
    vpblendd wmm7, ymm0, ymm7, imm2
    vpblendd wmm6, ymm1, ymm6, imm2
    vpblendd wmm5, ymm2, ymm5, imm2
    vpblendd wmm4, ymm3, ymm4, imm2
ENDM

blendDis7_1Rev MACRO imm1, imm2	; blend (8,15),(9,14), des ymm0-7
    vpblendd ymm0, wmm0, wmm7, imm1
    vpblendd ymm1, wmm1, wmm6, imm1
    vpblendd ymm2, wmm2, wmm5, imm1
    vpblendd ymm3, wmm3, wmm4, imm1
    vpblendd ymm7, wmm0, wmm7, imm2
    vpblendd ymm6, wmm1, wmm6, imm2
    vpblendd ymm5, wmm2, wmm5, imm2
    vpblendd ymm4, wmm3, wmm4, imm2
ENDM

permuteDis4S MACRO imm  			;permute to get left or right 4 columns
    vperm2i128 wmm0, ymm0, ymm4, imm
    vperm2i128 wmm1, ymm1, ymm5, imm
    vperm2i128 wmm2, ymm2, ymm6, imm
    vperm2i128 wmm3, ymm3, ymm7, imm
ENDM

permute4_7Rev MACRO imm 		;permute ymm12-15
    vperm2i128 wmm4, wmm4, wmm4, imm
    vperm2i128 wmm5, wmm5, wmm5, imm
    vperm2i128 wmm6, wmm6, wmm6, imm
    vperm2i128 wmm7, wmm7, wmm7, imm
ENDM

halfPermute MACRO imm
    vperm2i128 wmm0, ymm0, ymm1, imm
    vperm2i128 wmm1, ymm2, ymm3, imm
    vperm2i128 wmm2, ymm4, ymm6, imm
    vperm2i128 wmm3, ymm6, ymm7, imm
ENDM

columnPermute MACRO imm1, imm2
    vperm2i128 ymm0, wmm0, ymm0, imm1
    vperm2i128 ymm2, wmm1, ymm2, imm1
    vperm2i128 ymm4, wmm2, ymm4, imm1
    vperm2i128 ymm6, wmm3, ymm6, imm1
    vperm2i128 ymm1, wmm0, ymm1, imm2
    vperm2i128 ymm3, wmm1, ymm3, imm2
    vperm2i128 ymm5, wmm2, ymm5, imm2
    vperm2i128 ymm7, wmm3, ymm7, imm2
ENDM

permuteDis4 MACRO imm1, imm2   ;permute (0,4),(1,5), des ymm8-15
    vperm2i128 wmm0, ymm0, ymm4, imm1
    vperm2i128 wmm1, ymm1, ymm5, imm1
    vperm2i128 wmm2, ymm2, ymm6, imm1
    vperm2i128 wmm3, ymm3, ymm7, imm1
    vperm2i128 wmm4, ymm0, ymm4, imm2
    vperm2i128 wmm5, ymm1, ymm5, imm2
    vperm2i128 wmm6, ymm2, ymm6, imm2
    vperm2i128 wmm7, ymm3, ymm7, imm2
ENDM

permuteDis4Rev MACRO imm1, imm2  ;permute (8,12),(9,13), dst ymm0-7
    vperm2i128 ymm0, wmm0, wmm4, imm1	
    vperm2i128 ymm1, wmm1, wmm5, imm1
    vperm2i128 ymm2, wmm2, wmm6, imm1
    vperm2i128 ymm3, wmm3, wmm7, imm1
    vperm2i128 ymm4, wmm0, wmm4, imm2
    vperm2i128 ymm5, wmm1, wmm5, imm2
    vperm2i128 ymm6, wmm2, wmm6, imm2
    vperm2i128 ymm7, wmm3, wmm7, imm2
ENDM

invertPermute47 MACRO src1, src2 	;invert items in ymm4-7
    vpermd ymm4, src1, ymm4
    vpermd ymm5, src2, ymm5
    vpermd ymm6, src1, ymm6
    vpermd ymm7, src2, ymm7
ENDM

transportation MACRO dst		;transport 4 sorted columns into rows
    vpshufd  wmm1, wmm1, 177
    vpshufd  wmm3, wmm3, 177
    vpblendd wmm6, wmm2, wmm3, 170
    vpblendd wmm7, wmm2, wmm3, 85
    vpblendd wmm5, wmm0, wmm1, 85
    vpblendd wmm4, wmm0, wmm1, 170
    vpshufd  wmm6, wmm6, 78
    vpshufd  wmm7, wmm7, 78
    vpblendd wmm0, wmm4, wmm6, 204
    vpblendd wmm1, wmm5, wmm7, 204
    vpblendd wmm2, wmm4, wmm6, 51
    vpblendd wmm3, wmm5, wmm7, 51
    vpshufd  wmm1, wmm1, 177
    vpshufd  wmm2, wmm2, 78
    vpshufd  wmm3, wmm3, 27
    halfStoreW dst
ENDM

leftEndTransport MACRO	dst		;transport left four columns
    permuteDis4S 32
    transportation dst
ENDM

leftPartialTransport MACRO dst
    halfPermute 32
    halfStoreW dst
ENDM

rightEndTransport MACRO dst		;transport right four columns
    permuteDis4S 49
    transportation dst
ENDM

rightPartialTransport MACRO dst
    halfPermute 49
    halfStoreW dst
ENDM

leftLoad MACRO src
    hadlfLoadW src
    columnPermute 48, 49
ENDM

rightLoad MACRO src
    hadlfLoadW src
    columnPermute 2, 18
ENDM

leftHalfGather MACRO  ;load sorted 32 items to left four columns
    vpxor	   wmm6, wmm6, wmm6
    vpxor 	   wmm7, wmm7, wmm7
    vpcmpeqd   wmm6, wmm6, wmm5
    vpcmpeqd   wmm7, wmm7, wmm5
    vpgatherdd ymm0, [rax + wmm4 * 8],       wmm6
    vpcmpeqd   wmm6, wmm6, wmm5
    vpgatherdd ymm1, [rax + wmm4 * 8 + 4],   wmm7
    vpcmpeqd   wmm7, wmm7, wmm5
    vpgatherdd ymm2, [rax + wmm4 * 8 + 8],   wmm6
    vpcmpeqd   wmm6, wmm6, wmm5
    vpgatherdd ymm3, [rax + wmm4 * 8 + 12],  wmm7
    vpcmpeqd   wmm7, wmm7, wmm5
    vpgatherdd ymm4, [rax + wmm4 * 8 + 16],  wmm6
    vpcmpeqd   wmm6, wmm6, wmm5
    vpgatherdd ymm5, [rax + wmm4 * 8 + 20],  wmm7
    vpcmpeqd   wmm7, wmm7, wmm5
    vpgatherdd ymm6, [rax + wmm4 * 8 + 24],  wmm6
    vpgatherdd ymm7, [rax + wmm4 * 8 + 28],  wmm7
ENDM

rightHalfGather MACRO  ;load sorted 32 items to right four columns
    vpxor	   wmm6, wmm6, wmm6
    vpxor 	   wmm7, wmm7, wmm7
    vpor       wmm6, wmm6, wmm5
    vpor   	   wmm7, wmm7, wmm5
    vpgatherdd ymm0, [rax + wmm4 * 8],       wmm6
    vpor       wmm6, wmm6, wmm5
    vpgatherdd ymm1, [rax + wmm4 * 8 + 4],   wmm7
    vpor       wmm7, wmm7, wmm5
    vpgatherdd ymm2, [rax + wmm4 * 8 + 8],   wmm6
    vpor       wmm6, wmm6, wmm5
    vpgatherdd ymm3, [rax + wmm4 * 8 + 12],  wmm7
    vpor       wmm7, wmm7, wmm5
    vpgatherdd ymm4, [rax + wmm4 * 8 + 16],  wmm6
    vpor       wmm6, wmm6, wmm5
    vpgatherdd ymm5, [rax + wmm4 * 8 + 20],  wmm7
    vpor       wmm7, wmm7, wmm5
    vpgatherdd ymm6, [rax + wmm4 * 8 + 24],  wmm6
    vpgatherdd ymm7, [rax + wmm4 * 8 + 28],  wmm7
ENDM

end3Sort MACRO ;last three step in each stage
    compareDis4
    compareDis2Rev
    compareDis1AND y, w
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
    vmovdqa wmm0, ymmword ptr [invertp]
    vmovdqa wmm1, ymmword ptr [invertp]
    invertPermute47 wmm0, wmm1
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

AVXBitonicSortMacro MACRO stage
    local Sort
Sort:	
    fullLoadY rcx ;fullLoad data to ymm0-7
    initialSort
    ; sort8_32
    ; sort32_64
    ; left&stage& rdx
    ; add rdx, 128
    ; right&stage& rdx    
    ; add rdx, 128
    ; fullStoreY rdx
    dataStore wmm, rdx, <0, 1, 2, 3, 4, 5, 6, 7>
    add rcx, 256
    add rdx, 256
    dec r8
    jnz Sort
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
    ; cmp r8, 1
    ; je  OnlyOne
    AVXBitonicSortMacro PartialTransport
    ; jmp ABTerminal
; OnlyOne:
;     AVXBitonicSortMacro EndTransport
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
    ; vmovdqa wmm5, ymmword ptr [masknum]
    ; leftHalfGather
    rightLoad rcx
    add 	  rcx,  128
    leftLoad  rbx
    add 	  rbx,  128
Merging:
    sort32_64p
    ; fullStoreY rdx
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
    leftEndTransport rdx
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
    rightEndTransport rdx
    ret
AVXMergeSortEnd ENDP

;rcx input address, rdx output address, r8 length of copy bytes
align 16
OddCopy PROC
    mov rax,  r8
    shr rax,  6
CopyLoop:
    fullLoadY  rcx
    fullStoreY rdx
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
