#ifndef ASSEMBLERX64_H
#define ASSEMBLERX64_H

#include "Assembler.h"
#include "Assert.h"
#include <stddef.h>

typedef enum {
	RAX = 0, // temp, 1. return
	RCX = 1, // 4. arg
	RDX = 2, // 3. arg, 2. return
	RBX = 3, // callee-saved register; optionally used as base pointer
	RSP = 4,
	RBP = 5,
	RSI = 6, // 2. arg
	RDI = 7, // 1. arg
	R8  = 8, // 5. arg
	R9  = 9, // 6. arg
	R10 = 10, // context pointer (static chain pointer)
	R11 = 11, // temp
	R12 = 12, // temp (calee-saved)
	R13 = 13, // temp (calee-saved)
	R14 = 14, // temp (calee-saved)
	R15 = 15, // temp (calee-saved)
	RIP = -1,
	NO_REGISTER = -2,
} Register;

typedef enum {
	AL = 0,
	CL = 1,
	DL = 2,
	BL = 3,
	AH = 4,
	CH = 5,
	DH = 6,
	BH = 7,
	SPL = 4 | (1 << 4),
	BPL = 5 | (1 << 4),
	SIL = 6 | (1 << 4),
	DIL = 7 | (1 << 4),
	R8L = 8,
	R9L = 9,
	R10L = 10,
	R11L = 11,
	R12L = 12,
	R13L = 13,
	R14L = 14,
	R15L = 15,
} ByteRegister;

#define TMP R10
#define CTX R12

enum {
	REX_W = 8,
	REX_R = 4,
	REX_X = 2,
	REX_B = 1,
};

enum {
	MOD_MEM = 0,
	MOD_DISP8 = 1,
	MOD_DISP32 = 2,
	MOD_REG = 3,
};

enum {
	SIB = 4,
	ZERO_INDEX = 4,
};

typedef enum {
	SS_1 = 0,
	SS_2 = 1,
	SS_4 = 2,
	SS_8 = 3,
} Scale;

enum {
	ADD = 0,
	SUB = 5,
};

enum {
	CALL_ABSOLUTE = 2,
	JUMP_ABSOLUTE = 4,
};

enum {
	COND_OVERFLOW =  0,
	COND_NO_OVERFLOW = 1,
	COND_BELOW = 2,
	COND_ABOVE_EQUAL = 3,
	COND_EQUAL = 4,
	COND_NOT_EQUAL = 5,
	COND_BELOW_EQUAL = 6,
	COND_ABOVE = 7,
	COND_SIGN = 8,
	COND_NOT_SIGN = 9,
	COND_PARITY_EVEN = 10,
	COND_PARITY_ODD = 11,
	COND_LESS = 12,
	COND_GREATER_EQUAL = 13,
	COND_LESS_EQUAL = 14,
	COND_GREATER = 15,
	COND_ZERO = COND_EQUAL,
	COND_NOT_ZERO = COND_NOT_EQUAL,
	COND_NEGATIVE = COND_SIGN,
	COND_POSITIVE = COND_NOT_SIGN,
	COND_CARRY = COND_BELOW,
	COND_NOT_CARRY = COND_ABOVE_EQUAL,
};

typedef struct {
	uint8_t mod;
	uint8_t reg;
	uint8_t rm;
	uint8_t scale;
	uint8_t index;
	uint8_t base;
	int32_t disp;
} Operands;

typedef struct {
	int8_t base;
	int8_t index;
	int8_t scale;
	int32_t disp;
} MemoryOperand;

static uint8_t Registers[] = { R11, R13, R14, R15, RBX, R9, R8, RCX, RDX, /*RSI, RDI*/ };
static uint8_t ArgumentsRegisters[] = { RDI, RSI, RDX, RCX, R8, R9 };
static AvailableRegs X64AvailableRegs = {
	.regsSize = sizeof(Registers),
	.regs = Registers,
};
static _Bool CalleeSavedRegisters[] = {
	/* RAX */ 0,
	/* RCX */ 0,
	/* RDX */ 0,
	/* RBX */ 1,
	/* RSP */ 1,
	/* RBP */ 1,
	/* RSI */ 0,
	/* RDI */ 0,
	/* R8  */ 0,
	/* R9  */ 0,
	/* R10 */ 0,
	/* R11 */ 0,
	/* R12 */ 1,
	/* R13 */ 1,
	/* R14 */ 1,
	/* R15 */ 1,
};

static _Bool asmIsCalleeSavedRegister(Register reg);
static MemoryOperand asmMem(Register base, Register index, Scale scale, ptrdiff_t disp);

static void asmPushq(AssemblerBuffer *buffer, Register src);
static void asmPushqImm(AssemblerBuffer *buffer, int32_t imm);
static void asmPushqMem(AssemblerBuffer *buffer, MemoryOperand operand);
static void asmPopq(AssemblerBuffer *buffer, Register dst);
static void asmPopqMem(AssemblerBuffer *buffer, MemoryOperand operand);

