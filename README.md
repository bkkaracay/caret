# caret
caret is a statically typed, general-purpose programming language. It aims to provide a modern, strict alternative to C while retaining its familiar syntax and minimalist philosophy. It utilizes a garbage-collected memory model for the time being.

### WIP: Implementation is in very early stage.

## Proposed Syntax
```cpp
#import <io>

void swap(int32 a^, int32 b^) {
    int32 temp = a^
    a^ = b^
    b^ = temp
}

void print_status(int32 status) {
    switch status {
        case 200, 201:
            io.println("Success!")
        case 404:
            io.println("Not found.")
        default:
            io.println("Unknown error.")
    }
}

int32 main(string args[]^) {
    defer print_status(200)

    int32 x = 5, y = 10
    swap(&x, &y)
}
```

## Documentation

### [Architecture](ARCHITECTURE.md)

Take a look at the implementation details and pipeline.

## Implementation Status

### The Pipeline
![Pipeline](docs/img/pipeline.png)

### Features

- [ ] Comments
- [ ] Data Types
    - [x] integers
    - [x] unsigned
    - [x] floating
    - [x] bool
    - [x] rune
    - [ ] String
- [ ] Operators
    - [x] +, -, *, /, %
    - [ ] Others
- [ ] Arrays
- [ ] Control Flows
    - [x] if
    - [ ] else if/else
    - [ ] switch-case
    - [ ] while
    - [ ] for
- [ ] User Defined Types
    - [ ] struct
    - [ ] enum
- [ ] Functions
- [ ] error
- [ ] defer
- [ ] Module System
