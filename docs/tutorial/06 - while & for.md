[<- back](05%20-%20if%20%26%20switch.md) | [next ->](07%20-%20struct%20&%20enum.md)

## while
```go
while my_bool {

}
```

## for

```go
for int16 i in -5..10 { //-5 included 10 excluded
    if i < 0 {
        continue
    }
    
    if i == 5 {
        break
    }

    io.println("{i}")
}

for uint8 byte in "Hello, World!" {

}

for uint32 k in my_arr {

}
```