static void asmRet(AssemblerBuffer *buffer);

static void asmMovq(AssemblerBuffer *buffer, Register src, Register dst);
static void asmMovqImm(AssemblerBuffer *buffer, int64_t imm, Register dst);
static void asmMovqToMem(AssemblerBuffer *buffer, Register src, MemoryOperand operand);
static void asmMovqMem(AssemblerBuffer *buffer, MemoryOperand operand, Register dst);
static void asmMovqMemImm(AssemblerBuffer *buffer, int64_t imm, MemoryOperand operand);
static void asmMovb(AssemblerBuffer *buffer, ByteRegister src, ByteRegister dst);
static void asmMovbToMem(AssemblerBuffer *buffer, ByteRegister src, MemoryOperand operand);
static void asmMovbMem(AssemblerBuffer *buffer, MemoryOperand operand, ByteRegister dst);
static void asmMovbMemImm(AssemblerBuffer *buffer, int8_t imm, MemoryOperand operand);

static void asmMovzxbMemq(AssemblerBuffer *buffer, MemoryOperand operand, Register dst);
static void asmMovzxwMemq(AssemblerBuffer *buffer, MemoryOperand operand, Register dst);

static void asmCmovq(AssemblerBuffer *buffer, uint8_t condition, Register src, Register dst);

static void asmLeaq(AssemblerBuffer *buffer, MemoryOperand operand, Register dst);

static void asmAddq(AssemblerBuffer *buffer, Register src, Register dst);
static void asmAddqImm(AssemblerBuffer *buffer, Register reg, int32_t imm);
static void asmAddqMem(AssemblerBuffer *buffer, MemoryOperand operand, Register dst);
static void asmAddqToMem(AssemblerBuffer *buffer, Register dst, MemoryOperand operand);
static void asmAddqMemImm(AssemblerBuffer *buffer, MemoryOperand operand, int32_t imm);
static void asmAddb(AssemblerBuffer *buffer, ByteRegister src, ByteRegister dst);
static void asmAddbMem(AssemblerBuffer *buffer, MemoryOperand operand, ByteRegister dst);
static void asmSubq(AssemblerBuffer *buffer, Register src, Register dst);
static void asmSubqImm(AssemblerBuffer *buffer, Register reg, int32_t imm);
static void asmSubqMem(AssemblerBuffer *buffer, MemoryOperand operand, Register dst);
static void asmImulq(AssemblerBuffer *buffer, Register src, Register dst);
static void asmImulqImm(AssemblerBuffer *buffer, Register src, int8_t imm, Register dst);
static void asmImulqMem(AssemblerBuffer *buffer, MemoryOperand operand, Register dst);
static void asmIdivq(AssemblerBuffer *buffer, Register divisor);
static void asmIdivqMem(AssemblerBuffer *buffer, MemoryOperand divisor);
static void asmSbbq(AssemblerBuffer *buffer, Register src, Register dst);
static void asmNegq(AssemblerBuffer *buffer, Register dst);
static void asmNegb(AssemblerBuffer *buffer, ByteRegister dst);

static void asmIncq(AssemblerBuffer *buffer, Register dst);
static void asmIncqMem(AssemblerBuffer *buffer, MemoryOperand operand);
static void asmDecq(AssemblerBuffer *buffer, Register dst);

static void asmAndq(AssemblerBuffer *buffer, Register src, Register dst);
static void asmAndqImm(AssemblerBuffer *buffer, Register dst, int32_t imm);
static void asmAndqMem(AssemblerBuffer *buffer, MemoryOperand operand, Register dst);
static void asmAndbImm(AssemblerBuffer *buffer, ByteRegister dst, int8_t imm);
static void asmOrq(AssemblerBuffer *buffer, Register src, Register dst);
static void asmOrqMem(AssemblerBuffer *buffer, MemoryOperand operand, Register dst);
static void asmOrbMemImm(AssemblerBuffer *buffer, MemoryOperand operand, int8_t imm);
static void asmXorq(AssemblerBuffer *buffer, Register src, Register dst);
static void asmXorqMem(AssemblerBuffer *buffer, MemoryOperand operand, Register dst);

static void asmSarqImm(AssemblerBuffer *buffer, Register dst, uint8_t imm);
static void asmShlq(AssemblerBuffer *buffer, Register dst);
static void asmShlqImm(AssemblerBuffer *buffer, Register dst, uint8_t imm);
static void asmShrq(AssemblerBuffer *buffer, Register dst);
static void asmShrqImm(AssemblerBuffer *buffer, Register dst, uint8_t imm);

static void asmTestq(AssemblerBuffer *buffer, Register src, Register dst);
static void asmTestqImm(AssemblerBuffer *buffer, Register reg, int32_t imm);
static void asmTestqMem(AssemblerBuffer *buffer, MemoryOperand operand, Register reg);
static void asmTestbImm(AssemblerBuffer *buffer, ByteRegister reg, int8_t imm);
static void asmTestbMem(AssemblerBuffer *buffer, MemoryOperand operand, ByteRegister reg);
static void asmTestbMemImm(AssemblerBuffer *buffer, MemoryOperand operand, int8_t imm);

