; Vernamova sifra na architekture DLX
; Samuel Dobron xdobro23

        .data 0x04          ; zacatek data segmentu v pameti
login:  .asciiz "xdobro23"  ; <-- nahradte vasim loginem
cipher: .space 9 ; sem ukladejte sifrovane znaky (za posledni nezapomente dat 0)

        .align 2            ; dale zarovnavej na ctverice (2^2) bajtu
laddr:  .word login         ; 4B adresa vstupniho textu (pro vypis)
caddr:  .word cipher        ; 4B adresa sifrovaneho retezce (pro vypis)

        .text 0x40          ; adresa zacatku programu v pameti
        .global main        ; 

main:
	xor r4, r4, r4			; r4 <- 0
	for:				; for (r4 = 0; r7 < 'a'){
		lb r7, login(r4)	; r7 <- login[r4]
		slti r26, r7, 97	; r26 = (r7 < 'a') ? 1 : 0
		bnez r26, end		; break
		nop
		
		andi r26, r4, 1		; r26 = (r4 % 2) ? 1 : 0 
		bnez r26, minus		; r4 is odd
		nop
		j plus			; r4 is even
		nop
		
		continue:
		sb cipher(r4), r7	; cipher[r4] <- r7
		addi r4, r4, 1		; r4++
		j for			; }
		nop

end:    addi r14, r0, caddr ; <-- pro vypis sifry nahradte laddr adresou caddr
        trap 5  ; vypis textoveho retezce (jeho adresa se ocekava v r14)
        trap 0  ; ukonceni simulace

left_barrel:
	addi r7, r7, 26			; r7 = 'z' - ('a' - r7 - 1)
	j continue
	nop

minus:
	subi r7, r7, 15			; r7 -= 15
	slti r26, r7, 97		; r26 = (r7 < 'a') ? 1 : 0
	bnez r26, left_barrel		; if (r7 < 'a') jump
	nop
	
	j continue
	nop

right_barrel:
	subi r7, r7, 26			; r7 = 'a' + (r7 - 'z')
	j continue
	nop
	

plus:
	addi r7, r7, 4			; r7 += 4 
	sgti r26, r7, 122		; r26 = (r7 > 'z') ? 1 : 0
	bnez r26, right_barrel		; if (r7 > 'z') jump
	nop

	j continue
	nop
