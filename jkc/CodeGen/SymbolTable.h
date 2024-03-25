#pragma once
#include <jkr/String.h>
#include <jkr/Vector.h>
#include <unordered_map>
#include <cassert>

template<typename T>
struct SymbolTable {
    using MapType = std::unordered_map<StringView, USize>;
    using VectorType = Vector<T>;

    SymbolTable() {}
    ~SymbolTable() {}

    [[nodiscard]] MapType::iterator begin() { return Map.begin(); }

    [[nodiscard]] MapType::const_iterator begin() const { return Map.begin(); }

    [[nodiscard]] MapType::iterator end() { return Map.end(); }

    [[nodiscard]] MapType::const_iterator end() const { return Map.end(); }
    
    decltype(auto) Add(const StringView& Key) {
        auto& element = Items.emplace_back();
        Map.emplace(
            Key, Items.size() - 1
        );
        return element;
    }

    MapType::iterator Find(const StringView& Key) {
        return Map.find(Key);
    }

    [[nodiscard]] constexpr T& Get(USize Index) {
        assert(Index < Items.size() && "Index out of range");
        return Items[Index]; 
    }

    [[nodiscard]] constexpr USize Size() { return Items.size(); }
    
    void Clear() {
        Map.clear();
        Items.clear();
    }

    MapType Map;
    VectorType Items;
};