static void asmCmpq(AssemblerBuffer *buffer, Register src, Register dst);
static void asmCmpqMem(AssemblerBuffer *buffer, MemoryOperand operand, Register reg);
static void asmCmpqImm(AssemblerBuffer *buffer, Register reg, int32_t imm);
static void asmCmpbMem(AssemblerBuffer *buffer, MemoryOperand operand, ByteRegister reg);
static void asmCmpbMemImm(AssemblerBuffer *buffer, MemoryOperand operand, int8_t imm);
static void asmCmpbImm(AssemblerBuffer *buffer, ByteRegister reg, int8_t imm);

static void asmCqo(AssemblerBuffer *buffer);

static void asmCallq(AssemblerBuffer *buffer, Register src);
static void asmCallqMem(AssemblerBuffer *buffer, MemoryOperand operand);
static void asmJmpq(AssemblerBuffer *buffer, Register src);
static void asmJmpqMem(AssemblerBuffer *buffer, MemoryOperand operand);
static void asmJmpdImm(AssemblerBuffer *buffer, int32_t imm);
static void asmJmpLabel(AssemblerBuffer *buffer, AssemblerLabel *label);
static void asmJ(AssemblerBuffer *buffer, uint8_t condition, AssemblerLabel *label);
static void asmInt3(AssemblerBuffer *buffer);

static void asmEmitAddImm(AssemblerBuffer *buffer, uint8_t op, Register reg, int32_t imm);
static void asmEmitShift(AssemblerBuffer *buffer, uint8_t op, Register dst);
static void asmEmitShiftImm(AssemblerBuffer *buffer, uint8_t op, Register dst, uint8_t imm);
static void asmInitMemoryOperand(Operands *operands, MemoryOperand operand);
static void asmEmitRexOperands(AssemblerBuffer *buffer, uint8_t rex, Operands *operands);
static void asmEmitRex(AssemblerBuffer *buffer, uint8_t rex);
static void asmEmitOperands(AssemblerBuffer *buffer, Operands *operands);


static _Bool asmIsCalleeSavedRegister(Register reg)
{
	return CalleeSavedRegisters[reg];
}


static MemoryOperand asmMem(Register base, Register index, Scale scale, ptrdiff_t disp)
{
	ASSERT(INT32_MIN <= disp && disp <= INT32_MAX);
	MemoryOperand operand = {.base = base, .index = index, .scale = scale, .disp = disp};
	return operand;
}


static void asmPushq(AssemblerBuffer *buffer, Register src)
{
	Operands operands = { .reg = 0, .rm = src };
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, 0, &operands);
	asmEmitUint8(buffer, 0x50 | (src & 7));
}


static void asmPushqImm(AssemblerBuffer *buffer, int32_t imm)
{
	asmEnsureCapacity(buffer);
	asmEmitUint8(buffer, 0x68);
	asmEmitInt32(buffer, imm);
}


static void asmPushqMem(AssemblerBuffer *buffer, MemoryOperand operand)
{
	Operands operands = {.reg = 6};
	asmEnsureCapacity(buffer);
	asmInitMemoryOperand(&operands, operand);
	asmEmitRexOperands(buffer, 0, &operands);
	asmEmitUint8(buffer, 0xFF);
	asmEmitOperands(buffer, &operands);
}


static void asmPopq(AssemblerBuffer *buffer, Register dst)
{
	Operands operands = { .reg = 0, .rm = dst };
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, 0, &operands);
	asmEmitUint8(buffer, 0x58 | (dst & 7));
}


static void asmPopqMem(AssemblerBuffer *buffer, MemoryOperand operand)
{
	Operands operands = {.reg = 0};
	asmEnsureCapacity(buffer);
	asmInitMemoryOperand(&operands, operand);
	asmEmitRexOperands(buffer, 0, &operands);
	asmEmitUint8(buffer, 0x8F);
	asmEmitOperands(buffer, &operands);
}


static void asmRet(AssemblerBuffer *buffer)
{
	asmEnsureCapacity(buffer);
	asmEmitUint8(buffer, 0xC3);
}


static void asmMovq(AssemblerBuffer *buffer, Register src, Register dst)
{
	Operands operands = {.mod = MOD_REG, .reg = dst, .rm = src};
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, REX_W, &operands);
	asmEmitUint8(buffer, 0x8B);
	asmEmitOperands(buffer, &operands);
}


static void asmMovqImm(AssemblerBuffer *buffer, int64_t imm, Register dst)
{
	asmEnsureCapacity(buffer);
	asmEmitRex(buffer, REX_W | dst >> 3);
	asmEmitUint8(buffer, 0xB8 | dst);
	asmEmitUint64(buffer, imm);
}


