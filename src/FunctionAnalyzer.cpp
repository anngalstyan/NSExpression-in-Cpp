#include "../include/FunctionAnalyzer.h"
#include "Expression.h"
#include <cmath>
#include <limits>
#include <algorithm>
#include <iomanip>
#include <ncurses.h>
#include <fstream>
#include <unordered_map>

// Constants for plotting and analysis
constexpr double PLOT_MIN = -10.0;
constexpr double PLOT_MAX = 10.0;
constexpr double PLOT_STEP = 0.1;
constexpr double EPSILON = 1e-10;

FunctionAnalyzer::FunctionAnalyzer(const std::shared_ptr<ExpressionNode>& expr) : expr(expr) {}

std::vector<std::pair<double, double>> FunctionAnalyzer::getRestrictedDomain(const std::shared_ptr<ExpressionNode>& node) const {
    // [Unchanged code from your original FunctionAnalyzer.cpp]
    if (auto func = std::dynamic_pointer_cast<FunctionNode>(node)) {
        if (func->getName() == "sqrt" || func->getName() == "cbrt") {
            return { {0.0, std::numeric_limits<double>::infinity()} };
        }
        if (func->getName() == "log" || func->getName() == "ln" || func->getName() == "log10" || func->getName() == "log2") {
            return { {1e-10, std::numeric_limits<double>::infinity()} };
        }
        if (func->getName() == "tan") {
            std::vector<std::pair<double, double>> intervals;
            const double pi = 3.141592653589793;
            for (int k = -3; k <= 3; ++k) {
                double start = k * pi - pi / 2 + EPSILON;
                double end = k * pi + pi / 2 - EPSILON;
                if (end >= PLOT_MIN && start <= PLOT_MAX) {
                    intervals.emplace_back(std::max(start, PLOT_MIN), std::min(end, PLOT_MAX));
                }
            }
            return intervals;
        }
        if (func->getName() == "asin" || func->getName() == "acos") {
            return { {-1.0, 1.0} };
        }
        if (func->getName() == "atan" || func->getName() == "sin" || func->getName() == "cos" || func->getName() == "exp") {
            return { {-std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity()} };
        }
        if (func->getArgs().size() == 1) {
            return getRestrictedDomain(func->getArgs()[0]);
        }
    } else if (auto bin = std::dynamic_pointer_cast<BinaryOpNode>(node)) {
        auto leftDomain = getRestrictedDomain(bin->getLeft());
        auto rightDomain = getRestrictedDomain(bin->getRight());
        if (bin->getOp() == "^") {
            if (auto rightConst = std::dynamic_pointer_cast<NumberNode>(bin->getRight())) {
                int power = static_cast<int>(rightConst->getValue());
                if (power % 2 == 0) {
                    return { {0.0, std::numeric_limits<double>::infinity()} }; // x^2, x^4
                } else {
                    return { {-std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity()} }; // x^3, x^5
                }
            }
        } else if (bin->getOp() == "/") {
            if (auto leftConst = std::dynamic_pointer_cast<NumberNode>(bin->getLeft())) {
                if (auto rightVar = std::dynamic_pointer_cast<VariableNode>(bin->getRight())) {
                    if (std::abs(leftConst->getValue() - 1.0) < EPSILON && rightVar->getName() == "x") {
                        return { {-std::numeric_limits<double>::infinity(), -EPSILON}, {EPSILON, std::numeric_limits<double>::infinity()} }; // 1/x
                    }
                }
            }
            std::vector<std::pair<double, double>> intervals;
            double last = PLOT_MIN;
            std::unordered_map<std::string, double> vars;
            bool open = true;
            for (double x = PLOT_MIN; x <= PLOT_MAX + EPSILON; x += PLOT_STEP) {
                vars["x"] = x;
                try {
                    double denom = bin->getRight()->evaluate(vars);
                    if (std::abs(denom) < EPSILON && open) {
                        if (x - last > EPSILON) {
                            intervals.emplace_back(last, x - EPSILON);
                        }
                        last = x + EPSILON;
                        open = false;
                    } else if (std::abs(denom) >= EPSILON && !open) {
                        open = true;
                    }
                } catch (const std::exception&) {
                    if (open) {
                        if (x - last > EPSILON) {
                            intervals.emplace_back(last, x - EPSILON);
                        }
                        last = x + EPSILON;
                        open = false;
                    }
                }
            }
            if (open && PLOT_MAX - last > EPSILON) {
                intervals.emplace_back(last, PLOT_MAX);
            }
            return intervals;
        }
        std::vector<std::pair<double, double>> result;
        for (const auto& l : leftDomain) {
            for (const auto& r : rightDomain) {
                double start = std::max(l.first, r.first);
                double end = std::min(l.second, r.second);
                if (start < end - EPSILON) {
                    result.emplace_back(start, end);
                }
            }
        }
        return result;
    } else if (auto unary = std::dynamic_pointer_cast<UnaryOpNode>(node)) {
        return getRestrictedDomain(unary->getOperand());
    } else if (auto var = std::dynamic_pointer_cast<VariableNode>(node)) {
        return { {-std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity()} };
    }
    return { {-std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity()} };
}

