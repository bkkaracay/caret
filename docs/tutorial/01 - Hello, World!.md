[<- back](00%20-%20Contents.md) | [next ->](02%20-%20Data%20Types.md)

# Hello, World!
## Semicolons
Unlike C and C-like languages caret doesn't need and allow semicolons at end. Any statement that ended with semicolon must cause a syntax error.

```go
#import <io>

int32 main(string args[]^) {
    io.println("Hello, World!"); //Compile-time error
}
```

```go
#import <io>

int32 main(string args[]^) {
    io.println("Hello, World!") //Success!
}
```

## Comments

```c
/*Multiline
comment
*/

//Single line comment
```