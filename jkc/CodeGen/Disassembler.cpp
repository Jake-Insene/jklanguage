#include "jkc/CodeGen/Disassembler.h"
#include "jkr/CodeFile/Header.h"
#include "jkr/CodeFile/Function.h"
#include "jkr/CodeFile/Global.h"
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

static void DisCode(StreamOutput& Output, StreamInput& File, UInt32 Size) {
	UInt16 i = 0;

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
		case codefile::OpCode::Mov4:
		{
			Byte b = 0;
			File.Read(&b, 1);
			i += 1;
			Output.Println(STR("mov.c4 {s}, #0x{xb:0}"), Registers[b & 0xF], Byte(b >> 4));
		}
		break;
		case codefile::OpCode::Mov8:
		{
			i += 2;
			Byte reg = 0;
			Byte c = 0;
			File.Read(&reg, 1);
			File.Read(&c, 1);
			Output.Println(STR("mov.c8 {s}, #0x{xb:0}"), Registers[reg], c);
		}
			break;
		case codefile::OpCode::Mov16:
		{
			i += 3;
			Byte reg = 0;
			UInt16 c = 0;
			File.Read(&reg, 1);
			File.Read(Cast<Byte*>(&c), sizeof(c));
			Output.Println(STR("mov.c16 {s}, #0x{xw:0}"), Registers[reg], c);
		}
			break;
		case codefile::OpCode::Mov32:
		{
			i += 5;
			Byte reg = 0;
			UInt32 c = 0;
			File.Read(&reg, 1);
			File.Read(Cast<Byte*>(&c), sizeof(c));
			Output.Println(STR("mov.c32 {s}, #0x{x:0}"), Registers[reg], c);
		}
			break;
		case codefile::OpCode::Mov64:
		{
			i += 9;
			Byte reg = 0;
			UInt c = 0;
			File.Read(&reg, 1);
			File.Read(Cast<Byte*>(&c), sizeof(c));
			Output.Println(STR("mov.c64 {s}, #0x{X:0}"), Registers[reg], c);
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
		{
			codefile::GIL gil = {}; 
			File.Read(Cast<Byte*>(&gil), sizeof(codefile::GIL));
			i += sizeof(codefile::GIL);
			Output.Println(STR("global.set #0x{x" GLOBAL_PREFIX ":0}, {s}"), Registers[gil.SrcDest], gil.Index);
		}
			break;
		case codefile::OpCode::GlobalGet:
		{
			codefile::GIL gil = {};
			File.Read(Cast<Byte*>(&gil), sizeof(codefile::GIL));
			i += sizeof(codefile::GIL);
			Output.Println(STR("global.get {s}, #0x{x" GLOBAL_PREFIX ":0}"), Registers[gil.SrcDest], gil.Index);
		}
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
			UInt16 c = 0;
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
			UInt16 c = 0;
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
			UInt16 c = 0;
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
			UInt16 c = 0;
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
			UInt16 c = 0;
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
			UInt16 c = 0;
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
			UInt16 c = 0;
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
			UInt16 c = 0;
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
		case codefile::OpCode::Call8:
		{
			Byte index = 0;
			File.Read(&index, 1);
			i++;
			Output.Println(STR("call.8 #0x{xb:0}"), index);
		}
		break;
		case codefile::OpCode::Call:
		{
			codefile::FunctionType index = 0;
			File.Read(Cast<Byte*>(&index), sizeof(codefile::FunctionType));
			i += sizeof(codefile::FunctionType);
			Output.Println(STR("call #0x{x:0}"), index);
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
			UInt16 c = 0;
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
			Output.Println(STR("global.ret #0x{x" GLOBAL_PREFIX ":0}"), global);
		}
			break;
		case codefile::OpCode::StringGet4:
		{
			Byte val = 0;
			File.Read(&val, 1);
			i++;
			Output.Println(STR("string.get4 {s}, #0x{xb:0}"), Registers[val&0xF], Byte(val>>4));
		}
			break;
		case codefile::OpCode::StringGet:
		{
			Byte reg = 0;
			codefile::StringType str = 0;
			
			File.Read(&reg, 1);
			File.Read(Cast<Byte*>(&str), sizeof(codefile::StringType));

			i += (1 + sizeof(codefile::StringType));
			Output.Println(STR("string.get {s}, #0x{x" STRING_PREFIX ":0}"), Registers[reg&0xF], str);
		}
			break;
		case codefile::OpCode::Push:
		{
			Byte reg = 0;
			File.Read(&reg, 1);
			i++;
			Output.Println(STR("push {s}"), Registers[reg]);
		}
		break;
		case codefile::OpCode::LocalPush:
		{
			codefile::LocalType local = 0;
			File.Read(Cast<Byte*>(&local), sizeof(codefile::LocalType));
			i += sizeof(codefile::LocalType);
			Output.Println(STR("local_push #{" LOCAL_PREFIX "}"), local);
		}
		break;
		case codefile::OpCode::GlobalPush:
		{
			codefile::GlobalType global = 0;
			File.Read(Cast<Byte*>(&global), sizeof(codefile::GlobalType));
			i += sizeof(codefile::LocalType);
			Output.Println(STR("global_push #{" GLOBAL_PREFIX "}"), global);
		}
		break;
		break;
		case codefile::OpCode::Push8:
		{
			Byte c = 0;
			File.Read(&c, 1);
			i++;
			Output.Println(STR("push #0x{xb}"), c);
		}
		break;
		case codefile::OpCode::Push16:
		{
			UInt16 c = 0;
			File.Read(Cast<Byte*>(&c), 1);
			i += 2;
			Output.Println(STR("push #0x{xw}"), c);
		}
		break;
		case codefile::OpCode::Push32:
		{
			UInt32 c = 0;
			File.Read(Cast<Byte*>(&c), 1);
			i += 4;
			Output.Println(STR("push #0x{x}"), c);
		}
		break;
		case codefile::OpCode::Push64:
		{
			UInt64 c = 0;
			File.Read(Cast<Byte*>(&c), 1);
			i += 8;
			Output.Println(STR("push #0x{X}"), c);
		}
		break;
		case codefile::OpCode::PopTop:
			Output.Println(STR("poptop"));
			break;
		case codefile::OpCode::Pop:
		{
			Byte reg = 0;
			File.Read(&reg, 1);
			i++;
			Output.Println(STR("pop {s}"), Registers[reg]);
		}
		break;
		default:
			Output.Println(STR("Invalid Opcode 0x{xb:0}"), IntCast<Byte>(opcode));
			break;
		}
	}
}

