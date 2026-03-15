/**
 * @file environment.hpp
 * @brief Environment (scope) management for the KWL interpreter.
 *
 * Defines the Environment class which manages variable scoping, including
 * variable declarations, assignments, lookups, and class/structure definitions.
 *
 * @author KWL Interpreter
 * @date 2026
 */

#pragma once
#include<map>
#include"types.hpp"
#include<string>
#include<algorithm>
#include<unordered_map>

// Forward declaration
class StructDecl;
#include "ast.hpp"

/**
 * @class Environment
 * @brief Manages variable scopes and symbol table for the KWL interpreter.
 *
 * Each Environment represents a lexical scope containing variable bindings.
 * Environments form a chain through parent pointers, allowing for proper
 * variable lookup in nested scopes (closures, functions, blocks).
 */
class Environment{
    public:
    /**
     * @brief Constructs a new root environment with no parent.
     */
    Environment();

    /**
     * @brief Constructs a new environment with a parent scope.
     * @param parentEnv The parent environment for variable resolution
     */
    Environment(Environment* parentEnv);

    /**
     * @brief Move constructor (default)
     */
    Environment(Environment&&) = default;

    /**
     * @brief Maps variable names to their runtime values in this scope.
     */
    std::unordered_map<std::string,RuntimeVal* > variables;

    /**
     * @brief Move assignment operator (default)
     */
    Environment& operator=(Environment&&) = default;

    /**
     * @brief Declares a new variable in this environment.
     * @param varname Name of the variable
     * @param value Initial value (pointer ownership transferred)
     * @param immut Whether the variable is immutable (const)
     */
    void declareVal (std::string varname, RuntimeVal* value, bool immut=false);

    /**
     * @brief Assigns a value to an existing variable.
     * @param varname Name of the variable
     * @param value New value (pointer ownership transferred)
     */
    void assignVal(std::string varname, RuntimeVal* value);

    /**
     * @brief Looks up a variable's value in this or parent scopes.
     * @param varname Name of the variable to find
     * @return Pointer to the RuntimeVal, or nullptr if not found
     */
    RuntimeVal* getVal(std::string varname);

    /**
     * @brief Resolves which environment contains a variable.
     * @param varname Name of the variable to resolve
     * @return Pointer to the Environment containing the variable, or nullptr
     */
    Environment* resolve(std::string varname);

    /**
     * @brief Declares a new class/structure type.
     * @param className Name of the class
     * @param classDecl Pointer to the StructDecl AST node
     */
    void declareClass(std::string className, StructDecl* classDecl);

    /**
     * @brief Looks up a class declaration by name.
     * @param className Name of the class
     * @return Pointer to the StructDecl, or nullptr if not found
     */
    StructDecl* getClass(std::string className);

    protected:
    Environment* parent;    ///< Parent environment for scope chain
    std::unordered_map<std::string,bool> IsConst; ///< Which variables are const
    std::unordered_map<std::string, StructDecl*> classes; ///< Class definitions
public:
    bool loginoutImported = false; ///< Flag for stdlib loginout module

    /**
     * @brief Checks if the loginout module has been imported.
     * @return true if loginout is available in this or parent scope
     */
    bool isLoginoutImported() const {
        return loginoutImported || (parent && parent->isLoginoutImported());
    }
};
