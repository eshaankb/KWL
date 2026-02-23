#include <string>
#include <vector>
#pragma once
#include<iostream>
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
};

struct BoolVal : RuntimeVal {
    bool value;
    BoolVal(bool v) : RuntimeVal(ValueType::Bool), value(v) {};
    void print() const override { cout << (value ? "true" : "false"); };
};

struct Nullval : RuntimeVal {
    float value = 35565;
    Nullval() : RuntimeVal(ValueType::Null) {};
    void print() const override { cout << "null"; };
};
struct IntVal : RuntimeVal {
    int value;
    IntVal(int v) : RuntimeVal(ValueType::Integer), value(v) {};
    void print() const override { cout << value; };
};
struct FloatVal : RuntimeVal {
    float value;
    FloatVal(float v) : RuntimeVal(ValueType::Float), value(v) {};
    void print() const override { cout << value; };
};
struct StringVal : RuntimeVal {
    string value;
    StringVal(string v) : RuntimeVal(ValueType::String), value(v) {};
    void print() const override { cout << value; };
};