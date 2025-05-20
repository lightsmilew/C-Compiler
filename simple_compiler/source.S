.data
score: .int 76, 82, 90, 86, 79, 62
credit: .int 2, 2, 1, 2, 2, 3
var_0: .asciz "please input your student number:"
var_1: .asciz "%d"
var_2: .asciz "the score of student number %d is %f higher than 60.\n"
var_3: .asciz "the score of student number %d is %f lower than 60.\n"
.bss
.lcomm bss_tmp, 4
.lcomm a, 4
.lcomm b, 4
.lcomm c, 4
.lcomm stu_number, 4
.lcomm mean, 4
.lcomm sum, 4
.lcomm temp, 4
.lcomm i, 4
.text
test:
movl %edi,a
movl %esi,b
movl a, %eax
addl b, %eax
movl %eax,bss_tmp
movl bss_tmp, %edi
movl %edi, c
movl c,%eax
popl %esi
popl %edi
ret
.global main
main:
finit
pushl $var_0
call printf
add $4, %esp
pushl $stu_number
pushl $var_1
call scanf
add $8, %esp
movl $0,sum
filds sum
fstps sum
movl $0, temp
movl $0, i
label_0:
movl i, %eax
movl $6, %ebx
cmp %eax,%ebx
jae label_1:
movl i, %edi
movl (score,%edi,4), %eax
movl i, %edi
mull (credit,%edi,4)
movl %eax,bss_tmp
flds sum
fiadd bss_tmp
fstps bss_tmp
fstps sum
movl temp, %eax
movl i, %edi
addl (credit,%edi,4), %eax
movl %eax,bss_tmp
movl bss_tmp, %edi
movl %edi, temp
incl i
jmp label_0
label_1:
flds sum
fidiv temp
fstps bss_tmp
fstps mean
flds mean
filds 60
fcom
fstsw %eax
sahf
jb label_2
flds mean
fisub 60
fstps bss_tmp
fstps mean
flds mean
subl $8, %esp
fstpl (%esp)
pushl stu_number
pushl $var_2
call printf
add $16, %esp
jmp label_3
label_2:
filds 60
fsub mean
fstps bss_tmp
fstps mean
flds mean
subl $8, %esp
fstpl (%esp)
pushl stu_number
pushl $var_3
call printf
add $16, %esp
label_3:
label_4:
movl $1, %eax
movl $2, %ebx
cmp %eax,%ebx
jne label_5
incl temp
jmp label_4
label_5:
pushl %eax
pushl %edi
movl $1,%edi
pushl %esi
movl $2,%esi
call test
pushl $0
call exit
