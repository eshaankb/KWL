
#pragma once
#include <string>
#include <vector>
#include<iostream>
#include<unordered_map>
using namespace std;
enum class ValueType {
    Integer,
    Float,
    String,
    Function,
    Bool,
    Structure,
    Null
};

struct RuntimeVal {
    ValueType type; 
    RuntimeVal(ValueType t) : type(t) {}    
    virtual ~RuntimeVal() = default;
    virtual void print() const = 0;
    virtual RuntimeVal* clone() const = 0;
};

struct BoolVal : RuntimeVal {
    bool value;
    BoolVal(bool v) : RuntimeVal(ValueType::Bool), value(v) {};
    RuntimeVal* clone() const override {
        return new BoolVal(value);
    }
    void print() const override { cout << (value ? "true" : "false"); };
};

struct Nullval : RuntimeVal {
    float value = 2147483647; //placeholder value, should never be used. Is equal to Int limit
    Nullval() : RuntimeVal(ValueType::Null) {};
    RuntimeVal* clone() const override {
        return new Nullval();
    }
    void print() const override { cout << "null"; };
};
struct IntVal : RuntimeVal {
    int value;
    IntVal(int v) : RuntimeVal(ValueType::Integer), value(v) {};
    RuntimeVal* clone() const override {
        return new IntVal(value);
    }
    void print() const override { cout << value; };
};
struct FloatVal : RuntimeVal {
    float value;
    FloatVal(float v) : RuntimeVal(ValueType::Float), value(v) {};
    RuntimeVal* clone() const override {
        return new FloatVal(value);
    }
    void print() const override { cout << value; };
};
struct StringVal : RuntimeVal {
    string value;
    StringVal(string v) : RuntimeVal(ValueType::String), value(v) {};
    RuntimeVal* clone() const override {
        return new StringVal(value);
    }
    void print() const override { cout << value; };
};

struct StructureVal : RuntimeVal {
    unordered_map<string, RuntimeVal*> fields;
    StructureVal(unordered_map<string, RuntimeVal*> f = {}) : RuntimeVal(ValueType::Structure), fields(f) {};
    RuntimeVal* clone() const override {
        unordered_map<string, RuntimeVal*> clonedFields;
        clonedFields = fields;
        return new StructureVal(clonedFields);
    }
    void print() const override {
        for (const auto& field : fields) {
            cout << field.first << ": ";
            field.second->print();
            cout << "; ";
        }
        cout << "}";
    };
};