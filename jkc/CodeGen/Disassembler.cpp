#include "jkc/CodeGen/Disassembler.h"
#include <jkr/CodeFile/Header.h>
#include <jkr/CodeFile/Function.h>
#include <jkr/CodeFile/Data.h>
#include <jkr/CodeFile/OpCodes.h>
#include <jkr/CodeFile/Type.h>
#include <stdjk/Mem/Utility.h>
#include <stdjk/String.h>
#include <malloc.h>

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
	STR("r15"),
};

Str Types[] = {
	STR("Byte"),
	STR("Int"),
	STR("UInt"),
	STR("Float"),
	STR("StringRef"),
	STR("Object"),
	STR("Any"),
	STR("Array"),
};

static void DisCode(StreamOutput& Output, StreamInput& File, UInt32 Size) {
	UInt32 i = 0;

	while (i < Size) {
		codefile::OpCode opcode = codefile::OpCode::Brk;
		File.Read(Cast<Byte*>(&opcode), 1);
		i++;

		Output.WriteArray({ '\t' });

		Output.Print(STR("{X} "), i - 1);

		switch (opcode) {
		case codefile::OpCode::Brk:
			Output.Println(STR("brk"));
			break;
		case codefile::OpCode::Mov:
		{
			Byte regs = 0;
			File.Read(&regs, 1);
			i++;
			Output.Println(STR("mov {s}, {s}"), Registers[regs & 0xF], Registers[regs >> 4]);
		}
		break;
		case codefile::OpCode::Mov4:
		{
			Byte b = 0;
			File.Read(&b, 1);
			i += 1;
			Output.Println(STR("mov.c4 {s}, #0x{Xb}"), Registers[b & 0xF], Byte(b >> 4));
		}
		break;
		case codefile::OpCode::Mov8:
		{
			i += 2;
			Byte reg = 0;
			Byte c = 0;
			File.Read(&reg, 1);
			File.Read(&c, 1);
			Output.Println(STR("mov.c8 {s}, #0x{Xb}"), Registers[reg], c);
		}
		break;
		case codefile::OpCode::Mov16:
		{
			i += 3;
			Byte reg = 0;
			UInt16 c = 0;
			File.Read(&reg, 1);
			File.Read(Cast<Byte*>(&c), sizeof(c));
			Output.Println(STR("mov.c16 {s}, #0x{Xw}"), Registers[reg], c);
		}
		break;
		case codefile::OpCode::Mov32:
		{
			i += 5;
			Byte reg = 0;
			UInt32 c = 0;
			File.Read(&reg, 1);
			File.Read(Cast<Byte*>(&c), sizeof(c));
			Output.Println(STR("mov.c32 {s}, #0x{X}"), Registers[reg], c);
		}
		break;
		case codefile::OpCode::Mov64:
		{
			i += 9;
			Byte reg = 0;
			UInt c = 0;
			File.Read(&reg, 1);
			File.Read(Cast<Byte*>(&c), sizeof(c));
			Output.Println(STR("mov.c64 {s}, #0x{Xq}"), Registers[reg], c);
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
			Output.Println(STR("local.set4 [fp + #0x{Xb}], {s}"), Byte(op >> 4), Registers[op & 0xF]);
		}
		break;
		case codefile::OpCode::LocalGet4:
		{
			i++;
			Byte op = 0;
			File.Read(&op, 1);
			Output.Println(STR("local.get4 {s}, [fp + #0x{Xb}]"), Registers[op & 0xF], Byte(op >> 4));
		}
		break;
		case codefile::OpCode::LocalSet:
		{
			UInt16 encoded = 0;
			File.Read(Cast<Byte*>(&encoded), 2);
			i += 2;
			Output.Println(STR("local.set [fp + #0x{Xb}], {s}"), encoded >> 8, Registers[encoded & 0xF]);
		}
		break;
		case codefile::OpCode::LocalGet:
		{
			UInt16 encoded = 0;
			File.Read(Cast<Byte*>(&encoded), 2);
			i += 2;
			Output.Println(STR("local.get {s}, [fp + #0x{Xb}]"), Registers[encoded & 0xF], encoded >> 8);
		}
		break;
		case codefile::OpCode::GlobalSet:
		{
			UInt32 encoded = 0;
			File.Read(Cast<Byte*>(&encoded), 4);
			i += 4;
			Output.Println(STR("global.set [ds + #0x{X}], {s}"), encoded >> 4, Registers[encoded & 0xF]);
		}
		break;
		case codefile::OpCode::GlobalGet:
		{
			UInt32 encoded = 0;
			File.Read(Cast<Byte*>(&encoded), 4);
			i += 4;
			Output.Println(STR("global.get {s}, [ds + #0x{X}]"), Registers[encoded & 0xF], encoded >> 4);
		}
		break;
		case codefile::OpCode::Inc:
		{
			i++;
			Byte reg = 0;
			File.Read(&reg, 1);
			codefile::IncDecType it = (codefile::IncDecType)((reg & 0xF0) >> 4);
			if (it == codefile::IncDecType::Signed) {
				Output.Println(STR("iinc {s}"), Registers[reg]);
			}
			else if (it == codefile::IncDecType::Unsigned) {
				Output.Println(STR("inc {s}"), Registers[reg]);
			}
			else if (it == codefile::IncDecType::Real) {
				Output.Println(STR("finc {s}"), Registers[reg]);
			}
		}
		break;
		case codefile::OpCode::Dec:
		{
			i++;
			Byte reg = 0;
			File.Read(&reg, 1);
			codefile::IncDecType it = (codefile::IncDecType)((reg & 0xF0) >> 4);
			if (it == codefile::IncDecType::Signed) {
				Output.Println(STR("idec {s}"), Registers[reg]);
			}
			else if (it == codefile::IncDecType::Unsigned) {
				Output.Println(STR("dec {s}"), Registers[reg]);
			}
			else if (it == codefile::IncDecType::Real) {
				Output.Println(STR("fdec {s}"), Registers[reg]);
			}
		}
		break;
		case codefile::OpCode::Add:
		{
			UInt16 regs = 0;
			File.Read(Cast<Byte*>(&regs), 2);
			i += 2;
			Output.Println(STR("add {s}, {s}, {s}"),
						   Registers[regs & 0xF], Registers[(regs >> 4) & 0xF], Registers[(regs >> 8) & 0xF]
			);
		}
		break;
		case codefile::OpCode::IAdd:
		{
			UInt16 regs = 0;
			File.Read(Cast<Byte*>(&regs), 2);
			i += 2;
			Output.Println(STR("iadd {s}, {s}, {s}"),
						   Registers[regs & 0xF], Registers[(regs >> 4) & 0xF], Registers[(regs >> 8) & 0xF]
			);
		}
		break;
		case codefile::OpCode::FAdd:
		{
			UInt16 regs = 0;
			File.Read(Cast<Byte*>(&regs), 2);
			i += 2;
			Output.Println(STR("fadd {s}, {s}, {s}"),
						   Registers[regs & 0xF], Registers[(regs >> 4) & 0xF], Registers[(regs >> 8) & 0xF]
			);
		}
		break;
		case codefile::OpCode::Sub:
		{
			UInt16 regs = 0;
			File.Read(Cast<Byte*>(&regs), 2);
			i += 2;
			Output.Println(STR("sub {s}, {s}, {s}"),
						   Registers[regs & 0xF], Registers[(regs >> 4) & 0xF], Registers[(regs >> 8) & 0xF]
			);
		}
		break;
		case codefile::OpCode::ISub:
		{
			UInt16 regs = 0;
			File.Read(Cast<Byte*>(&regs), 2);
			i += 2;
			Output.Println(STR("isub {s}, {s}, {s}"),
						   Registers[regs & 0xF], Registers[(regs >> 4) & 0xF], Registers[(regs >> 8) & 0xF]
			);
		}
		break;
		case codefile::OpCode::FSub:
		{
			UInt16 regs = 0;
			File.Read(Cast<Byte*>(&regs), 2);
			i += 2;
			Output.Println(STR("fsub {s}, {s}, {s}"),
						   Registers[regs & 0xF], Registers[(regs >> 4) & 0xF], Registers[(regs >> 8) & 0xF]
			);
		}
		break;
		case codefile::OpCode::Mul:
		{
			UInt16 regs = 0;
			File.Read(Cast<Byte*>(&regs), 2);
			i += 2;
			Output.Println(STR("mul {s}, {s}, {s}"),
						   Registers[regs & 0xF], Registers[(regs >> 4) & 0xF], Registers[(regs >> 8) & 0xF]
			);
		}
		break;
		case codefile::OpCode::IMul:
		{
			UInt16 regs = 0;
			File.Read(Cast<Byte*>(&regs), 2);
			i += 2;
			Output.Println(STR("imul {s}, {s}, {s}"),
						   Registers[regs & 0xF], Registers[(regs >> 4) & 0xF], Registers[(regs >> 8) & 0xF]
			);
		}
		break;
		case codefile::OpCode::FMul:
		{
			UInt16 regs = 0;
			File.Read(Cast<Byte*>(&regs), 2);
			i += 2;
			Output.Println(STR("fmul {s}, {s}, {s}"),
						   Registers[regs & 0xF], Registers[(regs >> 4) & 0xF], Registers[(regs >> 8) & 0xF]
			);
		}
		break;
		case codefile::OpCode::Div:
		{
			UInt16 regs = 0;
			File.Read(Cast<Byte*>(&regs), 2);
			i += 2;
			Output.Println(STR("div {s}, {s}, {s}"),
						   Registers[regs & 0xF], Registers[(regs >> 4) & 0xF], Registers[(regs >> 8) & 0xF]
			);
		}
		break;
		case codefile::OpCode::IDiv:
		{
			UInt16 regs = 0;
			File.Read(Cast<Byte*>(&regs), 2);
			i += 2;
			Output.Println(STR("idiv {s}, {s}, {s}"),
						   Registers[regs & 0xF], Registers[(regs >> 4) & 0xF], Registers[(regs >> 8) & 0xF]
			);
		}
		break;
		case codefile::OpCode::FDiv:
		{
			UInt16 regs = 0;
			File.Read(Cast<Byte*>(&regs), 2);
			i += 2;
			Output.Println(STR("fdiv {s}, {s}, {s}"),
						   Registers[regs & 0xF], Registers[(regs >> 4) & 0xF], Registers[(regs >> 8) & 0xF]
			);
		}
		break;
		case codefile::OpCode::Add8:
		{
			UInt16 regs = 0;
			File.Read(Cast<Byte*>(&regs), 2);
			i += 2;
			Output.Println(
				STR("add.c8 {s}, {s}, #0x{Xb}"),
				Registers[regs & 0xF], Registers[(regs >> 4) & 0xF],
				regs >> 8
			);
		}
		break;
		case codefile::OpCode::IAdd8:
		{
			UInt16 regs = 0;
			File.Read(Cast<Byte*>(&regs), 2);
			i += 2;
			Output.Println(
				STR("iadd.c8 {s}, {s}, #0x{Xb}"),
				Registers[regs & 0xF], Registers[(regs >> 4) & 0xF],
				regs >> 8
			);
		}
		break;
		case codefile::OpCode::Sub8:
		{
			UInt16 regs = 0;
			File.Read(Cast<Byte*>(&regs), 2);
			i += 2;
			Output.Println(
				STR("sub.c8 {s}, {s}, #0x{Xb}"),
				Registers[regs & 0xF], Registers[(regs >> 4) & 0xF],
				regs >> 8
			);
		}
		break;
		case codefile::OpCode::ISub8:
		{
			UInt16 regs = 0;
			File.Read(Cast<Byte*>(&regs), 2);
			i += 2;
			Output.Println(
				STR("isub.c8 {s}, {s}, #0x{Xb}"),
				Registers[regs & 0xF], Registers[(regs >> 4) & 0xF],
				regs >> 8
			);
		}
		break;
		case codefile::OpCode::Mul8:
		{
			UInt16 regs = 0;
			File.Read(Cast<Byte*>(&regs), 2);
			i += 2;
			Output.Println(
				STR("mul.c8 {s}, {s}, #0x{Xb}"),
				Registers[regs & 0xF], Registers[(regs >> 4) & 0xF],
				regs >> 8
			);
		}
		break;
		case codefile::OpCode::IMul8:
		{
			UInt16 regs = 0;
			File.Read(Cast<Byte*>(&regs), 2);
			i += 2;
			Output.Println(
				STR("imul.c8 {s}, {s}, #0x{Xb}"),
				Registers[regs & 0xF], Registers[(regs >> 4) & 0xF],
				regs >> 8
			);
		}
		break;
		case codefile::OpCode::Div8:
		{
			UInt16 regs = 0;
			File.Read(Cast<Byte*>(&regs), 2);
			i += 2;
			Output.Println(
				STR("div.c8 {s}, {s}, #0x{Xb}"),
				Registers[regs & 0xF], Registers[(regs >> 4) & 0xF],
				regs >> 8
			);
		}
		break;
		case codefile::OpCode::IDiv8:
		{
			UInt16 regs = 0;
			File.Read(Cast<Byte*>(&regs), 2);
			i += 2;
			Output.Println(
				STR("idiv.c8 {s}, {s}, #0x{Xb}"),
				Registers[regs & 0xF], Registers[(regs >> 4) & 0xF],
				regs >> 8
			);
		}
		break;
		case codefile::OpCode::Add16:
		{
			UInt32 regs = 0;
			File.Read(Cast<Byte*>(&regs), 3);
			i += 3;
			Output.Println(
				STR("add.c16 {s}, {s}, #0x{Xw}"),
				Registers[regs & 0xF], Registers[(regs >> 4) & 0xF],
				UInt16(regs >> 8)
			);
		}
		break;
		case codefile::OpCode::IAdd16:
		{
			UInt32 regs = 0;
			File.Read(Cast<Byte*>(&regs), 3);
			i += 3;
			Output.Println(
				STR("iadd.c16 {s}, {s}, #0x{Xw}"),
				Registers[regs & 0xF], Registers[(regs >> 4) & 0xF],
				UInt16(regs >> 8)
			);
		}
		break;
		case codefile::OpCode::Sub16:
		{
			UInt32 regs = 0;
			File.Read(Cast<Byte*>(&regs), 3);
			i += 3;
			Output.Println(
				STR("sub.c16 {s}, {s}, #0x{Xw}"),
				Registers[regs & 0xF], Registers[(regs >> 4) & 0xF],
				UInt16(regs >> 8)
			);
		}
		break;
		case codefile::OpCode::ISub16:
		{
			UInt32 regs = 0;
			File.Read(Cast<Byte*>(&regs), 3);
			i += 3;
			Output.Println(
				STR("isub.c16 {s}, {s}, #0x{Xw}"),
				Registers[regs & 0xF], Registers[(regs >> 4) & 0xF],
				UInt16(regs >> 8)
			);
		}
		break;
		case codefile::OpCode::Mul16:
		{
			UInt32 regs = 0;
			File.Read(Cast<Byte*>(&regs), 3);
			i += 3;
			Output.Println(
				STR("mul.c16 {s}, {s}, #0x{Xw}"),
				Registers[regs & 0xF], Registers[(regs >> 4) & 0xF],
				UInt16(regs >> 8)
			);
		}
		break;
		case codefile::OpCode::IMul16:
		{
			UInt32 regs = 0;
			File.Read(Cast<Byte*>(&regs), 3);
			i += 3;
			Output.Println(
				STR("imul.c16 {s}, {s}, #0x{Xw}"),
				Registers[regs & 0xF], Registers[(regs >> 4) & 0xF],
				UInt16(regs >> 8)
			);
		}
		break;
		case codefile::OpCode::Div16:
		{
			UInt32 regs = 0;
			File.Read(Cast<Byte*>(&regs), 3);
			i += 3;
			Output.Println(
				STR("div.c16 {s}, {s}, #0x{Xw}"),
				Registers[regs & 0xF], Registers[(regs >> 4) & 0xF],
				UInt16(regs >> 8)
			);
		}
		break;
		case codefile::OpCode::IDiv16:
		{
			UInt32 regs = 0;
			File.Read(Cast<Byte*>(&regs), 3);
			i += 3;
			Output.Println(
				STR("idiv.c16 {s}, {s}, #0x{Xw}"),
				Registers[regs & 0xF], Registers[(regs >> 4) & 0xF],
				UInt16(regs >> 8)
			);
		}
		break;
		case codefile::OpCode::Cmp:
		{
			Byte regs = 0;
			File.Read(&regs, 1);
			i++;
			Output.Println(STR("cmp {s}, {s}"), Registers[regs & 0xF], Registers[regs >> 4]);
		}
		break;
		case codefile::OpCode::ICmp:
		{
			Byte regs = 0;
			File.Read(&regs, 1);
			i++;
			Output.Println(STR("icmp {s}, {s}"), Registers[regs & 0xF], Registers[regs >> 4]);
		}
		break;
		case codefile::OpCode::FCmp:
		{
			Byte regs = 0;
			File.Read(&regs, 1);
			i++;
			Output.Println(STR("fcmp {s}, {s}"), Registers[regs & 0xF], Registers[regs >> 4]);
		}
		break;
		case codefile::OpCode::TestZ:
		{
			i++;
			Byte reg = 0;
			File.Read(&reg, 1);
			Output.Println(STR("test.z {s}"), Registers[reg & 0xF]);
		}
		break;
		case codefile::OpCode::Jmp:
		{
			UInt16 ip = 0;
			File.Read(Cast<Byte*>(&ip), 2);
			i += 2;
			Output.Println(STR("jmp #0x{X}"), ip);
		}
		break;
		case codefile::OpCode::Jez:
		{
			UInt16 ip = 0;
			File.Read(Cast<Byte*>(&ip), 2);
			i += 2;
			Output.Println(STR("jez #0x{X}"), ip);
		}
		break;
		case codefile::OpCode::Jnz:
		{
			UInt16 ip = 0;
			File.Read(Cast<Byte*>(&ip), 2);
			i += 2;
			Output.Println(STR("jnz #0x{X}"), ip);
		}
		break;
		case codefile::OpCode::Je:
		{
			UInt16 ip = 0;
			File.Read(Cast<Byte*>(&ip), 2);
			i += 2;
			Output.Println(STR("je #0x{X}"), ip);
		}
		break;
		case codefile::OpCode::Jne:
		{
			UInt16 ip = 0;
			File.Read(Cast<Byte*>(&ip), 2);
			i += 2;
			Output.Println(STR("jne #0x{X}"), ip);
		}
		break;
		case codefile::OpCode::Jl:
		{
			UInt16 ip = 0;
			File.Read(Cast<Byte*>(&ip), 2);
			i += 2;
			Output.Println(STR("je #0x{X}"), ip);
		}
		break;
		case codefile::OpCode::Jle:
		{
			UInt16 ip = 0;
			File.Read(Cast<Byte*>(&ip), 2);
			i += 2;
			Output.Println(STR("jle #0x{X}"), ip);
		}
		break;
		case codefile::OpCode::Jg:
		{
			UInt16 ip = 0;
			File.Read(Cast<Byte*>(&ip), 2);
			i += 2;
			Output.Println(STR("jg #0x{X}"), ip);
		}
		break;
		case codefile::OpCode::Jge:
		{
			UInt16 ip = 0;
			File.Read(Cast<Byte*>(&ip), 2);
			i += 2;
			Output.Println(STR("jge #0x{X}"), ip);
		}
		break;
		case codefile::OpCode::Jmp8:
		{
			Byte ip = 0;
			File.Read(&ip, 1);
			i++;
			Output.Println(STR("jmp.c8 #0x{Xb}"), ip);
		}
		break;
		case codefile::OpCode::Jez8:
		{
			Byte ip = 0;
			File.Read(&ip, 1);
			i++;
			Output.Println(STR("jez.c8 #0x{Xb}"), ip);
		}
		break;
		case codefile::OpCode::Jnz8:
		{
			Byte ip = 0;
			File.Read(&ip, 1);
			i++;
			Output.Println(STR("jnz.c8 #0x{Xb}"), ip);
		}
		break;
		case codefile::OpCode::Je8:
		{
			Byte ip = 0;
			File.Read(&ip, 1);
			i++;
			Output.Println(STR("je.c8 #0x{Xb}"), ip);
		}
		break;
		case codefile::OpCode::Jne8:
		{
			Byte ip = 0;
			File.Read(&ip, 1);
			i++;
			Output.Println(STR("jne.c8 #0x{Xb}"), ip);
		}
		break;
		case codefile::OpCode::Jl8:
		{
			Byte ip = 0;
			File.Read(&ip, 1);
			i++;
			Output.Println(STR("jl.c8 #0x{Xb}"), ip);
		}
		break;
		case codefile::OpCode::Jle8:
		{
			Byte ip = 0;
			File.Read(&ip, 1);
			i++;
			Output.Println(STR("jle.c8 #0x{Xb}"), ip);
		}
		break;
		case codefile::OpCode::Jg8:
		{
			Byte ip = 0;
			File.Read(&ip, 1);
			i++;
			Output.Println(STR("jg.c8 #0x{Xb}"), ip);
		}
		break;
		case codefile::OpCode::Jge8:
		{
			Byte ip = 0;
			File.Read(&ip, 1);
			i++;
			Output.Println(STR("jge.c8 #0x{Xb}"), ip);
		}
		break;
		case codefile::OpCode::Call8:
		{
			Byte index = 0;
			File.Read(&index, 1);
			i++;
			Output.Println(STR("call.8 #0x{Xb}"), index);
		}
		break;
		case codefile::OpCode::Call:
		{
			UInt32 index = 0;
			File.Read(Cast<Byte*>(&index), 4);
			i += 4;
			Output.Println(STR("call #0x{X}"), index);
		}
		break;
		case codefile::OpCode::Ret:
			Output.Println(STR("ret"));
			break;
		case codefile::OpCode::ArrayNew:
		{
			UInt16 encoded = 0;
			File.Read(Cast<Byte*>(&encoded), 2);
			i += 2;
			Output.Println(STR("array.new {s}, size={s}, type {s}"),
						   Registers[encoded & 0xF], Registers[(encoded >> 4) & 0xF],
						   Types[(encoded >> 8)]
			);
		}
		break;
		case codefile::OpCode::ArrayLen:
		{
			Byte regs = 0;
			File.Read(&regs, 1);
			i++;
			Output.Println(STR("array.len [{s}], {s}"), Registers[regs & 0xF], Registers[regs >> 4]);
		}
		break;
		case codefile::OpCode::ArraySet:
		{
			UInt16 encoded = 0;
			File.Read(Cast<Byte*>(&encoded), 2);
			i += 2;
			Output.Println(STR("array.set [{s} + {s}], {s}"),
						   Registers[encoded & 0xF], Registers[(encoded >> 4) & 0xF],
						   Registers[(encoded >> 8) & 0xF]
			);
		}
		break;
		case codefile::OpCode::ArrayGet:
		{
			UInt16 encoded = 0;
			File.Read(Cast<Byte*>(&encoded), 2);
			i += 2;
			Output.Println(STR("array.get {s}, [{s} + {s}]"),
						   Registers[(encoded >> 8) & 0xF],
						   Registers[encoded & 0xF], Registers[(encoded >> 4) & 0xF]
			);
		}
		break;
		case codefile::OpCode::ArrayDestroy:
		{
			Byte reg = 0;
			File.Read(&reg, 1);
			i++;
			Output.Println(STR("array.destroy [{s}]"), Registers[reg & 0xF]);
		}
		break;
		case codefile::OpCode::StringGet4:
		{
			Byte val = 0;
			File.Read(&val, 1);
			i++;
			Output.Println(STR("string.get4 {s}, [st + #0x{Xb}]"), Registers[val & 0xF], Byte(val >> 4));
		}
		break;
		case codefile::OpCode::StringGet:
		{
			UInt32 encoded = 0;
			File.Read(Cast<Byte*>(&encoded), 4);
			i += 4;
			Output.Println(STR("string.get {s}, [st + #0x{X}]"), Registers[encoded & 0xF], encoded >> 4);
		}
		break;
		case codefile::OpCode::Push8:
		{
			Byte c = 0;
			File.Read(&c, 1);
			i++;
			Output.Println(STR("push #0x{Xb}"), c);
		}
		break;
		case codefile::OpCode::Push16:
		{
			UInt16 c = 0;
			File.Read(Cast<Byte*>(&c), 1);
			i += 2;
			Output.Println(STR("push #0x{Xw}"), c);
		}
		break;
		case codefile::OpCode::Push32:
		{
			UInt32 c = 0;
			File.Read(Cast<Byte*>(&c), 1);
			i += 4;
			Output.Println(STR("push #0x{X}"), c);
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
		case codefile::OpCode::PushR0:
		case codefile::OpCode::PushR1:
		case codefile::OpCode::PushR2:
		case codefile::OpCode::PushR3:
		case codefile::OpCode::PushR4:
		case codefile::OpCode::PushR5:
		case codefile::OpCode::PushR6:
		case codefile::OpCode::PushR7:
		case codefile::OpCode::PushR8:
		case codefile::OpCode::PushR9:
		case codefile::OpCode::PushR10:
		case codefile::OpCode::PushR11:
		case codefile::OpCode::PushR12:
		case codefile::OpCode::PushR13:
		case codefile::OpCode::PushR14:
		case codefile::OpCode::PushR15:
		{
			Output.WriteArray({ 'p', 'u', 's', 'h', ' ' });
			Output.Write((Byte*)Registers[Byte(opcode) & 0xF], Strlen(Registers[Byte(opcode) & 0xF]));
			Output.WriteArray({ '\n' });
		}
		break;
		case codefile::OpCode::PopR0:
		case codefile::OpCode::PopR1:
		case codefile::OpCode::PopR2:
		case codefile::OpCode::PopR3:
		case codefile::OpCode::PopR4:
		case codefile::OpCode::PopR5:
		case codefile::OpCode::PopR6:
		case codefile::OpCode::PopR7:
		case codefile::OpCode::PopR8:
		case codefile::OpCode::PopR9:
		case codefile::OpCode::PopR10:
		case codefile::OpCode::PopR11:
		case codefile::OpCode::PopR12:
		case codefile::OpCode::PopR13:
		case codefile::OpCode::PopR14:
		case codefile::OpCode::PopR15:
		{
			Output.WriteArray({ 'p', 'o', 'p', ' ' });
			Output.Write((Byte*)Registers[Byte(opcode) & 0xF], Strlen(Registers[Byte(opcode) & 0xF]));
			Output.WriteArray({ '\n' });
		}
		break;
		case codefile::OpCode::PushLocal:
		{
			Byte index = 0;
			File.Read(&index, 1);
			i++;
			Output.Println(STR("push [fp + #0x{Xb}]"), index);
		}
		break;
		case codefile::OpCode::PopLocal:
		{
			Byte index = 0;
			File.Read(&index, 1);
			i++;
			Output.Println(STR("pop [fp + #0x{Xb}]"), index);
		}
			break;
		default:
			Output.Println(STR("Invalid Opcode 0x{xb}"), IntCast<Byte>(opcode));
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
	
	if (!mem::Cmp(&header.Signature, (void*)&codefile::Signature[0], sizeof(codefile::Signature)))
		return false;
	
	Output.Println(STR(".type 0x{xw}"), header.FileType);
	Output.Println(STR(".version {w}.{w}"), header.MajorVersion, header.MinorVersion);
	Output.Println(STR(".number_of_sections {xb}"), header.NumberOfSections);
	Output.Println(STR(".entry_point {x}\n"), header.EntryPoint);

	for (Byte i = 0; i < header.NumberOfSections; i++) {
		codefile::SectionHeader section = {};
		file.Read(Cast<Byte*>(&section), sizeof(codefile::SectionHeader));

		if (section.Type == codefile::SectionCode) {
			Output.Println(STR("section .code"));
			for (UInt32 f = 0; f < section.CountOfElements; f++) {
				codefile::FunctionHeader fn = {};
				file.Read(Cast<Byte*>(&fn), sizeof(codefile::FunctionHeader));

				Output.Println(STR(".function {x}:"), f);
				Output.Println(STR("\t.flags {w}"), fn.Flags);
				if (fn.Flags & codefile::FunctionNative) {
					Output.Println(STR("\t.entry {w}"), fn.LocalReserve);
					Output.Println(STR("\t.library {w}"), fn.SizeOfCode);
				}
				else {
					Output.Println(STR("\t.locals {w}"), fn.LocalReserve);
					Output.Println(STR("\t.size {w}"), fn.SizeOfCode);
					Output.Println(STR("\t.code"));
					DisCode(Output, file, fn.SizeOfCode);
				}

				Output.WriteArray({ '\n' });
			}
		}
		else if (section.Type == codefile::SectionData) {
			Output.Println(STR("section .data"));
			for (UInt32 g = 0; g < section.CountOfElements; g++) {
				codefile::DataHeader global = {};
				file.Read(Cast<Byte*>(&global), sizeof(codefile::DataHeader));

				Output.Print(STR(".global {d}: "), g);
				if (global.Primitive == codefile::PrimitiveByte) {
					Byte b = 0;
					file.Read(&b, 1);
					Output.Println(STR("byte #0x{Xb}"), b);
				}
				else if (global.Primitive == codefile::PrimitiveInt) {
					UInt c = 0;
					file.Read(Cast<Byte*>(&c), 8);
					Output.Println(STR("int #0x{Xq}"), c);
				}
				else if (global.Primitive == codefile::PrimitiveUInt) {
					UInt c = 0;
					file.Read(Cast<Byte*>(&c), 8);
					Output.Println(STR("uint #0x{Xq}"), c);
				}
				else if (global.Primitive == codefile::PrimitiveFloat) {
					union {
						UInt c = 0;
						Float real;
					};
					file.Read(Cast<Byte*>(&c), 8);
					Output.Println(STR("float Raw -> #0x{Xq}, Real -> {f}"), c, real);
				}
			}
			Output.WriteArray({ '\n' });
		}
		else if (section.Type == codefile::SectionST) {
			Output.Println(STR("section .st"));
			for (UInt32 s = 0; s < section.CountOfElements; s++) {
				UInt16 len = 0;
				file.Read(Cast<Byte*>(&len), 2);
				Char* str = (Char*)mem::Allocate(len);
				file.Read(Cast<Byte*>(str), len);
				str[len - 1] = 0;

				Output.Print(STR(".str {w}: \""), s);
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

				mem::Deallocate(str);
				Output.Println(STR("\""));
			}
		}
	}

	file.Close();

	return true;
}

}
