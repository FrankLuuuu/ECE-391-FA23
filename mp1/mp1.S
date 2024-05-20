
# mp1.S - missile-command version
# Good luck, and have fun!

.data
	
    # Constants for accessing the fields of a struct missile, 
    # struct missile is defined in rtc.h and mp1.h

	NEXT 	  = 0
	X	  = 4
	Y	  = 8
	VX	  = 12
	VY	  = 16
	DEST_X	  = 20
	DEST_Y	  = 24
	EXPLODED  = 28
	C	  = 32

	# Memory space for the struct - 4 * 8 + 1 = 33

	MEMORY = 33

    # Character to draw for an explosion - '@'

	EXPLOSION = 64
	
	# Character to draw for erasing the missile - ' '

	ERASE = 32

	# Character to draw for the crosshair - '+'

	CROSSHAIR = 43

	# Position for first city, + 40 for others

	FIRST_CITY = 3876

	CITY_DIFF = 34

	# # of columns

	COL = 80

	# # of rows

	ROW = 25


# Data shared between this file and rtc.c helper functions

# This '.globl' directive makes these symbols visible externally
.globl mp1_missile_list, base_alive, mp1_score

mp1_missile_list:	.long 0x0	# Head of list pointer
base_alive:		.long 0x0	# Array of 3 bytes, plus a padding byte
mp1_score:		.long 0x0	# Player's current score


# Data private to this  file

base_pic:     	    .string "/^^^\\"	# Picture of a live base
dead_base_pic: 	    .string "xxxxx"	# Picture of a dead base
crosshairs_x:		.long 0x0	# X-position of the crosshairs
crosshairs_y:		.long 0x0	# Y-position of the crosshairs
status:			.long 0x0   # Status of the game

.text

# void mp1_poke(void);
# You can use this function to write to video memory.
#
# Interface: Register-based arguments (not C-style)
#    Inputs: %cl  - The byte you wish to write
#            %eax - Offset from the start of video memory that you wish
#                   to write to
#   Outputs: Text-mode video screen is written to at location %eax with
#            the byte in %cl
# Registers: Clobbers EDX

mp1_poke:
		movl    vmem_base_addr(,1),%edx
		movb    %cl,(%edx,%eax,1)
		ret

#   ----------------- Exported functions ---------------------

# void mp1_rtc_tasklet(unsigned long garbage);
# Performs three tasks:
#	(1) updates the list of missiles (implement this in update_missiles,
#           below, and call it from here).
#	(2) Redraw the bases - they may have been overwritten with missiles
#	(3) Redraw the crosshairs - it may have been overwritten with missiles
# Inputs   : none
# Outputs  : none
# Registers: Standard C calling convention

.globl mp1_rtc_tasklet
mp1_rtc_tasklet:
		# conventional callee-save
		pushl %ebp
		movl %esp, %ebp
		pushl %ebx                  
		pushl %esi
		pushl %edi

		# call tasklet operations
		call update_missiles
		call draw_cities
		call draw_crosshairs 

		jmp done

# int mp1_ioctl(unsigned long arg, unsigned int cmd)
# The dispatch function for the MP1 ioctls - should use the cmd argument
# and a jumptable to execute one of the specific ioctls implemented below.
# Inputs   : unsigned long arg - parameter to the mp1_ioctl_....
#          : unsigned int cmd  - specifies which mp1_ioctl_... function 
#          :                     to execute
# Outputs  : Returns an integer - depends on which ioctl() function is called
# Registers: Standard C calling convention

.globl mp1_ioctl
mp1_ioctl:
		# get the cmd argument and check whether it is valid, jump to the function
		cmpl $4, 8(%esp)
		ja fails
		cmpl $0, 8(%esp)
		jl fails
		movl 8(%esp), %eax
		jmp *jump_table(, %eax, 4)

# return -1 if the operation fails
fails: 
		movl $-1, %eax
		ret

jump_table :
	.long mp1_ioctl_startgame, mp1_ioctl_addmissile, mp1_ioctl_movexhairs, mp1_ioctl_getstatus, mp1_ioctl_endgame

