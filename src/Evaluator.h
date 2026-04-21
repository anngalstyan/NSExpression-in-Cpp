#include "Evaluator.h"

double Evaluator::evaluate(const std::shared_ptr<ExpressionNode>& root, 
                          const std::unordered_map<std::string, double>& variables) {
    if (!root) {
        throw std::invalid_argument("Null expression node encountered");
    }
    return root->evaluate(variables);
}
