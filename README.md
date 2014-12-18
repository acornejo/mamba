# Mamba's Philosophy

In general, Mamba follows the DRY principle for software engineering.

Mamba is a systems programming language, what does that mean?. It means
that it should be possible to write an operating system entirely in
Mamba (with a minimal amount of assembler glue).
It also means that Mamba does not have a runtime, and programmers can
decide if their variables are allocated in the stack or heap. Mamba
should be as efficient as C, and yet provides abstractions to simplify
memory management that add as little cognitive burden on the programmer
as possible (reference counted pointers for heap allocated data).

Mamba recognizes that code is read much more often than it is written,
and thus Mamba code should be self-describing and (to the extent
possible) self-documenting. In general Mamba doesn't enable programming
"magic" one-liners which save the code-writer typing a handful of
characters, at the expense of having every code-reader decipher what is
happening. The syntax of the language aims to be natural and easily
readable by people, and is heavily inspired by python.
     
Mamba aims not not provide several constructs to perform the same
operation (i.e. switch vs if/else), but instead provides a single
canonical way of doing each thing.
To the extent possible, Mamba strives to be a programming language such
that if two programmers implement the same algorithm using Mamba, their
implementations should be very similar.

In the same spirit as above Mamba enforces strict naming conventions for
types, functions and variables. For that same reason, as python,
whitespace has a meaning in Mamba, and as such there are not multiple
"indentation" styles, but a single canonical indentation style.

# Naming rules

- Types must have the first letter be capital and can't have underscores
(i.e. UpperCamelCase).
- Functions must have the first letter be non-capital, and can't have
underscores (i.e. camelCase).
- Variables can't have capitals but can have underscores (i.e.
snake_case).

# POD Types
## Machine dependent
Bool
Float
Int
Unt
## Machine independent
Int{8,16,32,64}
Unt{8,16,32,64}
Float{32,64}
Char (unicode)
String

# Variable declaration
Variables must always be initialized when declared

    let x = 3 # implicit type inference
    let y = 4
    let Int x = 3 # explicit type inference
    let Int y = 4

## Declaring arrays
    let x = [2,3,4,5]
    let Int[] x = [2,3,4,5]
    let Int[4] x = [1,2,3,4]
    let Int[4] x = [0]

# Defining new types
Mamba supports record types, tuple types and union types (also called
variants in other languages).

    record Point:
        Float x
        Float y

    tuple Pair:
        Int
        String

    union Maybe<T>:
        None
        Some(T)

# Declaring variables with records, tuples or unions

    let point = Point{x = 0.0, y = 0.0}
    let pair = Pair(0, "hi there")
    let option = None

As with variable declarations of POD types, all elements of a record or
tuple type must be initialized when declared.

# Function definitions

    let dist = |Point p, Point q| -> Float:
        let x = p.x-q.x
        let y = p.y-q.y
        return sqrt(x*x+y*y)

    let p = Point{x = 1.0, y = 1.0}
    let q = Point{x = 3.0, y = 3.0}

    assert(dist(p,q) == sqrt(8.0))

# Defining interfaces

    iface Default<T>:
        default = || -> T

# Implementing interfaces

    impl Defautl<Point> for Point:
        default = || -> Point:
            return Point{x = 0.0, y = 0.0}

    let p = Point.default()

This is as good a place as any to mention that, unlike C++, Mamba does
not provide to the concept of type "constructors". However, as shown the
same concept can be implemented through interfaces. The above example
implements the "default constructor" for the type Point.

