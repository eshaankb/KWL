/**
 * @file main.cpp
 * @brief Main entry point for the KWL interpreter.
 *
 * Provides both a REPL (Read-Eval-Print Loop) for interactive use
 * and file processing mode for running .kwl script files.
 *
 * @author KWL Interpreter
 * @date 2026
 */

#include<string>
#include<vector>
#include<fstream>
#include"parser.hpp"
#include<iostream>
#include"interpreter.hpp"
#include"ast.hpp"
#include"values.hpp"
#include"environment.hpp"
using namespace std;

/**
 * @brief Converts a NodeType enum to its string representation.
 * @param t The NodeType to convert
 * @return Human-readable name of the node type
 */
string nodeTypeName(NodeType t) {
    switch(t){
        case NodeType::Program: return "Program";
        case NodeType::BlockStatement: return "BlockStatement";
        case NodeType::VariableDeclaration: return "VariableDeclaration";
        case NodeType::Assignment: return "Assignment";
        case NodeType::BinaryExpression: return "BinaryExpression";
        case NodeType::UnaryExpression: return "UnaryExpression";
        case NodeType::Literal: return "Literal";
        case NodeType::Identifier: return "Identifier";
        case NodeType::CallExpression: return "CallExpression";
        case NodeType::IndexExpression: return "IndexExpression";
        case NodeType::IfStatement: return "IfStatement";
        case NodeType::ElseStatement: return "ElseStatement";
        case NodeType::WhileStatement: return "WhileStatement";
        case NodeType::ReturnStatement: return "ReturnStatement";
        case NodeType::BuiltinCall: return "BuiltinCall";
        default: return "UnknownNodeType";
    }
};

/**
 * @brief Prints the AST structure for debugging purposes.
 * @param node The AST node to print
 * @param indent Indentation level for pretty printing
 */
void printAST(Stmt* node, int indent = 0) {
    string pad(indent, ' ');
    cout << pad << nodeTypeName(node->kind) << endl;

    if (node->kind == NodeType::Literal) {
        auto* lit = static_cast<Literal*>(node);
        cout << pad << "  value: " << lit->value << endl;
    }

    // add cases for BinaryExpr, BlockStmt, etc.
}

/**
 * @brief Runs the REPL (Read-Eval-Print Loop) for interactive KWL code entry.
 * @return 0 on successful exit
 *
 * The REPL reads KWL code from stdin, parses it, evaluates it,
 * and prints the result. Supports 'exit' or 'quit' to terminate.
 */
int repl() {
    Parser parser;
    Environment env; 
    while(true) {
        string input;
        getline(cin,input);
        if (input.empty() || input.find("exit") != string::npos||input.find("quit") != string::npos) {
            return 0;
        }

        Program program = parser.produceAST(input);
        //print the AST for debugging
        for (auto stmt : program.body) {
            cout << "Node Type: " << nodeTypeName(stmt->kind) << endl
                << "Node Address: " << stmt << endl;
            //cout << "Interpreted Value: " << Eval(program.body[0]).type << endl;
            printAST(stmt);
        }
        RuntimeVal* result = Eval(&program, env);
        result->print();
        cout << endl;
}
}

/**
 * @brief Processes a KWL source file and executes it.
 * @param filename Path to the .kwl file to execute
 * @return 0 on success, 1 on error
 */
int processFile(const string& filename) {
    Parser parser;
    Environment env; 
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return 1;
    }
    string sourcecode((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();
#ifdef DEBUG_TOKENS
    auto toks = tokenize(sourcecode);
    cout << "Tokens from file:\n";
    for (auto &t : toks) {
        cout << t.value << " (" << (int)t.type << ")\n";
    }
#endif
    Program program = parser.produceAST(sourcecode);
    RuntimeVal* result = Eval(&program, env);
    result->print();
    cout << endl;
    return 0;
}

/**
 * @brief Main entry point for the KWL interpreter.
 * @return 0 on successful execution, 1 on error
 *
 * Displays a welcome banner and prompts the user to choose between
 * REPL mode or file execution mode.
 */
    int main() {
    cout << "KWL TERMINAL INTERPRETER V -1.0.0" << endl;
    cout << "Init:" << endl;
    cout << "░█ ▄▀ ░█  ░█ ░█   \n░█▀▄  ░█░█░█ ░█   \n░█ ░█ ░█▄▀▄█ ░█▄▄█"<< endl;
    cout << "Open repl or run a file with 'kwl <filename>? 'repl/file'" << endl;
    string mode;
    getline(cin, mode);
    cerr << "[DEBUG] mode = '" << mode << "'" << endl;
    if (mode == "repl") {
        return repl();
    } else if (mode == "file") {
        cout << "Enter filename: ";
        string filename;
        getline(cin, filename);
        if(filename.empty()){
            cerr << "Error: No filename provided." << endl;
            return 1;
        }else if(filename.find(".kwl") == string::npos){
            cerr << "Error: Invalid file type. Please provide a .kwl file." << endl;
            return 1;
        }
        return processFile(filename);
    } else {
        cerr << "Invalid mode. Please enter 'repl' or 'file'." << endl;
        return 1;}
    }
