# Script Module

The script module provides a lightweight, embedded scripting system for the Unigine Legacy engine. It consists of two classes:

- **`Defines`** — a simple preprocessor symbol table (`#define`/`#ifdef` support)
- **`Parser`** — an expression evaluator, scripting interpreter, and file-based block parser

No third-party dependencies — the entire module is hand-written C++ using only the C standard library and C++ STL.

---

## Defines

A symbol table analogous to C/C++ `#define` directives. Used by the file-based `Parser` to handle `#ifdef`/`#ifndef`/`#else`/`#endif` preprocessing in script files.

```cpp
Defines defs;
defs.define("EDITOR");          // define a symbol
defs.isDefine("EDITOR");        // returns 1 (true)
defs.undef("EDITOR");           // remove the symbol
defs.isDefine("EDITOR");        // returns 0 (false)
```

- `define()` is idempotent — defining an existing symbol is a no-op.
- `undef()` is safe to call on non-existent symbols.
- Symbol matching is case-sensitive (`"FOO" != "foo"`).

---

## Parser

The `Parser` class does three things:

1. **File parsing** — reads a file, strips comments, processes `#ifdef`/`#ifndef`/`#else`/`#endif` directives, and extracts named `<block>` sections.
2. **Expression evaluation** — a numeric expression parser handling arithmetic, comparisons, logic, math functions, and variables.
3. **Script interpretation** — processes strings with control flow (`if`/`else`, `for`), variable assignment, and inline expression substitution, producing an output string.

---

### Expression Evaluation

`Parser::expression(str, variable, value)` evaluates a numeric expression and returns a `float`.

```cpp
Parser::expression("2 + 3");                          // 5.0
Parser::expression("10 / 3");                         // 3.333...
Parser::expression("(2 + 3) * 4");                    // 20.0
Parser::expression("-5");                             // -5.0
Parser::expression("10 % 3");                         // 1.0
Parser::expression("2 + 3 * 4 - 5 / (1 + 4)");        // 13.0
Parser::expression("((2 + 3) * (4 - 1)) / 5");        // 3.0
```

#### Operators (highest to lowest precedence)

| Precedence | Operators | Description |
|------------|-----------|-------------|
| 4 (unary) | `s S c C t T l e q f r` | `sin`, `asin`, `cos`, `acos`, `tan`, `atan`, `log`, `exp`, `sqrt`, `fabs`, `rand` |
| 3 | `* / %` | Multiply, divide, modulo |
| 2 | `+ -` | Add, subtract (and unary minus) |
| 1 | `< > = ! & \|` | Compare, equality, logical AND, logical OR |

```cpp
Parser::expression("3 < 5");                          // 1.0
Parser::expression("3 == 3");                         // 1.0
Parser::expression("3 != 3");                         // 0.0
Parser::expression("1 && 1");                         // 1.0
Parser::expression("1 || 0");                         // 1.0
```

#### Math Functions

All take a single argument in parentheses:

```cpp
Parser::expression("sin(0)");                         // 0.0
Parser::expression("cos(0)");                         // 1.0
Parser::expression("sqrt(9)");                        // 3.0
Parser::expression("fabs(-5)");                       // 5.0
Parser::expression("exp(0)");                         // 1.0
Parser::expression("log(1)");                         // 0.0
Parser::expression("asin(0)");                        // 0.0
Parser::expression("acos(1)");                        // 0.0
Parser::expression("tan(0)");                         // 0.0
Parser::expression("atan(0)");                        // 0.0
Parser::expression("rand(100)");                      // random in [0, 100]
```

#### Named Variable Substitution

A named variable in the expression text can be replaced with a numeric value:

```cpp
Parser::expression("time + 1", "time", 5.0);          // 6.0
Parser::expression("sin(time * 2) * 5", "time", 3.0); // ~-4.56
```

#### Dollar-Script Variables