static void asmMovqToMem(AssemblerBuffer *buffer, Register src, MemoryOperand operand)
{
	Operands operands = {.reg = src};
	asmInitMemoryOperand(&operands, operand);
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, REX_W, &operands);
	asmEmitUint8(buffer, 0x89);
	asmEmitOperands(buffer, &operands);
}


static void asmMovqMem(AssemblerBuffer *buffer, MemoryOperand operand, Register dst)
{
	Operands operands = {.reg = dst};
	asmInitMemoryOperand(&operands, operand);
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, REX_W, &operands);
	asmEmitUint8(buffer, 0x8B);
	asmEmitOperands(buffer, &operands);
}


static void asmMovqMemImm(AssemblerBuffer *buffer, int64_t imm, MemoryOperand operand)
{
	ASSERT(INT32_MIN <= imm && imm <= INT32_MAX);
	Operands operands = {.reg = 0};
	asmInitMemoryOperand(&operands, operand);
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, REX_W, &operands);
	asmEmitUint8(buffer, 0xC7);
	asmEmitOperands(buffer, &operands);
	asmEmitInt32(buffer, imm);
}


static void asmMovb(AssemblerBuffer *buffer, ByteRegister src, ByteRegister dst)
{
	Operands operands = {.mod = MOD_REG, .reg = dst, .rm = src};
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, 0, &operands);
	asmEmitUint8(buffer, 0x8A);
	asmEmitOperands(buffer, &operands);
}


static void asmMovbToMem(AssemblerBuffer *buffer, ByteRegister src, MemoryOperand operand)
{
	Operands operands = {.reg = src};
	asmInitMemoryOperand(&operands, operand);
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, 0, &operands);
	asmEmitUint8(buffer, 0x88);
	asmEmitOperands(buffer, &operands);
}


static void asmMovbMem(AssemblerBuffer *buffer, MemoryOperand operand, ByteRegister dst)
{
	Operands operands = {.reg = dst};
	asmInitMemoryOperand(&operands, operand);
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, 0, &operands);
	asmEmitUint8(buffer, 0x8A);
	asmEmitOperands(buffer, &operands);
}


static void asmMovbMemImm(AssemblerBuffer *buffer, int8_t imm, MemoryOperand operand)
{
	Operands operands = {.reg = 0};
	asmInitMemoryOperand(&operands, operand);
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, 0, &operands);
	asmEmitUint8(buffer, 0xC6);
	asmEmitOperands(buffer, &operands);
	asmEmitInt8(buffer, imm);
}


static void asmMovzxbMemq(AssemblerBuffer *buffer, MemoryOperand operand, Register dst)
{
	Operands operands = {.reg = dst};
	asmInitMemoryOperand(&operands, operand);
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, REX_W, &operands);
	asmEmitUint8(buffer, 0x0F);
	asmEmitUint8(buffer, 0xB6);
	asmEmitOperands(buffer, &operands);
}


static void asmMovzxwMemq(AssemblerBuffer *buffer, MemoryOperand operand, Register dst)
{
	Operands operands = {.reg = dst};
	asmInitMemoryOperand(&operands, operand);
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, REX_W, &operands);
	asmEmitUint8(buffer, 0x0F);
	asmEmitUint8(buffer, 0xB7);
	asmEmitOperands(buffer, &operands);
}


static void asmCmovq(AssemblerBuffer *buffer, uint8_t condition, Register src, Register dst)
{
	Operands operands = {.mod = MOD_REG, .reg = dst, .rm = src};
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, REX_W, &operands);
	asmEmitUint8(buffer, 0x0F);
	asmEmitUint8(buffer, 0x40 + condition);
	asmEmitOperands(buffer, &operands);
}


static void asmLeaq(AssemblerBuffer *buffer, MemoryOperand operand, Register dst)
{
	Operands operands = {.reg = dst};
	asmInitMemoryOperand(&operands, operand);
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, REX_W, &operands);
	asmEmitUint8(buffer, 0x8D);
	asmEmitOperands(buffer, &operands);
}


static void asmAddq(AssemblerBuffer *buffer, Register src, Register dst)
{
	Operands operands = {.mod = MOD_REG, .reg = src, .rm = dst};
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, REX_W, &operands);
	asmEmitUint8(buffer, 0x01);
	asmEmitOperands(buffer, &operands);
}


static void asmAddqImm(AssemblerBuffer *buffer, Register reg, int32_t imm)
{
	asmEnsureCapacity(buffer);
	asmEmitAddImm(buffer, ADD, reg, imm);
}


static void asmAddqMem(AssemblerBuffer *buffer, MemoryOperand operand, Register dst)
{
	Operands operands = {.reg = dst};
	asmInitMemoryOperand(&operands, operand);
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, REX_W, &operands);
	asmEmitUint8(buffer, 0x03);
	asmEmitOperands(buffer, &operands);
}


