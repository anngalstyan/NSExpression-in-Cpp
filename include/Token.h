#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <iostream>

enum class TokenType {
    NUMBER,
    OPERATOR,
    FUNCTION,
    PARENTHESIS,
    COMMA,
    IDENTIFIER,
    EQUALS,
    VARIABLE,
    CONSTANT,
    EOF_TOKEN
};

enum class OperatorPrecedence {
    LOW,
    MEDIUM,
    HIGH
};

class Token {
public:
    TokenType type;
    std::string value;
    OperatorPrecedence precedence;

    Token(TokenType t, const std::string& v);
    Token(TokenType t, const std::string& v, OperatorPrecedence p);

    void displayToken() const;

    friend std::ostream& operator<<(std::ostream& os, const Token& token);
};

#endif

