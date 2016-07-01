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

    let x = 3
    let y = 4.0

In cases where the variable type can be inferred from context, it is not
necessary to explicitly declare its type. Howeer, you can also be
explicit

    let x = Int{3}
    let y = Float{4.0}

# Arrays

The following three declarations are equivalent, and they all declare
`x` as an array of ten integers initialized to zero.

    let [Int] x = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    let [Int] x = [0; 10]

As with POD types, when possible mamba will infer the types from the
context, hence the following declares an array of ten integers
initialized to zero.

    let x = [0; 10]

# Tuples

Tuples are similar to arrays in that they both represent an ordered
sequence of some fixed length. The main difference is that while arrays
can only hold values of the same type, a tuple can hold values of
distinct types.

The next example shows a tuple that holds a string and an integer (for
example, to represent the name and age of death of famous writers).

    let (String, Int) jane = ("Jane Austen", 41)
    let (String, Int) ernest = ("Ernest Hemingway", 61)
    let (String, Int) mark = ("Mark Twain", 74)

As with POD types it is not necessary to explicitly annotate the type of
a tuple in cases where mamba can infer it by the context. For instance,
  we could have declared `jane` as follows:

    let jane = ("Jane Austen", 41)

# Dictionaries

    let [Str: Str] dict = ["hello": "hola", "world": "mundo"]


# Records

Records, like tuples, can also be used to hold a sequence of values of
different types. However, each value held by a record must be associated
with a name (unlike tuples and arrays which associate values with a
numerical index).

The following defines a record two hold a two-dimensional point.

    record Point:
        Float x
        Float y

    let p = Point{x=2.0, y=3.0}

OR:

    record Point:
        x: Float
        y: Float

# Unions

    union MaybeInt:
        None
        Some(Int)

    let option = MaybeInt.Some(3)

Alternatively, to initialize to None

    let option = MaybeInt.None

To determine the value of a union variable, you must use the
destructuring operator match.

match option:
    Some(x):
        do stuff with x
    None:
        print("not found!")

This is roughly equivalent to:

if option == Some(x):
    do stuff with x
elif option == None:
    print("not found!")

The important distinction is that you canno

Two things are worth noting. First, observe that when using the match
operator it is not necessary to specify the namespace of the union
variable, since this can be inferred by the variable type of option.


# Function definitions

We already learned how to declare new custom variables and define
variables. Functions definitions are similar.


    let dist = fun (Point p, Point q) -> Float:
        let x = p.x - q.x
        let y = p.y - q.y
        return sqrt(x*x + y*y)

    let p = Point{x = 1.0, y = 1.0}
    let q = Point{x = 3.0, y = 3.0}

    assert(dist(p,q) == sqrt(8.0))

# Functions that receive references

Mamba treats the input parameters received by functions as read-only.
Restricting functions to not have side effects on their input parameters
generally leads to cleaner code. For those rare cases when modifying the
input parameters is actually what you want, Mamba provides references.
A variable is declared to be a reference by prepending its type with the
`&` operator. Variables must be declared as references both in the
function definition and during function invocation.

    let increment = fun (&Int x) -> None:
        x = x + 1

    let a = 1
    assert(a == 1)
    increment(&a)
    assert(a == 2)
    increment(&a)
    assert(a == 3)

# Closures

It is sometimes desirable to define functions which capture variables
available in the scope they were declared in. For instance,

    let incrementor = fun (Int start) -> (Int) -> Int:
        return fun [start](Int step) -> Int:
           return start + step


    let f = incrementor(10)
    let g = incrementor(20)

    assert(f(1) == 11)
    assert(g(1) == 21)

# Defining interfaces

The following interface defines an empty type constructor. The reserved
type `Self` is a placeholder for whatever type implements the
interface.

    iface Default:
        default = fun () -> Self

    iface Shape:
        area = fun (Shape) -> Float
        perim = fun (Shape) -> Float

We can now implement the default constructor interface for the custom
type Point as follows:

    let Point.default = fun () -> Point:
        return Point(x = 0.0, y = 0.0)

    let p = Point.default()

Alternative def:

    iface Default:
       fun () -> Self default

    iface Shape:
       fun (Shape) -> Float area
       fun (Shape) -> Float perim

OR:

    iface Shape:
       area: fun (Shape) -> Float
       perim: fun (Shape) -> Float