static void asmAddqToMem(AssemblerBuffer *buffer, Register dst, MemoryOperand operand)
{
	Operands operands = {.reg = dst};
	asmInitMemoryOperand(&operands, operand);
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, REX_W, &operands);
	asmEmitUint8(buffer, 0x01);
	asmEmitOperands(buffer, &operands);
}


static void asmAddqMemImm(AssemblerBuffer *buffer, MemoryOperand operand, int32_t imm)
{
	Operands operands = {.reg = 0};
	asmInitMemoryOperand(&operands, operand);
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, REX_W, &operands);
	asmEmitUint8(buffer, 0x81);
	asmEmitOperands(buffer, &operands);
	asmEmitInt32(buffer, imm);
}


static void asmAddb(AssemblerBuffer *buffer, ByteRegister src, ByteRegister dst)
{
	Operands operands = {.mod = MOD_REG, .reg = src, .rm = dst};
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, 0, &operands);
	asmEmitUint8(buffer, 0x00);
	asmEmitOperands(buffer, &operands);
}


static void asmAddbMem(AssemblerBuffer *buffer, MemoryOperand operand, ByteRegister dst)
{
	Operands operands = {.reg = dst};
	asmInitMemoryOperand(&operands, operand);
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, 0, &operands);
	asmEmitUint8(buffer, 0x02);
	asmEmitOperands(buffer, &operands);
}


static void asmSubq(AssemblerBuffer *buffer, Register src, Register dst)
{
	Operands operands = {.mod = MOD_REG, .reg = src, .rm = dst};
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, REX_W, &operands);
	asmEmitUint8(buffer, 0x29);
	asmEmitOperands(buffer, &operands);
}


static void asmSubqImm(AssemblerBuffer *buffer, Register reg, int32_t imm)
{
	asmEnsureCapacity(buffer);
	asmEmitAddImm(buffer, SUB, reg, imm);
}


static void asmSubqMem(AssemblerBuffer *buffer, MemoryOperand operand, Register dst)
{
	Operands operands = {.reg = dst};
	asmInitMemoryOperand(&operands, operand);
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, REX_W, &operands);
	asmEmitUint8(buffer, 0x2B);
	asmEmitOperands(buffer, &operands);
}


static void asmEmitAddImm(AssemblerBuffer *buffer, uint8_t op, Register reg, int32_t imm)
{
	Operands operands = {.mod = MOD_REG, .reg = op, .rm = reg};
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, REX_W, &operands);
	asmEmitUint8(buffer, 0x81);
	asmEmitOperands(buffer, &operands);
	asmEmitInt32(buffer, imm);
}


static void asmImulq(AssemblerBuffer *buffer, Register src, Register dst)
{
	Operands operands = {.mod = MOD_REG, .reg = dst, .rm = src};
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, REX_W, &operands);
	asmEmitUint8(buffer, 0x0F);
	asmEmitUint8(buffer, 0xAF);
	asmEmitOperands(buffer, &operands);
}


static void asmImulqImm(AssemblerBuffer *buffer, Register src, int8_t imm, Register dst)
{
	Operands operands = {.mod = MOD_REG, .reg = dst, .rm = src};
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, REX_W, &operands);
	asmEmitUint8(buffer, 0x6B);
	asmEmitOperands(buffer, &operands);
	asmEmitInt8(buffer, imm);
}


static void asmImulqMem(AssemblerBuffer *buffer, MemoryOperand operand, Register dst)
{
	Operands operands = {.reg = dst};
	asmInitMemoryOperand(&operands, operand);
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, REX_W, &operands);
	asmEmitUint8(buffer, 0x0F);
	asmEmitUint8(buffer, 0xAF);
	asmEmitOperands(buffer, &operands);
}


static void asmIdivq(AssemblerBuffer *buffer, Register divisor)
{
	Operands operands = {.mod = MOD_REG, .reg = 7, .rm = divisor};
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, REX_W, &operands);
	asmEmitUint8(buffer, 0xF7);
	asmEmitOperands(buffer, &operands);
}


static void asmIdivqMem(AssemblerBuffer *buffer, MemoryOperand divisor)
{
	Operands operands = {.reg = 7};
	asmInitMemoryOperand(&operands, divisor);
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, REX_W, &operands);
	asmEmitUint8(buffer, 0xF7);
	asmEmitOperands(buffer, &operands);
}


static void asmSbbq(AssemblerBuffer *buffer, Register src, Register dst)
{
	Operands operands = {.mod = MOD_REG, .reg = src, .rm = dst};
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, REX_W, &operands);
	asmEmitUint8(buffer, 0x19);
	asmEmitOperands(buffer, &operands);
}


static void asmNegq(AssemblerBuffer *buffer, Register dst)
{
	Operands operands = {.mod = MOD_REG, .reg = 3, .rm = dst};
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, REX_W, &operands);
	asmEmitUint8(buffer, 0xF7);
	asmEmitOperands(buffer, &operands);
}


