#pragma once

#include "jkc/AST/Expresions.h"
#include "jkc/AST/Statement.h"
#include "jkc/AST/Program.h"

#include <algorithm>

namespace AST {

template<typename It, typename T>
constexpr USize FindIndex(It begin, It end, T& Val) {
    for (; begin != end; begin++) {
        if (begin == &Val)
            return end - begin;
    }
}

constexpr void InsertBlockOnBody(List<mem::Ptr<AST::Statement>>& Body, mem::Ptr<Statement>& Start,
                                 Block* InsertBlock) {
    auto index = FindIndex(Body.begin(), Body.end(), Start);
    for (auto& stat : InsertBlock->Statements) {
        Body.Insert(index, std::move(stat));
    }
}

constexpr void InsertPrograms(AST::Program& TargetProgram, List<AST::Program>& Programs) {
    for (auto& pro : Programs) {
        auto start = pro.Statements.begin();
        auto end = pro.Statements.end();
        while (--end != start - 1) {
            TargetProgram.Statements.Insert(0, *end);
            *end = mem::Ptr<Statement>::New(nullptr);
        }
    }
}

}
