#include<fstream>
#include<string>
#include<vector>
#include<iostream>
#include<algorithm>
#include<cctype>
#include<map>
#include <sstream>
#include"lexer.hpp"
//static_assert(__cplusplus >= 202002L, "C++20 not enabled");
using namespace std;

bool isBlockKeyword(const string& s) {
    static const vector<string> blockKeywords = {
        "if", "lp", "el"
    };
    return find(blockKeywords.begin(), blockKeywords.end(), s) != blockKeywords.end();
}
bool isKeyword(const string& s) {
    static const vector<string> keywords = {
        "and", "or", "nt", "mod", "gbl", "mkimmutable", "crclass"
    };
    return find(keywords.begin(), keywords.end(), s) != keywords.end();
}

bool isType(const string& s) {
    static const vector<string> types = {
        "int", "int64", "fl", "fl64", "str", "bool"
    };
    return find(types.begin(), types.end(), s) != types.end();
}

bool isBoolLiteral(const string& s) {
    return s == "true" || s == "false";
}

bool isNullLiteral(const string& s) {
    return s == "null";
}
vector<Token> tokenize(string sourcecode) {
    bool uclosedbslsh = false;
    vector<Token> tokens;
    vector<char> src(sourcecode.begin(), sourcecode.end());
    while (src.size()) {
        if (isspace(src[0])) {
            src.erase(src.begin());
            continue;
        }
        if (src[0] == '#') {
            // Block comment ### ###
            if (src.size() >= 3 && src[1] == '#' && src[2] == '#') {
                src.erase(src.begin(), src.begin() + 3);
                while (src.size() >= 3 &&
                       !(src[0] == '#' && src[1] == '#' && src[2] == '#')) {
                    src.erase(src.begin());
                }
                if (src.size() >= 3)
                    src.erase(src.begin(), src.begin() + 3);
                tokens.push_back(Token("###", TokenType::CommentBlock));
            }
            // Line comment #
             {
                while (src.size() && src[0] != '\n')
                    src.erase(src.begin());
                tokens.push_back(Token("#", TokenType::CommentLine));
            }
            continue;
        }
        
        if (isdigit(src[0])) {
            string num;
            bool isFloat = false;

            while (src.size() && (isdigit(src[0]) || src[0] == '.')) {
                if (src[0] == '.') isFloat = true;
                num += src[0];
                src.erase(src.begin());
            }
            if (isFloat){
                tokens.push_back(Token(num, TokenType::FloatLiteral));
            }else {
                tokens.push_back(Token(num, TokenType::IntLiteral));
            }
            //tokens.push_back(num, isFloat ? FloatLiteral : IntLiteral);
            continue;
        } if (isalpha(src[0])) {
            string keyw;
            while(src.size()>0 && isalpha(src[0])){
                keyw += src[0];
                src.erase(src.begin());
            }
            if(isBlockKeyword(keyw)){
                tokens.push_back(Token(keyw, TokenType::BlockKeyword));}
            else if (isBoolLiteral(keyw))
                {tokens.push_back(Token(keyw, TokenType::BoolLiteral));}
            else if (isNullLiteral(keyw))
                {tokens.push_back(Token(keyw, TokenType::Null));}
            else if (isType(keyw))
                {tokens.push_back(Token(keyw, TokenType::TypeIdent));}
            else if (isKeyword(keyw))
                if(keyw=="and"||keyw=="or"||keyw=="nt")
                    {tokens.push_back(Token(keyw, TokenType::LogicalOp));}
                else
                {tokens.push_back(Token(keyw, TokenType::Keyword));}
            else
                {tokens.push_back(Token(keyw, TokenType::Identifier));}
            continue;

        }
        if (src[0] == '"') {
            src.erase(src.begin());
            string str = "\"";
            
            while (src.size() > 0 && src[0] != '"') {\
                if (src[0] == '\\' && src.size() > 1 && src[1] == '"') {
                    str += '"';
                    src.erase(src.begin(), src.begin() + 2);
                } else {
                    str += src[0];
                    src.erase(src.begin());
                }
            }

            if (src.size() > 0 && src[0] == '"') {
                src.erase(src.begin()); 
            } else {
                throw std::runtime_error("Lexer Error: Unclosed string literal.");
            }
            str.append("\"");
            tokens.push_back(Token(str, TokenType::StringLiteral));
            continue;
        }
        if (src[0] == '\'') {
            src.erase(src.begin());
            string str = "";
            
            while (src.size() > 0 && src[0] != '\'') {\
                if (src[0] == '\\' && src.size() > 1 && src[1] == '\'') {
                    str += '\'';
                    src.erase(src.begin(), src.begin() + 2);
                } else {
                    str += src[0];
                    src.erase(src.begin());
                }
            }

            if (src.size() > 0 && src[0] == '\'') {
                src.erase(src.begin()); 
            } else {
                throw std::runtime_error("Lexer Error: Unclosed string literal.");
            }

            tokens.push_back(Token(str, TokenType::StringLiteral));
            continue;
        }
        if (src[0] == '=' && src.size() > 1 && src[1] == '=') {
            tokens.push_back(Token("==", TokenType::ComparisonOp));
            src.erase(src.begin(), src.begin() + 2);
            continue;
        }
        if (src[0] == 'n' && src.size() > 1 && src[1] == '=') {
            tokens.push_back(Token("n=", TokenType::ComparisonOp));
            src.erase(src.begin(), src.begin() + 2);
            continue;
        }
        if ((src[0] == '+' || src[0] == '-' || src[0] == '*' || src[0] == '/') &&
            src.size() > 1 && src[1] == '=') {
            string op;
            op += src[0];
            op += '=';
            tokens.push_back(Token(op, TokenType::AssignmentOp));
            src.erase(src.begin(), src.begin() + 2);
            continue;
        }
        if (src[0] == '=') {
            tokens.push_back(Token("=", TokenType::AssignmentOp));
            src.erase(src.begin());
            continue;
        }if ((src[0] == '<' || src[0] == '>') && src.size() > 1 && src[1] == '=') {
            string op;
            op += src[0];
            op += '=';
            tokens.push_back(Token(op, TokenType::ComparisonOp));
            src.erase(src.begin(), src.begin() + 2);
            continue;
        }
        if (src[0] == '~') {
            tokens.push_back(Token("~", TokenType::RangeOp));
            src.erase(src.begin());
            continue;
        }
        if (src[0] == '<' || src[0] == '>') {
            tokens.push_back(Token(string(1, src[0]), TokenType::ComparisonOp));
            src.erase(src.begin());
            continue;
        }
        
         if (src[0]=='[') {
            tokens.push_back(Token(string(1, src[0]), TokenType::LBracket));
            src.erase(src.begin());
            continue;
        }  if (src[0]==']') {
            tokens.push_back(Token(string(1,src[0]),TokenType::RBracket));
            src.erase(src.begin());
            continue;
        } if (src[0] == '\\') {
            tokens.push_back(Token("\\", TokenType::Backslash));
            src.erase(src.begin());
            continue;
        }
        if (src[0]=='+'||src[0]=='-'||src[0]=='/'||src[0]=='*'){
            tokens.push_back(Token(string(1,src[0]),TokenType::ArithmeticOp));
            src.erase(src.begin());
            continue;
        }  if (src[0]==';'){
            tokens.push_back(Token(string(1,src[0]),TokenType::Semicolon));
            src.erase(src.begin());
            continue;
        }  if (src[0]==','){
            tokens.push_back(Token(string(1,src[0]),TokenType::Comma));
            src.erase(src.begin());
            continue;
        }  if (src[0]=='|') {
            tokens.push_back(Token(string(1,src[0]),TokenType::Pipe));
            src.erase(src.begin());
            continue;
        }  if (src[0]=='`') {
            tokens.push_back(Token(string(1,src[0]),TokenType::Backtick));
            src.erase(src.begin());
            continue;
        }
        tokens.push_back(Token(string(1, src[0]), TokenType::Invalid));
        src.erase(src.begin());
    }
    tokens.push_back(Token("", TokenType::EndOfFile));
    return tokens;
}
string tokenTypeName(TokenType t) {
    switch (t) {
        case TokenType::Null: return "Null";
        case TokenType::Identifier: return "Identifier";
        case TokenType::Keyword: return "Keyword";
        case TokenType::BuiltinIdentifier: return "BuiltinIdentifier";
        case TokenType::TypeIdent: return "TypeIdent";
        case TokenType::IntLiteral: return "IntLiteral";
        case TokenType::FloatLiteral: return "FloatLiteral";
        case TokenType::StringLiteral: return "StringLiteral";
        case TokenType::BoolLiteral: return "BoolLiteral";
        case TokenType::ArithmeticOp: return "ArithmeticOp";
        case TokenType::LogicalOp: return "LogicalOp";
        case TokenType::ComparisonOp: return "ComparisonOp";
        case TokenType::AssignmentOp: return "AssignmentOp";
        case TokenType::RangeOp: return "RangeOp";
        case TokenType::LBracket: return "LBracket";
        case TokenType::RBracket: return "RBracket";
        case TokenType::Backslash: return "Backslash";
        case TokenType::Pipe: return "Pipe";
        case TokenType::Comma: return "Comma";
        case TokenType::Semicolon: return "Semicolon";
        case TokenType::Backtick: return "Backtick";
        case TokenType::CommentLine: return "CommentLine";
        case TokenType::CommentBlock: return "CommentBlock";
        case TokenType::EndOfFile: return "EndOfFile";
        case TokenType::Invalid: return "Invalid";
        case TokenType::BlockKeyword: return "BlockKeyword";
    }
    return "Unknown";
}

void printTokens(const vector<Token>& tokens) {
    for (const Token& t : tokens) {
        cout << t.value << " : " << tokenTypeName(t.type) << endl;
    }
}


/*int main() {
    cout << "Enter file path: ";
    string filen;
    cin >> filen;

    ifstream file(filen);
    if (!file.is_open()) {
        cerr << "Failed to open file\n";
        return 1;
    }

    stringstream buffer;
    buffer << file.rdbuf();  // read entire file
    string contents = buffer.str();

    vector<Token> tokens = tokenize(contents);
    printTokens(tokens);

    file.close();
    return 0;
}*/