# ----------------- Functions private to this file -------------------

# void update_missiles();
# Iterates through all the missiles in the linked list, updates the 
# positions of the missiles and draws explosions when necessary
# Inputs   : none
# Outputs  : the updated linked list of missiles
# Registers: Standard C calling convention
update_missiles:
		# conventional callee-save
		pushl %ebp
		movl %esp, %ebp
		pushl %ebx                  
		pushl %esi
		pushl %edi

		# store the head and prev of the list
		movl mp1_missile_list, %ebx
		movl $0, %edx

	update_list:
			# check if the current node is NULL
			cmpl $0, %ebx
			je done

			# erase the missile since in every case the missile is redrawn
			# push the arguments for drawing the space
			pushl $ERASE 
			movl X(%ebx), %edi
			movl Y(%ebx), %esi
			sarl $16, %edi
			sarl $16, %esi
			pushl %edi
			pushl %esi

			# draw space at the missile position and pop the arguments
			call draw_helper
			popl %esi
			popl %edi
			addl $4, %esp

			# check whether missile is currently exploding
			cmpl $0, EXPLODED(%ebx)
			jne missile_exploding

		check_bound:
				# update new positions with velocities
				movl VX(%ebx), %ecx
				addl %ecx, X(%ebx)
				movl X(%ebx), %edi
				sarl $16, %edi
				movl VY(%ebx), %ecx
				addl %ecx, Y(%ebx)
				movl Y(%ebx), %esi
				sarl $16, %esi
				
				# check if the new positions are out of bounds
				cmpl $0, %edi
				jl out_of_screen
				cmpl $COL, %edi
				jae out_of_screen

				cmpl $0, %esi
				jl out_of_screen
				cmpl $ROW, %esi
				jae out_of_screen

				# check if missile is at the destination
				cmpl DEST_X(%ebx), %edi
				jne draw_missile

				cmpl DEST_Y(%ebx), %esi
				jne draw_missile

		missile_exploding:
				# conventional caller-save + save register used
				pushl %eax
				pushl %edx
				pushl %ecx
				pushl %ebx

				# missile exploded
				call missile_explode

				# caller teardown
				popl %ebx
				popl %ecx
				popl %edx

				# check if the status of the game changed 
				cmpl $0, %eax
				je decrement
				popl %eax

				# conventional caller-save
				pushl %eax
				pushl %edx
				pushl %ecx

				# notify the user
				call mp1_notify_user
				
				# conventional caller-teardown
				popl %ecx
				popl %edx
				popl %eax
			
			decrement:
					# decrement the length of explosion
					subl $1, EXPLODED(%ebx)

					# check the missile if done exploding
					cmpl $0, EXPLODED(%ebx)
					jne draw_explosion

					# conventional caller-save
					pushl %eax
					pushl %edx
					pushl %ecx

					# pushing arguments
					call mp1_missile_remove

					# conventional caller-teardown
					popl %ecx
					popl %edx
					popl %eax

					jmp update_list
				
				draw_explosion:
						# push the arguments for drawing the explosion
						pushl $EXPLOSION 
						movl X(%ebx), %edi
						movl Y(%ebx), %esi
						shrl $16, %edi
						shrl $16, %esi
						pushl %edi
						pushl %esi

						# draw space at the missile position and pop the arguments
						call draw_helper
						addl $12, %esp

						# move on to the next node
						jmp next_node

		draw_missile:
				# push the arguments for drawing the missile
				pushl C(%ebx) 
				movl X(%ebx), %edi
				movl Y(%ebx), %esi
				shrl $16, %edi
				shrl $16, %esi
				pushl %edi
				pushl %esi

				# draw space at the missile position and pop the arguments
				call draw_helper
				addl $12, %esp

		next_node:
				# store curr in prev and next of curr in curr
				movl %ebx, %edx
				movl NEXT(%ebx), %ebx
				jmp update_list

		out_of_screen:
				# conventional caller-save
				pushl %eax
				pushl %edx
				pushl %ecx

				call mp1_missile_remove

				# conventional caller-teardown
				popl %ecx
				popl %edx
				popl %eax

				# restart loop
				jmp update_list

