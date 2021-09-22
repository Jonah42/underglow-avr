#define __SFR_OFFSET 0 
#include <avr/io.h> 

;extern void output_grb(u8 * ptr, u16 count)
;
; r18 = data byte
; r19 = 7-bit count
; r20 = 1 output
; r21 = 0 output
; r22 = SREG save
; r24:25 = 16-bit count
; r26:27 (X) = data pointer

.equ      OUTBIT,   0
.equ      DIV,   12


.global output_grb_reduced
output_grb_reduced:
        mov   r26, r24      ;r26:27 = X = p_buf
        mov   r27, r25
        mov   r24, r22      ;r24:25 = count
        mov   r25, r23
        ;; r23 byte
        ;; r30 rep
        ldi r23, 3
        ldi r30, DIV
        ;cpi   r24, 3
        ;breq  setled
        ;ldi   r22, 2
        ;nop
        ;out   PORTC, r22
        ;ret
setled: 
        in     r22, SREG     ;save SREG (global int state)
        cli                  ;no interrupts from here on, we're cycle-counting
        in     r20, PORTC
        ori    r20, (1<<OUTBIT)         ;our '1' output
        in     r21, PORTC
        andi   r21, ~(1<<OUTBIT)        ;our '0' output
        ldi    r19, 7        ;7 bit counter (8th bit is different)
        ld     r18,X+        ;get first data byte
loop1:
        out    PORTC, r20    ; 1   +0 start of a bit pulse
        lsl    r18           ; 1   +1 next bit into C, MSB first
        nop                  ; 1   +2
        nop                  ; 1   +3
        brcs   L1            ; 1/2 +4 branch if 1
        out    PORTC, r21    ; 1   +5 end hi for '0' bit (5 clocks hi)
        nop                  ; 1   +6
        bst    r18, 7        ; 1   +7 save last bit of data for fast branching
        subi   r19, 1        ; 1   +8 how many more bits for this byte?
        nop                  ; 1   +9
        nop                  ; 1   +10
        nop                  ; 1   +11
        breq   bit8          ; 1/2 +12 last bit, do differently
        rjmp   loop1         ; 2   +13, 15 total for 0 bit
L1:
        nop                  ; 1   +6
        nop                  ; 1   +7
        bst    r18, 7        ; 1   +8 save last bit of data for fast branching
        subi   r19, 1        ; 1   +9 how many more bits for this byte
        out    PORTC, r21    ; 1   +10 end hi for '1' bit (10 clocks hi)
        nop                  ; 1   +11
        nop                  ; 1   +12
        brne   loop1         ; 2/1 +13 15 total for 1 bit (fall thru if last bit)
bit8:
        ldi    r19, 7        ; 1   +14 bit count for next byte
        out    PORTC, r20    ; 1   +0 start of a bit pulse
        ;brts   L2            ; 1/2 +1 branch if last bit is a 1
        subi r30, 1          ; 1   +1 decrement rep
        brne s1              ; 1/2 +2 branch if rep is not 0 (proceed to either next byte or reset to start of colour)
        ldi r23, 3           ; 1   +3 reset byte
        ldi r30, DIV           ; 1   +4 reset rep
        out    PORTC, r21    ; 1   +5 end hi for '0' bit (5 clocks hi)
        nop                  ; 1   +6
        nop                  ; 1   +7
        nop                  ; 1   +8
        ld     r18, X+       ; 2   +9 fetch next byte
        sbiw   r24, 1        ; 2   +11 dec byte counter
        brne   loop1         ; 2   +13 loop back or return
        out    SREG, r22     ; restore global int flag
        ret
s1:
        subi r23, 1          ; 1   +4 decrement byte
        out    PORTC, r21    ; 1   +5 end hi for '0' bit (5 clocks hi)
        brne s2              ; 1/2 +6 branch if byte not 0 (just normal increment then)
        ldi r23, 3           ; 1   +7 reset byte
        sbiw r26, 3          ; 2   +8 sub 3 from X
        nop                  ; 1   +10
        ld     r18, X+       ; 2   +11 fetch next byte
        brne   loop1         ; 2   +13 loop back or return
        out    SREG, r22     ; restore global int flag
        ret
s2:
        nop                  ; 1   +8
        nop                  ; 1   +9
        nop                  ; 1   +10
        ld     r18, X+       ; 2   +11 fetch next byte
        brne   loop1         ; 2   +13 loop back or return
        out    SREG, r22     ; restore global int flag
        ret