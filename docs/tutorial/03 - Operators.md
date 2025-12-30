[<- back](02%20-%20Data%20Types.md) | [next ->](04%20-%20Arrays.md)

# Operators

## Referance Operator
As its name suggests, caret uses the caret symbol (`^`) for references (pointers). This design choice resolves the operator precedence ambiguities often associated with the asterisk (`*`) in C-like languages.

```go
int32 i = 123
int32 ref^ //Definition

ref = &i
ref^ = 12 //Dereferance
```

## Bitwise NOT
Using the caret symbol (^) for pointers solves ambiguity, but it consumes a commonly used symbol. Since standard keyboards have a limited set of symbols, this requires a trade-off.

However, since bool is a distinct type in Caret (not just an integer), we can safely overload the ! symbol.
- For integers, ! acts as Bitwise NOT.
- For booleans, ! acts as Logical NOT.

```go
uint8 h = 0

!h //255
```

```go
bool t = true

!t //false
```

## Bitwise XOR
Since ^ is dedicated to pointers, the tilde symbol (~) is used for Bitwise XOR operations.


## Operator Precedence
| Precedence | Operator | Description | Associativity |
| --- | --- | --- | --- |
| **1**  | `^`            | Indirection (dereference) | |
|        | `[]`           | Array subscripting |  |
|        | `->`           | Structure/Union member access (pointer) |  |
|        | `.`            | Structure/Union member access (object) |  |
|        | `++` `--`      | Postfix increment/decrement |  |
|        | `()`           | Function call | Left-to-Right |
| **2**  | `++` `--`      | Prefix increment/decrement | **Right-to-Left** |
|        | `+` `-`        | Unary plus/minus |  |
|        | `!`            | Logical NOT / Bitwise NOT |  |
|        | `(type)`       | Cast |  |
|        | `&`            | Address-of |  |
|        | `lengthof`     | Length of |  |
| **3**  | `*` `/` `%`    | Multiplication, Division, Modulo | Left-to-Right |
| **4**  | `+` `-`        | Addition, Subtraction | Left-to-Right |
| **5**  | `<<` `>>`      | Bitwise left/right shift | Left-to-Right |
| **6**  | `<` `<=`       | Relational less than/less than or equal | Left-to-Right |
|        | `>` `>=`       | Relational greater than/greater than or equal |  |
| **7**  | `==` `!=`      | Relational equal/not equal | Left-to-Right |
| **8**  | `&`            | Bitwise AND | Left-to-Right |
| **9**  | `~`            | Bitwise XOR (exclusive OR) | Left-to-Right |
| **10** | ` \| `         | Bitwise OR (inclusive OR) | Left-to-Right |
| **11** | `&&`           | Logical AND | Left-to-Right |
| **12** | `\|\|`           | Logical OR | Left-to-Right |
| **13** | `=`            | Simple assignment | **Right-to-Left** |
|        | `+=` `-=`      | Assignment by sum/difference |  |
|        | `*=` `/=` `%=` | Assignment by product/quotient/remainder |  |
|        | `<<=` `>>=`    | Assignment by bitwise shift |  |
|        | `&=` `~=` `\|=` | Assignment by bitwise AND/XOR/OR |
