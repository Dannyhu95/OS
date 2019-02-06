BITS 16     ; Use 16-bit assembly
ORG 0x7C00  ; Assume code will be loaded at 0x7c00

start:
    mov ah, 0x00    ; 0x00 - set video mode
    mov al, 0x03    ; 0x03 - 80x25 text mode  
    int 0x10   
get_message:
    mov si, message ; load the offset of our message into si
    
print_char:
    lodsb           ; loads a single byte from [si] into al and increments si
    test al, al     ; checks to see if the byte is 0
    jz input_char       ; if so, jump out (jz jumps if ZF in EFLAGS is set)
    mov ah, 0x0E    ; 0x0E is the BIOS code to print the single character
    int 0x10        ; call into the BIOS using a software interrupt
    jmp print_char  ; go back to the start of the loop
    
input_char:
    xor ah, ah 
    int 0x16       ;input is stored in al here
    mov ah, 0x0E   ;print input
    int 0x10
    mov bl,al
    jmp random    

random:
    xor al, al
    out 0x70, al    ;should generate random number on al
    in al,0x71
    mov ah,0
    mov cl,10
    div cl
    mov al, ah
    add al,'0'
    mov ah,bl
    jmp test

test:
    cmp ah, al     ;comparison
    je equal
    jne not_equal

equal:       ;if equal
    mov si, success
    jmp succeed

succeed:
    lodsb           ; loads a single byte from [si] into al and increments si
    test al, al     ; checks to see if the byte is 0
    jz done         ; if so, jump out (jz jumps if ZF in EFLAGS is set)
    mov ah, 0x0E    ; 0x0E is the BIOS code to print the single character
    int 0x10        ; call into the BIOS using a software interrupt
    jmp succeed     ; go back to the start of the loop

not_equal:    ;if not equal
    mov si, failure
    jmp failed

failed:
    lodsb           ; loads a single byte from [si] into al and increments si
    test al, al     ; checks to see if the byte is 0
    jz done      ; if so, jump out (jz jumps if ZF in EFLAGS is set)
    mov ah, 0x0E    ; 0x0E is the BIOS code to print the single character
    int 0x10        ; call into the BIOS using a software interrupt
    jmp failed  ; go back to the start of the loop

done:
   mov al, 0x0A   ; new line
   mov ah, 0x0E 
   int 0x10
   jmp get_message       ; loop forever


; The db command just puts in literal bytes.
; 0x0d and 0x0a are carriage return and line feed, respectively
message:    db     "What number am I thinking of (0-9)?", 0x0d, 0x0a, 0
success:    db     "Correct", 0x0d, 0x0a, 0
failure:    db     "Wrong number", 0x0d, 0x0a, 0

    times 510-($-$$) db 0
    db 0x55
    db 0xAA