bool Dis(StreamOutput& Output, Str FilePath) {
	io::File file = io::File::Open(FilePath);
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
	
	Output.Println(STR(".type 0x{xw}"), header.FileType);
	Output.Println(STR(".version {w}.{w}"), header.MajorVersion, header.MinorVersion);
	Output.Println(STR(".functions {" FUNCTION_PREFIX "}"), header.NumberOfFunctions);
	Output.Println(STR(".objects {d}"), header.NumberOfStructs);
	Output.Println(STR(".globals {" GLOBAL_PREFIX "}"), header.NumberOfGlobals);
	Output.Println(STR(".strings {" STRING_PREFIX "}"), header.NumberOfStrings);
	Output.Println(STR(".start {" FUNCTION_PREFIX "}\n"), header.EntryPoint);

	for (codefile::FunctionType i = 0; i < header.NumberOfFunctions; i++) {
		codefile::FunctionHeader fn = {};
		file.Read(Cast<Byte*>(&fn), sizeof(codefile::FunctionHeader));

		Output.Println(STR(".function {" FUNCTION_PREFIX "}:"), i);
		Output.Println(STR("\t.attribs {w}"), fn.Attributes);
		if (fn.Attributes & codefile::FunctionNative) {
			Output.Println(STR("\t.entry {w}"), fn.EIndex);
			Output.Println(STR("\t.library {w}"), fn.LIndex);
		}
		else {
			Output.Println(STR("\t.args {w}"), fn.NonNative.Arguments);
			Output.Println(STR("\t.locals {w}"), fn.NonNative.LocalCount);
			Output.Println(STR("\t.size {w}"), fn.SizeOfCode);
			Output.Println(STR("\t.code"));
		}

		DisCode(Output, file, fn.SizeOfCode);
		Output.WriteArray({ '\n' });
	}

	for (codefile::GlobalType i = 0; i < header.NumberOfGlobals; i++) {
		codefile::GlobalHeader global = {};
		file.Read(Cast<Byte*>(&global), sizeof(codefile::GlobalHeader));

		Output.Print(STR(".global {" GLOBAL_PREFIX "}: "), i);
		if (global.Primitive == codefile::PrimitiveByte) {
			Byte b = 0;
			file.Read(&b, 1);
			Output.Println(STR("byte #0x{xb:0}"), b);
		}
		else if (global.Primitive == codefile::PrimitiveInt) {
			UInt c = 0;
			file.Read(Cast<Byte*>(&c), 8);
			Output.Println(STR("int #0x{X:0}"), c);
		}
		else if (global.Primitive == codefile::PrimitiveUInt) {
			UInt c = 0;
			file.Read(Cast<Byte*>(&c), 8);
			Output.Println(STR("uint #0x{X:0}"), c);
		}
		else if (global.Primitive == codefile::PrimitiveFloat) {
			UInt c = 0;
			file.Read(Cast<Byte*>(&c), 8);
			Output.Println(STR("float #0x{X:0}"), c);
		}
	}
	
	Output.WriteArray({ '\n' });

	for (codefile::StringType i = 0; i < header.NumberOfStrings; i++) {
		UInt16 len = 0;
		file.Read(Cast<Byte*>(&len), 2);
		Char* str = (Char*)alloca(len);
		file.Read(Cast<Byte*>(str), len);
		str[len - 1] = 0;

		Output.Print(STR(".str {w}: \""), i);
		for (UInt16 ci = 0; ci < len; ci++) {
			switch (str[ci]) {
			case '\0':
				Output.WriteArray({ '\\', '0' }); break;
			case '\n':
				Output.WriteArray({ '\\', 'n' }); break;
			case '\r':
				Output.WriteArray({ '\\', 'r' }); break;
			case '\t':
				Output.WriteArray({ '\\', 't' }); break;
			default:
				Output.WriteArray({ str[ci] });
				break;
			}
		}
		Output.Println(STR("\""));
	}

	file.Close();

	return true;
}

}
