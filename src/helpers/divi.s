; Spec: SPRAB89B 8.2 Table 8-6 (__c6xabi_divi: int32 x / y truncated toward zero, ISO C 6.5.5/6)
; and 8.3 Table 8-9 - it may change A0 A1 A2 A4 A6 B0 B1 B2 B4 B5 B30 B31 and nothing else. The
; magnitudes divided as divu does, the quotient negated when exactly one operand was negative (B5).
	.global	__c6xabi_divi
	.text
__c6xabi_divi:
	SHRU	A4, 31, A1
	SHRU	B4, 31, B2
	[A1]	NEG	A4, A4
	[B2]	NEG	B4, B4
	XOR	A1, B2, B5
	MV	A4, A6
	ZERO	A0
	ZERO	A2
	MVK	32, B1
divi_bit:
	SHRU	A6, 31, A1
	SHL	A2, 1, A2
	OR	A2, A1, A2
	SHL	A6, 1, A6
	CMPLTU	A2, B4, A1
	SHL	A0, 1, A0
	[!A1]	SUB	A2, B4, A2
	[!A1]	OR	1, A0, A0
	SUB	B1, 1, B1
	[B1]	B	divi_bit
	NOP	5
	MV	B5, B2
	MV	A0, A4
	[B2]	NEG	A4, A4
	B	B3
	NOP	5
