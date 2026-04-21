#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "Expression.h"
#include <memory>
#include <unordered_map>
#include <stdexcept>

class Evaluator {
    public:
    static double evaluate(const std::shared_ptr<ExpressionNode>& root, 
                          const std::unordered_map<std::string, double>& variables = {});
};

#endif