# void missile_remove();
# Removes the current missile and updates the rest of the 
# list accordingly. Frees the memory used by the missile.
# Inputs   : none
# Outputs  : none
# Registers: Standard C calling convention
mp1_missile_remove:
		# check if curr is NULL
		cmpl $0, %ebx
		je done

		# save curr
		movl %ebx, %esi

		# save next 
		movl NEXT(%ebx), %edi

		# conventional caller-save
		pushl %eax
		pushl %edx
		pushl %ecx
		pushl %edi
		pushl %esi

		# pushing argument
		pushl %ebx
		call mp1_free

		# conventional caller-teardown
		popl %ebx
		popl %esi
		popl %edi
		popl %ecx
		popl %edx
		popl %eax

		# edx = prev, esi = curr, edi = next
		cmpl $0, %edx
		jne not_head
		# check if the node is the head of the list
		head:
				movl %edi, mp1_missile_list
				jmp removed
		
		# check if the node is in the middle or end of the list
		not_head:
				movl %edi, NEXT(%edx)

		# move to the next node
		removed:		
				movl %edi, %ebx

		ret

# void draw_helper();
# Takes in the x and y positions of the character and 
# prints the character using mp1_poke. Calculates the 
# corresponding text-mode video location on screen
# 
# Inputs   : unsigned int y - y position of the char
#          : unsigned int x - x position of the char
#		   : char character - the char to be printed
#          :                     to execute
# Outputs  : none
# Registers: Standard C calling convention
draw_helper:
		# conventional callee-save
		pushl %ebp
		movl %esp, %ebp
		pushl %ebx                  
		pushl %esi
		pushl %edi

		# store the arguments in registers
		movl 8(%ebp), %ebx
		movl 12(%ebp), %esi
		movb 16(%ebp), %cl

		# calculate the position on the screen
		imull $2, %ebx, %ebx
		imull $COL, %ebx, %ebx
		imull $2, %esi, %esi
		addl %esi, %ebx
		movl %ebx, %eax

		# conventional caller-save
		pushl %eax
		pushl %edx
		pushl %ecx

		# draw the character in %cl at the location in %eax
		call mp1_poke

		# conventional caller-teardown
		popl %ecx
		popl %edx
		popl %eax

		jmp done

# void draw_cities();
# Draws the cities according to whether they are alive.
# Uses a loop to go through the three cities and print
# all five chars that make up each city
# Inputs   : none
# Outputs  : Prints the updated cities on the screen
# Registers: Standard C calling convention
draw_cities:
		# conventional callee-save
		pushl %ebp
		movl %esp, %ebp
		pushl %ebx                  
		pushl %esi
		pushl %edi

		# x coordinate of the first city
		movl $FIRST_CITY, %eax
		# the array of cities
		movl base_alive, %ebx

		cmpb $0, %bl
		je dead_base
		jmp live_base

		# second to thrid cities
		movl $2, %esi
	city_loop:
			cmpl $3, %esi
			ja done
			incl %esi

			addl $CITY_DIFF, %eax
			sarl $8, %ebx
			cmpb $0, %bl
			je dead_base
			jmp live_base

	# store the city pictures
	live_base:
			leal base_pic, %edi
			jmp draw_city_chars

	dead_base:
			leal dead_base_pic, %edi
	
	# loop through each character in the character array and display on screen
	draw_city_chars:
			# first char
			movb (%edi), %cl
			call mp1_poke

			incl %edi
			movb (%edi), %cl
			addl $2, %eax
			call mp1_poke

			incl %edi
			movb (%edi), %cl
			addl $2, %eax
			call mp1_poke

			incl %edi
			movb (%edi), %cl
			addl $2, %eax
			call mp1_poke

			incl %edi
			movb (%edi), %cl
			addl $2, %eax
			call mp1_poke

			jmp city_loop

