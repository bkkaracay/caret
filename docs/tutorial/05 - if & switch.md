[<- back](04%20-%20Arrays.md) | [next ->](06%20-%20while%20&%20for.md)

# Control Flows

## if-else
```go
if my_var < 12 {
    
} else if my_var == 12 {

} else {

}

```

Braces (`{}`) are **mandatory** for all `if` statements, even for single-line blocks.

```go
if my_var < 12 // Compile-time error: Braces are required
    io.println("Illegal!")

```

caret enforces a strict coding style: `else` and `else if` keywords **must** be on the same line as the preceding closing brace `}`.

```go
if my_var < 12 {

}
else if my_var == 12 { // Compile-time error: 'else' must be on the same line as '}'

}
else { // Compile-time error

}

```

## switch-case

Unlike traditional `switch` statements (e.g., in C), caret's switch **does not fall through** and does not require a `break` keyword.

```go
switch my_str {
    case "Hello":
        io.println("World!")
    case "hello":
        io.println("world!")
    default:
        io.println("what?")
}

```

Multiple values can be grouped in a single `case` by separating them with commas.

```go
switch my_str {
    case "hello", "world":
        io.println("~ok~")
    default:
        io.println("what?")
}

```