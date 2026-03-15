
/**
 * @file values.hpp
 * @brief Runtime value type definitions for the KWL interpreter.
 *
 * Defines concrete runtime value types that represent all data in KWL
 * at runtime. Each struct corresponds to a ValueType and implements
 * the required RuntimeVal interface (print, clone).
 *
 * @author KWL Interpreter
 * @date 2026
 */

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

/**
 * @struct BoolVal
 * @brief Represents a boolean value (true/false) in KWL.
 */
struct BoolVal : RuntimeVal {
    bool value;             ///< The boolean value
    BoolVal(bool v) : RuntimeVal(ValueType::Bool), value(v) {};
    RuntimeVal* clone() const override {
        return new BoolVal(value);
    }
    void print() const override { cout << (value ? "true" : "false"); };
};

/**
 * @struct Nullval
 * @brief Represents the null/nil value in KWL.
 */
struct Nullval : RuntimeVal {
    float value = 2147483647; //placeholder value, should never be used. Is equal to Int limit
    Nullval() : RuntimeVal(ValueType::Null) {};
    RuntimeVal* clone() const override {
        return new Nullval();
    }
    void print() const override { cout << "null"; };
};

/**
 * @struct IntVal
 * @brief Represents a 32-bit signed integer in KWL.
 */
struct IntVal : RuntimeVal {
    int value;              ///< The integer value
    IntVal(int v) : RuntimeVal(ValueType::Integer), value(v) {};
    RuntimeVal* clone() const override {
        return new IntVal(value);
    }
    void print() const override { cout << value; };
};

/**
 * @struct Int64Val
 * @brief Represents a 64-bit signed integer in KWL.
 */
struct Int64Val : RuntimeVal {
    long long value;        ///< The 64-bit integer value
    Int64Val(long long v) : RuntimeVal(ValueType::Integer64), value(v) {};
    RuntimeVal* clone() const override {
        return new Int64Val(value);
    }
    void print() const override { cout << value; };
};

/**
 * @struct FloatVal
 * @brief Represents a single-precision floating point number in KWL.
 */
struct FloatVal : RuntimeVal {
    float value;            ///< The float value
    FloatVal(float v) : RuntimeVal(ValueType::Float), value(v) {};
    RuntimeVal* clone() const override {
        return new FloatVal(value);
    }
    void print() const override { cout << value; };
};

/**
 * @struct Float64Val
 * @brief Represents a double-precision floating point number in KWL.
 */
struct Float64Val : RuntimeVal {
    double value;           ///< The double-precision float value
    Float64Val(double v) : RuntimeVal(ValueType::Float64), value(v) {};
    RuntimeVal* clone() const override {
        return new Float64Val(value);
    }
    void print() const override { cout << value; };
};

/**
 * @struct StringVal
 * @brief Represents a string value in KWL.
 */
struct StringVal : RuntimeVal {
    string value;           ///< The string content
    StringVal(string v) : RuntimeVal(ValueType::String), value(v) {};
    RuntimeVal* clone() const override {
        return new StringVal(value);
    }
    void print() const override { cout << value; };
};

/**
 * @struct StructureVal
 * @brief Represents an instance of a structure/class in KWL.
 *
 * Contains a map of field names to their runtime values, plus
 * the class name for type identification.
 */
struct StructureVal : RuntimeVal {
    unordered_map<string, RuntimeVal*> fields;  ///< Field name to value mapping
    std::string className;                      ///< Name of the class this is an instance of
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

/**
 * @struct TrueClassVal
 * @brief Represents a class definition with methods in KWL.
 *
 * This is used for TrueClass - full class definitions that include
 * both fields and methods.
 */
struct TrueClassVal : RuntimeVal {
    TrueClassVal() : RuntimeVal(ValueType::TrueClass) {};
    RuntimeVal* clone() const override {
        return new TrueClassVal();
    }
    void print() const override { cout << "<trueclass>"; };
};

/**
 * @struct FunctionVal
 * @brief Represents a function value in KWL.
 *
 * Contains parameter information and a pointer to the function body.
 */
struct FunctionVal : RuntimeVal {
    vector<pair<string, ValueType>> paramNames;  ///< Parameter names and types
    BlockStmt* body;                            ///< Pointer to the function body AST

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

/**
 * @struct RetVal
 * @brief Exception type used to implement early function returns.
 *
 * This is both a RuntimeVal and a std::exception, allowing it to
 * propagate return values through the call stack.
 */
struct RetVal : RuntimeVal, std::exception {
    RuntimeVal* value;      ///< The value being returned
    RetVal(RuntimeVal* v) : RuntimeVal(ValueType::Null), value(v) {};
    RuntimeVal* clone() const override {
        return new RetVal(value->clone());
    }
    void print() const override {
        cout << "<return> ";
        value->print();
    }
};

/**
 * @struct ArrayVal
 * @brief Represents an array/list value in KWL.
 *
 * Contains a vector of RuntimeVal pointers representing the elements.
 */
struct ArrayVal : RuntimeVal {
    vector<RuntimeVal*> elements;  ///< Array elements
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

/**
 * @struct RangeVal
 * @brief Represents a range value for iteration in KWL.
 *
 * Used in for loops and slicing operations. The range is half-open [start, end).
 */
struct RangeVal : RuntimeVal {
    int start;              ///< Starting value (inclusive)
    int end;                ///< Ending value (exclusive)
    RangeVal(int s, int e) : RuntimeVal(ValueType::Range), start(s), end(e) {};
    RuntimeVal* clone() const override {
        return new RangeVal(start, end);
    }
    void print() const override {
        cout << "[" << start << ":" << end << "]";
    }
};

/**
 * @struct ModuleVal
 * @brief Represents an imported module in KWL.
 *
 * Contains a pointer to the module's environment, which holds
 * all exported symbols from the module.
 */
struct ModuleVal : RuntimeVal {
    Environment* moduleEnv;  ///< Pointer to the module's environment
    ModuleVal(Environment* mod)
        : RuntimeVal(ValueType::Module), moduleEnv(mod)  {}
    
    RuntimeVal* clone() const override {
        return new ModuleVal(moduleEnv);
    }
    
    void print() const override {
        cout << "<module ";
    }
};
