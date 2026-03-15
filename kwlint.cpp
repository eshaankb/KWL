/**
 * @file kwlint.cpp
 * @brief KWL production interpreter / linting tool.
 *
 * A streamlined entry point for running KWL source files. This is the
 * main executable used in production to interpret .kwl files. It parses
 * the file, evaluates it, and reports any runtime errors.
 *
 * Usage: kwlint <file.kwl>
 *
 * @author KWL Interpreter
 * @date 2026
 */

// kwlint — KWL Production Interpreter
// Usage: kwlint <file.kwl>

#include <iostream>
#include <fstream>
#include <string>
#include "parser.hpp"
#include "interpreter.hpp"
#include "environment.hpp"

/**
 * @brief Main entry point for the kwlint interpreter.
 * @param argc Argument count (should be 2)
 * @param argv Arguments: [0] = program name, [1] = .kwl file path
 * @return 0 on successful execution, 1 on error
 *
 * Reads a .kwl file, parses it into an AST, and executes it.
 * Any runtime errors are caught and displayed.
 */
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "KWL Interpreter\n"
                  << "Usage: kwlint <file.kwl>\n";
        return 1;
    }

    std::string filename = argv[1];

    if (filename.size() < 5 ||
        filename.substr(filename.size() - 4) != ".kwl") {
        std::cerr << "Error: '" << filename << "' is not a .kwl file.\n";
        return 1;
    }

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open '" << filename << "'.\n";
        return 1;
    }

    std::string source((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();

    try {
        Parser parser;
        Environment env;
        Program program = parser.produceAST(source);
        Eval(&program, env);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
