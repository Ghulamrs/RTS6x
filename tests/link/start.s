; Spec: SPRAB89B 3 (B15 the stack pointer, 8-byte aligned; B14 the data page pointer, B3 the return
; address, A4 the result) - a stand-in entry for tests until M1 brings RTS6x's own: stack, data
; pointer, main, and its result left in A4 at C$$EXIT. Linked with --ram_model, so no .cinit to run.
	.global	_c_int00
	.global	C$$EXIT
	.ref	main
	.ref	__TI_STACK_END
	.ref	__TI_STATIC_BASE
	.text
_c_int00:
	MVKL	__TI_STACK_END, B15
	MVKH	__TI_STACK_END, B15
	SUB	B15, 8, B15
	AND	-8, B15, B15
	MVKL	__TI_STATIC_BASE, B14
	MVKH	__TI_STATIC_BASE, B14
	MVKL	main, A3
	MVKH	main, A3
	MVKL	returned, B3
	MVKH	returned, B3
	B	A3
	NOP	5
returned:
C$$EXIT:
	B	C$$EXIT
	NOP	5
