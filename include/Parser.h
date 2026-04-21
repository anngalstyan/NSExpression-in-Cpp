#ifndef PARSER_H
#define PARSER_H

#include "Lexer.h"
#include "Expression.h"
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>

class Parser {
public:
    Parser(const std::vector<Token>& tokens, std::unordered_map<std::string, double>& variables);
    std::shared_ptr<ExpressionNode> parse();
private:
    std::vector<Token> tokens;
    size_t pos;
    std::unordered_map<std::string, double>& variables;
    Token currentToken() const;
    void advance();
    std::shared_ptr<ExpressionNode> parseExpression();
    std::shared_ptr<ExpressionNode> parseTerm();
    std::shared_ptr<ExpressionNode> parseFactor();
    std::shared_ptr<ExpressionNode> parsePrimary();
};

#endif // PARSER_H