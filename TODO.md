# summary

- Add casting operator (as)
- Add interfaces
- Add generics
- Add containers Map, Set, Vec, Deque, List, Stack, Queue, Heap
- Add io
- Add sockets
- Add threads
- Add macros

# define type casting

    var x = 1 as Float

# type casting for interfaces too
internally use two pointers, one for the object and the other for the vtable of the interface.

    var c = Circle{radius=2.0} as Shape

# define how to move from stack to heap and back

The memory used by stack allocated variables is freed automatically
when leaving the scope where the variable was declared. For cases when
it is useful to keep a variable alive beyond the scope where it was
declared, it is possible to use heap allocated variables instead.

Declaring heap allocated variables in Mamba is very simple.

    var p = *Point{x = 2.0, y = 2.0}

Now if you want a second reference to a variable in the heap, you can
simple declare another variable.

    var q = p

The variables p and q now point to the same heap allocated Point record.
Mamba will use reference counting to keep track of the number of
references to any heap allocated variable, and will free up the memory
when there are no references to it. Reference counting instructions are
inserted during compile time, which incurs in very little overhead and
allows Mamba to operate without a garbage collector. To deal with
circular references its possible to use weak reference counting
(described elsewhere).


When you

    var u = q.copyToHeap()
    var v = p.copyToStack()

Both of these operations create a copy of the type that leaves in a
different part of memory. For small objects this does not incur in
significant overhead, but if your type holds (for example) a 3MB byte
array that stores an image, then the overhead will not be negligible.
It is for this reason that instead of providing a shorthand syntax,
Mamba makes these operations intentionally verbose.

A common programming error in languages such as C/C++ is to store a
pointer to something that was allocated in the stack, which usually
results in a dangling pointer. This is not possible in Mamba, since the
compiler will not allow you to store a pointer to a stack allocated
variable.

# add support for tuple unpacking

    var (x, y) = tupleReturningFunc()
