#include "Expression.h"
#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <vector>
#include <stdexcept>

// EquationNode implementation
EquationNode::EquationNode(std::shared_ptr<ExpressionNode> left, std::shared_ptr<ExpressionNode> right)
    : left(left), right(right) {}

double EquationNode::evaluate(const std::unordered_map<std::string, double>& vars) const {
    return left->evaluate(vars) - right->evaluate(vars);
}

bool EquationNode::isEquation() const {
    return true;
}

std::vector<double> EquationNode::solveNonLinear(const std::string& var, const std::unordered_map<std::string, double>& variables) const {
    auto f = [&](double x) {
        std::unordered_map<std::string, double> vars = variables;
        vars[var] = x;
        return left->evaluate(vars) - right->evaluate(vars);
    };

    auto df = [&](double x) {
        const double h = 1e-6;
        return (f(x + h) - f(x - h)) / (2 * h);
    };

    std::vector<double> solutions;
    const double tolerance = 1e-6;
    const int max_iterations = 100;

    // Try different initial points
    for (double x0 = -10.0; x0 <= 10.0; x0 += 1.0) {
        double x = x0;
        for (int i = 0; i < max_iterations; ++i) {
            double fx = f(x);
            if (std::abs(fx) < tolerance) {
                // Check if this solution is unique
                bool is_unique = true;
                for (double sol : solutions) {
                    if (std::abs(x - sol) < tolerance) {
                        is_unique = false;
                        break;
                    }
                }
                if (is_unique) {
                    solutions.push_back(x);
                }
                break;
            }
            double dfx = df(x);
            if (std::abs(dfx) < 1e-10) break;
            x = x - fx / dfx;
        }
    }

    return solutions;
}

double EquationNode::solveFor(const std::string& var, const std::unordered_map<std::string, double>& variables) const {
    // For linear equations, we can solve directly
    double coeff = 0.0;
    double constant = 0.0;
    extractLinear(left, coeff, constant, var);
    extractLinear(right, coeff, constant, var);
    
    if (std::abs(coeff) < 1e-10) {
        if (std::abs(constant) < 1e-10) {
            throw std::runtime_error("Infinite solutions");
        } else {
            throw std::runtime_error("No solution");
        }
    }
    
    return -constant / coeff;
}

