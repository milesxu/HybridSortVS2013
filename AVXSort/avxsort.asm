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
    endm
ENDM

; very similar with above macro, store data from ymm register to memory
dataStore MACRO src, dst, num
    for i, <num>
        vmovdqa ymmword ptr [dst + i * 32], &src&&i
    endm
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
compareInOrder MACRO src, dst, dis
    local j, k
    k = 0
    j = dis
    for i, <0, 1, 2, 3, 4, 5, 6, 7>
        if i GE k and i LT j
            vpminsd &dst&&i, &src&&i, @catstr(src, %(i + dis))
        elseif i EQ j
            k = j + dis
            j = k + dis
        endif
    endm
    k = dis
    j = dis + k
    for i, <0, 1, 2, 3, 4, 5, 6, 7>
        if i GE k and i LT j
            vpmaxsd &dst&&i, @catstr(src, %(i - dis)), &src&&i
        elseif i EQ j
            k = j + dis
            j = k + dis
        endif
    endm
ENDM

;compare two ymm registers that the sum of their inices are equal to dis
compareSqueeze MACRO src, dst, dis
    local j, k, m
    k = 0
    j = dis / 2
    for i, <0, 1, 2, 3, 4, 5, 6, 7>
        if i GE k and i LT j
            vpminsd &dst&&i, &src&&i, @catstr(src, %(j * 2 - i - 1))
        elseif i EQ j
            k = k + dis
            j = j + dis
        endif
    endm
    k = 0
    j = dis / 2
    for i, <0, 1, 2, 3, 4, 5, 6, 7>
        if i GE k and i LT j
            m catstr <dst>, %(j * 2 - i - 1)
            vpmaxsd m, &src&&i, @catstr(src, %(j * 2 - i - 1))
        elseif i EQ j
            k = k + dis
            j = j + dis
        endif
    endm
ENDM

initialSort MACRO			;initial column sort, for unsorted lists
    compareInOrder ymm, wmm, 1
    compareSqueeze wmm, ymm, 4
    compareInOrder ymm, wmm, 1
    compareSqueeze wmm, ymm, 8
    compareInOrder ymm, wmm, 2
    compareInOrder wmm, ymm, 1
ENDM

shuffleR MACRO src, dst, imm, num
    for i, <num>
        vpshufd &dst&&i, &src&&i, imm
    endm
ENDM

blendInOrder MACRO src, dst, dis, imm1, imm2
    local j, k
    k = 0
    j = dis
    for i, <0, 1, 2, 3, 4, 5, 6, 7>
        if i GE k and i LT j
            vpblendd &dst&&i, &src&&i, @catstr(src, %(i + dis)), imm1
        elseif i EQ j
            k = j + dis
            j = k + dis
        endif
    endm
    k = 0
    j = dis
    for i, <0, 1, 2, 3, 4, 5, 6, 7>
        if (i - dis) GE k and (i - dis) LT j
            vpblendd &dst&&i, @catstr(src, %(i - dis)), &src&&i, imm2
        elseif i EQ j
            k = j + dis
            j = k + dis
        endif
    endm
ENDM

blendSqueeze MACRO src, dst, dis, imm1, imm2
    local j, k, m
    k = 0
    j = dis / 2
    for i, <0, 1, 2, 3, 4, 5, 6, 7>
        if i GE k and i LT j
            vpblendd &dst&&i, &src&&i, @catstr(src, %(j * 2 - i - 1)), imm1
        elseif i EQ j
            k = k + dis
            j = j + dis
        endif
    endm
    k = 0
    j = dis / 2
    for i, <0, 1, 2, 3, 4, 5, 6, 7>
        if i GE k and i LT j
            m catstr <dst>, %(j * 2 - i - 1)
            vpblendd m, &src&&i, @catstr(src, %(j * 2 - i - 1)), imm2
        elseif i EQ j
            k = k + dis
            j = j + dis
        endif
    endm
ENDM

permuteRUp MACRO src, dst, dis, imm
    local j, k
    k = 0
    j = dis
    for i, <0, 1, 2, 3, 4, 5, 6, 7>
        if i GE k and i LT j
            vperm2i128 &dst&&i, &src&&i, @catstr(src, %(i + dis)), imm
        elseif i EQ j
            k = j + dis
            j = k + dis
        endif
    endm
ENDM

permuteRDown MACRO src, dst, dis, imm
    local j, k
    k = dis
    j = dis + k
    for i, <0, 1, 2, 3, 4, 5, 6, 7>
        if i GE k and i LT j
            vperm2i128 &dst&&i, @catstr(src, %(i - dis)), &src&&i, imm
        elseif i EQ j
            k = j + dis
            j = k + dis
        endif
    endm
ENDM