std::vector<std::pair<double, double>> FunctionAnalyzer::getDomain() const {
    return getRestrictedDomain(expr);
}

std::pair<double, double> FunctionAnalyzer::getRange() const {
    // [Unchanged code]
    return {-10, 10};
    auto func = std::dynamic_pointer_cast<FunctionNode>(expr);
    if (func) {
        if (func->getName() == "sin" || func->getName() == "cos") {
            return { -1.0, 1.0 };
        }
        if (func->getName() == "tan" || func->getName() == "atan") {
            return { -std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity() };
        }
        if (func->getName() == "asin") {
            return { -3.141592653589793 / 2, 3.141592653589793 / 2 };
        }
        if (func->getName() == "acos") {
            return { 0.0, 3.141592653589793 };
        }
        if (func->getName() == "sqrt") {
            return { 0.0, std::numeric_limits<double>::infinity() };
        }
        if (func->getName() == "cbrt") {
            return { -std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity() };
        }
        if (func->getName() == "exp") {
            return { 0.0, std::numeric_limits<double>::infinity() };
        }
        if (func->getName() == "log" || func->getName() == "ln" || func->getName() == "log10" || func->getName() == "log2") {
            return { -std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity() };
        }
    } else if (auto bin = std::dynamic_pointer_cast<BinaryOpNode>(expr)) {
        if (bin->getOp() == "^") {
            if (auto rightConst = std::dynamic_pointer_cast<NumberNode>(bin->getRight())) {
                int power = static_cast<int>(rightConst->getValue());
                if (power % 2 == 0) {
                    return { 0.0, std::numeric_limits<double>::infinity() }; // x^2, x^4
                } else {
                    return { -std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity() }; // x^3, x^5
                }
            }
        } else if (bin->getOp() == "/") {
            if (auto leftConst = std::dynamic_pointer_cast<NumberNode>(bin->getLeft())) {
                if (auto rightVar = std::dynamic_pointer_cast<VariableNode>(bin->getRight())) {
                    if (std::abs(leftConst->getValue() - 1.0) < EPSILON && rightVar->getName() == "x") {
                        return { -std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity() }; // 1/x
                    }
                }
            }
        }
    }

    auto domains = getDomain();
    double minY = std::numeric_limits<double>::infinity();
    double maxY = -std::numeric_limits<double>::infinity();
    std::unordered_map<std::string, double> vars;
    for (const auto& domain : domains) {
        double minX = domain.first;
        double maxX = domain.second;
        for (double x = minX; x <= maxX; x += PLOT_STEP) {
            if (std::abs(x) < EPSILON) continue;
            vars["x"] = x;
            try {
                double y = expr->evaluate(vars);
                if (std::isfinite(y)) {
                    minY = std::min(minY, y);
                    maxY = std::max(maxY, y);
                }
            } catch (const std::exception&) {
                // Skip undefined points
            }
        }
    }
    if (!std::isfinite(minY) || !std::isfinite(maxY)) {
        return { -std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity() };
    }
    return { minY, maxY };
}