std::vector<Complex> EquationNode::solveComplex(const std::string& var) const {
    // For quadratic equations, we can solve using the quadratic formula
    auto f = [&](const Complex& x) {
        std::unordered_map<std::string, double> vars;
        vars[var] = x.getReal();
        double leftVal = left->evaluate(vars);
        double rightVal = right->evaluate(vars);
        return Complex(leftVal - rightVal);
    };

    // Try to solve as a quadratic equation
    double a = 0.0, b = 0.0, c = 0.0;
    bool isQuadratic = false;

    // Check if it's a quadratic equation in standard form: ax^2 + bx + c = 0
    if (auto bin = std::dynamic_pointer_cast<BinaryOpNode>(left)) {
        if (bin->getOp() == "+") {
            if (auto leftBin = std::dynamic_pointer_cast<BinaryOpNode>(bin->getLeft())) {
                if (leftBin->getOp() == "*") {
                    if (auto rightBin = std::dynamic_pointer_cast<BinaryOpNode>(leftBin->getRight())) {
                        if (rightBin->getOp() == "^") {
                            if (auto varNode = std::dynamic_pointer_cast<VariableNode>(rightBin->getLeft())) {
                                if (varNode->getName() == var) {
                                    if (auto numNode = std::dynamic_pointer_cast<NumberNode>(rightBin->getRight())) {
                                        if (std::abs(numNode->getValue() - 2.0) < 1e-10) {
                                            if (auto numNode2 = std::dynamic_pointer_cast<NumberNode>(leftBin->getLeft())) {
                                                a = numNode2->getValue();
                                                isQuadratic = true;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (isQuadratic) {
        // Extract b and c coefficients
        std::unordered_map<std::string, double> vars;
        vars[var] = 0.0;
        c = left->evaluate(vars) - right->evaluate(vars);
        vars[var] = 1.0;
        double f1 = left->evaluate(vars) - right->evaluate(vars);
        b = f1 - a - c;

        // Solve using quadratic formula
        Complex discriminant = Complex(b * b - 4 * a * c);
        Complex sqrtDisc = Complex(std::sqrt(std::abs(discriminant.getReal())));
        
        std::vector<Complex> solutions;
        Complex twoA(2.0 * a);
        Complex negB(-b);
        solutions.push_back((negB + sqrtDisc) / twoA);
        solutions.push_back((negB - sqrtDisc) / twoA);
        return solutions;
    }

    // If not quadratic, convert numerical solutions to complex
    std::vector<double> realSolutions = solveNonLinear(var, {});
    std::vector<Complex> complexSolutions;
    for (double sol : realSolutions) {
        complexSolutions.push_back(Complex(sol));
    }
    return complexSolutions;
}

void EquationNode::extractLinear(const std::shared_ptr<ExpressionNode>& node, double& coeff, double& constant, const std::string& var) {
    if (auto bin = std::dynamic_pointer_cast<BinaryOpNode>(node)) {
        if (bin->getOp() == "+") {
            extractLinear(bin->getLeft(), coeff, constant, var);
            extractLinear(bin->getRight(), coeff, constant, var);
        } else if (bin->getOp() == "-") {
            double rightCoeff = 0.0;
            double rightConstant = 0.0;
            extractLinear(bin->getLeft(), coeff, constant, var);
            extractLinear(bin->getRight(), rightCoeff, rightConstant, var);
            coeff -= rightCoeff;
            constant -= rightConstant;
        } else if (bin->getOp() == "*") {
            if (auto varNode = std::dynamic_pointer_cast<VariableNode>(bin->getLeft())) {
                if (varNode->getName() == var) {
                    if (auto numNode = std::dynamic_pointer_cast<NumberNode>(bin->getRight())) {
                        coeff += numNode->getValue();
                    }
                }
            } else if (auto varNode = std::dynamic_pointer_cast<VariableNode>(bin->getRight())) {
                if (varNode->getName() == var) {
                    if (auto numNode = std::dynamic_pointer_cast<NumberNode>(bin->getLeft())) {
                        coeff += numNode->getValue();
                    }
                }
            }
        }
    } else if (auto varNode = std::dynamic_pointer_cast<VariableNode>(node)) {
        if (varNode->getName() == var) {
            coeff += 1.0;
        }
    } else if (auto numNode = std::dynamic_pointer_cast<NumberNode>(node)) {
        constant += numNode->getValue();
    }
}

Complex EquationNode::evaluateComplex(const std::shared_ptr<ExpressionNode>& node, const Complex& x) const {
    if (auto bin = std::dynamic_pointer_cast<BinaryOpNode>(node)) {
        Complex leftVal = evaluateComplex(bin->getLeft(), x);
        Complex rightVal = evaluateComplex(bin->getRight(), x);
        
        if (bin->getOp() == "+") return leftVal + rightVal;
        if (bin->getOp() == "-") return leftVal - rightVal;
        if (bin->getOp() == "*") return leftVal * rightVal;
        if (bin->getOp() == "/") return leftVal / rightVal;
        if (bin->getOp() == "^") {
            // Handle integer powers
            if (auto numNode = std::dynamic_pointer_cast<NumberNode>(bin->getRight())) {
                int power = static_cast<int>(numNode->getValue());
                Complex result(1.0);
                for (int i = 0; i < std::abs(power); ++i) {
                    result = result * leftVal;
                }
                return power < 0 ? Complex(1.0) / result : result;
            }
        }
    } else if (auto varNode = std::dynamic_pointer_cast<VariableNode>(node)) {
        return x;
    } else if (auto numNode = std::dynamic_pointer_cast<NumberNode>(node)) {
        return Complex(numNode->getValue());
    }
    return Complex(0.0);
}

// NumberNode implementation
NumberNode::NumberNode(double value) : value(value) {}

double NumberNode::evaluate(const std::unordered_map<std::string, double>&) const {
    return value;
}

double NumberNode::getValue() const { return value; }

// VariableNode implementation
VariableNode::VariableNode(const std::string& name) : name(name) {}

double VariableNode::evaluate(const std::unordered_map<std::string, double>& vars) const {
    auto it = vars.find(name);
    if (it == vars.end()) {
        throw std::runtime_error("Undefined variable: " + name);
    }
    return it->second;
}

std::string VariableNode::getName() const { return name; }

// BinaryOpNode implementation
BinaryOpNode::BinaryOpNode(std::shared_ptr<ExpressionNode> left, std::shared_ptr<ExpressionNode> right, const std::string& op)
    : left(left), right(right), op(op) {}

double BinaryOpNode::evaluate(const std::unordered_map<std::string, double>& vars) const {
    double leftVal = left->evaluate(vars);
    double rightVal = right->evaluate(vars);
    
    if (op == "+") return leftVal + rightVal;
    if (op == "-") return leftVal - rightVal;
    if (op == "*") return leftVal * rightVal;
    if (op == "/") {
        if (std::abs(rightVal) < 1e-10) {
            throw std::runtime_error("Division by zero");
        }
        return leftVal / rightVal;
    }
    if (op == "^") return std::pow(leftVal, rightVal);
    
    throw std::runtime_error("Unknown operator: " + op);
}

std::string BinaryOpNode::getOp() const { return op; }
std::shared_ptr<ExpressionNode> BinaryOpNode::getLeft() const { return left; }
std::shared_ptr<ExpressionNode> BinaryOpNode::getRight() const { return right; }

// UnaryOpNode implementation
UnaryOpNode::UnaryOpNode(std::shared_ptr<ExpressionNode> operand, const std::string& op)
    : operand(operand), op(op) {}

double UnaryOpNode::evaluate(const std::unordered_map<std::string, double>& vars) const {
    double val = operand->evaluate(vars);
    if (op == "-") return -val;
    throw std::runtime_error("Unknown unary operator: " + op);
}

std::string UnaryOpNode::getOp() const { return op; }
std::shared_ptr<ExpressionNode> UnaryOpNode::getOperand() const { return operand; }

// FunctionNode implementation
FunctionNode::FunctionNode(const std::string& name, const std::vector<std::shared_ptr<ExpressionNode>>& args)
    : name(name), args(args) {}

double FunctionNode::evaluate(const std::unordered_map<std::string, double>& vars) const {
    if (args.size() != 1) {
        throw std::runtime_error("Only single-argument functions supported");
    }
    
    double arg = args[0]->evaluate(vars);
    
    if (name == "sin") return std::sin(arg);
    if (name == "cos") return std::cos(arg);
    if (name == "tan") return std::tan(arg);
    if (name == "asin") {
        if (arg < -1 || arg > 1) throw std::runtime_error("asin domain error");
        return std::asin(arg);
    }
    if (name == "acos") {
        if (arg < -1 || arg > 1) throw std::runtime_error("acos domain error");
        return std::acos(arg);
    }
    if (name == "atan") return std::atan(arg);
    if (name == "log" || name == "ln") {
        if (arg <= 0) throw std::runtime_error("Logarithm undefined for non-positive argument");
        return std::log(arg);
    }
    if (name == "log10") {
        if (arg <= 0) throw std::runtime_error("Log10 undefined for non-positive argument");
        return std::log10(arg);
    }
    if (name == "log2") {
        if (arg <= 0) throw std::runtime_error("Log2 undefined for non-positive argument");
        return std::log2(arg);
    }
    if (name == "exp") return std::exp(arg);
    if (name == "sqrt") {
        if (arg < 0) throw std::runtime_error("Square root undefined for negative argument");
        return std::sqrt(arg);
    }
    if (name == "cbrt") return std::cbrt(arg);
    
    throw std::runtime_error("Unknown function: " + name);
}

std::string FunctionNode::getName() const { return name; }
const std::vector<std::shared_ptr<ExpressionNode>>& FunctionNode::getArgs() const { return args; }