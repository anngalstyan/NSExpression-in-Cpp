#ifndef COMPLEX_H
#define COMPLEX_H

#include <cmath>
#include <string>
#include <sstream>
#include <iomanip>

class Complex {
private:
    double real;
    double imag;

public:
    Complex(double r = 0.0, double i = 0.0) : real(r), imag(i) {}

    double getReal() const { return real; }
    double getImag() const { return imag; }

    Complex operator+(const Complex& other) const {
        return Complex(real + other.real, imag + other.imag);
    }

    Complex operator-(const Complex& other) const {
        return Complex(real - other.real, imag - other.imag);
    }

    Complex operator*(const Complex& other) const {
        return Complex(real * other.real - imag * other.imag,
                      real * other.imag + imag * other.real);
    }

    Complex operator/(const Complex& other) const {
        double denom = other.real * other.real + other.imag * other.imag;
        if (denom == 0) throw std::runtime_error("Division by zero");
        return Complex((real * other.real + imag * other.imag) / denom,
                      (imag * other.real - real * other.imag) / denom);
    }

    bool operator==(const Complex& other) const {
        return std::abs(real - other.real) < 1e-10 && std::abs(imag - other.imag) < 1e-10;
    }

    double magnitude() const {
        return std::sqrt(real * real + imag * imag);
    }

    std::string toString() const {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2);
        if (std::abs(imag) < 1e-10) {
            ss << real;
        } else if (std::abs(real) < 1e-10) {
            ss << imag << "i";
        } else {
            ss << real << (imag >= 0 ? " + " : " - ") << std::abs(imag) << "i";
        }
        return ss.str();
    }
};

#endif 
