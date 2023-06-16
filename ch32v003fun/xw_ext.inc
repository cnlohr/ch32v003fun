/*
Encoder for some of the proprietary 'XW' RISC-V instructions present on the QingKe RV32 processor.
Examples:
	XW_C_LBU(a3, a1, 27); // c.xw.lbu a3, 27(a1)
	XW_C_SB(a0, s0, 13);  // c.xw.sb a0, 13(s0)

	XW_C_LHU(a5, a5, 38); // c.xw.lhu a5, 38(a5)
	XW_C_SH(a2, s1, 14);  // c.xw.sh a2, 14(s1)
*/

// Let us do some compile-time error checking.
#define ASM_ASSERT(COND) .if (!(COND)); .err; .endif

// Integer encodings of the possible compressed registers.
#define C_s0 0
#define C_s1 1
#define C_a0 2
#define C_a1 3
#define C_a2 4
#define C_a3 5
#define C_a4 6
#define C_a5 7

// register to encoding
#define REG2I(X) (C_ ## X)

// XW opcodes
#define XW_OP_LBUSP 0b1000000000000000
#define XW_OP_STSP  0b1000000001000000

#define XW_OP_LHUSP 0b1000000000100000
#define XW_OP_SHSP  0b1000000001100000

#define XW_OP_LBU   0b0010000000000000
#define XW_OP_SB    0b1010000000000000

#define XW_OP_LHU   0b0010000000000010
#define XW_OP_SH    0b1010000000000010

// The two different XW encodings supported at the moment.
#define XW_ENCODE1(OP, R1, R2, IMM) ASM_ASSERT((IMM) >= 0 && (IMM) < 32); .2byte ((OP) | (REG2I(R1) << 2) | (REG2I(R2) << 7) | \
	(((IMM) & 0b1) << 12) | (((IMM) & 0b110) << (5 - 1)) | (((IMM) & 0b11000) << (10 - 3)))

#define XW_ENCODE2(OP, R1, R2, IMM) ASM_ASSERT((IMM) >= 0 && (IMM) < 32); .2byte ((OP) | (REG2I(R1) << 2) | (REG2I(R2) << 7) | \
	(((IMM) & 0b11) << 5) | (((IMM) & 0b11100) << (10 - 2))

// Compressed load byte, zero-extend result
#define XW_C_LBU(RD, RS, IMM) XW_ENCODE1(XW_OP_LBU, RD, RS, IMM)

// Compressed store byte
#define XW_C_SB(RS1, RS2, IMM) XW_ENCODE1(XW_OP_SB, RS1, RS2, IMM)

// Compressed load half, zero-extend result
#define XW_C_LHU(RD, RS, IMM) ASM_ASSERT(((IMM) & 1) == 0); XW_ENCODE2(XW_OP_LHU, RD, RS, ((IMM) >> 1)))

// Compressed store half
#define XW_C_SH(RS1, RS2, IMM)  ASM_ASSERT(((IMM) & 1) == 0); XW_ENCODE2(XW_OP_SH, RS1, RS2, ((IMM) >> 1)))
