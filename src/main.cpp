#include "Evaluator.h"
#include "Lexer.h"
#include "Parser.h"
#include "FunctionAnalyzer.h"
#include <iostream>
#include <string>
#include <unordered_map>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <iomanip>

int main() {
    std::unordered_map<std::string, double> variables;
    variables["pi"] = 3.14159;
    variables["e"] = 2.71828;

    std::cout << "Enter an expression, equation, or 'quit' to exit.\n";
    std::cout << "Use 'analyze' to analyze a function or 'plot' to graph it.\n";

    while (true) {
        std::cout << "> ";
        std::string input;
        std::getline(std::cin, input);
        if (std::cin.eof() || std::cin.fail()) {
            std::cout << "\nInput stream closed. Exiting.\n";
            break;
        }
        if (input == "quit") {
            break;
        }

        try {
            auto trim = [](std::string& s) {
                s.erase(0, s.find_first_not_of(" \t"));
                s.erase(s.find_last_not_of(" \t") + 1);
                return s;
            };
            std::string trimmed_input = trim(input);

            if (trimmed_input.empty()) {
                throw std::runtime_error("Empty input");
            }

            bool isAnalyze = false, isPlot = false;
            std::string expression;
            std::string filename;

            auto starts_with = [](const std::string& str, const std::string& prefix) {
                if (str.length() < prefix.length()) return false;
                for (size_t i = 0; i < prefix.length(); ++i) {
                    if (std::tolower(str[i]) != std::tolower(prefix[i])) return false;
                }
                return true;
            };

            if (starts_with(trimmed_input, "analyze ") && trimmed_input.length() > 7) {
                isAnalyze = true;
                expression = trimmed_input.substr(7);
                trim(expression);
            } else if (starts_with(trimmed_input, "plot ") && trimmed_input.length() > 5) {
                isPlot = true;
                expression = trimmed_input.substr(5);
                trim(expression);
                size_t to_pos = expression.find(" to ");
                if (to_pos != std::string::npos) {
                    filename = expression.substr(to_pos + 4);
                    trim(filename);
                    expression = expression.substr(0, to_pos);
                    trim(expression);
                }
            } else if (trimmed_input == "analyze" || trimmed_input == "plot") {
                throw std::runtime_error("Command '" + trimmed_input + "' requires an expression");
            } else {
                expression = trimmed_input;
            }

            if (expression.empty()) {
                throw std::runtime_error("No expression provided after command");
            }

            Lexer lexer(expression);
            auto tokens = lexer.tokenize();
            Parser parser(tokens, variables);
            auto expr = parser.parse();

            if (isAnalyze) {
                FunctionAnalyzer analyzer(expr);
                try {
                    std::cout << "Computing domain...\n";
                    auto domains = analyzer.getDomain();
                    std::cout << "Domain: ";
                    if (domains.empty()) {
                        std::cout << "none";
                    } else {
                        for (size_t i = 0; i < domains.size(); ++i) {
                            std::cout << "(";
                            if (std::isinf(-domains[i].first)) {
                                std::cout << "-inf";
                            } else if (std::abs(domains[i].first) <= 1e-6) {
                                std::cout << "0";
                            } else {
                                std::cout << std::fixed << std::setprecision(2) << domains[i].first;
                            }
                            std::cout << ", ";
                            if (std::isinf(domains[i].second)) {
                                std::cout << "inf";
                            } else if (std::abs(domains[i].second) <= 1e-6) {
                                std::cout << "0";
                            } else {
                                std::cout << std::fixed << std::setprecision(2) << domains[i].second;
                            }
                            std::cout << ")";
                            if (i < domains.size() - 1) {
                                std::cout << " U ";
                            }
                        }
                    }
                    std::cout << "\n";

                    std::cout << "Computing range...\n";
                    auto range = analyzer.getRange();
                    std::cout << "Range: [";
                    if (std::isinf(-range.first)) {
                        std::cout << "-inf";
                    } else {
                        std::cout << std::fixed << std::setprecision(2) << range.first;
                    }
                    std::cout << ", ";
                    if (std::isinf(range.second)) {
                        std::cout << "inf";
                    } else {
                        std::cout << std::fixed << std::setprecision(2) << range.second;
                    }
                    std::cout << "]\n";

                    std::cout << "Computing symmetry...\n";
                    std::cout << "Symmetry: ";
                    if (analyzer.isEven()) {
                        std::cout << "Even (symmetric about y-axis)";
                    } else if (analyzer.isOdd()) {
                        std::cout << "Odd";
                    } else {
                        std::cout << "Neither" << (analyzer.isSymmetric() ? " (symmetric about y-axis)" : "");
                    }
                    std::cout << "\n";

                    std::cout << "Computing y-intercept...\n";
                    auto yIntercept = analyzer.getYAxisIntercept();
                    std::cout << "Y-intercept: ";
                    if (std::isnan(yIntercept.second)) {
                        std::cout << "undefined";
                    } else {
                        std::cout << "(" << std::fixed << std::setprecision(2) << yIntercept.first << ", " 
                                  << std::fixed << std::setprecision(2) << yIntercept.second << ")";
                    }
                    std::cout << "\n";

                    std::cout << "Computing x-intercepts...\n";
                    auto xIntercepts = analyzer.getXAxisIntercepts();
                    std::cout << "X-intercepts: ";
                    if (xIntercepts.empty()) {
                        std::cout << "none";
                    } else {
                        bool isSin = std::dynamic_pointer_cast<FunctionNode>(expr) &&
                                     std::dynamic_pointer_cast<FunctionNode>(expr)->getName() == "sin";
                        bool isCos = std::dynamic_pointer_cast<FunctionNode>(expr) &&
                                     std::dynamic_pointer_cast<FunctionNode>(expr)->getName() == "cos";
                        bool isTan = std::dynamic_pointer_cast<FunctionNode>(expr) &&
                                     std::dynamic_pointer_cast<FunctionNode>(expr)->getName() == "tan";
                        for (const auto& intercept : xIntercepts) {
                            double x = intercept.first;
                            if (isSin || isTan) {
                                double k = x / 3.141592653589793;
                                int k_int = std::round(k);
                                if (std::abs(k - k_int) < 1e-6) {
                                    if (k_int == 0) {
                                        std::cout << "(0, 0) ";
                                    } else if (k_int == 1) {
                                        std::cout << "(pi, 0) ";
                                    } else if (k_int == -1) {
                                        std::cout << "(-pi, 0) ";
                                    } else {
                                        std::cout << "(" << k_int << "pi, 0) ";
                                    }
                                } else {
                                    std::cout << "(" << std::fixed << std::setprecision(2) << x << ", 0) ";
                                }
                            } else if (isCos) {
                                double k = (x - 3.141592653589793 / 2) / 3.141592653589793;
                                int k_int = std::round(k);
                                if (std::abs(k - k_int) < 1e-6) {
                                    if (k_int == 0) {
                                        std::cout << "(pi/2, 0) ";
                                    } else if (k_int == -1) {
                                        std::cout << "(-pi/2, 0) ";
                                    } else {
                                        std::cout << "(" << (k_int * 2 + 1) << "pi/2, 0) ";
                                    }
                                } else {
                                    std::cout << "(" << std::fixed << std::setprecision(2) << x << ", 0) ";
                                }
                            } else {
                                std::cout << "(" << std::fixed << std::setprecision(2) << x << ", " 
                                          << std::fixed << std::setprecision(2) << intercept.second << ") ";
                            }
                        }
                        if (isSin || isCos || isTan) {
                            std::cout << "(and multiples of pi)";
                        }
                    }
                    std::cout << "\n";
                } catch (const std::exception& e) {
                    std::cout << "Error during analysis: " << e.what() << "\n";
                }
            } else if (isPlot) {
                FunctionAnalyzer analyzer(expr);
                analyzer.plotNcurses(filename);
                // Clear any leftover input after ncurses
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue; // Go to next prompt
            } else if (expr->isEquation()) {
                auto equation = std::dynamic_pointer_cast<EquationNode>(expr);
                auto complexSolutions = equation->solveComplex("x");
                if (complexSolutions.empty()) {
                    std::cout << "No solutions found\n";
                } else {
                    std::cout << "Solutions for x: ";
                    for (size_t i = 0; i < complexSolutions.size(); ++i) {
                        std::cout << complexSolutions[i].toString();
                        if (i < complexSolutions.size() - 1) std::cout << ", ";
                    }
                    std::cout << "\n";
                }
            } else {
                Evaluator evaluator;
                double result = evaluator.evaluate(expr, variables);
                std::cout << "Result: " << std::fixed << std::setprecision(2) << result << "\n";
            }
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << "\n";
        }
    }

    return 0;
}