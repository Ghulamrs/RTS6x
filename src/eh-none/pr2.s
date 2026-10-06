; Spec: SPRAB89B 11.4.3 (PR2, __c6xabi_unwind_cpp_pr2, named by a table past 64 KB) and
; the ARM EHABI's _URC_FAILURE (9). printf6x.lib's stand-in: it has no unwinder to call this, so a
; throw cannot reach it; were one to, the answer is "cannot unwind". M5 brings the real one.
	.global	__c6xabi_unwind_cpp_pr2
	.text
__c6xabi_unwind_cpp_pr2:
	MVK	9, A4
	B	B3
	NOP	5
