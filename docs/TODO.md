# summary

- Add casting operator (as)
- Add interfaces
- Add generics
- Add containers Map, Set, Vec, Deque, List, Stack, Queue, Heap
- Add io
- Add sockets
- Add threads
- Add macros

# define c compatbility

To call C from mamba we provide c_str for strings, and perahps c_struct
for structs?

To call mamba from C we must provide a mamba-dev package which has
wrappers. we need mamba_str

# define type casting
    let x = Float{1}

# type casting for interfaces too
internally use two pointers, one for the object and the other for the vtable of the interface.

    let c = Circle{radius=2.0} as Shape

# define how to move from stack to heap and back

The memory used by stack allocated variables is freed automatically
when leaving the scope where the variable was declared. For cases when
it is useful to keep a variable alive beyond the scope where it was
declared, it is possible to use heap allocated variables instead.

Declaring heap allocated variables in Mamba is very simple.

    let p = *Point{x = 2.0, y = 2.0}

Now if you want a second reference to a variable in the heap, you can
simple declare another variable.

    let q = p

The variables p and q now point to the same heap allocated Point record.
Mamba will use reference counting to keep track of the number of
references to any heap allocated variable, and will free up the memory
when there are no references to it. Reference counting instructions are
inserted at compile time, which incurs in very little overhead and
allows Mamba to operate without a garbage collector. To deal with
circular references its possible to use weak reference counting
(described elsewhere).


When you

    let u = q.copyToHeap()
    let v = p.copyToStack()

Both of these operations create a copy of the type that leaves in a
different part of memory. For small objects this does not incur in
array that stores an image, then the overhead will not be negligible.
It is for this reason that instead of providing a shorthand syntax,
Mamba makes these operations intentionally verbose.

A common programming error in languages such as C/C++ is to store a
pointer to something that was allocated in the stack, which usually
results in a dangling pointer. This is not possible in Mamba, since the
compiler will not allow you to store a pointer to a stack allocated
variable.

# add support for unpacking

    let (x, y) = tupleReturningFunc()

    let [x, rest...] = arrayReturningFunc()

    let {x, y, other...} = objectReturningFunc()

For tuples and objects we can type check to avoid errors, but with
arrays, what happens if the returned array has length zero?

# simplify variables and functions

Perhaps NEVER specify the type of a variable declaration and always rely
on inference? When would this be bad?

let a = [1, 2, 3, 4]
type(a) = [Int]

let x = Float64(2.0)
type(x) = Float64

let x = Float32(1.0)
type(x) = Float32

With Float and Int as default types when unspecified then things are
easy:

let x = 1
let y = 3.0

to force unsigned int we could support:

let x = 1u

and for specific sizes:

let y = Unt8(2)
      = 2u8 // 4 chars shorter, but maybe harder to read?

      in arrays you only need to specify the first type anyway

      = [Unt8(2), 3, 4, 5, 8, 9]

let z = Int32(8)


let y = 3.0d
let z = 3.0f


let add = (Int a, Int b) -> Int:
    return a + b
