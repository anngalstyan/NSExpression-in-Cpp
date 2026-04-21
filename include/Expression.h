#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <memory>
#include <string>
#include <unordered_map>
#include <cmath>
#include <stdexcept>
#include <vector>
#include "Complex.h"

class ExpressionNode {
public:
    virtual ~ExpressionNode() = default;
    virtual double evaluate(const std::unordered_map<std::string, double>& variables) const = 0;
    virtual bool isEquation() const { return false; }
};

class NumberNode : public ExpressionNode {
private:
    double value;
public:
    NumberNode(double value);
    double evaluate(const std::unordered_map<std::string, double>&) const override;
    double getValue() const;
};

class VariableNode : public ExpressionNode {
private:
    std::string name;
public:
    VariableNode(const std::string& name);
    double evaluate(const std::unordered_map<std::string, double>& variables) const override;
    std::string getName() const;
};

class BinaryOpNode : public ExpressionNode {
private:
    std::shared_ptr<ExpressionNode> left;
    std::shared_ptr<ExpressionNode> right;
    std::string op;
public:
    BinaryOpNode(std::shared_ptr<ExpressionNode> left, std::shared_ptr<ExpressionNode> right, const std::string& op);
    double evaluate(const std::unordered_map<std::string, double>& variables) const override;
    std::string getOp() const;
    std::shared_ptr<ExpressionNode> getLeft() const;
    std::shared_ptr<ExpressionNode> getRight() const;
};

class UnaryOpNode : public ExpressionNode {
private:
    std::shared_ptr<ExpressionNode> operand;
    std::string op;
public:
    UnaryOpNode(std::shared_ptr<ExpressionNode> operand, const std::string& op);
    double evaluate(const std::unordered_map<std::string, double>& variables) const override;
    std::string getOp() const;
    std::shared_ptr<ExpressionNode> getOperand() const;
};

class FunctionNode : public ExpressionNode {
private:
    std::string name;
    std::vector<std::shared_ptr<ExpressionNode>> args;
public:
    FunctionNode(const std::string& name, const std::vector<std::shared_ptr<ExpressionNode>>& args);
    double evaluate(const std::unordered_map<std::string, double>& variables) const override;
    std::string getName() const;
    const std::vector<std::shared_ptr<ExpressionNode>>& getArgs() const;
};

class EquationNode : public ExpressionNode {
private:
    std::shared_ptr<ExpressionNode> left;
    std::shared_ptr<ExpressionNode> right;
    static void extractLinear(const std::shared_ptr<ExpressionNode>& node, double& coeff, double& constant, const std::string& var);
    Complex evaluateComplex(const std::shared_ptr<ExpressionNode>& node, const Complex& x) const;
public:
    EquationNode(std::shared_ptr<ExpressionNode> left, std::shared_ptr<ExpressionNode> right);
    double evaluate(const std::unordered_map<std::string, double>& variables) const override;
    bool isEquation() const override;
    double solveFor(const std::string& var, const std::unordered_map<std::string, double>& variables) const;
    std::vector<Complex> solveComplex(const std::string& var) const;
    std::vector<double> solveNonLinear(const std::string& var, const std::unordered_map<std::string, double>& variables) const;
};

#endif