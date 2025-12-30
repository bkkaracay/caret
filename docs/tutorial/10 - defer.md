[<- back](09%20-%20Error%20Handling.md) | [next ->](11%20-%20Modules.md)

## defer
```go
int32 main(string args[]^) {
    defer io.println("1 ")
    defer io.println("2 ")
    io.println("3 ")
    io.println("4 ")
}
```
*Output:*
```
3 4 2 1
```