bool FunctionAnalyzer::isOdd() const {
    // [Unchanged code]
    auto func = std::dynamic_pointer_cast<FunctionNode>(expr);
    if (func) {
        if (func->getName() == "sin" || func->getName() == "tan" || func->getName() == "cbrt" ||
            func->getName() == "asin" || func->getName() == "atan") {
            return true;
        }
        if (func->getName() == "cos" || func->getName() == "acos" || 
            func->getName() == "log" || func->getName() == "ln" ||
            func->getName() == "log10" || func->getName() == "log2" || 
            func->getName() == "exp" || func->getName() == "sqrt") {
            return false;
        }
    } else if (auto bin = std::dynamic_pointer_cast<BinaryOpNode>(expr)) {
        if (bin->getOp() == "^") {
            if (auto rightConst = std::dynamic_pointer_cast<NumberNode>(bin->getRight())) {
                int power = static_cast<int>(rightConst->getValue());
                return power % 2 == 1;
            }
        } else if (bin->getOp() == "/") {
            if (auto leftConst = std::dynamic_pointer_cast<NumberNode>(bin->getLeft())) {
                if (auto rightVar = std::dynamic_pointer_cast<VariableNode>(bin->getRight())) {
                    if (std::abs(leftConst->getValue() - 1.0) < EPSILON && rightVar->getName() == "x") {
                        return true;
                    }
                }
            }
        }
    }

    auto domains = getDomain();
    if (!std::isinf(domains[0].first) && domains[0].first >= 0) {
        return false;
    }
    std::unordered_map<std::string, double> vars;
    bool hasValidPoint = false;
    for (const auto& domain : domains) {
        double minX = domain.first;
        double maxX = domain.second;
        for (double x = minX; x <= maxX; x += PLOT_STEP) {
            if (std::abs(x) < EPSILON) continue;
            vars["x"] = x;
            double f_x;
            try {
                f_x = expr->evaluate(vars);
            } catch (const std::exception&) {
                continue;
            }
            vars["x"] = -x;
            bool inDomain = false;
            for (const auto& d : domains) {
                if (-x >= d.first - EPSILON && -x <= d.second + EPSILON) {
                    inDomain = true;
                    break;
                }
            }
            if (!inDomain) continue;
            double f_neg_x;
            try {
                f_neg_x = expr->evaluate(vars);
            } catch (const std::exception&) {
                continue;
            }
            if (std::abs(f_x + f_neg_x) > EPSILON) {
                return false;
            }
            hasValidPoint = true;
        }
    }
    return hasValidPoint;
}

