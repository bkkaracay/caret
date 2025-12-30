[<- back](10%20-%20defer.md)

# Modules

Caret utilizes a simple, directory-based module system. **Every folder is a module**, and the folder name serves as the namespace. Short, lowercase names are recommended for modules.


```
project/
├── say/
│   └── hello.caret
└── main.caret
```


*say/hello.caret:*
```go
#import <io>

public void hello() {
    io.println("hello")
}
```

*main.caret:*

```go
#import "say"

int32 main(string args[]^) {
    // Accessing function via namespace
    say.hello()
}
```

## Visibility (`public`)

By default, all definitions (functions, structs, etc.) are **private**. Only items marked with `public` can be imported and accessed from outside the module.

However, files within the **same module (folder)** can access all definitions (even private ones) of their sibling files.

```
project/
├── say/
│   ├── hello_world.caret
│   └── world.caret
│
└── main.caret

```

*say/world.caret:*

```go
#import <io>

// Private function (no 'public' keyword)
void world() {
    io.println("world")
}
```

*say/hello_world.caret:*

```go
#import <io>

public void hello_world() {
    io.println("hello ")
    world() // Accessible here (same module)!
}
```

*main.caret:*

```go
#import "say"

int32 main(string args[]^) {
    say.hello_world()
    // say.world() // Compile-time error: 'world' is private
}
```

### Structs & Encapsulation

Marking a `struct` as `public` exports the type itself, but its fields remain **private** by default. This allows for encapsulation (hiding implementation details).

```
project/
├── edu/
│   ├── student.caret
│   └── helper.caret
│
└── main.caret
```

*edu/student.caret:*

```go
public struct Student {
    uint32 id,
    string name,
}
```

*edu/helper.caret:*

```go
#import <io>

public Student new_student(uint32 id, string name) {
    Student st
    // Fields are accessible here because we are in the same module 'edu'
    st.id = id 
    st.name = name 
    
    return st
}

public void print_student(Student st) {
    io.println(st.name)
}
```

*main.caret:*

```go
#import "edu"

int32 main(string args[]^) {
    edu.Student st0 
    // st0.id   // Unreachable (private field)
    // st0.name // Unreachable (private field)

    st0 = edu.new_student(12, "Jane")
    edu.print_student(st0)
}
```

### Public Struct Fields

To expose specific fields to other modules, you must explicitly mark them as `public`.

```
project/
├── edu/
│   ├── student.caret
│   └── helper.caret
│
└── main.caret
```

*edu/student.caret:*

```go
public struct Student {
    uint32 id,         // Private
    public string name, // Public
}
```

*edu/helper.caret:*

```go
#import <io>

public Student new_student(uint32 id, string name) {
    Student st
    st.id = id
    st.name = name
    
    return st
}
```

*main.caret:*

```go
#import <io>
#import "edu"

int32 main(string args[]^) {
    edu.Student st0
    // st0.id // Still unreachable

    st0 = edu.new_student(12, "Jane")
    io.println(st0.name) // Reachable now!
}
```

### Other Types

The `public` keyword is not limited to structs and functions. It applies to `enums`, `errors`, and other top-level declarations.