When no named variable is given, `$` followed by a letter references `$a`–`$z` from the script interpreter's variable table:

```cpp
Parser::variables[0] = 10.0;  // $a
Parser::variables[1] = 20.0;  // $b
Parser::expression("$a + $b");                        // 30.0
```

---

### Script Interpretation

`Parser::interpret(src)` processes a script string and returns a newly allocated `char*` containing the output. The caller must `delete[]` the result.

#### Text Passthrough

Most text is copied verbatim to the output:

```cpp
Parser::interpret("hello world");                     // "hello world"
```

#### Expression Substitution: `$(expression)`

Inline expressions are evaluated and their result inserted as text:

```cpp
Parser::interpret("result = $(2+3)");                 // "result = 5"
Parser::interpret("$((1+2)*(3+4))");                  // "21"
```

#### Variable Output: `$letter`

Outputs the current value of a script variable:

```cpp
// after $b = 42
Parser::interpret("value is $b");                     // "value is 42"
```

#### Variable Assignment

| Operator | Description | Example |
|----------|-------------|---------|
| `=` | Direct assignment | `$a=10` |
| `+=` | Compound add | `$a+=5` |
| `-=` | Compound subtract | `$a-=3` |
| `++` | Postfix increment | `$a++` |
| `--` | Postfix decrement | `$a--` |

Assignment statements produce no output:

```cpp
Parser::interpret("$b=42");                           // ""
Parser::interpret("$b=42 $b");                        // " 42"
Parser::interpret("$c=5 $c++ $c");                    // " 6"
Parser::interpret("$d=5 $d-- $d");                    // " 4"
Parser::interpret("$e=10 $e+=5 $e");                  // " 15"
Parser::interpret("$f=10 $f-=3 $f");                  // " 7"
```

#### Control Flow: `if`/`else`

Condition is a numeric expression; non-zero is truthy. Braces are mandatory.

```cpp
Parser::interpret("if(1){yes}");                      // "yes"
Parser::interpret("if(0){yes}");                      // ""
Parser::interpret("if(1){yes}else{no}");              // "yes"
Parser::interpret("if(0){yes}else{no}");              // "no"
```

#### Loops: `for`

`for(init; condition; increment) { body }` — all three clauses use script variable syntax.

```cpp
Parser::interpret("for($a=0;$a<3;$a++){x}");          // "xxx"
Parser::interpret("for($a=0;$a<3;$a++){$a}");         // "012"
```

#### Nesting

Control flow composes arbitrarily:

```cpp
Parser::interpret("for($g=0;$g<5;$g++){if($g<3){a}}"); // "aaa"
```

---

### Important Notes

- **Single-letter variables only** — `$a` through `$z` (26 variables max). No user-defined variable names.
- **Static variable table** — `Parser::variables[26]` is a `static float` array. Values persist across `interpret()` and `expression()` calls and are shared by all users.
- **Memory management** — `interpret()` allocates a 2 MB output buffer with `new char[]`. The caller must `delete[]` the returned pointer.
- **Thread safety** — not thread-safe due to shared static state.
- **Error handling** — throws `EngineException` on parse errors (unbalanced brackets, unknown tokens, etc.).
- **No string variables** — only numeric values can be stored. Strings are literal passthrough text.

---

### File-Based Block Parser

Construct a `Parser` with a filename and `Defines` reference to read structured files:

```cpp
Defines defs;
defs.define("EDITOR");
Parser parser("level.map", defs);
char* data = parser.get("blocks");
```

The file parser:
- Strips `//` line comments and `/* */` block comments.
- Processes `#ifdef`/`#ifndef`/`#else`/`#endif` directives (up to 32 nesting levels).
- Extracts named blocks delimited by `<blockname>` markers.
- The block content can then be passed through `interpret()` to resolve inline expressions.

This is used across the engine for `.map` (map files), `.mat` (material files), `.shader` (shader files), and `.rag` (ragdoll configuration) files.