static void asmNegb(AssemblerBuffer *buffer, ByteRegister dst)
{
	Operands operands = {.mod = MOD_REG, .reg = 3, .rm = dst};
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, 0, &operands);
	asmEmitUint8(buffer, 0xF6);
	asmEmitOperands(buffer, &operands);
}


static void asmIncq(AssemblerBuffer *buffer, Register dst)
{
	Operands operands = {.mod = MOD_REG, .reg = 0, .rm = dst};
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, REX_W, &operands);
	asmEmitUint8(buffer, 0xFF);
	asmEmitOperands(buffer, &operands);
}


static void asmIncqMem(AssemblerBuffer *buffer, MemoryOperand operand)
{
	Operands operands = {.reg = 0};
	asmInitMemoryOperand(&operands, operand);
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, REX_W, &operands);
	asmEmitUint8(buffer, 0xFF);
	asmEmitOperands(buffer, &operands);
}


static void asmDecq(AssemblerBuffer *buffer, Register dst)
{
	Operands operands = {.mod = MOD_REG, .reg = 1, .rm = dst};
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, REX_W, &operands);
	asmEmitUint8(buffer, 0xFF);
	asmEmitOperands(buffer, &operands);
}


static void asmAndq(AssemblerBuffer *buffer, Register src, Register dst)
{
	Operands operands = {.mod = MOD_REG, .reg = src, .rm = dst};
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, REX_W, &operands);
	asmEmitUint8(buffer, 0x21);
	asmEmitOperands(buffer, &operands);
}


static void asmAndqImm(AssemblerBuffer *buffer, Register dst, int32_t imm)
{
	Operands operands = {.mod = MOD_REG, .reg = 4, .rm = dst};
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, REX_W, &operands);
	asmEmitUint8(buffer, 0x81);
	asmEmitOperands(buffer, &operands);
	asmEmitInt32(buffer, imm);
}


static void asmAndqMem(AssemblerBuffer *buffer, MemoryOperand operand, Register dst)
{
	Operands operands = {.reg = dst};
	asmInitMemoryOperand(&operands, operand);
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, REX_W, &operands);
	asmEmitUint8(buffer, 0x23);
	asmEmitOperands(buffer, &operands);
}


static void asmAndbImm(AssemblerBuffer *buffer, ByteRegister dst, int8_t imm)
{
	Operands operands = {.mod = MOD_REG, .reg = 4, .rm = dst};
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, 0, &operands);
	asmEmitUint8(buffer, 0x80);
	asmEmitOperands(buffer, &operands);
	asmEmitInt8(buffer, imm);
}


static void asmOrq(AssemblerBuffer *buffer, Register src, Register dst)
{
	Operands operands = {.mod = MOD_REG, .reg = src, .rm = dst};
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, REX_W, &operands);
	asmEmitUint8(buffer, 0x09);
	asmEmitOperands(buffer, &operands);
}


static void asmOrqMem(AssemblerBuffer *buffer, MemoryOperand operand, Register dst)
{
	Operands operands = {.reg = dst};
	asmInitMemoryOperand(&operands, operand);
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, REX_W, &operands);
	asmEmitUint8(buffer, 0x0B);
	asmEmitOperands(buffer, &operands);
}


static void asmOrbMemImm(AssemblerBuffer *buffer, MemoryOperand operand, int8_t imm)
{
	Operands operands = {.reg = 1};
	asmInitMemoryOperand(&operands, operand);
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, 0, &operands);
	asmEmitUint8(buffer, 0x80);
	asmEmitOperands(buffer, &operands);
	asmEmitInt8(buffer, imm);
}


static void asmXorq(AssemblerBuffer *buffer, Register src, Register dst)
{
	Operands operands = {.mod = MOD_REG, .reg = src, .rm = dst};
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, REX_W, &operands);
	asmEmitUint8(buffer, 0x31);
	asmEmitOperands(buffer, &operands);
}


static void asmXorqMem(AssemblerBuffer *buffer, MemoryOperand operand, Register dst)
{
	Operands operands = {.reg = dst};
	asmInitMemoryOperand(&operands, operand);
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, REX_W, &operands);
	asmEmitUint8(buffer, 0x33);
	asmEmitOperands(buffer, &operands);
}


static void asmSarqImm(AssemblerBuffer *buffer, Register dst, uint8_t imm)
{
	asmEnsureCapacity(buffer);
	asmEmitShiftImm(buffer, 7, dst, imm);
}


static void asmShlq(AssemblerBuffer *buffer, Register dst)
{
	asmEnsureCapacity(buffer);
	asmEmitShift(buffer, 4, dst);
}


static void asmShlqImm(AssemblerBuffer *buffer, Register dst, uint8_t imm)
{
	asmEnsureCapacity(buffer);
	asmEmitShiftImm(buffer, 4, dst, imm);
}


static void asmShrq(AssemblerBuffer *buffer, Register dst)
{
	asmEnsureCapacity(buffer);
	asmEmitShift(buffer, 5, dst);
}


