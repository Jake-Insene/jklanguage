#pragma once
#include "jkr/CoreHeader.h"
#include <unordered_map>
#include <string>
#include <vector>

template<typename T>
struct SymbolTable {
    using MapType = std::unordered_map<std::u8string, USize>;
    using VectorType = std::vector<T>;

    SymbolTable() {}
    ~SymbolTable() {}

    [[nodiscard]] MapType::iterator begin() { return Map.begin(); }

    [[nodiscard]] MapType::const_iterator begin() const { return Map.begin(); }

    [[nodiscard]] MapType::iterator end() { return Map.end(); }

    [[nodiscard]] MapType::const_iterator end() const { return Map.end(); }
    
    decltype(auto) Emplace(this SymbolTable& Self, const std::u8string& Name) {
        auto& element = Self.Data.emplace_back();
        Self.Map.emplace(
            Name, Self.Data.size() - 1
        );
        return element;
    }

    MapType::iterator Find(this SymbolTable& Self, const std::u8string& Name) {
        return Self.Map.find(Name);
    }

    [[nodiscard]] constexpr T& Get(this SymbolTable& Self, USize Index) {
        assert(Index < Self.Data.size() && "Index out of range");
        return Self.Data[Index]; 
    }

    [[nodiscard]] constexpr USize Size(this SymbolTable& Self) { return Self.Data.size(); }
    
    void Clear(this SymbolTable& Self) {
        Self.Map.clear();
        Self.Data.clear();
    }

    MapType Map = {};
    VectorType Data = VectorType();
};
