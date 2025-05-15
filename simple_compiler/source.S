.data
score: .int 76, 82, 90, 86, 79, 62
credit: .int 2, 2, 1, 2, 2, 3
label_0: .asciz "please input your student number:"
label_1: .asciz "%d"
label_6: .asciz "the score of student number %d is %f higher than 60.\n"
label_7: .asciz "the score of student number %d is %f lower than 60.\n"
.bss
.lcomm bss_tmp, 4
.lcomm stu_number, 4
.lcomm mean, 4
.lcomm sum, 4
.lcomm temp, 4
.lcomm i, 4
.text
.global main
main:
finit
pushl $label_0
call printf
add $4, %esp
pushl $stu_number
pushl $label_1
call scanf
add $8, %esp
movl $0,sum
filds sum
fstps sum
movl $0, temp
movl $0, i
label_2:
jmp label_2
label_3:
flds mean
subl $8, %esp
fstpl (%esp)
pushl stu_number
pushl $label_6
call printf
add $16, %esp
jmp label_5
label_4:
flds mean
subl $8, %esp
fstpl (%esp)
pushl stu_number
pushl $label_7
call printf
add $16, %esp
label_5:
pushl $0
call exit
