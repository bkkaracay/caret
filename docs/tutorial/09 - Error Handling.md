[<- back](08%20-%20Functions.md) | [next ->](10%20-%20defer.md)

# Error Handling

Caret adopts a simple, pragmatic approach to error handling. Instead of complex exception systems (like `try-catch`), it refines the traditional C-style error codes by adding structure and type safety.

## The `error` type
The `error` keyword is used to group related error codes. It acts as a **namespace** for grouping constants rather than defining a complex new data structure.

```go
// Uppercase is recommended for group names
error HTTP.{
    NOT_FOUND,
    INTERNAL_ERROR,
}

error my_err = HTTP.NOT_FOUND
```

## Handling Errors

Functions return an `error` type to indicate status.

* **Success:** The function returns `null`.
* **Failure:** The function returns a specific error code.

You handle errors by checking the return value against `null` (usually with an `if` statement).

```go
error AGE_ERR.{
    NEGATIVE,
    TOO_MUCH
}

error is_valid_age(int32 age) {
    if age <= 0 {
        return AGE_ERR.NEGATIVE
    }

    if age >= 140 {
        return AGE_ERR.TOO_MUCH
    }

    return null // Success (No error)
}

int32 main(string args[]^) {
    error e = is_valid_age(20)
    
    // Check if an error occurred
    if e != null {
        switch(e) {
            case AGE_ERR.NEGATIVE:
                return 200
            case AGE_ERR.TOO_MUCH: 
                return 300
        }

        return 1 // Generic error
    }

    return 0 // Success
}
```