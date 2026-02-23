#include <string>
#include <vector>
#pragma once
#include<iostream>
#include<memory>
using namespace std;
enum class ValueType {
    Integer,
    Float,
    String,
    Function,
    Bool,
    Null
};

struct RuntimeVal {
    ValueType type; 
    RuntimeVal(ValueType t) : type(t) {}    
    virtual ~RuntimeVal() = default;
    virtual void print() const = 0;
    virtual unique_ptr<RuntimeVal> clone() const = 0;
};

struct BoolVal : RuntimeVal {
    bool value;
    BoolVal(bool v) : RuntimeVal(ValueType::Bool), value(v) {};
    unique_ptr<RuntimeVal> clone() const override {
        return std::make_unique<BoolVal>(value);
    }
    void print() const override { cout << (value ? "true" : "false"); };
};

struct Nullval : RuntimeVal {
    float value = 2147483647; //placeholder value, should never be used. Is equal to Int limit
    Nullval() : RuntimeVal(ValueType::Null) {};
    unique_ptr<RuntimeVal> clone() const override {
        return std::make_unique<Nullval>();
    }
    void print() const override { cout << "null"; };
};
struct IntVal : RuntimeVal {
    int value;
    IntVal(int v) : RuntimeVal(ValueType::Integer), value(v) {};
    unique_ptr<RuntimeVal> clone() const override {
        return std::make_unique<IntVal>(value);
    }
    void print() const override { cout << value; };
};
struct FloatVal : RuntimeVal {
    float value;
    FloatVal(float v) : RuntimeVal(ValueType::Float), value(v) {};
    unique_ptr<RuntimeVal> clone() const override {
        return std::make_unique<FloatVal>(value);
    }
    void print() const override { cout << value; };
};
struct StringVal : RuntimeVal {
    string value;
    StringVal(string v) : RuntimeVal(ValueType::String), value(v) {};
    unique_ptr<RuntimeVal> clone() const override {
        return std::make_unique<StringVal>(value);
    }
    void print() const override { cout << value; };
};