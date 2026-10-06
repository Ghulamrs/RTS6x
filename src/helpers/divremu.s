; Spec: SPRAB89B 8.2 Table 8-6 and its text - __c6xabi_divremu: x / y in A4 and x % y in A5, unsigned;
; 8.3 Table 8-9 - it may change A0 A1 A2 A4 A6 B0 B1 B2 B4 B30 B31, and A5, which the text returns in.
; One bit of quotient a turn, as divu finds it, the remainder what is left.
	.global	__c6xabi_divremu
	.text
__c6xabi_divremu:
	MV	A4, A2
	ZERO	A6
	ZERO	A4
	MVK	32, B0
divremu_bit:
	SHRU	A2, 31, A1
	SHL	A6, 1, A6
	OR	A6, A1, A6
	SHL	A2, 1, A2
	CMPLTU	A6, B4, A1
	SHL	A4, 1, A4
	[!A1]	SUB	A6, B4, A6
	[!A1]	OR	1, A4, A4
	SUB	B0, 1, B0
	[B0]	B	divremu_bit
	NOP	5
	MV	A6, A5
	B	B3
	NOP	5
