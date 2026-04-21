#include "Token.h"
#include <iostream>

Token::Token(TokenType t, const std::string& v)
    : type(t), value(v), precedence(OperatorPrecedence::LOW) {}

Token::Token(TokenType t, const std::string& v, OperatorPrecedence p)
    : type(t), value(v), precedence(p) {}

void Token::displayToken() const {
    std::cout << "Token Type: " << static_cast<int>(type) << ", Value: " << value << std::endl;
}

std::ostream& operator<<(std::ostream& os, const Token& token) {
    os << "Token Type: " << static_cast<int>(token.type) << ", Value:  " << token.value;
    return os;
}