bool FunctionAnalyzer::isEven() const {
    // [Unchanged code]
    auto func = std::dynamic_pointer_cast<FunctionNode>(expr);
    if (func) {
        if (func->getName() == "cos") {
            return true;
        }
        if (func->getName() == "sin" || func->getName() == "tan" || func->getName() == "asin" || 
            func->getName() == "acos" || func->getName() == "atan" || func->getName() == "log" ||
            func->getName() == "ln" || func->getName() == "log10" || func->getName() == "log2" ||
            func->getName() == "exp" || func->getName() == "sqrt" || func->getName() == "cbrt") {
            return false;
        }
    } else if (auto bin = std::dynamic_pointer_cast<BinaryOpNode>(expr)) {
        if (bin->getOp() == "^") {
            if (auto rightConst = std::dynamic_pointer_cast<NumberNode>(bin->getRight())) {
                int power = static_cast<int>(rightConst->getValue());
                return power % 2 == 0;
            }
        } else if (bin->getOp() == "/") {
            if (auto leftConst = std::dynamic_pointer_cast<NumberNode>(bin->getLeft())) {
                if (auto rightVar = std::dynamic_pointer_cast<VariableNode>(bin->getRight())) {
                    if (std::abs(leftConst->getValue() - 1.0) < EPSILON && rightVar->getName() == "x") {
                        return false;
                    }
                }
            }
        }
    }

    auto domains = getDomain();
    if (!std::isinf(domains[0].first) && domains[0].first >= 0) {
        return false;
    }
    std::unordered_map<std::string, double> vars;
    bool hasValidPoint = false;
    for (const auto& domain : domains) {
        double minX = domain.first;
        double maxX = domain.second;
        for (double x = minX; x <= maxX; x += PLOT_STEP) {
            if (std::abs(x) < EPSILON) continue;
            vars["x"] = x;
            double f_x;
            try {
                f_x = expr->evaluate(vars);
            } catch (const std::exception&) {
                continue;
            }
            vars["x"] = -x;
            bool inDomain = false;
            for (const auto& d : domains) {
                if (-x >= d.first - EPSILON && -x <= d.second + EPSILON) {
                    inDomain = true;
                    break;
                }
            }
            if (!inDomain) continue;
            double f_neg_x;
            try {
                f_neg_x = expr->evaluate(vars);
            } catch (const std::exception&) {
                continue;
            }
            if (std::abs(f_x - f_neg_x) > EPSILON) {
                return false;
            }
            hasValidPoint = true;
        }
    }
    return hasValidPoint;
}

bool FunctionAnalyzer::isSymmetric() const {
    return isEven();
}

std::vector<std::pair<double, double>> FunctionAnalyzer::getXAxisIntercepts() const {
    // [Unchanged code]
    auto func = std::dynamic_pointer_cast<FunctionNode>(expr);
    if (func) {
        if (func->getName() == "sin") {
            std::vector<std::pair<double, double>> intercepts;
            const double pi = 3.141592653589793;
            const int n = 3;
            for (int k = -n; k <= n; ++k) {
                intercepts.emplace_back(k * pi, 0.0);
            }
            std::sort(intercepts.begin(), intercepts.end(),
                      [](const auto& a, const auto& b) { return a.first < b.first; });
            return intercepts;
        }
        if (func->getName() == "cos") {
            std::vector<std::pair<double, double>> intercepts;
            const double pi = 3.141592653589793;
            const int n = 3;
            for (int k = -n; k <= n; ++k) {
                intercepts.emplace_back(pi / 2 + k * pi, 0.0);
            }
            std::sort(intercepts.begin(), intercepts.end(),
                      [](const auto& a, const auto& b) { return a.first < b.first; });
            return intercepts;
        }
        if (func->getName() == "tan") {
            std::vector<std::pair<double, double>> intercepts;
            const double pi = 3.141592653589793;
            const int n = 3;
            for (int k = -n; k <= n; ++k) {
                intercepts.emplace_back(k * pi, 0.0);
            }
            std::sort(intercepts.begin(), intercepts.end(),
                      [](const auto& a, const auto& b) { return a.first < b.first; });
            return intercepts;
        }
        if (func->getName() == "asin" || func->getName() == "atan") {
            return { {0.0, 0.0} };
        }
        if (func->getName() == "acos") {
            return { {1.0, 0.0} };
        }
        if (func->getName() == "sqrt" || func->getName() == "cbrt") {
            return { {0.0, 0.0} };
        }
        if (func->getName() == "exp") {
            return {};
        }
        if (func->getName() == "log" || func->getName() == "ln" || func->getName() == "log10" || func->getName() == "log2") {
            return { {1.0, 0.0} };
        }
    } else if (auto bin = std::dynamic_pointer_cast<BinaryOpNode>(expr)) {
        if (bin->getOp() == "^") {
            if (auto rightConst = std::dynamic_pointer_cast<NumberNode>(bin->getRight())) {
                return { {0.0, 0.0} };
            }
        } else if (bin->getOp() == "/") {
            if (auto leftConst = std::dynamic_pointer_cast<NumberNode>(bin->getLeft())) {
                if (auto rightVar = std::dynamic_pointer_cast<VariableNode>(bin->getRight())) {
                    if (std::abs(leftConst->getValue() - 1.0) < EPSILON && rightVar->getName() == "x") {
                        return {};
                    }
                }
            }
        }
    }

    auto domains = getDomain();
    std::vector<std::pair<double, double>> intercepts;
    std::unordered_map<std::string, double> vars;
    for (const auto& domain : domains) {
        double minX = domain.first;
        double maxX = domain.second;
        for (double x = minX; x <= maxX; x += PLOT_STEP / 10) {
            if (std::abs(x) < EPSILON) continue;
            vars["x"] = x;
            try {
                double y = expr->evaluate(vars);
                if (std::abs(y) < EPSILON) {
                    intercepts.emplace_back(x, 0.0);
                } else {
                    vars["x"] = x - PLOT_STEP / 10;
                    try {
                        double y_prev = expr->evaluate(vars);
                        if (y * y_prev < 0 && std::isfinite(y) && std::isfinite(y_prev)) {
                            intercepts.emplace_back(x - PLOT_STEP / 20, 0.0);
                        }
                    } catch (const std::exception&) {
                        // Skip undefined points
                    }
                }
            } catch (const std::exception&) {
                // Skip undefined points
            }
        }
    }
    std::sort(intercepts.begin(), intercepts.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });
    return intercepts;
}

