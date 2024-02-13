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

static void DisStack(StreamOutput& Output, StreamInput& File, codefile::OpCodeStack S, Uint32& i) {
	switch (S) {
	case codefile::OpCodeStack::RPush:
	{
		Byte reg = 0;
		File.Read(&reg, 1);
		i++;
		Output.Println(STR("rpush {s}"), Registers[reg]);
	}
		break;
	case codefile::OpCodeStack::LPush:
	{
		codefile::LocalType local = 0;
		File.Read(Cast<Byte*>(&local), 1);
		i += sizeof(codefile::LocalType);
		Output.Println(STR("rpush #{" LOCAL_PREFIX "}"), local);
	}
		break;
	case codefile::OpCodeStack::GPush:
		break;
	case codefile::OpCodeStack::Push8:
	{
		Byte c = 0;
		File.Read(&c, 1);
		i++;
		Output.Println(STR("push #{b}"), c);
	}
		break;
	case codefile::OpCodeStack::Push16:
	{
		Uint16 c = 0;
		File.Read(Cast<Byte*>(&c), 1);
		i += 2;
		Output.Println(STR("push #{w}"), c);
	}
		break;
	case codefile::OpCodeStack::Push32:
	{
		Uint32 c = 0;
		File.Read(Cast<Byte*>(&c), 1);
		i += 4;
		Output.Println(STR("push #{d}"), c);
	}
		break;
	case codefile::OpCodeStack::Push64:
	{
		Uint64 c = 0;
		File.Read(Cast<Byte*>(&c), 1);
		i += 8;
		Output.Println(STR("push #{q}"), c);
	}
		break;
	case codefile::OpCodeStack::PopTop:
		Output.Println(STR("poptop"));
		break;
	case codefile::OpCodeStack::RPop:
	{
		Byte reg = 0;
		File.Read(&reg, 1);
		i++;
		Output.Println(STR("rpop {s}"), Registers[reg]);
	}
		break;
	default:
		Output.Println(STR("Invalid Opcode {b}"), IntCast<Byte>(S));
		break;
	}
}

