
.data					# section declaration

        # Useful offset constants for accessing members of a 
        # struct mp1_blink_struct structure
        LOCATION   = 0    
        ON_CHAR    = 2
        OFF_CHAR   = 3 
        ON_LENGTH  = 4
        OFF_LENGTH = 6
        COUNTDOWN  = 8
        STATUS     = 10
        NEXT       = 12

        STRUCT_SIZE = 16

# Pointer to head of list (initialized to NULL)
mp1_list_head:
        .long   0

.text					# section declaration

# Export the function symbol names

.global mp1_rtc_tasklet
.global mp1_ioctl

/*# void mp1_poke(void);
#
# Interface: Register-based arguments (not C-style)
#    Inputs: %cl  - The byte you wish to write
#            %eax - Offset from the start of video memory that you wish
#                   to write to
#   Outputs: Text-mode video screen is written to at location %eax with
#            the byte in %cl
# Registers: Clobbers EDX*/
mp1_poke:
        
	movl    vmem_base_addr(,1),%edx
	movb    %cl,(%edx,%eax,1)
	ret






/*# void mp1_rtc_tasklet (unsigned long);
#
# Interface: Register-based arguments (not C-style)
#    Inputs: %edi - The mp1_list_head, and it will store the
#                   current address of the structure in the list 
#   Outputs: none
#            
# Registers: all reserved*/
mp1_rtc_tasklet:
        
        pushl %ebp
        movl %esp, %ebp
        
        # Save callee-saved registers
        pushl %edx
        pushl %ebx
        pushl %edi
        pushl %eax
        pushl %ecx

        # Walk down the mp1_list_head list and examine each structure
        movl mp1_list_head, %edi        # Store the start address of the list
        cmpl $0, %edi                   # Check if it is NULL
        je mp1_rtc_tasklet_down

tasklet_loop_start:
        # Step 4-1 decrement countdown
        andl $0, %ebx
        movw COUNTDOWN(%edi), %bx      
        addw $-1, %bx                  
        movw %bx, COUNTDOWN(%edi)      
        # Step 4-2 check countdown
        cmpw $0, %bx                  # if countdown not equal to 0, go to next struct
        jne struct_update             
        # Step 4-3 check status
        andl $0, %ebx
        movw STATUS(%edi), %bx          
        cmpw $1, %bx                  
        jne status_zero               # if statius not equal to 1, go to 0 status part
status_one:
        # Step 4-4 Change status
        andl $0, %ebx
        movw $0, %bx
        movw %bx, STATUS(%edi)
        # Step 4-5:
        # make a call to mp1_poke with the correct register parameters
        # to immediately display the off_char on the text-mode video screen
        # First, save caller-saved registers
        pushl %edx
        # Save callee-saved registers
        pushl %ebx
        pushl %edi
        pushl %ecx
        pushl %eax
        # Second, push arguments of void* mp1_malloc(unsigned long size)
        # Clear the color stored in the higher bit
        andl $0, %ecx
        andl $0, %eax
        # Store the value
        movb OFF_CHAR(%edi),%cl         # store byte to write
        movzwl LOCATION(%edi), %eax     # store offset
        shll $1, %eax                   # left shift by one bit
        # call mp1_poke
        call mp1_poke
        # Third, pop callee-saved registers
        popl %eax
        popl %ecx
        popl %edi
        popl %ebx
        popl %edx

        # Step 4-6: copy off_length to countdown
        movw OFF_LENGTH(%edi), %bx
        movw %bx, COUNTDOWN(%edi)
        
        jmp struct_update
status_zero:
        # Step 4-4 Change status
        andl $0, %ebx
        movw $1, %bx
        movw %bx, STATUS(%edi)
        # Step 4-5:
        # make a call to mp1_poke with the correct register parameters
        # to immediately display the on_char on the text-mode video screen
        # First, save caller-saved registers
        pushl %edx
        # Save callee-saved registers
        pushl %ebx
        pushl %edi
        pushl %ecx
        pushl %eax                      # push offset
        # Second, push arguments of void* mp1_malloc(unsigned long size)
        # Clear the color stored in the higher bit
        andl $0, %ecx                  
        andl $0, %eax
        # Store the value
        movb ON_CHAR(%edi),%cl          # store byte to write
        movzwl LOCATION(%edi), %eax     # store offset
        shll $1, %eax                   # left shift by one bit       
        # call mp1_poke
        call mp1_poke
        # Third, pop callee-saved registers
        popl %eax
        popl %ecx
        popl %edi
        popl %ebx
        popl %edx

        # Step 4-6: copy off_length to countdown
        movw ON_LENGTH(%edi), %bx
        movw %bx, COUNTDOWN(%edi)
        