std::pair<double, double> FunctionAnalyzer::getYAxisIntercept() const {
    // [Unchanged code]
    auto bin = std::dynamic_pointer_cast<BinaryOpNode>(expr);
    if (bin && bin->getOp() == "/") {
        if (auto leftConst = std::dynamic_pointer_cast<NumberNode>(bin->getLeft())) {
            if (auto rightVar = std::dynamic_pointer_cast<VariableNode>(bin->getRight())) {
                if (std::abs(leftConst->getValue() - 1.0) < EPSILON && rightVar->getName() == "x") {
                    return { 0.0, std::numeric_limits<double>::quiet_NaN() };
                }
            }
        }
    }

    auto domains = getDomain();
    bool includesZero = false;
    for (const auto& domain : domains) {
        if (domain.first <= 0.0 + EPSILON && domain.second >= 0.0 - EPSILON) {
            includesZero = true;
            break;
        }
    }
    if (!includesZero) {
        return { 0.0, std::numeric_limits<double>::quiet_NaN() };
    }
    std::unordered_map<std::string, double> vars;
    vars["x"] = 0.0;
    try {
        double y = expr->evaluate(vars);
        return { 0.0, std::abs(y) < EPSILON ? 0.0 : y };
    } catch (const std::exception&) {
        return { 0.0, std::numeric_limits<double>::quiet_NaN() };
    }
}

