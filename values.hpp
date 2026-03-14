
#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>
#include "types.hpp"
#include "environment.hpp"

// Forward declaration
struct BlockStmt;
#include "ast.hpp"

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
struct Int64Val : RuntimeVal {
    long long value;
    Int64Val(long long v) : RuntimeVal(ValueType::Integer64), value(v) {};
    RuntimeVal* clone() const override {
        return new Int64Val(value);
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
struct Float64Val : RuntimeVal {
    double value;
    Float64Val(double v) : RuntimeVal(ValueType::Float64), value(v) {};
    RuntimeVal* clone() const override {
        return new Float64Val(value);
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
    std::string className;
    StructureVal(unordered_map<string, RuntimeVal*> f = {}, const std::string& cls = "")
        : RuntimeVal(ValueType::Structure), fields(f), className(cls) {};
    RuntimeVal* clone() const override {
        unordered_map<string, RuntimeVal*> clonedFields = fields;
        return new StructureVal(clonedFields, className);
    }
    void print() const override {
        cout << "<" << className << "> {";
        for (const auto& field : fields) {
            cout << field.first << ": ";
            field.second->print();
            cout << "; ";
        }
        cout << "}";
    };
};

struct TrueClassVal : RuntimeVal {
    TrueClassVal() : RuntimeVal(ValueType::TrueClass) {};
    RuntimeVal* clone() const override {
        return new TrueClassVal();
    }
    void print() const override { cout << "<trueclass>"; };
};

struct FunctionVal : RuntimeVal {
    vector<pair<string, ValueType>> paramNames;
    BlockStmt* body;

    FunctionVal(const vector<pair<string,ValueType>>& params = {}, BlockStmt* b=nullptr)
        : RuntimeVal(ValueType::Function), paramNames(params), body(b) {}
    
    RuntimeVal* clone() const override {
        return new FunctionVal(paramNames, body);
    }
    
    void print() const override {
        cout << "<function(";
        for (size_t i = 0; i < paramNames.size(); ++i) {
            cout << paramNames[i].first;
            if (i < paramNames.size() - 1) cout << ", ";
        }
        cout << ")>";
    }
};

struct RetVal : RuntimeVal {
    RuntimeVal* value;
    RetVal(RuntimeVal* v) : RuntimeVal(ValueType::Null), value(v) {};
    RuntimeVal* clone() const override {
        return new RetVal(value->clone());
    }
    void print() const override {
        cout << "<return> ";
        value->print();
    }
};

struct ArrayVal : RuntimeVal {
    vector<RuntimeVal*> elements;
    ArrayVal(vector<RuntimeVal*> elems) : RuntimeVal(ValueType::Array), elements(elems) {};
    RuntimeVal* clone() const override {
        vector<RuntimeVal*> clonedElements;
        for (auto e : elements) {
            clonedElements.push_back(e->clone());
        }
        return new ArrayVal(clonedElements);
    }
    void print() const override {
        cout << "[";
        for (size_t i = 0; i < elements.size(); i++) {
            elements[i]->print();
            if (i < elements.size() - 1) cout << ", ";
        }
        cout << "]";
    }
};

struct RangeVal : RuntimeVal {
    int start;
    int end;
    RangeVal(int s, int e) : RuntimeVal(ValueType::Range), start(s), end(e) {};
    RuntimeVal* clone() const override {
        return new RangeVal(start, end);
    }
    void print() const override {
        cout << "[" << start << ":" << end << "]";
    }
};

struct ModuleVal : RuntimeVal {
    Environment* moduleEnv;
    ModuleVal(Environment* mod)
        : RuntimeVal(ValueType::Module), moduleEnv(mod)  {}
    
    RuntimeVal* clone() const override {
        return new ModuleVal(moduleEnv);
    }
    
    void print() const override {
        cout << "<module ";
    }
};