; Spec: SPRAB89B 8.2 Table 8-6 (__c6xabi_divu: uint32 x / y, x in A4, y in B4, the quotient in A4)
; and 8.3 Table 8-9 - it may change A0 A1 A2 A4 A6 B0 B1 B2 B4 B30 B31 and nothing else. Restoring
; division, a bit a turn: the remainder r in A2, the dividend shifting out of A6, the quotient in A0.
	.global	__c6xabi_divu
	.text
__c6xabi_divu:
	MV	A4, A6
	ZERO	A0
	ZERO	A2
	MVK	32, B1
divu_bit:
	SHRU	A6, 31, A1
	SHL	A2, 1, A2
	OR	A2, A1, A2
	SHL	A6, 1, A6
	CMPLTU	A2, B4, A1
	SHL	A0, 1, A0
	[!A1]	SUB	A2, B4, A2
	[!A1]	OR	1, A0, A0
	SUB	B1, 1, B1
	[B1]	B	divu_bit
	NOP	5
	MV	A0, A4
	B	B3
	NOP	5