# void draw_crosshairs();
# Draws the crosshair according to the stored x & y values
# 
# Inputs   : none
# Outputs  : Prints the crosshair on the screen
# Registers: Standard C calling convention
draw_crosshairs:
	# conventional callee-save
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx                  
	pushl %esi
	pushl %edi

	# draw the crosshair at x & y
	pushl $CROSSHAIR
	pushl crosshairs_x
	pushl crosshairs_y
	call draw_helper

	# pop the arguments
	addl $12, %esp

	jmp done

# int mp1_ioctl_startgame (unsigned long ignore);
# Initializes all variables that are used in this program.
# 
# Inputs   : none
# Outputs  : none
# Registers: Standard C calling convention
mp1_ioctl_startgame:
		# conventional callee-save
		pushl %ebp
		movl %esp, %ebp
		pushl %ebx
		pushl %esi
		pushl %edi

		# initialize all variables
		movl $0, mp1_missile_list
		movl $0x00FFFFFF, base_alive
		movl $0, mp1_score
		movl $40, crosshairs_x
		movl $12, crosshairs_y

		jmp done
# int mp1 ioctl addmissile (struct missile* user missile);
# Adds a new missile to the game. Allocates memory for the
# missile and data copy from the user.
# 
# Inputs   : a pointer to a struct missile in user space
# Outputs  : 0 or -1 according to whether adding the missile
#			 succeeds
# Registers: Standard C calling convention
mp1_ioctl_addmissile:
		# conventional callee-save
		pushl %ebp
		movl %esp, %ebp
		pushl %ebx
		pushl %esi
		pushl %edi

		# store the new missile
		movl 8(%ebp), %edi

		# conventional caller-save
		# pushl %eax
		pushl %edx
		pushl %ecx
		pushl %edi

		# push the amount of memory needed 
		pushl $MEMORY
		call mp1_malloc

		# check if malloc failed
		cmpl $0, %eax
		je copy_fails

		# pop the arguemnt & conventional caller-teardown
		addl $4, %esp
		popl %edi
		popl %ecx
		popl %edx
		# popl %eax

		# conventional caller-save
		pushl %eax
		pushl %edx
		pushl %ecx

		# pushing arguments
		pushl $MEMORY
		pushl %edi
		pushl %eax
		call mp1_copy_from_user

		# pop the arguemnts & conventional caller-teardown
		addl $12, %esp
		popl %edx
		popl %ecx

		# check if the data copy failed
		cmpl $0, %eax
		je success_copy

		# free the allocated memory and set return value as -1
		call mp1_free
		popl %eax
		movl $-1, %eax
		jmp done

	success_copy:
			# pop eax so that we have a point back to the missle list
			popl %eax

			# add the new missile into the linked list by setting it as the new head	
			movl mp1_missile_list, %edx
			movl %edx, NEXT(%eax)
			movl %eax, mp1_missile_list

			# set return value as 0 
			movl $0, %eax

			jmp done

# int mp1_ioctl_movexhairs (unsigned long xhair_delta_packed);
# Moves the crosshair according to the parameter.
# 
# Inputs   : a 32-bit integer containing two signed 16-bit 
#			 integers packed into its low and high words. 
#			 The low 16 bits contain the amount by which the 
#			 x component of the crosshair position should
#			 change, and the high 16 bits contain the amount 
#			 by which the y component should change
# Outputs  : always returns 0
# Registers: Standard C calling convention
mp1_ioctl_movexhairs:
		# conventional callee-save
        pushl %ebp
        movl %esp, %ebp
        pushl %ebx
        pushl %esi
        pushl %edi
        
        # erase the old crosshair
		pushl $ERASE
		pushl crosshairs_x
		pushl crosshairs_y
		call draw_helper

		# pop the arguments
		addl $12, %esp
		
		# store the low/high 16 bits
		movw 8(%ebp), %dx
		movw 10(%ebp), %cx
		shll $16, %edx
		shll $16, %ecx
		sarl $16, %edx
		sarl $16, %ecx

	# check if the new positions are out of bounds
	min_x: 
			movl %edx, %ebx
			addl crosshairs_x, %ebx
			cmpl $0, %ebx
			jl min_y
		
	max_x: 
			cmpl $COL, %ebx
			jae max_y
			addl %edx, crosshairs_x

	min_y:
			movl %ecx, %ebx
			addl crosshairs_y, %ebx
			cmpl $0, %ebx
			jb new_crosshair

	max_y:
			cmpl $ROW, %ebx
			jae new_crosshair
			addl %ecx, crosshairs_y

	# draw crosshair at new postions
    new_crosshair:
			# push the arguments to draw the crosshair
			pushl $CROSSHAIR
			pushl crosshairs_x
			pushl crosshairs_y
			call draw_helper

			# pop the arguments
			addl $12, %esp

			# aset the return value to 0
			movl $0, %eax

            jmp done