permuteRUpMultiple MACRO src, dst, dis, imm
    local j, k, m, n
    m = 0
    k = 0
    j = dis
    for i, <0, 1, 2, 3, 4, 5, 6, 7>
        if i GE k and i LT j
            n catstr <dst>, %m
            vperm2i128 n, &src&&i, @catstr(src, %(i + dis)), imm
            m = m + 1
        elseif i EQ j
            k = j + dis
            j = k + dis
        endif
    endm
ENDM

permuteRSame MACRO src, dst, imm, num
    for i, <num>
        vperm2i128 &dst&&i, &src&&i, &src&&i, imm
    endm
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

invertPermute47 MACRO reg, d, src1, src2 	;invert items in ymm4-7
    local i
    i = 0
    while i LT 4
        vpermd @catstr(reg, %(i+d)), src1, @catstr(reg, %(i+d))
        vpermd @catstr(reg, %(i+d+1)), src2, @catstr(reg, %(i+d+1))
        i = i + 2
    endm
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
    permuteRUpMultiple ymm, wmm, 4, 32
    transportation dst
ENDM

leftPartialTransport MACRO dst
    permuteRUpMultiple ymm, wmm, 1, 32
    halfStoreW dst
ENDM

rightEndTransport MACRO dst		;transport right four columns
    permuteRUpMultiple ymm, wmm, 4, 49
    transportation dst
ENDM

rightPartialTransport MACRO dst
    permuteRUpMultiple ymm, wmm, 1, 49
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
    compareInOrder ymm, wmm, 4
    compareInOrder wmm, ymm, 2
    compareInOrder ymm, wmm, 1
ENDM

sort8_32 MACRO			;sort 4 columns
    shuffleR ymm, ymm, 177, <4, 5, 6, 7>
    compareSqueeze ymm, wmm, 8
    blendSqueeze wmm, ymm, 8, 170, 85
    shuffleR ymm, ymm, 177, <4, 5, 6, 7>
    end3Sort 		;each 16 items sorted
    shuffleR wmm, wmm, 27, <4, 5, 6, 7>
    compareSqueeze wmm, ymm, 8
    blendInOrder ymm, wmm, 4, 170, 85
    shuffleR wmm, wmm, 177, <4, 5, 6, 7>
    compareInOrder wmm, ymm, 4
    blendSqueeze ymm, wmm, 8, 204, 51
    shuffleR wmm, ymm, 216, <0, 1, 2, 3>
    shuffleR wmm, ymm, 114, <4, 5, 6, 7>
    end3Sort		;each 32 items sorted
ENDM

sort32_64 MACRO  ;sort all 8 columns
    permuteRSame wmm, wmm, 1, <4, 5, 6, 7>
    shuffleR wmm, wmm, 27, <4, 5, 6, 7>
    compareSqueeze wmm, ymm, 8
    permuteRUp   ymm, wmm, 4, 32
    permuteRDown ymm, wmm, 4, 49
    blendInOrder wmm, ymm, 4, 102, 153
    shuffleR ymm, ymm, 78, <4, 5, 6, 7>
    compareInOrder ymm, wmm, 4
    blendInOrder wmm, ymm, 4, 170, 85
    shuffleR ymm, ymm, 27, <4, 5, 6, 7>
    compareInOrder ymm, wmm, 4
    blendSqueeze wmm, ymm, 8, 102, 153
    shuffleR ymm, wmm, 120, <0, 1, 2, 3>
    shuffleR ymm, wmm, 210, <4, 5, 6, 7>
    compareInOrder wmm, ymm, 4
    compareInOrder ymm, wmm, 2
    compareInOrder wmm, ymm, 1
ENDM

sort32_64p MACRO
    vmovdqa wmm0, ymmword ptr [invertp]
    vmovdqa wmm1, ymmword ptr [invertp]
    invertPermute47 ymm, 4, wmm0, wmm1
    compareSqueeze ymm, wmm, 8
    permuteRUp   wmm, ymm, 4, 32
    permuteRDown wmm, ymm, 4, 49
    blendInOrder ymm, wmm, 4, 102, 153
    shuffleR wmm, wmm, 78, <4, 5, 6, 7>
    compareInOrder wmm, ymm, 4
    blendInOrder ymm, wmm, 4, 170, 85
    shuffleR wmm, wmm, 27, <4, 5, 6, 7>
    compareInOrder wmm, ymm, 4
    blendSqueeze ymm, wmm, 8, 102, 153
    shuffleR wmm, wmm, 120, <0, 1, 2, 3>
    shuffleR wmm, wmm, 210, <4, 5, 6, 7>
    compareInOrder wmm, ymm, 4
    compareInOrder ymm, wmm, 2
    compareInOrder wmm, ymm, 1
ENDM

AVXBitonicSortMacro MACRO stage
    local Sort
