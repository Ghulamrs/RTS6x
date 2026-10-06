; Spec: SPRAB89B 8.2 Table 8-6 and its text - __c6xabi_divremi: x / y in A4 and x % y in A5, as C has
; them; 8.3 Table 8-9 - it may change A1 A2 A4 A5 A6 B0 B1 B2 B4 B30 B31 and nothing else. The
; magnitudes divided as divu does; the quotient negated when exactly one sign was set, the remainder when x's.
	.global	__c6xabi_divremi
	.text
__c6xabi_divremi:
	SHRU	A4, 31, A2
	SHRU	B4, 31, B2
	[A2]	NEG	A4, A4
	[B2]	NEG	B4, B4
	XOR	B2, A2, B30
	MV	A4, A5
	ZERO	A6
	ZERO	A4
	MVK	32, B0
divremi_bit:
	SHRU	A5, 31, A1
	SHL	A6, 1, A6
	OR	A6, A1, A6
	SHL	A5, 1, A5
	CMPLTU	A6, B4, A1
	SHL	A4, 1, A4
	[!A1]	SUB	A6, B4, A6
	[!A1]	OR	1, A4, A4
	SUB	B0, 1, B0
	[B0]	B	divremi_bit
	NOP	5
	MV	B30, B1
	[B1]	NEG	A4, A4
	MV	A6, A5
	[A2]	NEG	A5, A5
	B	B3
	NOP	5