# Objected oriented programming without objects

Mamba is NOT an object oriented language; as such concepts like
inheritance are not available in the language.

That being said, most of the object oriented programming patterns can be
achieved within mamba through the use of interfaces. The minimal example
below should illustrate how the some of the classic patterns in object
oriented languages can be easily translated to Mamba.

    interface Shape:
        let area = fun (Shape) -> Float
        let perim = fun (Shape) -> Float

    let Shape.descibe = fun (Shape self) -> None:
        print!("area is #{self.area()} and perimeter #{self.perim()})

    record Rect:
        Float width
        Float height

    let Rect.area = fun (Rect self) -> Float:
        return self.width*self.height

    let Rect.perim = fun (Rect self) -> Float:
        return self.width*2+self.height*2

    record Circle:
        Float radius

    let Circle.area = fun (Circle self) -> Float:
        return math.PI*self.radius**2

    let Circle.perim = fun (Circle self) -> Float:
        return 2*math.Pi*self.radius

    let shape_list = [Rect(width=3.0, height=3.0) as Shape, Circle(radius = 2.0) as Shape]

    for shape in shape_list:
        shape.describe()

# static element access checking for tuples and records

On records you can always access existing members but you cannot access
or create new members. In other words, records should not be confused
with dictionaries (for that there are Maps).

    let p = Point.default()

    p.hello = 3.0 # won't compile

For tuples, you must always use an index whose values is known at
compile time.

    let v = (1, "hi there)
    v[3] = 2.0 # won't compile

    v[i] = 4.0 # won't compile

If you want to index using values which are computed at runtime, then
you should use arrays instead. By default there is bounds checking
performed on arrays at runtime, although this can be disabled.

# unwrapping unions
Unions are particularly useful to send and receive parameters which may
or may not have a value. For example, suppose you want to find
the largest value in a list. What should you return if a user
attempts to find the largest element of an empty list?

    let list = [1,5,10,3,7,15,8,4]
    let MaybeInt val = find_max(list)

    match val:
        None:
            print!("list is empty")
        Some(x):
            print!("largest value is #{x}")

In general, the Maybe union type can be used in cases where you would
usually use a placeholder value. For instance, here is how the code of a
function that finds the maximum element could look like if using a list.

    let find_max = fun (Int[] list) -> MaybeInt:
        let MaybeInt max = None
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

    let find_max = fun (Int[] list) -> MaybeInt:
        if list.length == 0:
            return None
        let max = list[0]
        for i in list[1:]:
            if i > max:
                max = i
        return Some(max)

For small lists there will be no difference, but for larger lists you
will save one branch for every item in the array, since we replaced the
match inside the for loop with a single if outside the loop.

# Allocating in heap
    let *Int32 x = *3
    let *Float32 y = *4.0
    let *Point p = *Point(x=3.0, y=2.0)

# type inference works here too
    let x = *3
    let y = *4.0
    let p = *Point(x=3.0, y=2.0)

# Generic function definitions

    alias T to generic Orderable

    let max<T> = (T x, T y) -> T:
        if x > y:
            return x
        else:
            return y

    let min<T> = (T x, T y) -> T:
        if x > y:
            return y
        else:
            return x

    alias T to generic Copyable

    let swap<T> = (&T x, &T y) -> None:
        let t = x
        x = y
        y = t

Example usage of generic functions:

    let big = max(3,4)
    let small = min(3,4)
    swap(&big, &small)


# Generic types and interfaces

vectors and maps (or hashes) are builtin.


    alias Key to generic Orderable
    alias Val to generic Copyable

    record MapIterator <Key, Val>:
        MapEntry <Key, Val> data
        Maybe <Self> next

    record <Val> Stack:
        [Val] raw_data

    let Stack<Val>.push (Self &self, Val v) -> None:
        self.raw_data.append(v)

    let Stack<Val>.pop (Self &self) -> Val:
        let Val v = self.raw_data[-1]
        self.raw_data.pop_back()
        return v

    record <Key, Val> Map:
        MapEntry<Key,Val> test

    let <K,V> Map.iter (Self self) -> <K,V> MapIterator:
        return MapIterator <K,V> (self)


our stl needs:

adapters for
- stack
- queue
- single list
- double list
- priority queue
- set
- multiset
- map
- multimap