struct_update:
        movl NEXT(%edi), %edi           # store the next structure address
        cmpl $0, %edi                   
        je mp1_rtc_tasklet_down         # if the next structure address is 0, means the end of list
                                        # We just start the return procedure
        jmp tasklet_loop_start          # else, modify the next structure
mp1_rtc_tasklet_down:
        popl %ecx
        popl %eax
        popl %edi
        popl %ebx
        popl %edx
        popl %ebp

	ret






/*# int mp1_ioctl(unsigned long arg, unsigned long cmd);
#
# Interface: Register-based arguments (not C-style)
#    Inputs: %ebx - The argument used in four core functions 
#            %edi - The cmd argument determining which of the next four functions to jump to 
#   Outputs: %eax - an integer, with 0 to show success and -1 to show fail
#            (return value must be stored in eax, conventionally)
# Registers: Clobbers EDX*/
mp1_ioctl:    
        pushl %ebp
        movl %esp, %ebp 
        # Save callee-saved registers
        pushl %ebx
        pushl %edi
        pushl %esi
        # get arg and cmd and store them
        movl 8(%ebp), %ebx      # get arg and store it in ebx
        cmpl $0, %ebx           # Check whether it is NULL
        je mp1_ioctl_fail
        movl 12(%ebp), %edi     # get cmd and store it in edi
        
        # Judge whether cmd is one of 0, 1, 2, 3 and branch
check_0:
        cmpl $0, %edi            # if cmd == 0?
        jne check_1              # if not, check cmd == 1
        jmp *jump_table(,%edi,4) # else, execute mp1_ioctl_add
        # Note: jmp *jump_table(,%edi,4) means jump to M[jump_table+4*%edi]
check_1:
        cmpl $1, %edi            # if cmd == 1? 
        jne check_2              # if not, check cmd == 2
        jmp *jump_table(,%edi,4) # else, execute mp1_ioctl_remove
check_2:
        cmpl $2, %edi            # if cmd == 2?  
        jne check_3              # if not, check cmd == 3
        jmp *jump_table(,%edi,4) # else, execute mp1_ioctl_find
check_3:
        cmpl $3, %edi            # if cmd == 3? 
        jne mp1_ioctl_fail       # if not, fail and return -1
        jmp *jump_table(,%edi,4) # else, execute mp1_ioctl_sync

mp1_ioctl_success:
        movl $0, %eax
        jmp mp1_ioctl_down  

mp1_ioctl_fail:
        movl $-1, %eax

mp1_ioctl_down:
        popl %esi
        popl %edi              
        popl %ebx              
        popl %ebp              
	ret                    







