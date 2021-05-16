/*
 * This file is part of 'pdp', a PDP-11 simulator.
 *
 * For information contact:
 *
 *   Computer Science House
 *   Attn: Eric Edwards
 *   Box 861
 *   25 Andrews Memorial Drive
 *   Rochester, NY 14623
 *
 * Email:  mag@potter.csh.rit.edu
 * FTP:    ftp.csh.rit.edu:/pub/csh/mag/pdp.tar.Z
 * 
 * Copyright 1994, Eric A. Edwards
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  Eric A. Edwards makes no
 * representations about the suitability of this software
 * for any purpose.  It is provided "as is" without expressed
 * or implied warranty.
 */

#ifndef DEFINES_INCLUDED
#define DEFINES_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

//#define EIS_ALLOWED
//#define SHIFTS_ALLOWED

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Type definitions for PDP data types.
 */

typedef uint32_t c_addr;	/* core or BK Q-bus address (17 bit so far) */
typedef unsigned short l_addr;	/* logical address (16 bit) */
typedef unsigned short d_word;	/* data word (16 bit) */
typedef unsigned char d_byte;	/* data byte (8 bit) */
typedef unsigned char flag_t;	/* for boolean or small value flags */

/*
 * PDP processor defines.
 */

#define R5	5	/* return register for MARK */
#define SP	6	/* stack pointer */
#define PC	7	/* program counter */


typedef struct _pdp_regs {
	d_word regs[8];		/* general registers */
	d_byte psw;		/* processor status byte (LSI-11) */
	d_word ir;		/* current instruction register */
	d_word ea_addr;		/* stored address for dest modifying insts */
	unsigned long total;	/* count of instructions executed */
	unsigned look_time;	/* when to handle things, saves time */
} pdp_regs;


/*
 * Definitions for the memory map and memory operations.
 */


#define OK		0	/* memory and instruction results */
#define ODD_ADDRESS	1
#define BUS_ERROR	2
#define MMU_ERROR	3
#define CPU_ILLEGAL	4
#define CPU_HALT	5
#define CPU_WAIT	6
#define CPU_NOT_IMPL	7
#define CPU_TRAP	8
#define CPU_EMT		9
#define CPU_BPT		10
#define CPU_IOT		11
#define CPU_RTT		12
#define CPU_TURBO_FAIL	13

/*
 * Q-bus device addresses.
 */

#define BASIC           0120000
#define BASIC_SIZE      (24 * 512)      /* 24 Kbytes */
#define PORT_REG        0177714         /* printer, mouse, covox, ... */
#define PORT_SIZE       1
#define IO_REG          0177716         /* tape, speaker, memory */
#define IO_SIZE         1
#define TTY_REG         0177660
#define TTY_SIZE        3
#define LINE_REG        0176560
#define LINE_SIZE       4
#define TIMER_REG       0177706
#define TIMER_SIZE      3
#define SECRET_REG	0177700
#define SECRET_SIZE	3

int ll_word(pdp_regs* p, c_addr addr, d_word* word);
int sl_word(pdp_regs* p, c_addr addr, d_word word);
int ll_byte(pdp_regs* p, c_addr addr, d_byte* byte);
int sl_byte(pdp_regs* p, c_addr addr, d_byte byte);

void q_reset();
int pop(pdp_regs* p, d_word* data);
int push(pdp_regs* p, d_word data);
int storeb_dst(pdp_regs* p, d_byte data);
int storeb_dst_2(pdp_regs* p, d_byte data);
int store_dst(pdp_regs* p, d_word data);
int load_src(pdp_regs* p, d_word* data);
int load_ea(pdp_regs* p, d_word* addr);
int load_dst(pdp_regs* p, d_word* data);
int loadb_dst(pdp_regs* p, d_byte* data);
int store_dst_2(pdp_regs* p, d_word data);
int loadb_src(pdp_regs* p, d_byte* data);
int brx(pdp_regs* p, unsigned int clear, unsigned int set);
int service(d_word);

int ev_register(unsigned int priority, int (*handler)(d_word info), unsigned long delay, d_word info);
int ev_fire(int priority);

/*
 * Defines for the event handling system.
 */

#define NUM_PRI     2

/* Timer interrupt has higher priority */
#define TIMER_PRI	0
#define TTY_PRI		1

typedef struct _event {
	int (*handler)(d_word info); /* handler function */
	d_word info;			     /* info or vector number */
	double when;			     /* when to fire this event */
} event;


/*
 * Instruction Table for Fast Decode.
 */

struct _itab {
	int (*func)(pdp_regs* p);
};


/*
 * Global variables.
 */

extern unsigned short last_branch;
extern flag_t io_stop_happened;
extern struct _itab itab[];
extern pdp_regs pdp;
extern double ticks;
extern flag_t in_wait_instr;
extern unsigned long pending_interrupts;

