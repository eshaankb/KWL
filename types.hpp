#pragma once

enum class ValueType {
    Integer,
    Float,
    String,
    Function,
    Bool,
    Structure,
    Null,
    Array,
    Range,
    TrueClass,
    Module,
    Integer64,
    Float64
};

struct RuntimeVal {
    ValueType type; 
    RuntimeVal(ValueType t) : type(t) {}    
    virtual ~RuntimeVal() = default;
    virtual void print() const = 0;
    virtual RuntimeVal* clone() const = 0;
};