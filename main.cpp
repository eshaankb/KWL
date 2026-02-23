#include<string>
#include<vector>
#include<memory>
#include<fstream>
#include"parser.hpp"
#include<iostream>
#include"interpreter.hpp"
#include"ast.hpp"
#include"values.hpp"
#include"environment.hpp"
using namespace std;
void printAST(Stmt* node, int indent = 0) {
    string pad(indent, ' ');
    cout << pad << nodeTypeName(node->kind) << endl;

    if (node->kind == NodeType::Literal) {
        auto* lit = static_cast<Literal*>(node);
        cout << pad << "  value: " << lit->value << endl;
    }

    // add cases for BinaryExpr, BlockStmt, etc.
}

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
        unique_ptr<RuntimeVal> result = Eval(&program, env);
        result->print();
        cout << endl;
}
}

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
    Program program = parser.produceAST(sourcecode);
    unique_ptr<RuntimeVal> result = Eval(&program, env);
    result->print();
    cout << endl;
    return 0;
}

    int main() {
    cout << "KWL TERMINAL INTERPRETER V -1.0.0" << endl;
    cout << "Init:" << endl;
    cout << "░█ ▄▀ ░█  ░█ ░█   \n░█▀▄  ░█░█░█ ░█   \n░█ ░█ ░█▄▀▄█ ░█▄▄█"<< endl;
    cout << "Open repl or run a file with 'kwl <filename>? 'repl/file'" << endl;
    string mode;
    getline(cin, mode);
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