/*# int mp1_ioctl_add(unsigned long arg);
#
# Interface: Register-based arguments (not C-style)
#    Inputs: %ebx - The argument used in four core functions 
#           (operation has down in mp1_ioctl function)
#   Outputs: %eax - an integer, with 0 to show success and -1 to show fail
#            (return value must be stored in eax, conventionally)
# Registers: Clobbers EDX*/
mp1_ioctl_add:

        # Step 1:
        # allocate memory using the mp1 malloc 
        # function to store a copy of the structure
        pushl %edx
        movl $STRUCT_SIZE, %edx         # push arguments size = 16
        pushl %edx                      
        call mp1_malloc
        addl $4, %esp                   # 4 Bytes for 1 arg
        popl %edx

        # Step 2:
        # Check whether malloc successes
        cmpl $0, %eax
        je mp1_ioctl_add_fail 
        
        # Step 3:
        # Copy the entire mp1 blink struct from the 
        # user-space structure to the newly-allocated memory 
        # unsigned long mp1_copy_from_user (void *to, const void *from, unsigned long n)

        pushl %edx                      # save caller-saved registers
        movl $STRUCT_SIZE, %edx         # store size = 16 into edx
        movl %eax, %esi                 # store the start address structure to ecx
        pushl %edx                      # push n
        pushl %ebx                      # push *from
        pushl %esi                      # push *to
        call mp1_copy_from_user
        addl $12, %esp                  # pop off arguments(12 Bytes for 3 arg)
        popl %edx

        # Step 4:
        # Check whether mp1_copy_from_user successes
        cmpl $0, %eax
        jne mp1_ioctl_add_fail

        # Step 5:
        # execution of operation sequences
        pushl %edx
        movw ON_LENGTH(%esi), %dx       # 1. Set countdown to on_length
        movw %dx,COUNTDOWN(%esi)
        movw $1, STATUS(%esi)           # 2. Set status to 1
        movl mp1_list_head, %edx        # 3. Insert it at the head of linked list
        movl %edx, NEXT(%esi)
        movl %esi, mp1_list_head

        # Step 6:
        # Check location within [0,1999]
        movw LOCATION(%esi),%dx
        cmpw $0, %dx
        jb mp1_ioctl_add_fail
        cmpw $1999, %dx                 # 1999 is the upper bound
        ja mp1_ioctl_add_fail
        popl %edx

        # Step 7:
        # make a call to mp1 poke with the correct register parameters
        # to immediately display the character on the text-mode video screen
        
        pushl %edx
        pushl %ebx
        pushl %edi
        pushl %esi
        pushl %ecx
        pushl %eax

        # Clear the color stored in the higher bit
        andl $0, %ecx
        andl $0, %eax
        # Store the value
        movb ON_CHAR(%esi),%cl          # store byte to write
        movzwl LOCATION(%esi), %eax     # store offset
        shll $1, %eax                   # left shift by one bit
        # call mp1_poke
        call mp1_poke
        
        popl %eax
        popl %ecx
        popl %esi
        popl %edi
        popl %ebx
        popl %edx
        # If success, finished
        movl $0, %eax
        jmp mp1_ioctl_add_down

mp1_ioctl_add_fail:
        # First free the space by call mp_free
        pushl %edx
        pushl %ebx
        pushl %edi
        pushl %esi              # push arg, the address of the malloc-space       
        call mp1_free      
        addl $4, %esp
        popl %edi
        popl %ebx
        popl %edx
        # Then return -1
        movl $-1, %eax

mp1_ioctl_add_down:
        popl %esi
        popl %edi
        popl %ebx
        popl %ebp
	ret







/*# int mp1_ioctl_remove(unsigned long arg);
#
# Interface: Register-based arguments (not C-style)
#    Inputs: %ebx - location
#                    
#   Outputs: %eax - 0 if success, -1 if fail
#            
# Registers: all reserved*/
mp1_ioctl_remove:

        # Step 1: save caller-saved registers
        pushl %edx
        pushl %ecx
        pushl %esi
        pushl %edi
	
        # Step 2: judge whether the location is valid
        cmpl $0, %ebx
        jb mp1_ioctl_remove_fail          
        cmpl $1999, %ebx
        ja mp1_ioctl_remove_fail          

        # Step 3: Find the element to move
        movl %ebx, %ecx
        call linked_list_search
        # Check if search successes
        cmpl $0, %eax
        je mp1_ioctl_remove_fail

        # Find the next of its previous struct               
        movl mp1_list_head, %edi                # Store the start address into edi
        cmpl %edi, %eax
        je prev_is_start
prev_find_loop:
        cmpl NEXT(%edi), %eax                   # Check if it matches the location
        je prev_find_success
        movl NEXT(%edi), %edi
        # Check if it is end of the list
        cmpl $0, %edi
        je prev_find                            # If yes, search fail
        jmp prev_find_loop                      # If no, continue back
prev_find_success:
        movl NEXT(%eax), %esi
        movl %esi, NEXT(%edi)  
        jmp prev_find                     
prev_is_start:
        movl NEXT(%eax), %esi
        movl %esi, mp1_list_head
        jmp prev_find 
        
prev_find:
        # Free the memory
        # First free the space by call mp_free
        # First, save caller-saved registers
        pushl %edx
        # Save callee-saved registers
        pushl %ecx
        pushl %esi
        pushl %ebx
        pushl %edi
        # Then, push arg
        pushl %eax                              # push the address of the malloc-space
        # Third, call mp_free
        call mp1_free
        # Pop off arguments
        addl $4, %esp
        popl %edi
        popl %ebx
        popl %esi
        popl %ecx
        popl %edx
        movl $0, %eax
        jmp mp1_ioctl_remove_down

