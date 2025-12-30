[<- back](03%20-%20Operators.md) | [next ->](05%20-%20if%20&%20switch.md)

# Arrays

## Stack Arrays
Stack arrays have a fixed size that **must be known at compile-time**. Arrays with different sizes are considered **distinct types**. Stack arrays do not store any size information at runtime.

```go
uint32 stack[10][10]
```

## Heap Arrays
Heap arrays store both **length** and **capacity** information at runtime. They function essentially as dynamic lists (like `ArrayList` or `Vector` in other languages). They can be initialized with a specific length and capacity, and their elements are zero-initialized (0 or NULL) by default.

Unlike stack arrays, all heap arrays of a certain element type belong to the **same type**, regardless of their size.

```go
void take_heap_arr(uint32 heap[;]) {
    // do something
}

int32 main(string args[]^) {
    // [Length; Capacity]
    uint32 heap0[12; 20] // Length: 12, Capacity: 20
    uint32 heap1[;] // Length: 0, Capacity: 0 (empty)

    take_heap_arr(heap0)
    take_heap_arr(heap1)
}
```

Heap arrays can be expanded using the `append` function. `append` adds new elements starting from the end of the current length.

```go
uint32 heap[12; 20]

for uint32 i in 0..30 {
    heap = append(heap, i) // Appends indices 12 to 42
}
```

**Note:** Even if the capacity is sufficient, it is illegal to access an index beyond the current `length`.

```go
uint32 heap[12; 20]

heap[16] //Runtime error
```

## Slices
A slice is fundamentally a reference to an array that also stores the length of that array. This allows for safe usage of stack arrays within functions without knowing their size at compile-time.

```go
void take_slice(uint32 arr[]^) {
    // Size is known via the slice metadata
}

int32 main(string args[]^) {
    uint32 stack[30]
    uint32 heap[12; 30]

    // Create a slice from a stack array
    uint32 ref[]^ = &stack
    take_slice(ref)

    // Create a slice from a heap array
    ref = &heap
    take_slice(ref)
}
```

Slices must be explicitly **dereferenced** (^) when accessing elements.

```go
uint32 stack[30]
uint32 ref[]^ = &stack

ref^[12]
```

**Important:** Slices are **not expandable**. The `append` function cannot be used on slices, even if the slice refers to a heap array. Functions that need to expand an array must take the heap array itself, not a slice.

```go
void try_expand(uint32 arr[]^) {
    arr = append(arr, 1) //Compile-time error
}

int32 main(string args[]^) {
    uint32 heap[12;]
    uint32 ref[]^ = &stack

    try_expand(ref)
}
```

## The `type name[]` Notation

It is illegal to define a stack array with empty brackets. The size must always be specified.

```go
uint32 illegal[] //Compile-time err
```