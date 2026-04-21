#include "Parser.h"
#include <stdexcept>
#include <cmath>

Parser::Parser(const std::vector<Token>& tokens, std::unordered_map<std::string, double>& variables)
    : tokens(tokens), pos(0), variables(variables) {}

Token Parser::currentToken() const {
    if (pos < tokens.size()) {
        return tokens[pos];
    }
    return Token{TokenType::EOF_TOKEN, ""};
}

void Parser::advance() {
    ++pos;
}

std::shared_ptr<ExpressionNode> Parser::parseExpression() {
    auto left = parseTerm();
    while (pos < tokens.size() && currentToken().type == TokenType::OPERATOR) {
        std::string op = currentToken().value;
        if (op != "+" && op != "-") break;
        advance();
        auto right = parseTerm();
        left = std::make_shared<BinaryOpNode>(left, right, op);
    }
    return left;
}

std::shared_ptr<ExpressionNode> Parser::parseTerm() {
    auto left = parseFactor();
    while (pos < tokens.size()) {
        if (currentToken().type == TokenType::OPERATOR) {
            std::string op = currentToken().value;
            if (op != "*" && op != "/" && op != "^") break;
            advance();
            auto right = parseFactor();
            left = std::make_shared<BinaryOpNode>(left, right, op);
        } else if (currentToken().type == TokenType::VARIABLE ||
                   (currentToken().type == TokenType::PARENTHESIS && currentToken().value == "(") ||
                   currentToken().type == TokenType::IDENTIFIER ||
                   currentToken().type == TokenType::CONSTANT ||
                   currentToken().type == TokenType::FUNCTION) {
            auto right = parseFactor();
            left = std::make_shared<BinaryOpNode>(left, right, "*");
        } else {
            break;
        }
    }
    return left;
}

std::shared_ptr<ExpressionNode> Parser::parseFactor() {
    if (pos < tokens.size() && currentToken().type == TokenType::OPERATOR && currentToken().value == "-") {
        advance();
        auto operand = parseFactor();
        return std::make_shared<UnaryOpNode>(operand, "-");
    }
    return parsePrimary();
}

std::shared_ptr<ExpressionNode> Parser::parsePrimary() {
    if (pos >= tokens.size()) {
        throw std::runtime_error("Unexpected end of input");
    }

    Token token = currentToken();
    advance();

    if (token.type == TokenType::NUMBER) {
        try {
            double value = std::stod(token.value);
            return std::make_shared<NumberNode>(value);
        } catch (const std::exception&) {
            throw std::runtime_error("Invalid number: " + token.value);
        }
    } else if (token.type == TokenType::VARIABLE || token.type == TokenType::IDENTIFIER || token.type == TokenType::CONSTANT) {
        return std::make_shared<VariableNode>(token.value);
    } else if (token.type == TokenType::FUNCTION) {
        if (pos < tokens.size() && currentToken().type == TokenType::PARENTHESIS && currentToken().value == "(") {
            advance();
            std::vector<std::shared_ptr<ExpressionNode>> args;
            if (pos < tokens.size() && !(currentToken().type == TokenType::PARENTHESIS && currentToken().value == ")")) {
                args.push_back(parseExpression());
            }
            if (pos >= tokens.size() || !(currentToken().type == TokenType::PARENTHESIS && currentToken().value == ")")) {
                throw std::runtime_error("Expected closing parenthesis in function call");
            }
            advance();
            return std::make_shared<FunctionNode>(token.value, args);
        }
        throw std::runtime_error("Expected parenthesis after function: " + token.value);
    } else if (token.type == TokenType::PARENTHESIS && token.value == "(") {
        auto expr = parseExpression();
        if (pos >= tokens.size() || !(currentToken().type == TokenType::PARENTHESIS && currentToken().value == ")")) {
            throw std::runtime_error("Expected closing parenthesis");
        }
        advance();
        return expr;
    } else {
        throw std::runtime_error("Unexpected token: " + token.value);
    }
}

std::shared_ptr<ExpressionNode> Parser::parse() {
    if (tokens.empty()) {
        throw std::runtime_error("Empty input");
    }

    auto expr = parseExpression();

    if (pos < tokens.size() && currentToken().type == TokenType::EQUALS) {
        advance();
        auto right = parseExpression();
        expr = std::make_shared<EquationNode>(expr, right);
    }

    if (pos < tokens.size() && currentToken().type != TokenType::EOF_TOKEN) {
        throw std::runtime_error("Unexpected token after expression: " + currentToken().value);
    }

    return expr;
}