mp1_ioctl_remove_fail:
        movl $-1, %eax
mp1_ioctl_remove_down:
        popl %edi
        popl %esi
        popl %ecx
        popl %edx

        popl %esi
        popl %edi
        popl %ebx
        popl %ebp

	ret






/*# int mp1_ioctl_find(unsigned long arg);
#
# Interface: Register-based arguments (not C-style)
#    Inputs: %ebx - A pointer to mp1_blink_struct, and it will store the
#                   current address of the structure in the list 
#   Outputs: %eax - 0 if success, -1 if fail
#            
# Registers: all reserved*/
mp1_ioctl_find:

        # Step 1: save caller-saved registers
        pushl %edx
        pushl %ecx
        pushl %esi
        pushl %edi

        # Step 2: Change the user space pointers information to kernel
        # malloc a new space
        pushl %edx
        movl $STRUCT_SIZE, %edx         # push arguments size = 16
        pushl %edx                      
        call mp1_malloc
        addl $4, %esp                   # 4 Bytes for 1 arg
        popl %edx

        # Check whether malloc successes
        cmpl $0, %eax
        je mp1_ioctl_find_fail

        # Copy the entire mp1 blink struct from the 
        # user-space structure to the newly-allocated memory 
        # unsigned long mp1_copy_from_user (void *to, const void *from, unsigned long n)
        pushl %edx                      # save caller-saved registers
        movl $STRUCT_SIZE, %edx         # store size = 16 into edx
        movl %eax, %edi                 # store the start address structure to edi
        pushl %edx                      # push n
        pushl %ebx                      # push *from
        pushl %edi                      # push *to
        call mp1_copy_from_user
        addl $12, %esp                  # pop off arguments(12 Bytes for 3 arg)
        popl %edx

        # Check whether mp1_copy_from_user successes
        cmpl $0, %eax
        jne mp1_ioctl_find_fail

        # Step 3: Extract the location and judge whether it is valid
        andl $0, %ecx
        movw LOCATION(%edi), %cx        # extract the location
        cmpw $0, %cx
        jb mp1_ioctl_find_fail          
        cmpw $1999, %cx                 # 1999 is the upper bound
        ja mp1_ioctl_find_fail          
        
        # Step 3: find the matched location
        call linked_list_search
        # Check if search successes
        cmpl $0, %eax
        je mp1_ioctl_find_fail
        # If not, execute copy step

        # Step 4:  copy the entire element
        # Copy the entire mp1 blink struct from the kernel
        # to the user-level structure pointed to by the parameter that was passed in
        # unsigned long mp1_copy_to_user (void *to, const void *from, unsigned long n)

        # Start mp1_copy_to_user
        pushl %edx
        # Push arguments
        movl %eax, %esi                 # store the start address structure to esi
        movl $STRUCT_SIZE, %edx         # store size = 16 into edx       
        pushl %edx                      # push n
        pushl %esi                      # push *from
        pushl %ebx                      # push *to        
        call mp1_copy_to_user        
        addl $12, %esp                  # pop off arguments(12 Bytes for 3 arg)
        popl %edx

        # Step 5: success! move 0 to eax
        movl $0, %eax
        jmp mp1_ioctl_find_down
        
mp1_ioctl_find_fail:
        movl $-1, %eax
mp1_ioctl_find_down:
        # First free the space by call mp_free
        pushl %edx
        pushl %ebx
        pushl %esi
        pushl %edi              # push arg, the address of the malloc-space       
        call mp1_free      
        addl $4, %esp
        popl %esi
        popl %ebx
        popl %edx
        
        popl %edi
        popl %esi
        popl %ecx
        popl %edx

        popl %esi
        popl %edi              
        popl %ebx              
        popl %ebp              

	ret








