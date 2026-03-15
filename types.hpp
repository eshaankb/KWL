/**
 * @file types.hpp
 * @brief Type definitions for the KWL interpreter.
 *
 * Defines the ValueType enumeration representing all runtime data types
 * in KWL, and the base RuntimeVal class for the value object system.
 *
 * @author KWL Interpreter
 * @date 2026
 */

#pragma once

/**
 * @enum ValueType
 * @brief Enumeration of all runtime value types in KWL.
 *
 * Represents the type of a value at runtime. This is used for type checking,
 * type inference, and runtime type identification.
 */
enum class ValueType {
    Integer,      ///< 32-bit signed integer
    Float,        ///< Single-precision floating point
    String,       ///< Unicode string
    Function,     ///< Function value
    Bool,         ///< Boolean (true/false)
    Structure,    ///< User-defined structure/class instance
    Null,         ///< Null/nil value
    Array,        ///< Dynamic array
    Range,         ///< Range value (for iteration)
    TrueClass,    ///< Full class definition with methods
    Module,       ///< Imported module namespace
    Integer64,    ///< 64-bit signed integer
    Float64       ///< Double-precision floating point
};

/**
 * @struct RuntimeVal
 * @brief Base class for all runtime values in KWL.
 *
 * All values in KWL (integers, strings, arrays, functions, etc.) are represented
 * by subclasses of RuntimeVal. This provides a common interface for type
 * identification, printing, and cloning.
 */
struct RuntimeVal {
    ValueType type;       ///< The type of this runtime value

    /**
     * @brief Constructs a RuntimeVal with the specified type.
     * @param t The ValueType for this value
     */
    RuntimeVal(ValueType t) : type(t) {}

    /**
     * @brief Virtual destructor for proper cleanup of derived types.
     */
    virtual ~RuntimeVal() = default;

    /**
     * @brief Prints the value to stdout.
     *
     * Each derived class implements this to provide appropriate
     * string representation of its value.
     */
    virtual void print() const = 0;

    /**
     * @brief Creates a deep copy of this value.
     * @return A new RuntimeVal that is a copy of this value.
     */
    virtual RuntimeVal* clone() const = 0;
};
