#include<string>
#include<vector>
#include<memory>
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


int main() {
    cout << "KWL TERMINAL INTERPRETER V -1.0.0" << endl;
    cout << "Init:" << endl;
    cout << "░█ ▄▀ ░█  ░█ ░█   \n░█▀▄  ░█░█░█ ░█   \n░█ ░█ ░█▄▀▄█ ░█▄▄█"<< endl;
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
}}