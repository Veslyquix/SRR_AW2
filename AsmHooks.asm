.thumb 

.global function1
.type function1, %function 
function1: 
push {lr} 
@bl function2
pop {r0} 
bx r0 
.ltorg 