// bkmodel == 0 is BK-0010, 1 is BK-0011M.
extern flag_t bkmodel;

/*
 * Inline defines.
 */

/* For BK-0010 */

#define CC_N	010
#define CC_Z	04
#define CC_V	02
#define CC_C	01

#define CLR_CC_V()	p->psw &= ~CC_V
#define CLR_CC_C()	p->psw &= ~CC_C
#define CLR_CC_Z()	p->psw &= ~CC_Z
#define CLR_CC_N()	p->psw &= ~CC_N
#define CLR_CC_ALL()	p->psw &= ~(CC_V|CC_C|CC_Z|CC_N)

#define SET_CC_V()	p->psw |= CC_V
#define SET_CC_C()	p->psw |= CC_C
#define SET_CC_Z()	p->psw |= CC_Z
#define SET_CC_N()	p->psw |= CC_N

#define SRC_MODE	(( p->ir & 07000 ) >> 9 )
#define SRC_REG		(( p->ir & 0700 ) >> 6 )
#define DST_MODE	(( p->ir & 070 ) >> 3 )
#define DST_REG		( p->ir & 07 )

#define LSBIT	1		/*  least significant bit */

#define	MPI	0077777		/* most positive integer */
#define MNI	0100000		/* most negative integer */
#define NEG_1	0177777		/* negative one */
#define SIGN	0100000		/* sign bit */
#define CARRY   0200000		/* set if carry out */

#define	MPI_B	0177		/* most positive integer (byte) */
#define MNI_B	0200		/* most negative integer (byte) */
#define NEG_1_B	0377		/* negative one (byte) */
#define SIGN_B	0200		/* sign bit (byte) */
#define CARRY_B	0400		/* set if carry out (byte) */

#define LOW16( data )	(( data ) & 0177777 )	/* mask the lower 16 bits */
#define LOW8( data )	(( data ) & 0377 )	/* mask the lower 8 bits */

#define CHG_CC_N( d )	if ((d) & SIGN ) \
					SET_CC_N(); \
				else \
					CLR_CC_N()

#define CHGB_CC_N( d )	if ((d) & SIGN_B ) \
				SET_CC_N(); \
			else \
				CLR_CC_N()

#define CHG_CC_Z( d )	if ( d ) \
					CLR_CC_Z(); \
				else \
					SET_CC_Z()

#define CHGB_CC_Z( d )	if ( LOW8( d )) \
				CLR_CC_Z(); \
			else \
				SET_CC_Z()

#define CHG_CC_C( d )	if ((d) & CARRY ) \
					SET_CC_C(); \
				else \
					CLR_CC_C()

#define CHG_CC_IC( d )	if ((d) & CARRY ) \
					CLR_CC_C(); \
				else \
					SET_CC_C()

#define CHGB_CC_IC( d )	if ((d) & CARRY_B ) \
				CLR_CC_C(); \
			else \
				SET_CC_C()

#define CHG_CC_V( d1, d2, d3 )	\
				if ((( d1 & SIGN ) == ( d2 & SIGN )) \
				&& (( d1 & SIGN ) != ( d3 & SIGN ))) \
					SET_CC_V(); \
				else \
					CLR_CC_V()

#define CHG_CC_VC( d1, d2, d3 )	\
				if ((( d1 & SIGN ) != ( d2 & SIGN )) \
				&& (( d2 & SIGN ) == ( d3 & SIGN ))) \
					SET_CC_V(); \
				else \
					CLR_CC_V()

#define CHG_CC_VS( d1, d2, d3 )	\
				if ((( d1 & SIGN ) != ( d2 & SIGN )) \
				&& (( d1 & SIGN ) == ( d3 & SIGN ))) \
					SET_CC_V(); \
				else \
					CLR_CC_V()

#define CHGB_CC_V( d1, d2, d3 )	\
				if ((( d1 & SIGN_B ) == ( d2 & SIGN_B )) \
				&& (( d1 & SIGN_B ) != ( d3 & SIGN_B ))) \
					SET_CC_V(); \
				else \
					CLR_CC_V()

#define CHGB_CC_VC(d1,d2,d3)	\
				if ((( d1 & SIGN_B ) != ( d2 & SIGN_B )) \
				&& (( d2 & SIGN_B ) == ( d3 & SIGN_B ))) \
					SET_CC_V(); \
				else \
					CLR_CC_V()

#define CHG_CC_V_XOR_C_N()	\
				if ((( p->psw & CC_C ) && \
				   ( p->psw & CC_N )) \
				|| ((!( p->psw & CC_C )) && \
				   ( ! ( p->psw & CC_N )))) \
					CLR_CC_V(); \
				else \
					SET_CC_V()

#ifdef __cplusplus
}
#endif

#endif
