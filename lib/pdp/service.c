/*
 * Used to be
 * Copyright 1994, Eric A. Edwards
 * After substantial changes
 * Copyright 2003, Leonid A. Broukhis
 */

/*
 * service.c - Event handler system and interrupt service.
 */


#include "defines.h"

/* 1 if interrupt requested while WAIT was in effect */

unsigned char in_wait_instr = 0;

/* Allows up to 32 different HW interrupts,
 * there can be no more than 1 pending interrupt of each kind.
 */
unsigned long pending_interrupts = 0;
double earliest = 0.0;
event events[NUM_PRI];

/*
 * ev_init() - Initialize the event system.
 */

int ev_init()
{
	pending_interrupts = 0;
	return OK;
}

/*
 * ev_register() - Register an event.
 */

int
ev_register( priority, handler, delay, info )
unsigned priority;
int (*handler)(d_word info);
unsigned long delay;	/* in clock ticks */
d_word info;
{

	if (pending_interrupts & (1 << priority)) {
		/* There is one pending already */
		return OK;
	}
	/*
	 * Fill in the info.
	 */

	events[priority].handler = handler;
	events[priority].info = info;
	events[priority].when = ticks + delay;
	if (!pending_interrupts || ticks + delay < earliest) {
		earliest = ticks + delay;
	}
	pending_interrupts |= 1 << priority;
	/* fprintf(stderr, "Registering pri %d @ %g\n", priority, earliest); */
	return OK;
}


/*
 * ev_fire() - Fire off any pending event by the mask
 * priority list.
 */

int ev_fire( int priority )
{
	int x;
	unsigned long mask = 0;
	switch (priority) {
		case 0: mask = ~0; break;
		case 1: mask = ~0; break;
		case 2: mask = ~0; break;
		case 3: mask = ~0; break;
		/* BK-0010 uses MTPS #200 to block keyboard interrupts */
		case 4: mask = 1; break;
		case 5: mask = 0; break;
		case 6: mask = 0; break;
		case 7: mask = 0; break;
	}
	if (!(mask & pending_interrupts) || ticks < earliest) {
		return OK;
	}

	earliest = 10.0e38;

	for( x = 0; x < NUM_PRI && (1<<x) <= pending_interrupts; ++x ) {
		if ((mask & pending_interrupts & (1<<x)) &&
			ticks >= events[x].when) {
			events[x].handler(events[x].info);
			/* fprintf(stderr, "Firing pri %d\n", x); */
			pending_interrupts &= ~(1 << x);
			mask = 0;
		} else if ((pending_interrupts & (1<<x)) &&
			events[x].when < earliest) {
			earliest = events[x].when;
		}
	}
	/* fprintf(stderr, "Earliest is %g\n", earliest); */
	return OK;
}


/*
 * service() - Handle a Trap.
 */

int
service( vector )
d_word vector;
{
	register pdp_regs *p = &pdp;
	int result;
	d_word oldpsw;
	d_word oldpc;
	d_word newpsw;
	d_word newpc;

	last_branch = p->regs[PC];

	oldpsw = p->psw;
	oldpc = p->regs[PC];
	
	/* If we're servicing an interrupt while a WAIT instruction
	 * was executing, we need to return after the WAIT.
	 */
	if (in_wait_instr) {
		oldpc += 2;
		in_wait_instr = 0;
	}

	if (( result = ll_word(p, vector, &newpc)) != OK)
		return result;
	if (( result = ll_word(p, vector + 2, &newpsw)) != OK)
		return result;

	// fprintf(stderr, "Int %o to %06o\n", vector, p->regs[PC]);
	//addtocybuf(-vector);

	if (( result = push( p, oldpsw )) != OK )
		return result;
	if (( result = push( p, oldpc )) != OK )
		return result;

	p->psw = newpsw;
	p->regs[PC] = newpc;
	return OK;
}


/*
 * rti() - Return from Interrupt Instruction.
 */

int rti( p )
register pdp_regs *p;
{
	d_word newpsw;
	d_word newpc;
	int result;

	last_branch = p->regs[PC];
	if (( result = pop( p, &newpc )) != OK )
		return result;
	if (( result = pop( p, &newpsw )) != OK )
		return result;

	p->psw = newpsw;
	p->regs[PC] = newpc;

	return OK;
}


/*
 * rtt() - Return from Interrupt Instruction.
 */

int rtt( p )
register pdp_regs *p;
{
	d_word newpsw;
	d_word newpc;
	int result;

	last_branch = p->regs[PC];
	if (( result = pop( p, &newpc )) != OK )
		return result;
	if (( result = pop( p, &newpsw )) != OK )
		return result;

	p->psw = newpsw;
	p->regs[PC] = newpc;

	return CPU_RTT;
}
