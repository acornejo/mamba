# Naming rules

- Types must have the first letter be capital and can't have underscores
(i.e. UpperCamelCase).
- Functions must have the first letter be non-capital, and can't have
underscores (i.e. camelCase).
- Variables can't have capitals but can have underscores (i.e. snake_case).
- Constants are all capitals (i.e. CONSTANT)

# POD Types
## Machine independent
- Bool
- Int{8,16,32,64}
- Unt{8,16,32,64}
- Float{32,64}
- Char (unicode)
- Byte
- Str

# Type aliases (machine dependent)
- alias Byte Unt8
- alias Int Int32 or Int64
- alias Unt Unt32 or Unt64
- alias Float Float32 or Float64
- alias Imem Unt32 or Unt64

# Variable declaration
Variables must always be initialized when declared.

    var Int x = 3
    var Float y = 4.0

In cases where the variable type can be inferred from context, it is not
necessary to explicitly declare its type.

    var x = 3
    var y = 4.0

Mamba does not perform any implicit type conversions. Therefore the
following example does *not* compile, since you can't store an integer
in a variable declared as a floating point.

    var Float z = 50

However, either of the following will compile.

    var Float z = 50.0
    var Float z = 50 as Float

The `as` keyword used above can be used to perform explicit type conversions.


# Arrays

The following three declarations are equivalent, and they all declare
`x` as an array of ten integers initialized to zero.

    var [Int] x = [0, 0, 0, 0, 0, 0, 0, 0, 0 ,0,0]
    var [Int] x = [0; 10]

As with POD types, when possible mamba will infer the types from the
context, hence the following declares an array of ten integers
initialized to zero.

    var x = [0; 10]

# Tuples

Tuples are similar to arrays in that they both represent an ordered
sequence of some fixed length. The main difference is that while arrays
can only hold values of the same type, a tuple can hold values of
distinct types.

The next example shows a tuple that holds a string and an integer (for
example, to represent the name and age of death of famous writers).

    var (String, Int) jane = ("Jane Austen", 41)
    var (String, Int) ernest = ("Ernest Hemingway", 61)
    var (String, Int) mark = ("Mark Twain", 74)

As with POD types it is not necessary to explicitly annotate the type of
a tuple in cases where mamba can infer it by the context. For instance,
  we could have declared `jane` as follows:

    var jane = ("Jane Austen", 41)

# Dictionaries

    var [Str: Str] dict = ["hello": "hola", "world": "mundo"]


# Records

Records, like tuples, can also be used to hold a sequence of values of
different types. However, each value held by a record must be associated
with a name (unlike tuples and arrays which associate values with a
numerical index).

The following defines a record two hold a two-dimensional point.

    record Point:
        Float x
        Float y


    var p = Point(x=2.0, y=3.0)

# Unions

    union Maybe{T}:
        None
        Some(T)

    var option = Some(3)

It is worth noting that if we wanted to initialize option to `None`, the
following will *not* work:

    var option = None

This is because there isn't enough information to determine if option
is of type `Mayba<Int>`, `Maybe<Str>`, etc. We can work around this by
explicitly declaring the variable type. That is:

    var Maybe<Int> option = None


# Function definitions

We already learned how to declare new custom variables and define
variables. Functions definitions are similar.


    fun dist (Point p, Point q) -> Float:
        var x = p.x - q.x
        var y = p.y - q.y
        return sqrt(x*x + y*y)

    var p = Point(x = 1.0, y = 1.0)
    var q = Point(x = 3.0, y = 3.0)

    assert(dist(p,q) == sqrt(8.0))


An alternative way of declaring the function dist would be as follows:

    var dist = (Point p, Point q) -> Float:
        var x = p.x - q.x
        var y = p.y - q.y
        return sqrt(x*x + y*y)

These two forms are *not* equivalent. When declaring
functions using the `fun` keyword it is possible to bind several
different function definitions to the same function name; this feature
is commonly known as function overloading.
On the other hand, when using the `var` keyword the function definition
is bound to a variable name, and in Mamba variables have a single
binding.

Therefore, if we wanted to define two functions named `dist`, one for
computing the euclidean distance between two points (shown above), and
another for computing the statistical distance between two random
variables, we can only do it through the `fun` keyword.

As a rule of thumb, you should always use the `fun` keyword when
declaring functions, the only exception is when you want to explicitly
prevent overloading.

# Functions that receive references

