#ifndef FUNCTION_ANALYZER_H
#define FUNCTION_ANALYZER_H

#include "Expression.h"
#include <vector>
#include <memory>
#include <string>
#include <utility>

class FunctionAnalyzer {
public:
    explicit FunctionAnalyzer(const std::shared_ptr<ExpressionNode>& expr);

    // Domain and range analysis
    std::vector<std::pair<double, double>> getDomain() const;
    std::pair<double, double> getRange() const;

    // Symmetry analysis
    bool isEven() const;
    bool isOdd() const;
    bool isSymmetric() const;

    // Intercept analysis
    std::vector<std::pair<double, double>> getXAxisIntercepts() const;
    std::pair<double, double> getYAxisIntercept() const;

    // Plotting
    void plotNcurses(const std::string& filename) const;

private:
    std::shared_ptr<ExpressionNode> expr;
    std::vector<std::pair<double, double>> getRestrictedDomain(const std::shared_ptr<ExpressionNode>& node) const;
};

#endif // FUNCTION_ANALYZER_H 