/*# int mp1_ioctl_find(unsigned long arg);
#
# Interface: Register-based arguments (not C-style)
#    Inputs: %ebx - (location1,location2)
#                    
#   Outputs: %eax - 0 if success, -1 if fail
#            
# Registers: all reserved*/
mp1_ioctl_sync:

        # Step 1: save caller-saved registers
        pushl %edx
        pushl %ecx
        pushl %esi
        pushl %edi
        pushl %ebx

        # Step 2: Extract the location and judge whether it is valid
        andl $0, %ecx
        andl $0, %edx
        movw %bx, %dx                   # extract the second location
        shrl $16, %ebx                  # right shift 16 bits
        movw %bx, %cx                   # extract the first location
        cmpw $0, %cx
        jb mp1_ioctl_sync_fail          
        cmpw $1999, %cx                 # 1999 is the upper bound
        ja mp1_ioctl_sync_fail          
        cmpw $0, %dx
        jb mp1_ioctl_sync_fail          
        cmpw $1999, %dx                 # 1999 is the upper bound
        ja mp1_ioctl_sync_fail          
        
        # Step 3: find the matched location
        call linked_list_search
        # Check if search successes
        cmpl $0, %eax
        je mp1_ioctl_sync_fail
        # If not, store the first pointer to esi
        movl %eax, %esi

        movw %dx, %cx
        call linked_list_search
        # Check if search successes
        cmpl $0, %eax
        je mp1_ioctl_sync_fail
        # If not, store the first pointer to edi
        movl %eax, %edi

        # Step 4: copy 4 attributes of element 1 to element2
        andl $0, %edx
        movw ON_LENGTH(%esi), %dx
        movw %dx, ON_LENGTH(%edi)
        movw OFF_LENGTH(%esi), %dx
        movw %dx, OFF_LENGTH(%edi)
        movw COUNTDOWN(%esi), %dx
        movw %dx, COUNTDOWN(%edi)
        movw STATUS(%esi), %dx
        movw %dx, STATUS(%edi)

        # Call mp1_poke to show element2
        # Step 7:
        # make a call to mp1 poke with the correct register parameters
        # to immediately display the character on the text-mode video screen
        # First, save caller-saved registers
        pushl %edx
        # Save callee-saved registers
        pushl %ebx
        pushl %edi
        pushl %esi
        pushl %ecx
        pushl %eax
        # Second, push arguments
        # Clear the color stored in the higher bit
        andl $0, %ecx
        andl $0, %eax
        # Store the value
        cmpw $1, %dx
        jne sync_zero
        movb ON_CHAR(%edi),%cl          # store byte to write
        jmp sync_next
sync_zero:
        movb OFF_CHAR(%edi),%cl          # store byte to write
sync_next:
        movzwl LOCATION(%edi), %eax     # store offset
        shll $1, %eax                   # left shift by one bit
        
        # call mp1_poke
        call mp1_poke
        # Third, pop callee-saved registers
        popl %eax
        popl %ecx
        popl %esi
        popl %edi
        popl %ebx
        popl %edx
        # If success. Return 0
        movl $0, %eax
        jmp mp1_ioctl_sync_down          # finish the process

mp1_ioctl_sync_fail:
        movl $-1, %eax

mp1_ioctl_sync_down:
        popl %ebx
        popl %edi
        popl %esi
        popl %ecx
        popl %edx

        popl %esi
        popl %edi
        popl %ebx
        popl %ebp

	ret








/* void* linked_list_search(unsigned short location);
#
# Interface: Register-based arguments (not C-style)
#    Inputs: %cx  - The standard location to match
# 
#   Outputs: %eax - The address of the matched structure
#                   not found, return 0
#   Outputs: Text-mode video screen is written to at location %eax with
#            the byte in %cl
# Registers: Clobbers EDX */
linked_list_search:
        pushl %edi
        pushl %ebx

        andl $0, %ebx
        movl mp1_list_head, %edi                # Store the start address into edi
        # First check if input address is valid
        cmpl $0, %edi
        je search_fail
search_loop:
        movw LOCATION(%edi), %bx                # Store the current location into bx
        cmpw %cx, %bx                           # Check if it matches the location
        je search_success
        movl NEXT(%edi), %edi
        # Check if it is end of the list
        cmpl $0, %edi
        je search_fail                          # If yes, search fail
        jmp search_loop                         # If no, continue back
search_success:
        movl %edi, %eax
        jmp search_end
search_fail:
        movl $0, %eax
search_end:
        popl %ebx
        popl %edi

        ret








# Define the jump table as required to help mp1_ioctl function
jump_table:
.long mp1_ioctl_add, mp1_ioctl_remove, mp1_ioctl_find, mp1_ioctl_sync


.end