static void DisCode(StreamOutput& Output, StreamInput& File, Uint32 Size) {
	Uint32 i = 0;

	while (i < Size) {
		codefile::OpCode opcode = codefile::OpCode::Brk;
		File.Read(Cast<Byte*>(&opcode), 1);
		i++;

		Output.WriteArray({ '\t' });

		Output.Print(STR("{d} => "), i - 1);

		switch (opcode) {
		case codefile::OpCode::Brk:
			Output.Println(STR("brk"));
			break;
		case codefile::OpCode::Mov:
		{
			codefile::LI li = {};
			File.Read(Cast<Byte*>(&li), sizeof(codefile::LI));
			i += sizeof(codefile::LI);
			Output.Println(STR("mov {s}, {s}"), Registers[li.Dest], Registers[li.Src]);
		}
			break;
		case codefile::OpCode::Mov8:
		{
			i += 2;
			Byte reg = 0;
			Byte c = 0;
			File.Read(&reg, 1);
			File.Read(&c, sizeof(c));
			Output.Println(STR("mov {s}, #{b}"), Registers[reg], c);
		}
			break;
		case codefile::OpCode::Mov16:
		{
			i += 3;
			Byte reg = 0;
			Uint16 c = 0;
			File.Read(&reg, 1);
			File.Read(Cast<Byte*>(&c), sizeof(c));
			Output.Println(STR("mov {s}, #{w}"), Registers[reg], c);
		}
			break;
		case codefile::OpCode::Mov32:
		{
			i += 5;
			Byte reg = 0;
			Uint32 c = 0;
			File.Read(&reg, 1);
			File.Read(Cast<Byte*>(&c), sizeof(c));
			Output.Println(STR("mov {s}, #{d}"), Registers[reg], c);
		}
			break;
		case codefile::OpCode::Mov64:
		{
			i += 9;
			Byte reg = 0;
			UInt c = 0;
			File.Read(&reg, 1);
			File.Read(Cast<Byte*>(&c), sizeof(c));
			Output.Println(STR("mov {s}, #{q}"), Registers[reg], c);
		}
			break;
		case codefile::OpCode::MovRes:
		{
			i++;
			Byte reg = 0;
			File.Read(&reg, 1);
			Output.Println(STR("movres {s}"), Registers[reg]);
		}
			break;
		case codefile::OpCode::LocalSet:
		{
			codefile::LIL lil = {};
			File.Read(Cast<Byte*>(&lil), sizeof(codefile::LIL));
			i += sizeof(codefile::LIL);
			Output.Println(STR("local_set #{" LOCAL_PREFIX "}, {s}"), lil.Index, Registers[lil.SrcDest]);
		}
			break;
		case codefile::OpCode::LocalGet:
		{
			codefile::LIL lil = {};
			File.Read(Cast<Byte*>(&lil), sizeof(codefile::LIL));
			i += sizeof(codefile::LIL);
			Output.Println(STR("local_get {s}, #{" LOCAL_PREFIX "}"), Registers[lil.SrcDest], lil.Index);
		}
			break;
		case codefile::OpCode::GlobalSet:
			break;
		case codefile::OpCode::GlobalGet:
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
		case codefile::OpCode::Cmp:
		{
			codefile::LI li = {};
			File.Read(Cast<Byte*>(&li), sizeof(codefile::LI));
			i += sizeof(codefile::LI);
			Output.Println(STR("cmp {s}, {s}"), Registers[li.Dest], Registers[li.Src]);
		}
			break;
		case codefile::OpCode::ICmp:
		{
			codefile::LI li = {};
			File.Read(Cast<Byte*>(&li), sizeof(codefile::LI));
			i += sizeof(codefile::LI);
			Output.Println(STR("icmp {s}, {s}"), Registers[li.Dest], Registers[li.Src]);
		}
			break;
		case codefile::OpCode::FCmp:
		{
			codefile::LI li = {};
			File.Read(Cast<Byte*>(&li), sizeof(codefile::LI));
			i += sizeof(codefile::LI);
			Output.Println(STR("fcmp {s}, {s}"), Registers[li.Dest], Registers[li.Src]);
		}
			break;
		case codefile::OpCode::Jmp:
		{
			Uint32 ip = 0;
			File.Read(Cast<Byte*>(&ip), 4);
			i += 4;
			Output.Println(STR("jmp #{d}"), ip);
		}
			break;
		case codefile::OpCode::Jez:
		{
			Uint32 ip = 0;
			File.Read(Cast<Byte*>(&ip), 4);
			i += 4;
			Output.Println(STR("jez #{d}"), ip);
		}
			break;
		case codefile::OpCode::Jnz:
		{
			Uint32 ip = 0;
			File.Read(Cast<Byte*>(&ip), 4);
			i += 4;
			Output.Println(STR("jnz #{d}"), ip);
		}
			break;
		case codefile::OpCode::Je:
		{
			Uint32 ip = 0;
			File.Read(Cast<Byte*>(&ip), 4);
			i += 4;
			Output.Println(STR("je #{d}"), ip);
		}
			break;
		case codefile::OpCode::Jne:
		{
			Uint32 ip = 0;
			File.Read(Cast<Byte*>(&ip), 4);
			i += 4;
			Output.Println(STR("jne #{d}"), ip);
		}
			break;
		case codefile::OpCode::Jl:
		{
			Uint32 ip = 0;
			File.Read(Cast<Byte*>(&ip), 4);
			i += 4;
			Output.Println(STR("je #{d}"), ip);
		}
			break;
		case codefile::OpCode::Jle:
		{
			Uint32 ip = 0;
			File.Read(Cast<Byte*>(&ip), 4);
			i += 4;
			Output.Println(STR("jle #{d}"), ip);
		}
			break;
		case codefile::OpCode::Jg:
		{
			Uint32 ip = 0;
			File.Read(Cast<Byte*>(&ip), 4);
			i += 4;
			Output.Println(STR("jg #{d}"), ip);
		}
			break;
		case codefile::OpCode::Jge:
		{
			Uint32 ip = 0;
			File.Read(Cast<Byte*>(&ip), 4);
			i += 4;
			Output.Println(STR("jge #{d}"), ip);
		}
			break;
		case codefile::OpCode::Call:
		{
			codefile::FunctionType index = 0;
			File.Read(Cast<Byte*>(&index), 4);
			i += sizeof(codefile::FunctionType);
			Output.Println(STR("call #{d}"), index);
		}
			break;
		case codefile::OpCode::Ret:
			Output.Println(STR("ret"));
			break;
		case codefile::OpCode::RRet:
		{
			Byte reg = 0;
			File.Read(&reg, 1);
			i++;
			Output.Println(STR("rret {s}"), Registers[reg]);
		}
			break;
		case codefile::OpCode::LRet:
		{
			codefile::LocalType local = 0;
			File.Read(Cast<Byte*>(&local), sizeof(codefile::LocalType));
			Output.Println(STR("lret #{" LOCAL_PREFIX "}"), local);
		}
			break;
		case codefile::OpCode::GRet:
			break;
		case codefile::OpCode::MemoryPrefix:
			break;
		case codefile::OpCode::StackPrefix:
		{
			codefile::OpCodeStack stack = codefile::OpCodeStack::RPush;
			File.Read(Cast<Byte*>(&stack), 1);
			i++;
			DisStack(Output, File, stack, i);
		}
			break;
		default:
			Output.Println(STR("Invalid Opcode {b}"), IntCast<Byte>(opcode));
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
	
	Output.Println(STR("Attributes: {w}"), header.Attributes);
	Output.Println(STR("Version: {w}.{w}"), header.MajorVersion, header.MinorVersion);
	Output.Println(STR("Functions => {d}"), header.CountOfFunctions);
	Output.Println(STR("Objects => {d}"), header.CountOfObjects);
	Output.Println(STR("Globals => {d}"), header.CountOfGlobals);
	Output.Println(STR("Imports => {d}"), header.CountOfImports);
	Output.Println(STR("Exports => {d}"), header.CountOfExports);
	Output.Println(STR("Strings => {d}"), header.CountOfExports);
	Output.Println(STR("Start => {d}\n"), header.EntryPoint);

	for (Uint32 i = 0; i < header.CountOfFunctions; i++) {
		codefile::FunctionHeader fn = {};
		file.Read(Cast<Byte*>(&fn), sizeof(codefile::FunctionHeader));

		Output.Println(STR("Function: -> @{d} =>"), i);
		Output.Println(STR("\tAttributes: {d}"), fn.Attributes);
		Output.Println(STR("\tArguments: {d}"), fn.Arguments);
		Output.Println(STR("\tLocalCount: {d}"), fn.LocalCount);
		Output.Println(STR("\tSizeOfCode: {d}"), fn.SizeOfCode);
		Output.Println(STR("\t--- Code ---"));

		DisCode(Output, file, fn.SizeOfCode);
		Output.WriteArray({ '\n' });
	}

	file.Close();

	return true;
}

}
