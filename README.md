# NSExpression

A mathematical expression parser and evaluator built from scratch in C++17, inspired by Swift's `NSExpression` class. Parses and evaluates arithmetic expressions, solves equations, and analyzes functions — all without any external math libraries.


---

## How It Works

Input goes through a classic compiler front-end pipeline:

```
"2 * sin(x) + 1"
       │
       ▼
    Lexer          tokenizes the input string
       │
       ▼
    Parser         recursive descent → builds an Abstract Syntax Tree
       │
       ▼
  AST (Expression) tree of nodes: NumberNode, BinaryOpNode, FunctionNode, ...
       │
       ▼
   Evaluator       walks the tree and computes the result
```

The AST is built entirely from polymorphic `ExpressionNode` subclasses. Each node knows how to evaluate itself given a variable map, so evaluation is a simple recursive tree walk.

---

## Features

**Expression evaluation**
- Arithmetic operators: `+`, `-`, `*`, `/`, `^`, `%`
- Operator precedence and associativity handled by the parser
- Built-in functions: `sin`, `cos`, `tan`, `log`, `sqrt`, `abs`, `min`, `max`
- Named variables and built-in constants (`pi`, `e`)

**Equation solving**
- Linear equations solved analytically via coefficient extraction
- Nonlinear equations solved numerically with Newton's method
- Complex roots via companion matrix / iterative methods
- Example: `x^2 + 1 = 0` → `i`, `-i`

**Function analysis** (`analyze` command)
- Domain and range
- Even / odd / neither symmetry
- X and Y intercepts (with exact π notation for trig functions)

**Terminal plotting** (`plot` command)
- ASCII waveform rendered in the terminal via ncurses
- Export to file with `plot <expr> to <filename>`

---

## Usage

```
> 2 + 3 * sin(pi)
Result: 2.00

> x^2 - 4 = 0
Solutions for x: 2.00, -2.00

> x^2 + 1 = 0
Solutions for x: 0.00+1.00i, 0.00-1.00i

> analyze sin(x)
Domain: (-inf, inf)
Range: [-1.00, 1.00]
Symmetry: Odd
Y-intercept: (0.00, 0.00)
X-intercepts: (0, 0) (pi, 0) (-pi, 0) (and multiples of pi)

> plot sin(x)
[ncurses waveform displayed in terminal]

> quit
```

---

## Build

**Dependencies**

| Dependency | Purpose |
|---|---|
| CMake 3.10+ | Build system |
| ncurses | Terminal plotting |

**macOS**
```bash
brew install ncurses
```

**Linux**
```bash
sudo apt install libncurses-dev
```

**Compile**
```bash
git clone https://github.com/anngalstyan/NSExpression-in-Cpp.git
cd NSExpression-in-Cpp
mkdir build && cd build
cmake ..
make
./NSExpression_CPP
```

---

## Project Structure

```
include/
  Token.h             Token types and definitions
  Lexer.h             Tokenizer interface
  Parser.h            Recursive descent parser interface
  Expression.h        AST node class hierarchy
  Evaluator.h         Tree evaluation interface
  Complex.h           Complex number type
  FunctionAnalyzer.h  Function analysis and plotting
  Utilities.h         Helper functions
src/
  Lexer.cpp           Tokenization logic
  Parser.cpp          AST construction with precedence climbing
  Expression.cpp      Node evaluation implementations
  FunctionAnalyzer.cpp Domain, range, intercepts, ncurses plotting
  Evaluator.cpp       Evaluator entry point
  main.cpp            Interactive REPL
```

---

## What I'd Improve

- Differentiation — the AST structure already supports it, each node type has a clear derivative rule
- Multi-variable expressions and partial derivatives
- Better error messages pointing to the exact position in the input where parsing failed
- Unit tests for the parser and evaluator covering edge cases

---

## License

MIT