# int mp1_ioctl_getstatus (unsigned long* user_status);
# Retrieves the current score and the status of the three cities.
# Copies the current score into the low 16-bits of that integer, 
# and thestatus of the three cities into bits 16, 17, and 18.
# 
# Inputs   : a pointer to a 32-bit integer in user space
# Outputs  : return 0 if the copy to user space succeeds, 
#			 and -1 if it fails
# Registers: Standard C calling convention
mp1_ioctl_getstatus:
		# conventional callee-save
		pushl %ebp
		movl %esp, %ebp
		pushl %ebx
		pushl %esi
		pushl %edi

		# store the pointer
		movl 8(%ebp), %ebx

		# store the score into ecx
		movl mp1_score, %ecx

	# check whether the cities are alive and store into ecx
	city1:
			movl base_alive, %edi
			andl $0x00FF0000, %edi
			cmpl $0, %edi
			jne city1_alive
	
	city2:
			movl base_alive, %edi
			andl $0x0000FF00, %edi
			cmpl $0, %edi
			jne city2_alive
	
	city3:
			movl base_alive, %edi
			andl $0x000000FF, %edi
			cmpl $0, %edi
			jne city3_alive
			jmp update_status

	city1_alive:
			addl $0x00010000, %ecx
			jmp city2
	
	city2_alive:
			addl $0x00020000, %ecx
			jmp city3
	
	city3_alive:
			addl $0x00040000, %ecx
		
	# Copy this information from kernel space to user space
	# unsigned long mp1 copy_to_user (void *to, const void *from, unsigned long n);
	update_status:
			# get the memory address of ecx
			pushl %ecx
			movl %esp, %eax

			# conventional caller-save
			pushl %eax
			pushl %ecx
			pushl %edx

			# push the arguments and copy to user
			pushl $4 
			pushl %eax
			pushl %ebx
			call mp1_copy_to_user

			# pop the arugments & conventional caller-teardown
			addl $12, %esp
			popl %edx	
			popl %ecx

			# check if the data copy to user failed
			cmpl $0, %eax
			jne copy_fails
			popl %eax
			popl %ecx

			# set return value to 0
			movl $0, %eax
			jmp done

	copy_fails:
			movl $-1, %eax
			jmp done
		
# int mp1_ioctl_endgame (unsigned long ignore);
# Frees all the memory being used by the linked list 
# and then return success.
# 
# Inputs   : none
# Outputs  : none
# Registers: Standard C calling convention
mp1_ioctl_endgame:
		# conventional callee-save
		pushl %ebp
		movl %esp, %ebp
		pushl %ebx
		pushl %esi
		pushl %edi


		# loop through all missiles
		remove_all_missiles:
				# store the head of the linked list
				movl mp1_missile_list, %ebx

				# check if all the missiles are removed
				cmpl $0, %ebx
				je done

				# store the next node & assign the head
				movl NEXT(%ebx), %ecx
				movl %ecx, mp1_missile_list

				# conventional caller-save
				pushl %eax
				pushl %edx
				pushl %ecx

				# push the head to the stack and free it
				pushl %ebx
				call mp1_free

				# pop the arguement & conventional caller-teardown
				popl %ebx
				popl %ecx
				popl %edx
				popl %eax

				# move on to the next node
				jmp remove_all_missiles

		jmp done

done:
		# conventional callee-teardown
		popl %edi
		popl %esi
		popl %ebx
		leave
		ret
