; Spec: SPRAB89B 8.2 Table 8-6 (__c6xabi_remi: int32 x % y, the sign of x, ISO C 6.5.5/6) and 8.3
; Table 8-9 - it may change A1 A2 A4 A5 A6 B0 B1 B2 B4 B30 B31 and nothing else. The magnitudes'
; remainder as remu finds it, negated when x was negative (A2).
	.global	__c6xabi_remi
	.text
__c6xabi_remi:
	SHRU	A4, 31, A2
	SHRU	B4, 31, B2
	[A2]	NEG	A4, A4
	[B2]	NEG	B4, B4
	MV	A4, A5
	ZERO	A6
	MVK	32, B0
remi_bit:
	SHRU	A5, 31, A1
	SHL	A6, 1, A6
	OR	A6, A1, A6
	SHL	A5, 1, A5
	CMPLTU	A6, B4, A1
	[!A1]	SUB	A6, B4, A6
	SUB	B0, 1, B0
	[B0]	B	remi_bit
	NOP	5
	MV	A6, A4
	[A2]	NEG	A4, A4
	B	B3
	NOP	5
