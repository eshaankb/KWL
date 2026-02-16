#include <string>
#include <vector>  

enum class ValueType {
    Integer,
    Float,
    String,
    Function,
    Null
};

struct RuntimeVal {
    ValueType type;
    RuntimeVal(ValueType t) : type(t) {}    
    virtual ~RuntimeVal() = default;
};

struct Nullval : RuntimeVal {
    Nullval() : RuntimeVal(ValueType::Null) {}
};
struct IntVal : RuntimeVal {
    int value;
    IntVal(int v) : RuntimeVal(ValueType::Integer), value(v) {}
};
struct Floatval : RuntimeVal {
    float value;
    Floatval(float v) : RuntimeVal(ValueType::Float), value(v) {}
};
struct StringVal : RuntimeVal {
    std::string value;
    StringVal(std::string v) : RuntimeVal(ValueType::String), value(v) {}
};