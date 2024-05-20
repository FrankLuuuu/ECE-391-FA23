/*
 * tab:2
 *
 * edit_dist_asm.S - inplementation of computing edit distance between two (linked) lists
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * IN NO EVENT SHALL THE AUTHOR OR THE UNIVERSITY OF ILLINOIS BE LIABLE TO
 * ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
 * DAMAGES ARISING OUT  OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF THE AUTHOR AND/OR THE UNIVERSITY OF ILLINOIS HAS BEEN ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE AUTHOR AND THE UNIVERSITY OF ILLINOIS SPECIFICALLY DISCLAIM ANY
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND NEITHER THE AUTHOR NOR
 * THE UNIVERSITY OF ILLINOIS HAS ANY OBLIGATION TO PROVIDE MAINTENANCE,
 * SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
 *
 * Author:        Xiang Li
 * Version:       1
 * Creation Date: Sat Aug 20 2022
 * History:
 *    XL    1    Sat Aug 20 2022
 *        First written.
 */

.data
  UNUSED  = 0
  VAL   = 4
  NEXT = 8

.text
.global edit_dist_asm

# Edit Dist ASM (Assembly)
# Declaration is in edit_dist.h
#
# Registers:
#   EAX - Return Value
#   EBX - root1
#   BSI - root2
#   you may choose how to store your local/temp variables
#

edit_dist_asm:
  pushl %ebp
  movl %esp, %ebp

  # --- STUDENT CODE STARTS HERE ---
  PUSHL %EBX                  # standard callee-save
  PUSHL %ESI
  PUSHL %EDI
  SUBL $12, %ESP              # accounting for saved varibales

  MOVL 8(%EBP), %EBX          # store root1 in EBX, root2 in ESI
  MOVL 12(%EBP), %ESI

  MOVL $0, %EAX               # store 0 as the return value
  CMPL $0, %EBX               # check if root1 == NULL
  JE ROOT1NULL

  CMPL $0, %ESI               # check if root2 == NULL
  JE ROOT2NULL

  MOVL VAL(%EBX), %EDX        # check if the val of roots are equal
  CMPL %EDX, VAL(%ESI)       
  JE EQUALVAL


INSERT:
  PUSHL %ESI                  # calculate insert
  PUSHL NEXT(%EBX)
  CALL edit_dist_asm

  POPL %EDX                   # pop obsolete variables into arbitrary register
  POPL %EDX
  MOVL %EAX, (%ESP)           # store insert on the stack

DELETE:
  PUSHL NEXT(%ESI)            # calculate delete
  PUSHL %EBX
  CALL edit_dist_asm

  POPL %EDX                   # pop obsolete variables into arbitrary register
  POPL %EDX
  MOVL %EAX, 4(%ESP)          # store delete on the stack

REPLACE:
  PUSHL NEXT(%ESI)            # calculate replace
  PUSHL NEXT(%EBX)
  CALL edit_dist_asm

  POPL %EDX                   # pop obsolete variables into arbitrary register
  POPL %EDX
  MOVL %EAX, 8(%ESP)          # store replace on the stack


IF1:
  MOVL 4(%ESP), %EDX          # if insert is not larger than either delete or replace
  CMPL %EDX, (%ESP)        
  JA IF2
  MOVL 8(%ESP), %EDX
  CMPL %EDX, (%ESP)      
  JA IF2
  MOVL (%ESP), %EAX           # set return value to 1 + insert
  ADDL $1, %EAX
  JMP DONE

IF2:
  MOVL 8(%ESP), %EDX
  CMPL %EDX, 4(%ESP)        # if delete is not larger than replace
  JA ELSE
  MOVL 4(%ESP), %EAX           # set return value to 1 + delete
  ADDL $1, %EAX
  JMP DONE

ELSE:
  MOVL 8(%ESP), %EAX           # set return value to 1 + replace
  ADDL $1, %EAX
  JMP DONE


ROOT1NULL:              
  CMPL $0, %ESI               # if root2 == NULL, return 0
  JE DONE

  PUSHL NEXT(%ESI)            # if only root1 == NULL, go to next of root2
  PUSHL %EBX
  CALL edit_dist_asm

  ADDL $1, %EAX               # after returning, add 1 to the return value
  POPL %EDX                   # pop obsolete variables into arbitrary register
  POPL %EDX
  JMP DONE

ROOT2NULL:              
  PUSHL %ESI                  # if only root2 == NULL, go to next of root1
  PUSHL NEXT(%EBX)
  CALL edit_dist_asm

  ADDL $1, %EAX               # after returning, add 1 to the return value
  POPL %EDX                   # pop obsolete variables into arbitrary register
  POPL %EDX
  JMP DONE


EQUALVAL:
  PUSHL NEXT(%ESI)            # if vals are equal, go to next of root1 & root2
  PUSHL NEXT(%EBX)
  CALL edit_dist_asm

  POPL %EDX                   # pop obsolete variables into arbitrary register
  POPL %EDX
  JMP DONE
  

DONE:
  SUBL $-12, %ESP             # standard teardown stack
  POPL %EDI                  
  POPL %ESI
  POPL %EBX         
  # --- STUDENT CODE ENDS HERE ---

  leave
  ret
