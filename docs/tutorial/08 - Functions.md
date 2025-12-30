[<- back](07%20-%20struct%20%26%20enum.md) | [next ->](09%20-%20Error%20Handling.md)

# Functions

## Function Definition
Functions are defined with a return type, a name, and a parameter list.

```go
string my_func(string name, string surname) {
    return name + surname
}
```

Caret **does not support** function overloading. Every function in a scope must have a unique name.

```go
string my_func(string name, string surname) {
    return name + surname
}

uint32 my_func(uint32 a, uint32 b) { // Compile-time error: Function name already exists
    return a + b
}
```

Global functions do not require forward declaration; they can be used before they are defined in the source file.

```go
int32 main(string args[]^) {
    if (lengthof(args) != 2) {
        return 1
    }
    
    // Using a function defined below
    my_func(args^[0], args^[1])
}

string my_func(string name, string surname) {
    return name + surname
}
```

## Multiple Value Return

Caret supports returning multiple values from a single function. This is useful for returning a result along with an error code or status.

```go
// Returns a pair of uint32
(uint32, uint32) sum_multiply(uint32 a, uint32 b) {
    return a + b, a * b
}

int32 main(string args[]^) {
    uint32 sum

    // Destructuring assignment
    sum, uint32 mult = sum_multiply(12, 34)
}
```