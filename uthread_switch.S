	.text

/* Switch from current_thread to next_thread. Make next_thread
 * the current_thread, and set next_thread to 0.
 * Use eax as a temporary register; it is caller saved.
 */
	.globl thread_switch
thread_switch:
	/* YOUR CODE HERE */
	/* save all registers */
	pushal
	/* save current esp into current_thread->sp */
	movl current_thread, %eax
	movl %esp, (%eax)
	/* load next thread's sp into esp */
	movl next_thread, %eax
	movl (%eax), %esp
	/* restore all registers */
	popal
	/* current thread <= next thread */
	movl next_thread, %eax
	movl %eax, current_thread
	/* next_thread <= 0 */
	movl $0, next_thread
	ret				/* pop return address from stack */