void FunctionAnalyzer::plotNcurses(const std::string& filename) const {
    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);  // Hide cursor
    timeout(100);
    flushinp();

    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    int width = max_x - 2;
    int height = max_y - 2;

    // Get domain and range
    auto domains = getDomain();
    double minX = std::numeric_limits<double>::infinity();
    double maxX = -std::numeric_limits<double>::infinity();
    for (const auto& domain : domains) {
        minX = std::min(minX, domain.first);
        maxX = std::max(maxX, domain.second);
    }
    minX = std::max(PLOT_MIN, minX);
    maxX = std::min(PLOT_MAX, maxX);

    double x_center = (minX + maxX) / 2.0;
    double x_range = maxX - minX;
    double zoom = 1.0;
    double y_center = 0.0;
    double y_range = x_range;

    std::vector<std::pair<double, double>> coordinates;
    bool running = true;

    while (running) {
        clear();
        erase();

        getmaxyx(stdscr, max_y, max_x);
        width = max_x - 2;
        height = max_y - 2;

        if (max_y < 10 || max_x < 20) {
            mvprintw(0, 0, "Terminal too small, please resize");
            refresh();
            continue;
        }

        double current_minX = x_center - (x_range * zoom) / 2.0;
        double current_maxX = x_center + (x_range * zoom) / 2.0;
        double current_minY = y_center - (y_range * zoom) / 2.0;
        double current_maxY = y_center + (y_range * zoom) / 2.0;

        // Calculate points
        coordinates.clear();
        std::unordered_map<std::string, double> vars;
        bool hasFiniteValues = false;
        double step = PLOT_STEP * zoom;
        for (double x = current_minX; x <= current_maxX + step / 2; x += step) {
            vars["x"] = x;
            try {
                double y = expr->evaluate(vars);
                if (std::isfinite(y)) {
                    coordinates.emplace_back(x, y);
                    hasFiniteValues = true;
                }
            } catch (const std::exception&) {
                // Skip invalid points
            }
        }

        if (!hasFiniteValues) {
            mvprintw(max_y / 2, (max_x - 30) / 2, "No finite values in range");
            refresh();
            continue;
        }

        // Compute y-range
        double minY = std::numeric_limits<double>::infinity();
        double maxY = -std::numeric_limits<double>::infinity();
        for (const auto& coord : coordinates) {
            minY = std::min(minY, coord.second);
            maxY = std::max(maxY, coord.second);
        }
        minY = std::min(minY, std::min(0.0, current_minY));
        maxY = std::max(maxY, std::max(0.0, current_maxY));
        double rangePadding = (maxY - minY) * 0.05;
        if (maxY == minY) {
            minY -= 1.0;
            maxY += 1.0;
        }
        minY -= rangePadding;
        maxY += rangePadding;

        // Draw axes
        int yAxis = static_cast<int>((0 - current_minX) / (current_maxX - current_minX) * width);
        int xAxis = static_cast<int>(height - 1 - (0 - minY) / (maxY - minY) * (height - 1));
        yAxis = std::max(0, std::min(width - 1, yAxis));
        xAxis = std::max(0, std::min(height - 1, xAxis));

        // Draw coordinate system
        for (int i = 0; i < height; ++i) {
            mvaddch(i + 1, yAxis + 1, '|');
        }
        for (int j = 0; j < width; ++j) {
            mvaddch(xAxis + 1, j + 1, '-');
        }
        mvaddch(xAxis + 1, yAxis + 1, '+');

        // Plot points
        for (const auto& coord : coordinates) {
            double x = coord.first;
            double y = coord.second;
            if (std::isfinite(y) && x >= current_minX && x <= current_maxX && y >= minY && y <= maxY) {
                int col = static_cast<int>((x - current_minX) / (current_maxX - current_minX) * (width - 1));
                int row = static_cast<int>(height - 1 - (y - minY) / (maxY - minY) * (height - 1));
                if (row >= 0 && row < height && col >= 0 && col < width) {
                    mvaddch(row + 1, col + 1, '*');
                }
            }
        }

        // Display information
        mvprintw(0, 0, "x: [%.2f, %.2f]", current_minX, current_maxX);
        mvprintw(1, 0, "y: [%.2f, %.2f]", minY, maxY);
        mvprintw(max_y - 1, 0, "q:quit up:zoom in down:zoom out left/right:x-pan +/-:y-pan");

        refresh();

        // Handle input
        int ch = getch();
        switch (ch) {
            case 'q':
                running = false;
                break;
            case KEY_LEFT:
                x_center -= x_range * zoom * 0.1;
                break;
            case KEY_RIGHT:
                x_center += x_range * zoom * 0.1;
                break;
            case '+':
                zoom *= 0.8;
                break;
            case '-':
                zoom /= 0.8;
                break;
            case KEY_UP:
                y_center += y_range * zoom * 0.1;
                break;
            case KEY_DOWN:
                y_center -= y_range * zoom * 0.1;
                break;
        }
        zoom = std::max(0.01, std::min(10.0, zoom));
    }

    // Cleanup
    endwin();
}