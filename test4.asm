LOAD 5
STORE y
READ y
PUSH
LOAD y
STACKW 0
PUSH
LOAD 0
STACKW 0
STACKR 0
POP
STORE calcV
STACKR 0
POP
SUB calcV
SUB calcV
BRZERO A
PUSH
LOAD y
STACKW 0
STACKR 0
POP
STORE printV
WRITE printV
A: NOOP
STOP
y 0
printV 0
calcV 0
