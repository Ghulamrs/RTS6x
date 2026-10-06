; Spec: SPRAB89B 8.2 Table 8-6 (__c6xabi_remu: uint32 x % y, x in A4, y in B4, the remainder in A4)
; and 8.3 Table 8-9 - it may change A1 A4 A5 A7 B0 B1 B2 B4 B30 B31 and nothing else. Restoring
; division without the quotient: the remainder in A7, the dividend shifting out of A5.
	.global	__c6xabi_remu
	.text
__c6xabi_remu:
	MV	A4, A5
	ZERO	A7
	MVK	32, B0
remu_bit:
	SHRU	A5, 31, A1
	SHL	A7, 1, A7
	OR	A7, A1, A7
	SHL	A5, 1, A5
	CMPLTU	A7, B4, A1
	[!A1]	SUB	A7, B4, A7
	SUB	B0, 1, B0
	[B0]	B	remu_bit
	NOP	5
	MV	A7, A4
	B	B3
	NOP	5
