[<- back](01%20-%20Hello,%20World!.md) | [next ->](03%20-%20Operators.md)

# Data Types

Caret provides a set of primitive data types to handle numbers, text, and logic.

| Category | Types | Size |
| :--- | :--- | :--- |
| **Integers** | `int8`, `int16`, `int32`, `int64` | 1-8 bytes |
| **Unsigned** | `uint8`, `uint16`, `uint32`, `uint64` | 1-8 bytes |
| **Floating** | `float32`, `float64` | 4-8 bytes |
| **Boolean** | `bool` | 1 byte |
| **Text** | `rune`, `string` | 4 bytes / Dynamic |

## Variable Definition

```go
int8 hello = 2, world = 4
```

## Strict Typing
Caret is strict about types. It does not support implicit type casting. You must convert types manually.

This code is invalid in Caret:
```go
int8 hello = 1
int16 world = hello // Compile-time error
```

You should do this instead:
```go
int8 hello = 1
int16 world = (int16) hello // Success
```

## Literals
In Caret, number literals (like 1 or 2.5) do not have a fixed type. Their type is determined by the context.

```go
int16 hello = 1
int16 world = hello + 2 // The literal '2' acts as an int16 here
```

If the context doesn't match, you get an error:
```go
int8 hello = 1
int16 world = hello + 2 // Compile-time error
```

However, float literals are different. They must contain a dot.

```go
float32 f = 2.0 //Success
```

```go
float32 f = 2 //Compile-time error. You cannot assign an integer literal to a float.
```

## Runes
A rune represents a single Unicode character. It takes 4 bytes of memory.

## Strings
Strings are immutable and always UTF-8 encoded. Note: Accessing a string by index returns a byte (uint8), not a character (rune).

```go
// { 197, 159, 97, 112, 107, 97 }
string str = "şapka"

str[1] // Returns 159 (part of 'ş')
```

```go
str[1] = 12 //Compile-time error: Compile-time error: Strings cannot be modified
```

## String Interpolation

```go
int8 i = 10
string s0 = "i = {i}" //i = 10

string s1 = "i = \{i\}" //i = {i}
```
```go
string s2 = "i = \{i}" //Compile-time error
string s3 = "i = {i\}" //Compile-time error
```