static void asmShrqImm(AssemblerBuffer *buffer, Register dst, uint8_t imm)
{

	asmEmitShiftImm(buffer, 5, dst, imm);
}


static void asmEmitShift(AssemblerBuffer *buffer, uint8_t op, Register dst)
{
	Operands operands = {.mod = MOD_REG, .reg = op, .rm = dst};
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, REX_W, &operands);
	asmEmitUint8(buffer, 0xD3);
	asmEmitOperands(buffer, &operands);
}


static void asmEmitShiftImm(AssemblerBuffer *buffer, uint8_t op, Register dst, uint8_t imm)
{
	Operands operands = {.mod = MOD_REG, .reg = op, .rm = dst};
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, REX_W, &operands);
	asmEmitUint8(buffer, 0xC1);
	asmEmitOperands(buffer, &operands);
	asmEmitUint8(buffer, imm);
}


static void asmTestq(AssemblerBuffer *buffer, Register src, Register dst)
{
	Operands operands = {.mod = MOD_REG, .reg = src, .rm = dst};
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, REX_W, &operands);
	asmEmitUint8(buffer, 0x85);
	asmEmitOperands(buffer, &operands);
}


static void asmTestqImm(AssemblerBuffer *buffer, Register reg, int32_t imm)
{
	Operands operands = {.mod = MOD_REG, .reg = 0, .rm = reg};
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, REX_W, &operands);
	asmEmitUint8(buffer, 0xF7);
	asmEmitOperands(buffer, &operands);
	asmEmitInt32(buffer, imm);
}


static void asmTestqMem(AssemblerBuffer *buffer, MemoryOperand operand, Register reg)
{
	Operands operands = {.reg = reg};
	asmInitMemoryOperand(&operands, operand);
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, REX_W, &operands);
	asmEmitUint8(buffer, 0x85);
	asmEmitOperands(buffer, &operands);
}


static void asmTestbImm(AssemblerBuffer *buffer, ByteRegister reg, int8_t imm)
{
	Operands operands = {.mod = MOD_REG, .reg = 0, .rm = reg};
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, 0, &operands);
	asmEmitUint8(buffer, 0xF6);
	asmEmitOperands(buffer, &operands);
	asmEmitInt8(buffer, imm);
}


static void asmTestbMem(AssemblerBuffer *buffer, MemoryOperand operand, ByteRegister reg)
{
	Operands operands = {.reg = reg};
	asmInitMemoryOperand(&operands, operand);
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, 0, &operands);
	asmEmitUint8(buffer, 0x84);
	asmEmitOperands(buffer, &operands);
}


static void asmTestbMemImm(AssemblerBuffer *buffer, MemoryOperand operand, int8_t imm)
{
	Operands operands = {.reg = 0};
	asmInitMemoryOperand(&operands, operand);
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, 0, &operands);
	asmEmitUint8(buffer, 0xF6);
	asmEmitOperands(buffer, &operands);
	asmEmitInt8(buffer, imm);
}


static void asmCmpq(AssemblerBuffer *buffer, Register src, Register dst)
{
	Operands operands = {.mod = MOD_REG, .reg = src, .rm = dst};
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, REX_W, &operands);
	asmEmitUint8(buffer, 0x3B);
	asmEmitOperands(buffer, &operands);
}


static void asmCmpqMem(AssemblerBuffer *buffer, MemoryOperand operand, Register reg)
{
	Operands operands = {.reg = reg};
	asmInitMemoryOperand(&operands, operand);
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, REX_W, &operands);
	asmEmitUint8(buffer, 0x3B);
	asmEmitOperands(buffer, &operands);
}


static void asmCmpqImm(AssemblerBuffer *buffer, Register reg, int32_t imm)
{
	Operands operands = {.mod = MOD_REG, .reg = 7, .rm = reg};
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, REX_W, &operands);
	asmEmitUint8(buffer, 0x81);
	asmEmitOperands(buffer, &operands);
	asmEmitInt32(buffer, imm);
}


static void asmCmpbMem(AssemblerBuffer *buffer, MemoryOperand operand, ByteRegister reg)
{
	Operands operands = {.reg = reg};
	asmInitMemoryOperand(&operands, operand);
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, 0, &operands);
	asmEmitUint8(buffer, 0x3A);
	asmEmitOperands(buffer, &operands);
}


static void asmCmpbMemImm(AssemblerBuffer *buffer, MemoryOperand operand, int8_t imm)
{
	Operands operands = {.reg = 7};
	asmInitMemoryOperand(&operands, operand);
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, 0, &operands);
	asmEmitUint8(buffer, 0x80);
	asmEmitOperands(buffer, &operands);
	asmEmitInt8(buffer, imm);
}


static void asmCmpbImm(AssemblerBuffer *buffer, ByteRegister reg, int8_t imm)
{
	Operands operands = {.mod = MOD_REG, .reg = 7, .rm = reg};
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, 0, &operands);
	asmEmitUint8(buffer, 0x80);
	asmEmitOperands(buffer, &operands);
	asmEmitInt8(buffer, imm);
}


