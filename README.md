# Mamba's Philosophy

Mamba follows the DRY principle for software engineering.

Mamba is a systems programming language. This means it is possible to
write an operating system entirely in Mamba (with a minimal amount of
assembler glue).
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
types, functions and variables. For that same reason, whitespace has a
meaning in Mamba (as in python), and thus there are not multiple
"indentation" styles, but a single possible way to format your Mamba
code.

# Naming rules

- Types must have the first letter be capital and can't have underscores
(i.e. UpperCamelCase).
- Functions must have the first letter be non-capital, and can't have
underscores (i.e. camelCase).
- Variables can't have capitals but can have underscores (i.e.
snake_case).
- Constants are all capitals (i.e. CONSTANT)

# POD Types
## Machine independent
Bool
Int{8,16,32,64}
Unt{8,16,32,64}
Float{32,64}
Char (unicode)
Byte
String

## Machine dependent
Float
Int
Unt

# Type aliases
alias Unt8 as Byte
alias Int32 as Int
alias Unt32 as Unt
alias Float32 as Float

# Variable declaration
Variables must always be initialized when declared

    var x = 3 # implicit type inference
    var y = 4
    var Int x = 3 # explicit type inference
    var Int y = 4

## Declaring arrays
    var x = [2,3,4,5]
    var Int[] x = [2,3,4,5]
    var Int[4] x = [1,2,3,4]
    var Int[4] x = [0]

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

Variables of custom types can be declared like so:

    var point = Point(x = 0.0, y = 0.0)
    var pair = Pair(0, "hi there")
    var option = None

As with variable declarations of POD types, all variable declarations of
custom types must be initialized when declared. In particular, record
and tuple types must always initialize all their elements through the
initializing constructor as shown above.

Mamba does not provide means of specifying alternative type
constructors, but the same effect can be achieved through interfaces
(explained below).

# Function definitions

    fun dist |Point p, Point q| -> Float:
        var x = p.x-q.x
        var y = p.y-q.y
        return sqrt(x*x+y*y)

    var p = Point(x = 1.0, y = 1.0)
    var q = Point(x = 3.0, y = 3.0)

    assert(dist(p,q) == sqrt(8.0))

# Defining interfaces

The following interface defines an empty type constructor. The reserved
typename `Self` is a placeholder for whatever type implements the
interface.

    iface Default:
        fun default || -> Self

We can now implement the default constructor interface for the custom
type Point as follows:

    impl Default for Point:
        fun default || -> Point:
            return Point(x = 0.0, y = 0.0)

    var p = Point.default()

Mamba doesn't have the concept of inheritance. However, most of the
object oriented programming patterns (including modularization and
polymorphism) can be achieved through the use of interfaces.
Below is a simple example of how to achieve the equivalent of having a
class Rect and class Circle both inherit from a class Shape, and then
use polymorphism to treat them both as the base class Shape.

    record Rect:
        Float width
        Float height

    record Circle:
        Float radius

    iface Shape:
        fun area |Self| -> Float
        fun perim |Self| -> Float

    fun Rect.area |Rect self| -> Float:
        return self.width*self.height

    fun Rect.perim |Rect self| -> Float:
        return self.width*2+self.height*2

    fun Circle.area |Circle self| -> Float:
        return math.PI*self.radius**2

    fun Circle.perim |Circle self| -> Float:
        return 2*math.Pi*self.radius

    var shape_list = [Rect(width=3.0, height=3.0) as Shape, Circle(radius = 2.0) as Shape]

    for shape in shape_list:
        print!("area is #{shape.area()}")

# alternative for implementing interfaces

The approach above is very similar to the way `go` handles implementing
interfaces. It has various benefits, one of them is that it does not
require any additional keywords and that interfaces can be defined after
they have been implemented by other types.

One drawback of this way of implementing interfaces, is that it is not
obvious how to reuse other (generic) implementations of the interface.
Here is an alternative


    impl Shape for Rect:
        fun area |Rect self| -> Float:
            return self.width*self.height
        
        fun perim |Rect self| -> Float:
            return 2*(self.width+self.height)


# static element access checking for tuples and records

On records you can always access existing members but you cannot access
or create new members. In other words, records should not be confused
with dictionaries or maps, for that purpose you can use a Map instead.

    var p = Point.default()

    p.z = 3.0 # won't compile

For tuples, you must always use an index whose values is known at
compile time. If you want to index using values which are computed at
runtime, then you should use arrays instead. By default there is bounds
checking performed on arrays at runtime, although this can be disabled.

    var v = Pair(1, "hi there)
    v[3] = 2.0 # won't compile

    var i = 3
    v[i] = 4.0 # won't compile

    var i = turingHaltingProblem()
    v[i] = 4.0 # won't compile

    
However, in almost all use cases its possible to use arrays through
iterators, which avoid bounds checking without sacrificing safety.

    var a = [1,2,3,4,5]

    for i in a:
        print!("#{i}")

# unwrapping unions
Unions are particularly useful to send and receive parameters which may
or may not have a value. For example, suppose you have a type that
implements a linked list, and you want to implement an interface to find
the largest value of the liked list. What should you return if a user
attempts to find the largest element of an empty linked list?

    var list = [1,5,10,3,7,15,8,4]
    var Maybe<int> val = find_max(list)

    match val:
        None:
            print!("list is empty")
        Some(x):
            print!("largest value is #{x}")

In general, the Maybe union type can be used in cases where you would
usually use a placeholder value. For instance, here is how the code of a
function that finds the maximum element could look like if using a list.

    fun find_max |Int[] list| -> Maybe<Int>:
        var max = None
        for i in list:
            match max:
                None:
                    max = Some(i)
                Some(x):
                    if x > i:
                        max = Some(i)
            return max

Of course, that code isn't particularly efficient, but it is good to
illustrate the point. Instead you should write:

    fun find_max |Int[] list| -> Maybe<Int>:
        if list.length == 0:
            return None
        var max = list[0]
        for i in list[1:]:
            if i > max:
            max = i
        return Some(max)

For small lists there will be no difference, but for larger lists you
will save one branch for every item in the array, since we replaced the
match inside the for loop with a single if outside the loop.

# unpacking tuples
    var Pair(z, w) = pair
    var (z, w) = pair

# Allocating in heap
    var *Int32 x = *3
    var *Float32 y = *4.0
    var *Point p = *Point{x=3.0, y=2.0}

# type inference works here too
    var x = *3
    var y = *4.0
    var p = *Point{x=3.0, y=2.0}

# Generic function definitions
    fun max<T is Orderable>|T x, T y| -> T:
        if x > y:
            return x
        else:
            return y

    fun max<T is Orderable>|T x, T y| -> T:
        if x > y:
            return y
        else:
            return x

    fun swap<T is Copyable>|&T x, &T y|:
        var t = x
        x = y
        y = t

    var big = max(3,4)
    var small = min(3,4)
    swap(&big,&small)


# Generic types and interfaces

    record MapIterator<Key is Orderable, Val is Copyable>
        MapEntry<Key,Val> data
        Maybe<Self> next

    impl Iterator<(T,V)> for MapIterator<T,V>
        fun next |Self &self| -> Maybe<(T,V)>

    record Map<Key is Orderable, Val is Copyable>:
        MapEntry<Key,Val> test

    impl Iterable<(T,V)> for Map:
        iter = fn|self| -> MapIterator<T,V> return MapIterator<T,V>(self)
