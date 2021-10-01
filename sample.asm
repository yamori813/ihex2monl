	ORG	09000H

	LD   HL,MSG	;
	CALL 052EDH	;
	LD   HL,0EF45H	;
	LD   A,002H	;
	LD   (HL),A	;
	RET		;
MSG:
	db	"HELLO", 0

