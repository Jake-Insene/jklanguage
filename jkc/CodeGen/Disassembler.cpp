#include "jkc/CodeGen/Disassembler.h"
#include "jkr/CodeFile/Header.h"
#include "jkr/CodeFile/Function.h"
#include "jkr/CodeFile/OpCodes.h"

namespace CodeGen {

Str Registers[] = {
	STR("r0"),
	STR("r1"),
	STR("r2"),
	STR("r3"),
	STR("r4"),
	STR("r5"),
	STR("r6"),
	STR("r7"),
	STR("r8"),
	STR("r9"),
	STR("r10"),
	STR("r11"),
	STR("r12"),
	STR("r13"),
	STR("r14"),
	STR("rmem"),
};

static void DisStack(StreamOutput& Output, StreamInput& File, codefile::OpCodeStack S, Uint16& i) {
	switch (S) {
	case codefile::OpCodeStack::Push:
	{
		Byte reg = 0;
		File.Read(&reg, 1);
		i++;
		Output.Println(STR("push {s}"), Registers[reg]);
	}
		break;
	case codefile::OpCodeStack::LocalPush:
	{
		codefile::LocalType local = 0;
		File.Read(Cast<Byte*>(&local), sizeof(codefile::LocalType));
		i += sizeof(codefile::LocalType);
		Output.Println(STR("local_push #{" LOCAL_PREFIX "}"), local);
	}
		break;
	case codefile::OpCodeStack::GlobalPush:
	{
		codefile::GlobalType global = 0;
		File.Read(Cast<Byte*>(&global), sizeof(codefile::GlobalType));
		i += sizeof(codefile::LocalType);
		Output.Println(STR("global_push #{" GLOBAL_PREFIX "}"), global);
	}
	break;
		break;
	case codefile::OpCodeStack::Push8:
	{
		Byte c = 0;
		File.Read(&c, 1);
		i++;
		Output.Println(STR("push #0x{xb}"), c);
	}
		break;
	case codefile::OpCodeStack::Push16:
	{
		Uint16 c = 0;
		File.Read(Cast<Byte*>(&c), 1);
		i += 2;
		Output.Println(STR("push #0x{xw}"), c);
	}
		break;
	case codefile::OpCodeStack::Push32:
	{
		Uint32 c = 0;
		File.Read(Cast<Byte*>(&c), 1);
		i += 4;
		Output.Println(STR("push #0x{x}"), c);
	}
		break;
	case codefile::OpCodeStack::Push64:
	{
		Uint64 c = 0;
		File.Read(Cast<Byte*>(&c), 1);
		i += 8;
		Output.Println(STR("push #0x{X}"), c);
	}
		break;
	case codefile::OpCodeStack::PopTop:
		Output.Println(STR("poptop"));
		break;
	case codefile::OpCodeStack::Pop:
	{
		Byte reg = 0;
		File.Read(&reg, 1);
		i++;
		Output.Println(STR("pop {s}"), Registers[reg]);
	}
		break;
	default:
		Output.Println(STR("Invalid Opcode {b}"), IntCast<Byte>(S));
		break;
	}
}

static void DisCode(StreamOutput& Output, StreamInput& File, Uint32 Size) {
	Uint16 i = 0;

	while (i < Size) {
		codefile::OpCode opcode = codefile::OpCode::Brk;
		File.Read(Cast<Byte*>(&opcode), 1);
		i++;

		Output.WriteArray({ '\t' });

		Output.Print(STR("{xw:0} "), i - 1);

		switch (opcode) {
		case codefile::OpCode::Brk:
			Output.Println(STR("brk"));
			break;
		case codefile::OpCode::Mov:
		{
			codefile::MIL bil = {};
			File.Read(Cast<Byte*>(&bil), sizeof(codefile::MIL));
			i += sizeof(codefile::MIL);
			Output.Println(STR("mov {s}, {s}"), Registers[bil.Dest], Registers[bil.Src]);
		}
			break;
		case codefile::OpCode::Const4:
		{
			Byte b = 0;
			File.Read(&b, 1);
			i += 1;
			Output.Println(STR("mov.c4 {s}, #0x{xb}"), Registers[b & 0xF], Byte(b >> 4));
		}
		break;
		case codefile::OpCode::Const8:
		{
			i += 2;
			Byte reg = 0;
			Byte c = 0;
			File.Read(&reg, 1);
			File.Read(&c, sizeof(c));
			Output.Println(STR("mov.c8 {s}, #0x{xb}"), Registers[reg], c);
		}
			break;
		case codefile::OpCode::Const16:
		{
			i += 3;
			Byte reg = 0;
			Uint16 c = 0;
			File.Read(&reg, 1);
			File.Read(Cast<Byte*>(&c), sizeof(c));
			Output.Println(STR("mov.c16 {s}, #0x{xw}"), Registers[reg], c);
		}
			break;
		case codefile::OpCode::Const32:
		{
			i += 5;
			Byte reg = 0;
			Uint32 c = 0;
			File.Read(&reg, 1);
			File.Read(Cast<Byte*>(&c), sizeof(c));
			Output.Println(STR("mov.c32 {s}, #0x{x}"), Registers[reg], c);
		}
			break;
		case codefile::OpCode::Const64:
		{
			i += 9;
			Byte reg = 0;
			UInt c = 0;
			File.Read(&reg, 1);
			File.Read(Cast<Byte*>(&c), sizeof(c));
			Output.Println(STR("mov.c64 {s}, #0x{X}"), Registers[reg], c);
		}
			break;
		case codefile::OpCode::MovRes:
		{
			i++;
			Byte reg = 0;
			File.Read(&reg, 1);
			Output.Println(STR("mov.res {s}"), Registers[reg]);
		}
			break;
		case codefile::OpCode::LocalSet4:
		{
			i++;
			Byte op = 0;
			File.Read(&op, 1);
			Output.Println(STR("local.set4 #0x{x" LOCAL_PREFIX ":0}, {s}"), Byte(op >> 4), Registers[op & 0xF]);
		}
		break;
		case codefile::OpCode::LocalGet4:
		{
			i++;
			Byte op = 0;
			File.Read(&op, 1);
			Output.Println(STR("local.get4 {s}, #0x{x" LOCAL_PREFIX ":0}"), Registers[op&0xF], Byte(op>>4));
		}
		break;
		case codefile::OpCode::LocalSet:
		{
			codefile::LIL lil = {};
			File.Read(Cast<Byte*>(&lil), sizeof(codefile::LIL));
			i += sizeof(codefile::LIL);
			Output.Println(STR("local.set #0x{x" LOCAL_PREFIX ":0}, {s}"), lil.Index, Registers[lil.SrcDest]);
		}
			break;
		case codefile::OpCode::LocalGet:
		{
			codefile::LIL lil = {};
			File.Read(Cast<Byte*>(&lil), sizeof(codefile::LIL));
			i += sizeof(codefile::LIL);
			Output.Println(STR("local.get {s}, #0x{x" LOCAL_PREFIX ":0}"), Registers[lil.SrcDest], lil.Index);
		}
			break;
		case codefile::OpCode::GlobalSet:
			break;
		case codefile::OpCode::GlobalGet:
			break;
		case codefile::OpCode::Inc:
		{
			i++;
			Byte reg = 0;
			File.Read(&reg, 1);
			Output.Println(STR("inc {s}"), Registers[reg]);
		}
		break;
		case codefile::OpCode::IInc:
		{
			i++;
			Byte reg = 0;
			File.Read(&reg, 1);
			Output.Println(STR("iinc {s}"), Registers[reg]);
		}
		break;
		case codefile::OpCode::FInc:
		{
			i++;
			Byte reg = 0;
			File.Read(&reg, 1);
			Output.Println(STR("finc {s}"), Registers[reg]);
		}
			break;
		case codefile::OpCode::Dec:
		{
			i++;
			Byte reg = 0;
			File.Read(&reg, 1);
			Output.Println(STR("dec {s}"), Registers[reg]);
		}
		break;
		case codefile::OpCode::IDec:
		{
			i++;
			Byte reg = 0;
			File.Read(&reg, 1);
			Output.Println(STR("idec {s}"), Registers[reg]);
		}
		break;
		case codefile::OpCode::FDec:
		{
			i++;
			Byte reg = 0;
			File.Read(&reg, 1);
			Output.Println(STR("fdec {s}"), Registers[reg]);
		}
		break;
		case codefile::OpCode::Add:
		{
			codefile::CIL cil = {};
			File.Read(Cast<Byte*>(&cil), sizeof(codefile::CIL));
			i += sizeof(codefile::CIL);
			Output.Println(STR("add {s}, {s}, {s}"), Registers[cil.Dest], Registers[cil.Src1], Registers[cil.Src2]);
		}
			break;
		case codefile::OpCode::IAdd:
		{
			codefile::CIL cil = {};
			File.Read(Cast<Byte*>(&cil), sizeof(codefile::CIL));
			i += sizeof(codefile::CIL);
			Output.Println(STR("iadd {s}, {s}, {s}"), Registers[cil.Dest], Registers[cil.Src1], Registers[cil.Src2]);
		}
			break;
		case codefile::OpCode::FAdd:
		{
			codefile::CIL cil = {};
			File.Read(Cast<Byte*>(&cil), sizeof(codefile::CIL));
			i += sizeof(codefile::CIL);
			Output.Println(STR("fadd {s}, {s}, {s}"), Registers[cil.Dest], Registers[cil.Src1], Registers[cil.Src2]);
		}
			break;
		case codefile::OpCode::Sub:
		{
			codefile::CIL cil = {};
			File.Read(Cast<Byte*>(&cil), sizeof(codefile::CIL));
			i += sizeof(codefile::CIL);
			Output.Println(STR("sub {s}, {s}, {s}"), Registers[cil.Dest], Registers[cil.Src1], Registers[cil.Src2]);
		}
			break;
		case codefile::OpCode::ISub:
		{
			codefile::CIL cil = {};
			File.Read(Cast<Byte*>(&cil), sizeof(codefile::CIL));
			i += sizeof(codefile::CIL);
			Output.Println(STR("isub {s}, {s}, {s}"), Registers[cil.Dest], Registers[cil.Src1], Registers[cil.Src2]);
		}
			break;
		case codefile::OpCode::FSub:
		{
			codefile::CIL cil = {};
			File.Read(Cast<Byte*>(&cil), sizeof(codefile::CIL));
			i += sizeof(codefile::CIL);
			Output.Println(STR("fsub {s}, {s}, {s}"), Registers[cil.Dest], Registers[cil.Src1], Registers[cil.Src2]);
		}
			break;
		case codefile::OpCode::Mul:
		{
			codefile::CIL cil = {};
			File.Read(Cast<Byte*>(&cil), sizeof(codefile::CIL));
			i += sizeof(codefile::CIL);
			Output.Println(STR("mul {s}, {s}, {s}"), Registers[cil.Dest], Registers[cil.Src1], Registers[cil.Src2]);
		}
			break;
		case codefile::OpCode::IMul:
		{
			codefile::CIL cil = {};
			File.Read(Cast<Byte*>(&cil), sizeof(codefile::CIL));
			i += sizeof(codefile::CIL);
			Output.Println(STR("imul {s}, {s}, {s}"), Registers[cil.Dest], Registers[cil.Src1], Registers[cil.Src2]);
		}
			break;
		case codefile::OpCode::FMul:
		{
			codefile::CIL cil = {};
			File.Read(Cast<Byte*>(&cil), sizeof(codefile::CIL));
			i += sizeof(codefile::CIL);
			Output.Println(STR("fmul {s}, {s}, {s}"), Registers[cil.Dest], Registers[cil.Src1], Registers[cil.Src2]);
		}
			break;
		case codefile::OpCode::Div:
		{
			codefile::CIL cil = {};
			File.Read(Cast<Byte*>(&cil), sizeof(codefile::CIL));
			i += sizeof(codefile::CIL);
			Output.Println(STR("div {s}, {s}, {s}"), Registers[cil.Dest], Registers[cil.Src1], Registers[cil.Src2]);
		}
			break;
		case codefile::OpCode::IDiv:
		{
			codefile::CIL cil = {};
			File.Read(Cast<Byte*>(&cil), sizeof(codefile::CIL));
			i += sizeof(codefile::CIL);
			Output.Println(STR("idiv {s}, {s}, {s}"), Registers[cil.Dest], Registers[cil.Src1], Registers[cil.Src2]);
		}
			break;
		case codefile::OpCode::FDiv:
		{
			codefile::CIL cil = {};
			File.Read(Cast<Byte*>(&cil), sizeof(codefile::CIL));
			i += sizeof(codefile::CIL);
			Output.Println(STR("fdiv {s}, {s}, {s}"), Registers[cil.Dest], Registers[cil.Src1], Registers[cil.Src2]);
		}
			break;
		case codefile::OpCode::Add8:
		{
			codefile::CIL cil = {};
			File.Read(Cast<Byte*>(&cil), sizeof(codefile::CIL));
			i += sizeof(codefile::CIL);
			Output.Println(
				STR("add.c8 {s}, {s}, #0x{xb:0}"),
				Registers[cil.Dest], Registers[cil.Src1],
				Byte(cil.Src2 | (cil.Extra << 4))
			);
		}
		break;
		case codefile::OpCode::Add16:
		{
			codefile::CIL cil = {};
			File.Read(Cast<Byte*>(&cil), sizeof(codefile::CIL));
			Uint16 c = 0;
			File.Read(Cast<Byte*>(&c), 2);
			i += 3;
			Output.Println(
				STR("add.c16 {s}, {s}, #0x{xw:0}"), 
				Registers[cil.Dest], Registers[cil.Src1], 
				c
			);
		}
		break;
		case codefile::OpCode::IAdd8:
		{
			codefile::CIL cil = {};
			File.Read(Cast<Byte*>(&cil), sizeof(codefile::CIL));
			i += sizeof(codefile::CIL);
			Output.Println(
				STR("iadd.c8 {s}, {s}, #0x{xb:0}"),
				Registers[cil.Dest], Registers[cil.Src1],
				Byte(cil.Src2 | (cil.Extra << 4))
			);
		}
		break;
		case codefile::OpCode::IAdd16:
		{
			codefile::CIL cil = {};
			File.Read(Cast<Byte*>(&cil), sizeof(codefile::CIL));
			Uint16 c = 0;
			File.Read(Cast<Byte*>(&c), 2);
			i += 3;
			Output.Println(
				STR("iadd.c16 {s}, {s}, #0x{xw:0}"),
				Registers[cil.Dest], Registers[cil.Src1],
				c
			);
		}
		break;
		case codefile::OpCode::Sub8:
		{
			codefile::CIL cil = {};
			File.Read(Cast<Byte*>(&cil), sizeof(codefile::CIL));
			i += sizeof(codefile::CIL);
			Output.Println(
				STR("sub.c8 {s}, {s}, #0x{xb:0}"),
				Registers[cil.Dest], Registers[cil.Src1],
				Byte(cil.Src2 | (cil.Extra << 4))
			);
		}
		break;
		case codefile::OpCode::Sub16:
		{
			codefile::CIL cil = {};
			File.Read(Cast<Byte*>(&cil), sizeof(codefile::CIL));
			Uint16 c = 0;
			File.Read(Cast<Byte*>(&c), 2);
			i += 3;
			Output.Println(
				STR("sub.c16 {s}, {s}, #0x{xw:0}"),
				Registers[cil.Dest], Registers[cil.Src1],
				c
			);
		}
		break;
		case codefile::OpCode::ISub8:
		{
			codefile::CIL cil = {};
			File.Read(Cast<Byte*>(&cil), sizeof(codefile::CIL));
			i += sizeof(codefile::CIL);
			Output.Println(
				STR("isub.c8 {s}, {s}, #0x{xb:0}"),
				Registers[cil.Dest], Registers[cil.Src1],
				Byte(cil.Src2 | (cil.Extra << 4))
			);
		}
		break;
		case codefile::OpCode::ISub16:
		{
			codefile::CIL cil = {};
			File.Read(Cast<Byte*>(&cil), sizeof(codefile::CIL));
			Uint16 c = 0;
			File.Read(Cast<Byte*>(&c), 2);
			i += 3;
			Output.Println(
				STR("isub.c16 {s}, {s}, #0x{xw:0}"),
				Registers[cil.Dest], Registers[cil.Src1],
				c
			);
		}
		break;
		case codefile::OpCode::Mul8:
		{
			codefile::CIL cil = {};
			File.Read(Cast<Byte*>(&cil), sizeof(codefile::CIL));
			i += sizeof(codefile::CIL);
			Output.Println(
				STR("mul.c8 {s}, {s}, #0x{xb:0}"),
				Registers[cil.Dest], Registers[cil.Src1],
				Byte(cil.Src2 | (cil.Extra << 4))
			);
		}
		break;
		case codefile::OpCode::Mul16:
		{
			codefile::CIL cil = {};
			File.Read(Cast<Byte*>(&cil), sizeof(codefile::CIL));
			Uint16 c = 0;
			File.Read(Cast<Byte*>(&c), 2);
			i += 3;
			Output.Println(
				STR("mul.c16 {s}, {s}, #0x{xw:0}"),
				Registers[cil.Dest], Registers[cil.Src1],
				c
			);
		}
		break;
		case codefile::OpCode::IMul8:
		{
			codefile::CIL cil = {};
			File.Read(Cast<Byte*>(&cil), sizeof(codefile::CIL));
			i += sizeof(codefile::CIL);
			Output.Println(
				STR("imul.c8 {s}, {s}, #0x{xb:0}"),
				Registers[cil.Dest], Registers[cil.Src1],
				Byte(cil.Src2 | (cil.Extra << 4))
			);
		}
		break;
		case codefile::OpCode::IMul16:
		{
			codefile::CIL cil = {};
			File.Read(Cast<Byte*>(&cil), sizeof(codefile::CIL));
			Uint16 c = 0;
			File.Read(Cast<Byte*>(&c), 2);
			i += 3;
			Output.Println(
				STR("imul.c16 {s}, {s}, #0x{xw:0}"),
				Registers[cil.Dest], Registers[cil.Src1],
				c
			);
		}
		break;
		case codefile::OpCode::Div8:
		{
			codefile::CIL cil = {};
			File.Read(Cast<Byte*>(&cil), sizeof(codefile::CIL));
			i += sizeof(codefile::CIL);
			Output.Println(
				STR("div.c8 {s}, {s}, #0x{xb:0}"),
				Registers[cil.Dest], Registers[cil.Src1],
				Byte(cil.Src2 | (cil.Extra << 4))
			);
		}
		break;
		case codefile::OpCode::Div16:
		{
			codefile::CIL cil = {};
			File.Read(Cast<Byte*>(&cil), sizeof(codefile::CIL));
			Uint16 c = 0;
			File.Read(Cast<Byte*>(&c), 2);
			i += 3;
			Output.Println(
				STR("div.c16 {s}, {s}, #0x{xw:0}"),
				Registers[cil.Dest], Registers[cil.Src1],
				c
			);
		}
		break;
		case codefile::OpCode::IDiv8:
		{
			codefile::CIL cil = {};
			File.Read(Cast<Byte*>(&cil), sizeof(codefile::CIL));
			i += sizeof(codefile::CIL);
			Output.Println(
				STR("idiv.c8 {s}, {s}, #0x{xb:0}"),
				Registers[cil.Dest], Registers[cil.Src1],
				Byte(cil.Src2 | (cil.Extra << 4))
			);
		}
		break;
		case codefile::OpCode::IDiv16:
		{
			codefile::CIL cil = {};
			File.Read(Cast<Byte*>(&cil), sizeof(codefile::CIL));
			Uint16 c = 0;
			File.Read(Cast<Byte*>(&c), 2);
			i += 3;
			Output.Println(
				STR("idiv.c16 {s}, {s}, #0x{xw:0}"),
				Registers[cil.Dest], Registers[cil.Src1],
				c
			);
		}
		break;
		case codefile::OpCode::Cmp:
		{
			codefile::MIL bil = {};
			File.Read(Cast<Byte*>(&bil), sizeof(codefile::MIL));
			i += sizeof(codefile::MIL);
			Output.Println(STR("cmp {s}, {s}"), Registers[bil.Dest], Registers[bil.Src]);
		}
			break;
		case codefile::OpCode::ICmp:
		{
			codefile::MIL bil = {};
			File.Read(Cast<Byte*>(&bil), sizeof(codefile::MIL));
			i += sizeof(codefile::MIL);
			Output.Println(STR("icmp {s}, {s}"), Registers[bil.Dest], Registers[bil.Src]);
		}
			break;
		case codefile::OpCode::FCmp:
		{
			codefile::MIL bil = {};
			File.Read(Cast<Byte*>(&bil), sizeof(codefile::MIL));
			i += sizeof(codefile::MIL);
			Output.Println(STR("fcmp {s}, {s}"), Registers[bil.Dest], Registers[bil.Src]);
		}
			break;
		case codefile::OpCode::Jmp:
		{
			codefile::AddressType ip = 0;
			File.Read(Cast<Byte*>(&ip), sizeof(codefile::AddressType));
			i += sizeof(codefile::AddressType);
			Output.Println(STR("jmp #0x{x:0}"), ip);
		}
			break;
		case codefile::OpCode::Jez:
		{
			codefile::AddressType ip = 0;
			File.Read(Cast<Byte*>(&ip), sizeof(codefile::AddressType));
			i += sizeof(codefile::AddressType);
			Output.Println(STR("jez #0x{x:0}"), ip);
		}
			break;
		case codefile::OpCode::Jnz:
		{
			codefile::AddressType ip = 0;
			File.Read(Cast<Byte*>(&ip), sizeof(codefile::AddressType));
			i += sizeof(codefile::AddressType);
			Output.Println(STR("jnz #0x{x:0}"), ip);
		}
			break;
		case codefile::OpCode::Je:
		{
			codefile::AddressType ip = 0;
			File.Read(Cast<Byte*>(&ip), sizeof(codefile::AddressType));
			i += sizeof(codefile::AddressType);
			Output.Println(STR("je #0x{x:0}"), ip);
		}
			break;
		case codefile::OpCode::Jne:
		{
			codefile::AddressType ip = 0;
			File.Read(Cast<Byte*>(&ip), sizeof(codefile::AddressType));
			i += sizeof(codefile::AddressType);
			Output.Println(STR("jne #0x{x:0}"), ip);
		}
			break;
		case codefile::OpCode::Jl:
		{
			codefile::AddressType ip = 0;
			File.Read(Cast<Byte*>(&ip), sizeof(codefile::AddressType));
			i += sizeof(codefile::AddressType);
			Output.Println(STR("je #0x{x:0}"), ip);
		}
			break;
		case codefile::OpCode::Jle:
		{
			codefile::AddressType ip = 0;
			File.Read(Cast<Byte*>(&ip), sizeof(codefile::AddressType));
			i += sizeof(codefile::AddressType);
			Output.Println(STR("jle #0x{x:0}"), ip);
		}
			break;
		case codefile::OpCode::Jg:
		{
			codefile::AddressType ip = 0;
			File.Read(Cast<Byte*>(&ip), sizeof(codefile::AddressType));
			i += sizeof(codefile::AddressType);
			Output.Println(STR("jg #0x{x:0}"), ip);
		}
			break;
		case codefile::OpCode::Jge:
		{
			codefile::AddressType ip = 0;
			File.Read(Cast<Byte*>(&ip), sizeof(codefile::AddressType));
			i += sizeof(codefile::AddressType);
			Output.Println(STR("jge #0x{x:0}"), ip);
		}
			break;
		case codefile::OpCode::Jmp8:
		{
			Byte ip = 0;
			File.Read(&ip, 1);
			i++;
			Output.Println(STR("jmp.c8 #0x{xb:0}"), ip);
		}
		break;
		case codefile::OpCode::Jez8:
		{
			Byte ip = 0;
			File.Read(&ip, 1);
			i++;
			Output.Println(STR("jez.c8 #0x{xb:0}"), ip);
		}
		break;
		case codefile::OpCode::Jnz8:
		{
			Byte ip = 0;
			File.Read(&ip, 1);
			i++;
			Output.Println(STR("jnz.c8 #0x{xb:0}"), ip);
		}
		break;
		case codefile::OpCode::Je8:
		{
			Byte ip = 0;
			File.Read(&ip, 1);
			i++;
			Output.Println(STR("je.c8 #0x{xb:0}"), ip);
		}
		break;
		case codefile::OpCode::Jne8:
		{
			Byte ip = 0;
			File.Read(&ip, 1);
			i++;
			Output.Println(STR("jne.c8 #0x{xb:0}"), ip);
		}
		break;
		case codefile::OpCode::Jl8:
		{
			Byte ip = 0;
			File.Read(&ip, 1);
			i++;
			Output.Println(STR("jl.c8 #0x{xb:0}"), ip);
		}
		break;
		case codefile::OpCode::Jle8:
		{
			Byte ip = 0;
			File.Read(&ip, 1);
			i++;
			Output.Println(STR("jle.c8 #0x{xb:0}"), ip);
		}
		break;
		case codefile::OpCode::Jg8:
		{
			Byte ip = 0;
			File.Read(&ip, 1);
			i++;
			Output.Println(STR("jg.c8 #0x{xb:0}"), ip);
		}
		break;
		case codefile::OpCode::Jge8:
		{
			Byte ip = 0;
			File.Read(&ip, 1);
			i++;
			Output.Println(STR("jge.c8 #0x{xb:0}"), ip);
		}
		break;
		case codefile::OpCode::Call:
		{
			codefile::FunctionType index = 0;
			File.Read(Cast<Byte*>(&index), 4);
			i += sizeof(codefile::FunctionType);
			Output.Println(STR("call #0x{x:0}"), index);
		}
			break;
		case codefile::OpCode::Call8:
		{
			Byte index = 0;
			File.Read(&index, 1);
			i++;
			Output.Println(STR("call.8 #0x{xb:0}"), index);
		}
		break;
		case codefile::OpCode::RetVoid:
			Output.Println(STR("ret.void"));
			break;
		case codefile::OpCode::Ret:
		{
			Byte reg = 0;
			File.Read(&reg, 1);
			i++;
			Output.Println(STR("ret {s}"), Registers[reg]);
		}
			break;
		case codefile::OpCode::Ret8:
		{
			Byte c = 0;
			File.Read(&c, 1);
			i++;
			Output.Println(STR("ret.c8 #0x{xb:0}"), c);
		}
		break;
		case codefile::OpCode::Ret16:
		{
			Uint16 c = 0;
			File.Read(Cast<Byte*>(&c), 1);
			i+=2;
			Output.Println(STR("ret.c16 #0x{xw:0}"), c);
		}
		break;
		case codefile::OpCode::RetLocal:
		{
			codefile::LocalType local = 0;
			File.Read(Cast<Byte*>(&local), sizeof(codefile::LocalType));
			i += sizeof(codefile::LocalType);
			Output.Println(STR("local.ret #0x{x" LOCAL_PREFIX "}"), local);
		}
			break;
		case codefile::OpCode::RetGlobal:
		{
			codefile::GlobalType global = 0;
			File.Read(Cast<Byte*>(&global), sizeof(codefile::LocalType));
			i += sizeof(codefile::GlobalType);
			Output.Println(STR("global.ret #0x{x}"), global);
		}
			break;
		case codefile::OpCode::MemoryPrefix:
			break;
		case codefile::OpCode::StackPrefix:
		{
			codefile::OpCodeStack stack = codefile::OpCodeStack::Push;
			File.Read(Cast<Byte*>(&stack), 1);
			i++;
			DisStack(Output, File, stack, i);
		}
			break;
		default:
			Output.Println(STR("Invalid Opcode 0x{xb}"), IntCast<Byte>(opcode));
			break;
		}
	}
}

bool Dis(StreamOutput& Output, Str FilePath) {
	io::File file = io::File::Open(FilePath, io::FileRead | io::FileBinary);
	if (file.Err != io::Success) {
		return false;
	}

	if (file.Size() < sizeof(codefile::FileHeader)) {
		return false;
	}

	codefile::FileHeader header{};
	file.Read(Cast<Byte*>(&header), sizeof(codefile::FileHeader));
	if (file.Size() != header.CheckSize)
		return false;
	
	Output.Println(STR(".attribs 0x{xw}"), header.Attributes);
	Output.Println(STR(".version {w}.{w}"), header.MajorVersion, header.MinorVersion);
	Output.Println(STR(".functions {d}"), header.CountOfFunctions);
	Output.Println(STR(".objects {d}"), header.CountOfObjects);
	Output.Println(STR(".globals {d}"), header.CountOfGlobals);
	Output.Println(STR(".imports {d}"), header.CountOfImports);
	Output.Println(STR(".exports {d}"), header.CountOfExports);
	Output.Println(STR(".strings {d}"), header.CountOfExports);
	Output.Println(STR(".start {" FUNCTION_PREFIX "}\n"), header.EntryPoint);

	for (Uint32 i = 0; i < header.CountOfFunctions; i++) {
		codefile::FunctionHeader fn = {};
		file.Read(Cast<Byte*>(&fn), sizeof(codefile::FunctionHeader));

		Output.Println(STR(".function {" FUNCTION_PREFIX "}:"), i);
		Output.Println(STR("\t.attribs {d}"), fn.Attributes);
		Output.Println(STR("\t.args {d}"), fn.Arguments);
		Output.Println(STR("\t.locals {d}"), fn.LocalCount);
		Output.Println(STR("\t.size {d}"), fn.SizeOfCode);
		Output.Println(STR("\t.code"));

		DisCode(Output, file, fn.SizeOfCode);
		Output.WriteArray({ '\n' });
	}

	file.Close();

	return true;
}

}
