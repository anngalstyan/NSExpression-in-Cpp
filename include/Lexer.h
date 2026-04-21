#ifndef LEXER_H
#define LEXER_H

#include "Token.h"
#include <vector>
#include <string>
#include <stdexcept>

class Lexer {
private:
    std::string input;
    size_t position;
    std::vector<Token> tokens;

    char currentChar() const;
    char nextChar();
    void advance();

    bool isWhitespace(char c) const;
    bool isDigit(char c) const;
    bool isOperator(char c) const;
    bool isParathesis(char c) const;
    bool isFunction(const std::string& str) const;
    bool isConstant(const std::string& str) const;

    void handleNumber();
    void handleOperator();
    void handleParanthesis();
    void handleFunction();
    void handleVariable();
    void handleComma();
    //void handleIdentifierOrFunction();

public:
    Lexer(const std::string& input) : input(input), position(0) {}
    std::vector<Token> tokenize();
};

#endif
