; Spec: none - the M0 probe: its own entry point, one reference into rts6x.lib, the first byte
; of what it finds left in A4 at C$$EXIT, where vm6747sim reads the exit status.
	.global	_c_int00
	.global	C$$EXIT
	.ref	__rts6x_version
	.text
_c_int00:
	MVKL	__rts6x_version, A3
	MVKH	__rts6x_version, A3
	LDB	*A3, A4
	NOP	4
C$$EXIT:
	B	C$$EXIT
	NOP	5