Mamba treats the input parameters received by a function as read-only.
Restricting functions to not have side effects on their input parameters
generally leads to cleaner code. Mamba provides references for those
rare cases when modifying the input parameters is actually what you
want.
A variable is declared to be a reference by prepending its type with the `&`
operator. They must be declared as references both in the function
definition and during function invocation.

    fun increment (&Int x) -> None:
        x = x + 1

    var a = 1
    assert(a == 1)
    increment(&a)
    assert(a == 2)
    increment(&a)
    assert(a == 3)

# Defining interfaces

The following interface defines an empty type constructor. The reserved
type `Self` is a placeholder for whatever type implements the
interface.

    iface Default:
        fun default () -> Self

We can now implement the default constructor interface for the custom
type Point as follows:

    fun Point.default () -> Point:
        return Point(x = 0.0, y = 0.0)

    var p = Point.default()

# Objected oriented programming without objects

Mamba is NOT an object oriented language; as such concepts like
inheritance are not available in the language.

That being said, most of the object oriented programming patterns can be
achieved within mamba through the use of interfaces. The minimal example
below should illustrate how the some of the classic patterns in object
oriented languages can be easily translated to Mamba.

    interface Shape:
        fun area (Shape) -> Float
        fun perim (Shape) -> Float

    fun Shape.descibe (Shape self):
        print!("area is #{self.area()} and perimeter #{self.perim()})

    record Rect:
        Float width
        Float height

    fun Rect.area (Rect self) -> Float:
        return self.width*self.height

    fun Rect.perim (Rect self) -> Float:
        return self.width*2+self.height*2

    record Circle:
        Float radius

    fun Circle.area (Circle self) -> Float:
        return math.PI*self.radius**2

    fun Circle.perim (Circle self) -> Float:
        return 2*math.Pi*self.radius

    var shape_list = [Rect(width=3.0, height=3.0) as Shape, Circle(radius = 2.0) as Shape]

    for shape in shape_list:
        shape.describe()

# static element access checking for tuples and records

On records you can always access existing members but you cannot access
or create new members. In other words, records should not be confused
with dictionaries (for that there are Maps).

    var p = Point.default()

    p.z = 3.0 # won't compile

For tuples, you must always use an index whose values is known at
compile time.

    var v = (1, "hi there)
    v[3] = 2.0 # won't compile

    v[i] = 4.0 # won't compile

If you want to index using values which are computed at runtime, then
you should use arrays instead. By default there is bounds checking
performed on arrays at runtime, although this can be disabled.

However, in almost all use cases its possible to use arrays through
iterators, which avoid bounds checking without sacrificing safety.

    var a = [1,2,3,4,5]

    for i in a:
        print!("#{i}")

# unwrapping unions
Unions are particularly useful to send and receive parameters which may
or may not have a value. For example, suppose you want to find
the largest value in a list. What should you return if a user
attempts to find the largest element of an empty list?

    var list = [1,5,10,3,7,15,8,4]
    var Maybe{int} val = find_max(list)

    match val:
        None:
            print!("list is empty")
        Some(x):
            print!("largest value is #{x}")

In general, the Maybe union type can be used in cases where you would
usually use a placeholder value. For instance, here is how the code of a
function that finds the maximum element could look like if using a list.

    fun find_max (Int[] list) -> Maybe{Int}:
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

    fun find_max (Int[] list) -> Maybe{Int}:
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
    var *Point p = *Point(x=3.0, y=2.0)

# type inference works here too
    var x = *3
    var y = *4.0
    var p = *Point(x=3.0, y=2.0)

# Generic function definitions
    fun max<T is Orderable>(T x, T y) -> T:
        if x > y:
            return x
        else:
            return y

    fun max<T is Orderable>(T x, T y) -> T:
        if x > y:
            return y
        else:
            return x

    fun swap<T is Copyable>(&T x, &T y) -> None:
        var t = x
        x = y
        y = t

    var big = max(3,4)
    var small = min(3,4)
    swap(&big,&small)


# Generic types and interfaces

    record MapIterator <Key is Orderable, Val is Copyable>:
        MapEntry <Key,Val> data
        Maybe <Self> next

    fun MapIterator.next <K,V> (&Self self) -> Maybe <Self>

    record <Key is Orderable, Val is Copyable> Map:
        MapEntry<Key,Val> test

    fun <K,V> Map.iter (Self self) -> <K,V> MapIterator:
        return MapIterator <K,V> (self)