Remember we said Mamba doesn't have constructors? Well, it also doesn't
have the concept of inheritance. However, most of the object oriented
programming patterns (including modularization and polymorphism) can be
achieved through the use of interfaces. Below is an example of how to
achieve the equivalent of having a class Rect and class Circle both
inherit from a class Shape, and then use polymorphism to treat them both
as the base class Shape.

    record Rect:
        Float width
        Float height

    record Circle:
        Float radius

    iface Shape:
        area = |self| -> Float

    impl Shape for Rect:
        area = |self| -> Float:
            return self.width*self.height

    impl Shape for Circle:
        area = |self| -> Float
            return math.pi*self.radius**2

    let shape_list = [Rect{width=3.0, height=3.0} as Shape, Circle{radius = 2.0} as Shape]

    for shape in shape_list:
        print!("area is #{shape.area()}")


# static element access checking for tuples and records
    let v = Pair(1, "hi there)
    let p = Point.default()

    p.z = 3.0 # won't compile

    v[3] = 2.0 # won't compile

    let i = 3
    v[i] = 4.0 # won't compile

    let i = turningHaltingMachine()
    v[i] = 4.0 # won't compile

On records you can always access existing members, and cannot add new
members (i.e. they are not a dictionary). For that purpose you should
use a Map instead.
    
For tuples, you must always use an index whose values is known at
compile time. If you want to index using values which are computed at
runtime, then use arrays instead. By default there is bounds checking
performed on arrays at runtime, although this can be disabled.

However, in almost all use cases its possible to use arrays through
iterators, which avoid bounds checking without sacrificing safety.

    let a = [1,2,3,4,5]

    for i in a:
        print!("#{i}")

# unwrapping unions
Unions are particularly useful to send and receive parameters which may
or may not have a value. For example, suppose you have a type that
implements a linked list, and you want to implement an interface to find
the largest value of the liked list. What will you return if a user
attempts to find the largest element of a linked list with no values?

    let list = [1,5,10,3,7,15,8,4]
    let val:Maybe<int> = find_max(list)

    match val:
        None:
            print!("list is empty")
        Some(x):
            print!("largest value is #{x}")

In general, the Maybe union type can be used in all sorts of cases where
you would usually use a placeholder value. For instance, here is how the
code of a function that finds the maximum element could look like if
using a list.

    let find_max = |Int[] list| -> Maybe<Int>
        let max = None
        for i in list:
            match max:
                None:
                    max = i
                Some(x):
                    if x > i:
                        max = Some(i)
            return max

Of course, that code isn't particularly efficient, but it is good to
illustrate the point. Instead you should write:

    let find_max = |Int[] list| -> Maybe<Int>
        if list.length == 0:
        return None
        let max = list[0]
        for i in list[1:]:
            if i > max:
            max = i
        return Some(max)

For small lists there will be no difference, but for larger lists you
will save one branch instruction for every item in the array, since we
replaced the match inside the for loop with a single if outside the
loop.

# unpacking tuples
    let Pair(z, w) = pair
    let (z, w) = pair

# Allocating in heap
    let *Int32 x = 3
    let *Float32 y = 4.0

# Generic function definitions
    let max = <T is Orderable>|T x, T y| -> T:
        if x > y:
            return x
        else:
            return y

    let min = <T is Orderable>|T x, T y| -> T:
        if x > y:
            return y
        else:
            return x

    let swap = <T is Copyable>|&T x, &T y|:
        let t = x.copy()
        x = y
        t = x

    let add = <T is Num>|T x, T y| -> T:
        return x + y

    let sub = <T is Num>|T x, T y| -> T:
        return x - y

    record MapEntry<Key is Orderable, Val is Copyable>:
        Key key
        Val val

    struct Map<Key is Orderable, Val is Copyable>:
        MapEntry<Key,Val> test

    struct MapIterator<Key is Orderable, Val is Copyable>
        MapEntry<Key,Val> x

    impl Iterator<(T,V)> for MapIterator<T,V>
        next = fn|self| -> Bool:
            return next?

    impl Iterable<(T,V)> for Map:
        iter = fn|self| -> MapIterator<T,V> return MapIterator<T,V>(self)

    let y = 5
    if not x:
        y = 3

    collection.apply(|Int x| -> Int:
        return x+1
    )