static void asmCqo(AssemblerBuffer *buffer)
{
	asmEnsureCapacity(buffer);
	asmEmitRex(buffer, REX_W);
	asmEmitUint8(buffer, 0x99);
}


static void asmCallq(AssemblerBuffer *buffer, Register src)
{
	Operands operands = {.mod = MOD_REG, .reg = CALL_ABSOLUTE, .rm = src};
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, 0, &operands);
	asmEmitUint8(buffer, 0xFF);
	asmEmitOperands(buffer, &operands);
}


static void asmCallqMem(AssemblerBuffer *buffer, MemoryOperand operand)
{
	Operands operands = {.reg = CALL_ABSOLUTE};
	asmInitMemoryOperand(&operands, operand);
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, 0, &operands);
	asmEmitUint8(buffer, 0xFF);
	asmEmitOperands(buffer, &operands);
}


static void asmJmpq(AssemblerBuffer *buffer, Register src)
{
	Operands operands = {.mod = MOD_REG, .reg = JUMP_ABSOLUTE, .rm = src};
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, 0, &operands);
	asmEmitUint8(buffer, 0xFF);
	asmEmitOperands(buffer, &operands);
}


static void asmJmpqMem(AssemblerBuffer *buffer, MemoryOperand operand)
{
	Operands operands = {.reg = 4};
	asmInitMemoryOperand(&operands, operand);
	asmEnsureCapacity(buffer);
	asmEmitRexOperands(buffer, 0, &operands);
	asmEmitUint8(buffer, 0xFF);
	asmEmitOperands(buffer, &operands);
}


static void asmJmpdImm(AssemblerBuffer *buffer, int32_t imm)
{
	asmEnsureCapacity(buffer);
	asmEmitUint8(buffer, 0xE9);
	asmEmitInt32(buffer, imm);
}


static void asmJmpLabel(AssemblerBuffer *buffer, AssemblerLabel *label)
{
	asmEnsureCapacity(buffer);
	asmEmitUint8(buffer, 0xE9);
	asmEmitLabel32(buffer, label);
}


static void asmJ(AssemblerBuffer *buffer, uint8_t condition, AssemblerLabel *label)
{
	asmEnsureCapacity(buffer);
	asmEmitUint8(buffer, 0x0F);
	asmEmitUint8(buffer, 0x80 + condition);
	asmEmitLabel32(buffer, label);
}


static void asmInt3(AssemblerBuffer *buffer)
{
	asmEnsureCapacity(buffer);
	asmEmitUint8(buffer, 0xCC);
}


static void asmInitMemoryOperand(Operands *operands, MemoryOperand operand)
{
	if (operand.base == RIP) {
		operands->mod = MOD_MEM;
		operand.base = 5;
	} else if (operand.disp == 0) {
		operands->mod = MOD_MEM;
	} else if (INT8_MIN <= operand.disp && operand.disp <= INT8_MAX) {
		operands->mod = MOD_DISP8;
	} else {
		operands->mod = MOD_DISP32;
	}
	if (operand.index != NO_REGISTER) {
		operands->rm = (operand.base & 8) | SIB;
		operands->base = operand.base;
		operands->index = operand.index;
		operands->scale = operand.scale;
	} else {
		operands->rm = operand.base;
		operands->index = 4; // none index in case SIB is emited
		operands->base = operands->rm; // in case of R12 which collides with SIB
	}
	operands->disp = operand.disp;
}


static void asmEmitRexOperands(AssemblerBuffer *buffer, uint8_t rex, Operands *operands)
{
	rex = rex | (operands->reg & 8) >> 1 | (operands->rm & 8) >> 3;
	if ((operands->rm & 7) == SIB) {
		rex = rex | (operands->index & 8) >> 2;
	}
	if (operands->reg >= 20 || operands->rm >= 20) {
		asmEmitRex(buffer, rex);
	} else if (rex != 0) {
		asmEmitRex(buffer, rex);
	}
}


static void asmEmitRex(AssemblerBuffer *buffer, uint8_t rex)
{
	asmEmitUint8(buffer, 0x40 | rex);
}


static void asmEmitOperands(AssemblerBuffer *buffer, Operands *operands)
{
	ASSERT(operands->mod <= 3);
	asmEmitUint8(buffer, (operands->mod & 7) << 6 | (operands->reg & 7) << 3 | (operands->rm & 7));
	if (operands->mod != MOD_REG && (operands->rm & 7) == SIB) {
		asmEmitUint8(buffer, operands->scale << 6 | (operands->index & 7) << 3 | (operands->base & 7));
	}
	if (operands->mod == MOD_DISP8) {
		asmEmitInt8(buffer, operands->disp);
	} else if (operands->mod == MOD_DISP32 || (operands->mod == MOD_MEM && operands->rm == 5)) {
		asmEmitInt32(buffer, operands->disp);
	}
}

#endif