Sort:	
    fullLoadY rcx ;fullLoad data to ymm0-7
    initialSort
    sort8_32
    sort32_64
    left&stage& rdx
    add rdx, 128
    right&stage& rdx    
    ; fullStoreY rdx
    ; dataStore wmm, rdx, <0, 1, 2, 3, 4, 5, 6, 7>
    add rcx, 256
    add rdx, 128
    ; add rdx, 256
    dec r8
    jnz Sort
ENDM

AVXMergeSortMacro MACRO stage
    local Merging, FirstLoad, SecondLoad, Terminal
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
    left&stage& rdx
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
    right&stage& rdx
ENDM

CoreSortStage1CallMacro MACRO stage
    ; push  rax
    mov   rcx, r13
    mov   rdx, r12
    push  rbx
    push  r10
    push  r11
    sub   rsp, 28h
    ; mov   dword ptr [rsp + 20h], eax
    call  &stage
    add   rsp, 28h
    pop   r11
    pop   r10
    pop   rbx
    ; pop   rax
ENDM

CoreSortStage1LoopMacro MACRO stage
    local CMergeLoop, CNormalMerge, OnlyCopy, CLoopJudge
CMergeLoop: 
    mov   r9,  r10
    cmp   r11, 2
    ja    CNormalMerge
    ; push  rax
    mov   rax, r10
    shl   rax, 1
    mul   r14
    mov   rcx, r15
    sub   rcx, rax
    ; pop   rax
    cmp   r11, 1
    je    OnlyCopy    
    sub   rcx, r10
    mov   r9,  rcx
CNormalMerge:
    mov   r8,  r10
    mov   rbx, r8
    add   rbx, r9
    CoreSortStage1CallMacro &stage
    sub   r11, 2
    jmp   CLoopJudge
OnlyCopy:
    mov   r8,  rcx
    mov   rbx, r8
    shr   r8,  2
    CoreSortStage1CallMacro OddCopy
    sub   r11, 1
CLoopJudge:
    inc   r14
    add   r12, rbx
    add   r13, rbx
    cmp   r11, 0
    ; je    AddressExchange
    ; jmp   CMergeLoop
    ja    CMergeLoop
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
    cmp r8, 1
    je  OnlyOne
    AVXBitonicSortMacro PartialTransport
    jmp ABTerminal
OnlyOne:
    AVXBitonicSortMacro EndTransport
ABTerminal:
    ret
AVXBitonicSort ENDP

;rcx input address, rdx output address, r8 length of one list, r9 length of 
;the other list
align 16
AVXMergeSort PROC uses r12		
    ; shl     r8,   2  	;byte length of input 1
    ; shl     r9,   2	;byte length of input 2 which follow input 1
    AVXMergeSortMacro PartialTransport
    ret
AVXMergeSort ENDP

;rcx input address, rdx output address, r8 length of one list, r9 length of 
;the other list, the last time in loop
align 16
AVXMergeSortEnd PROC uses r12		
    ; shl     r8,   2  	;byte length of input 1
    ; shl     r9,   2	;byte length of input 2 which follow input 1
    AVXMergeSortMacro EndTransport
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
    mov   r12, rcx		;input address backup
    mov   r13, rdx		;output address backup
    mov   r14, r8		;total length backup
    lzcnt rax, r8		;get the leading zero bits in r8, store into rax
    popcnt rbx,r8       ;if num in r8 is power of 2, then rbx <- 1
    cmp   rbx, 1        ;if num is not power of 2, then get the min power of 2
    je    BitsSet       ;that greater than it. So rbx <- 63.
    xor   rbx, rbx
BitsSet:    
    add   rbx, 63		
    sub   rbx, rax		;get the loop times needed
    test  rbx, 1		;decide whether need input address equal output address in FirstStep
    jz    FirstStep
    mov   r13, rcx
    mov   r12, rdx
    mov   rdx, rcx 		;if need input address equal output address
FirstStep:
    call  AVXBitonicSort
    add   rsp, 28h
    cmp   r14, 1
    je    CTerminal 	;only 64 items, sort complete.
    mov   r15, r14		;backup of num of blocks to be merged.
    shl   r15, 8		;total length in byte
    mov   r10, 256		;unit length
    mov   r11, r14
    xor   r14, r14
    cmp   r11, 2
    je    CoreSortLastTime
CoreSortNormal:
    CoreSortStage1LoopMacro AVXMergeSort
    jmp   AddressExchange
CoreSortLastTime:
    CoreSortStage1LoopMacro AVXMergeSortEnd
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
    cmp   r11, 2
    je    CoreSortLastTime
    jmp   CoreSortNormal
CTerminal:
    ret
CoreSortStage1 ENDP

align 16
CoreSortStage2 PROC
CoreSortStage2 ENDP

end
