[<- back](06%20-%20while%20%26%20for.md) | [next ->](08%20-%20Functions.md)

# User Defined Types

## struct
Structs are used to group related data fields. Trailing commas are allowed in definitions to make multi-line editing easier.

```go
struct Student {
    uint32 id,
    string name, // Trailing comma allowed
}
```

Unlike C, you do not need to repeat the `struct` keyword when defining an instance.

```go
// Field order does not matter during initialization
Student st0 = {
    id = 12,
    name = "Red"
}

Student st1 = {
    name = "John",
    id = 42,
}
```

You can also assign fields individually after declaration.

```go
Student s2
s2.name = "Patrick"
s2.id = 21
```

## enum

Enums define a set of named constants.

```go
enum MyType {
    INT8,
    UINT8,
}
```

Enum members are **scoped** to the enum name (namespaced). You must access them using the `EnumName.Member` syntax.

```go
MyType t0 = MyType.INT8
```

Enum values can be explicitly cast to `string` for debugging purposes.

```go
io.println((string) MyType.INT8) // Prints: "MyType.INT8"
```