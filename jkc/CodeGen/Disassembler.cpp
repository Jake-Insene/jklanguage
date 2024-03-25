#include "jkc/CodeGen/Disassembler.h"
#include <jkr/CodeFile/Header.h>
#include <jkr/CodeFile/Function.h>
#include <jkr/CodeFile/Data.h>
#include <jkr/CodeFile/OpCodes.h>
#include <jkr/CodeFile/Type.h>
#include <jkr/String.h>
#include <fstream>

namespace CodeGen {

constexpr const char* Registers[] = {
	"r0",
	"r1",
	"r2",
	"r3",
	"r4",
	"r5",
	"r6",
	"r7",
	"r8",
	"r9",
	"r10",
	"r11",
	"r12",
	"r13",
	"r14",
	"r15",
};

constexpr const char* VRegisters[] = {
	"v0",
	"v1",
	"v2",
	"v3",
	"v4",
	"v5",
	"v6",
	"v7",
	"v8",
	"v9",
	"v10",
	"v11",
	"v12",
	"v13",
	"v14",
	"v15",
};

constexpr const char* ArrayElement[] = {
	"1B",
	"8B",
	"32B",
};

constexpr const char* FileType[] = {
	"Executable",
	"Library",
};

constexpr const char* JmpType[] = {
	"jmp",
	"je",
	"jne",
	"jl",
	"jle",
	"jg",
	"jge",
};

constexpr const char* MathTypes[] = {
	"add",
	"sub",
	"mul",
	"div",
	"iadd",
	"isub",
	"imul",
	"idiv",
	"fadd",
	"fsub",
	"fmul",
	"fdiv",
	"add8",
	"sub8",
	"mul8",
	"div8",
	"iadd8",
	"isub8",
	"imul8",
	"idiv8",
	"add16",
	"sub16",
	"mul16",
	"div16",
	"iadd16",
	"isub16",
	"imul16",
	"idiv16",
};

constexpr const char* MathBin[] = {
	"or",
	"and",
	"xor",
	"shl",
	"shr",
	"or8",
	"and8",
	"xor8",
	"shl8",
	"shr8",
	"or16",
	"and16",
	"xor16",
};

#define READ_AND_ADVANCE(dest, count) File.read((char*)&dest, count); i+=count;

static void DisCode(FILE* Output, std::istream& File, UInt32 Size) {
	UInt32 i = 0;
	Byte util = 0;
	Byte util2 = 0;
	union {
		UInt16 word = 0;
		UInt32 dword;
		UInt64 qword;
	};

	while (i < Size) {
		codefile::OpCode opcode = (codefile::OpCode)0;
		File.read((char*)&opcode, 1);
		i++;

		fprintf(Output, "\n\t");
		fprintf(Output, "%08X: ", i - 1);

		switch (opcode) {
		case codefile::OpCode::Brk:
			fprintf(Output, "brk");
			break;
		case codefile::OpCode::Mov:
			READ_AND_ADVANCE(util, 1);
			fprintf(Output, "mov %s, %s", Registers[INST_ARG1(util)], Registers[INST_ARG2(util)]);
			break;
		case codefile::OpCode::Mov4:
			READ_AND_ADVANCE(util, 1);
			fprintf(Output, "mov %s, #0x%02X", Registers[INST_ARG1(util)], INST_ARG2(util));
			break;
		case codefile::OpCode::Mov8:
			READ_AND_ADVANCE(util, 1);
			READ_AND_ADVANCE(util2, 1);
			fprintf(Output, "mov %s, #0x%02X", Registers[INST_ARG2(util)], util2);
			break;
		case codefile::OpCode::Mov16:
			READ_AND_ADVANCE(util, 1);
			READ_AND_ADVANCE(word, 2);
			fprintf(Output, "mov %s, #0x%04X", Registers[INST_ARG2(util)], word);
			break;
		case codefile::OpCode::Mov32:
			READ_AND_ADVANCE(util, 1);
			READ_AND_ADVANCE(dword, 4);
			fprintf(Output, "mov %s, #0x%08X", Registers[INST_ARG2(util)], dword);
			break;
		case codefile::OpCode::Mov64:
			READ_AND_ADVANCE(util, 1);
			READ_AND_ADVANCE(qword, 8);
			fprintf(Output, "mov %s, #0x%016llX", Registers[INST_ARG2(util)], qword);
			break;
		case codefile::OpCode::Ldstr:
			READ_AND_ADVANCE(util, 1);
			READ_AND_ADVANCE(dword, 4);
			fprintf(Output, "load.string %s [st:#0x%08X]", Registers[INST_ARG1(util)], dword);
			break;
		case codefile::OpCode::Ldr:
			READ_AND_ADVANCE(util, 1);
			READ_AND_ADVANCE(word, 2);
			if (INST_ARG2(util) == codefile::BaseSP) {
				fprintf(Output, "load %s, [sp + %04Xh]", Registers[INST_ARG1(util)], word);
			}
			else if (INST_ARG2(util) == codefile::BaseFP) {
				fprintf(Output, "load %s, [fp + %04Xh]", Registers[INST_ARG1(util)], word);
			}
			else if (INST_ARG2(util) == codefile::BaseCS) {
				fprintf(Output, "load %s, [ds:%04Xh]", Registers[INST_ARG1(util)], word);
			}
			break;
		case codefile::OpCode::Str:
			READ_AND_ADVANCE(util, 1);
			READ_AND_ADVANCE(word, 2);
			if (INST_ARG2(util) == codefile::BaseSP) {
				fprintf(Output, "str %s, [sp + #0x%04X]", Registers[INST_ARG1(util)], word);
			}
			else if (INST_ARG2(util) == codefile::BaseFP) {
				fprintf(Output, "str %s, [fp + #0x%04X]", Registers[INST_ARG1(util)], word);
			}
			else if (INST_ARG2(util) == codefile::BaseCS) {
				fprintf(Output, "str %s, [ds:%04X]", Registers[INST_ARG1(util)], word);
			}
			break;
		case codefile::OpCode::Cmp:
			READ_AND_ADVANCE(util, 1);
			fprintf(Output, "cmp %s, %s", Registers[INST_ARG1(util)], Registers[INST_ARG2(util)]);
			break;
		case codefile::OpCode::FCmp:
			READ_AND_ADVANCE(util, 1);
			fprintf(Output, "fcmp %s, %s", Registers[INST_ARG1(util)], Registers[INST_ARG2(util)]);
			break;
		case codefile::OpCode::TestZ:
			READ_AND_ADVANCE(util, 1);
			fprintf(Output, "testz %s", Registers[INST_ARG1(util)]);
			break;
		case codefile::OpCode::Jmp:
		case codefile::OpCode::Je:
		case codefile::OpCode::Jne:
		case codefile::OpCode::Jl:
		case codefile::OpCode::Jle:
		case codefile::OpCode::Jg:
		case codefile::OpCode::Jge:
			READ_AND_ADVANCE(word, 2);
			fprintf(Output, "%s #0x%04X",
					JmpType[Byte(opcode) - Byte(codefile::OpCode::Jmp)],
					word
			);
			break;
		case codefile::OpCode::Call:
			READ_AND_ADVANCE(dword, 4);
			fprintf(Output, "call [cs:%08X]", dword);
			break;
		case codefile::OpCode::Ret:
			fprintf(Output, "ret");
			break;
		case codefile::OpCode::RetC:
			READ_AND_ADVANCE(dword, 4);
			fprintf(Output, "ret #0x%08X", dword);
			break;
		case codefile::OpCode::Inc:
			READ_AND_ADVANCE(util, 1);
			fprintf(Output, "inc %s", Registers[INST_ARG1(util)]);
			break;
		case codefile::OpCode::IInc:
			READ_AND_ADVANCE(util, 1);
			fprintf(Output, "iinc %s", Registers[INST_ARG1(util)]);
			break;
		case codefile::OpCode::FInc:
			READ_AND_ADVANCE(util, 1);
			fprintf(Output, "finc %s", Registers[INST_ARG1(util)]);
			break;
		case codefile::OpCode::Dec:
			READ_AND_ADVANCE(util, 1);
			fprintf(Output, "dec %s", Registers[INST_ARG1(util)]);
			break;
		case codefile::OpCode::IDec:
			READ_AND_ADVANCE(util, 1);
			fprintf(Output, "idec %s", Registers[INST_ARG1(util)]);
			break;
		case codefile::OpCode::FDec:
			READ_AND_ADVANCE(util, 1);
			fprintf(Output, "fdec %s", Registers[INST_ARG1(util)]);
			break;
		case codefile::OpCode::Add:
		case codefile::OpCode::Sub:
		case codefile::OpCode::Mul:
		case codefile::OpCode::Div:
		case codefile::OpCode::IAdd:
		case codefile::OpCode::ISub:
		case codefile::OpCode::IMul:
		case codefile::OpCode::IDiv:
		case codefile::OpCode::FAdd:
		case codefile::OpCode::FSub:
		case codefile::OpCode::FMul:
		case codefile::OpCode::FDiv:
			READ_AND_ADVANCE(word, 2);
			fprintf(Output, "%s %s, %s, %s",
					MathTypes[Byte(opcode) - Byte(codefile::OpCode::Add)],
					Registers[MATH_DEST(word)],
					Registers[MATH_SRC1(word)],
					Registers[MATH_SRC2(word)]
			);
			break;
		case codefile::OpCode::Add8:
		case codefile::OpCode::Sub8:
		case codefile::OpCode::Mul8:
		case codefile::OpCode::Div8:
		case codefile::OpCode::IAdd8:
		case codefile::OpCode::ISub8:
		case codefile::OpCode::IMul8:
		case codefile::OpCode::IDiv8:
			READ_AND_ADVANCE(util, 1);
			READ_AND_ADVANCE(util2, 1);
			fprintf(Output, "%s %s, %s, #0x%02X",
					MathTypes[Byte(opcode) - Byte(codefile::OpCode::Add)],
					Registers[MATH_DEST(util)],
					Registers[MATH_SRC1(util)],
					util2
			);
			break;
		case codefile::OpCode::Add16:
		case codefile::OpCode::Sub16:
		case codefile::OpCode::Mul16:
		case codefile::OpCode::Div16:
		case codefile::OpCode::IAdd16:
		case codefile::OpCode::ISub16:
		case codefile::OpCode::IMul16:
		case codefile::OpCode::IDiv16:
			READ_AND_ADVANCE(util, 1);
			READ_AND_ADVANCE(word, 2);
			fprintf(Output, "%s %s, %s, #0x%04X",
					MathTypes[Byte(opcode) - Byte(codefile::OpCode::Add)],
					Registers[MATH_DEST(util)],
					Registers[MATH_SRC1(util)],
					word
			);
			break;
		case codefile::OpCode::Or:
		case codefile::OpCode::And:
		case codefile::OpCode::XOr:
		case codefile::OpCode::Shl:
		case codefile::OpCode::Shr:
			READ_AND_ADVANCE(word, 2);
			fprintf(Output, "%s %s, %s, %s",
					MathBin[Byte(opcode) - Byte(codefile::OpCode::XOr)],
					Registers[MATH_DEST(word)],
					Registers[MATH_SRC1(word)],
					Registers[MATH_SRC2(word)]
			);
			break;
		case codefile::OpCode::Or8:
		case codefile::OpCode::And8:
		case codefile::OpCode::XOr8:
		case codefile::OpCode::Shl8:
		case codefile::OpCode::Shr8:
			READ_AND_ADVANCE(util, 1);
			READ_AND_ADVANCE(util2, 1);
			fprintf(Output, "%s %s, %s, #0x%02X",
					MathBin[Byte(opcode) - Byte(codefile::OpCode::XOr)],
					Registers[MATH_DEST(util)],
					Registers[MATH_SRC1(util)],
					util
			);
			break;
		case codefile::OpCode::Or16:
		case codefile::OpCode::And16:
		case codefile::OpCode::XOr16:
			READ_AND_ADVANCE(util, 1);
			READ_AND_ADVANCE(word, 2);
			fprintf(Output, "%s %s, %s, #0x%04X",
					MathBin[Byte(opcode) - Byte(codefile::OpCode::XOr)],
					Registers[MATH_DEST(util)],
					Registers[MATH_SRC1(util)],
					word
			);
			break;
		case codefile::OpCode::Not:
			READ_AND_ADVANCE(util, 1);
			fprintf(Output, "not %s", Registers[INST_ARG1(util)]);
			break;
		case codefile::OpCode::Neg:
			READ_AND_ADVANCE(util, 1);
			fprintf(Output, "neg %s", Registers[INST_ARG1(util)]);
			break;
		case codefile::OpCode::Push8:
			READ_AND_ADVANCE(util, 1);
			fprintf(Output, "push #0x%02X", util);
			break;
		case codefile::OpCode::Push16:
			READ_AND_ADVANCE(word, 2);
			fprintf(Output, "push #0x%04X", word);
			break;
		case codefile::OpCode::Push32:
			READ_AND_ADVANCE(dword, 4);
			fprintf(Output, "push #0x%08X", dword);
			break;
		case codefile::OpCode::Push64:
			READ_AND_ADVANCE(qword, 8);
			fprintf(Output, "push #0x%016llX", qword);
			break;
		case codefile::OpCode::Popd:
			fprintf(Output, "popd");
			break;
		case codefile::OpCode::Push:
			READ_AND_ADVANCE(util, 1);
			fprintf(Output, "push %s", Registers[INST_ARG1(util)]);
			break;
		case codefile::OpCode::Pop:
			READ_AND_ADVANCE(util, 1);
			fprintf(Output, "pop %s", Registers[INST_ARG1(util)]);
			break;
		case codefile::OpCode::ArrayNew:
			READ_AND_ADVANCE(util, 1);
			fprintf(Output, "array.new %s, size=%s, %s",
					Registers[INST_ARG1(util)],
					Registers[INST_ARG1(util)],
					ArrayElement[INST_ARG2(util)]
			);
			break;
		case codefile::OpCode::ArrayL:
			READ_AND_ADVANCE(util, 1);
			fprintf(Output, "array.length %s, [%s]",
					Registers[INST_ARG1(util)],
					Registers[INST_ARG2(util)]
			);
			break;
		case codefile::OpCode::ArrayLoad:
			READ_AND_ADVANCE(util, 1);
			READ_AND_ADVANCE(util2, 1);
			fprintf(Output, "array.load %s, [%s + %s]",
					Registers[INST_ARG1(util2)],
					Registers[INST_ARG1(util)],
					Registers[INST_ARG2(util)]
			);
			break;
		case codefile::OpCode::ArrayStore:
			READ_AND_ADVANCE(util, 1);
			READ_AND_ADVANCE(util2, 1);
			fprintf(Output, "array.store %s, [%s + %s]",
					Registers[INST_ARG1(util2)],
					Registers[INST_ARG1(util)],
					Registers[INST_ARG2(util)]
			);
			break;
		case codefile::OpCode::ArrayDestroy:
			READ_AND_ADVANCE(util, 1);
			fprintf(Output, "array.destroy [%s]",
					Registers[INST_ARG1(util)]
			);
			break;
		default:
			fprintf(Output, "Invalid Opcode 0x%02Xh", Byte(opcode));
			break;
		}
	}

	fputc('\n', Output);
}

bool Dis(FILE* Output, const char* FilePath) {
	std::ifstream file{ FilePath, std::ios::ate | std::ios::binary };
	if (!file.is_open()) {
		return false;
	}

	USize size = file.tellg();
	file.seekg(0);
	if (size < sizeof(codefile::FileHeader)) {
		return false;
	}

	codefile::FileHeader header{};
	file.read(reinterpret_cast<char*>(&header), sizeof(codefile::FileHeader));
	if (size != header.CheckSize)
		return false;

	if (memcmp(&header.Signature, (void*)codefile::Signature, sizeof(codefile::Signature)) != 0)
		return false;

	fprintf(Output, ".type %s\n", FileType[header.FileType]);
	fprintf(Output, ".version %d.%d\n", header.MajorVersion, header.MinorVersion);
	fprintf(Output, ".data_size %d\n", header.DataSize);
	fprintf(Output, ".function_size %d\n", header.FunctionSize);
	fprintf(Output, ".strings_size %d\n", header.StringsSize);
	fprintf(Output, ".entry_point %d\n\n", header.EntryPoint);

	if (header.DataSize) {
		fprintf(Output, "section .data\n");
		for (UInt32 g = 0; g < header.DataSize; g++) {
			codefile::DataHeader global = {};
			file.read((char*)&global, sizeof(codefile::DataHeader));

			fprintf(Output, ".global %d: ", g);
			if (global.Primitive == codefile::PrimitiveByte) {
				Byte b = 0;
				file.read((char*)&b, 1);
				fprintf(Output, "byte %02X = %d", b, b);
			}
			else if (global.Primitive == codefile::PrimitiveInt) {
				UInt64 c = 0;
				file.read(reinterpret_cast<char*>(&c), 8);
				fprintf(Output, "int %llX = %lli", c, c);
			}
			else if (global.Primitive == codefile::PrimitiveUInt) {
				UInt64 c = 0;
				file.read(reinterpret_cast<char*>(&c), 8);
				fprintf(Output, "uint %llX = %llu", c, c);
			}
			else if (global.Primitive == codefile::PrimitiveFloat) {
				union {
					UInt c = 0;
					Float real;
				};
				file.read(reinterpret_cast<char*>(&c), 8);
				fprintf(Output, "float Raw -> %016llX, Real -> %f", c, real);
			}
		}
		fputc('\n', Output);
	}

	if (header.FunctionSize) {
		fprintf(Output, "section .code\n");
		for (UInt32 f = 0; f < header.FunctionSize; f++) {
			codefile::FunctionHeader fn = {};
			file.read(reinterpret_cast<char*>(&fn), sizeof(codefile::FunctionHeader));

			fprintf(Output, ".function %d:\n", f);
			fprintf(Output, "\t.flags %d\n", fn.Flags);
			if (fn.Flags & codefile::FunctionNative) {
				fprintf(Output, "\t.entry st:%d\n", UInt32(fn.StackArguments | fn.LocalReserve << 16));
				fprintf(Output, "\t.library st:%d\n", fn.SizeOfCode);
			}
			else {
				fprintf(Output, "\t.locals %d\n", fn.LocalReserve);
				fprintf(Output, "\t.size %d\n", fn.SizeOfCode);
				fprintf(Output, "\t.code");
				DisCode(Output, file, fn.SizeOfCode);
			}

			fputc('\n', Output);
		}
	}

	if (header.StringsSize)
		fprintf(Output, "section .st\n");
	{
		for (UInt32 s = 0; s < header.StringsSize; s++) {
			UInt16 len = 0;
			file.read(reinterpret_cast<char*>(&len), 2);
			Char* str = new Char[len];
			file.read(reinterpret_cast<char*>(str), len);
			str[len - 1] = 0;

			fprintf(Output, ".str %d: \"", s);
			for (UInt16 ci = 0; ci < len; ci++) {
				switch (str[ci]) {
				case '\0':
					fputc('\\', Output);
					fputc('0', Output);
					break;
				case '\n':
					fputc('\\', Output);
					fputc('n', Output);
					break;
				case '\r':
					fputc('\\', Output);
					fputc('r', Output);
					break;
				case '\t':
					fputc('\\', Output);
					fputc('t', Output);
					break;
				default:
					fputc(str[ci], Output);
					break;
				}
			}

			delete[] str;
			fprintf(Output, "\"");
			fputc('\n', Output);
		}
		fputc('\n', Output);
	}

	file.close();

	return true;
}

}
