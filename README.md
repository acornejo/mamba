# Naming rules

- Type names must be UpperCammelCase
- Functions must be lowerCamelCase
- 'static' constants must be UPPER_SNAKE_CASE
- If not a type or function what should it be? lowerCamelCase, or perhaps snake_case?

# POD Types
## Machine independent

- Bool
- Int{8,16,32,64}
- Unt{8,16,32,64}
- Float{32,64}
- Char (unicode)
- Str

# Type aliases (machine dependent)

- alias Byte Unt8
- alias Int Int32 or Int64
- alias Unt Unt32 or Unt64
- alias Float Float32 or Float64
- alias Imem Unt32 or Unt64

# Control structures

    if condition {
        do some stuff
    } else if something {
        do other stuff
    } else {
        do other other stuff
    }

    while condition {
        do stuff
    }

    for i in range(10) {
        print(i)
    }

    for index, value in array {
        print(index, '=', value)
    }

    for key, value in dict {
        print(key, '=', value)
    }

# challenge, how to delete from a collection.

    list := LinkedList<int>.of(1,2,3)

    for elem in list {
        print(elem)
    }

    // remove even numbers
    while iter := list.iter(); !iter.empty() {
        value := iter.value()
        if value % 2 == 0 {
            iter.erase()
        } else {
            iter.next()
        }
    }


# Variable declaration

Variables must always be initialized when declared.

    x := 3
    y := 4.0
    z := "Hello world!"

# Array

Arrays represent an ordered sequence of fixed length where all elements
in the sequence have the same type.

The following declares an array of integers from 0 to 9.

    x := [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]

Declaring very large arrays using this syntax can quickly become
unmanageable. For that you can use the following more explicit notation:

    x := [0; 10]

// Type of array is [Int; 10] for an array of size 10.

# Slice

Slices represent contiguos subset of an array. Type is [Int]

    x := [0; 10]

    y := x[:5] // first 5 elements of x
    z := y[:2] // first two elements of x
    w := z[1:] // second element of x

# Dictionary

    dict := ["hello": "hola", "world": "mundo"]

Internally dictionaries are implemented using a hash table, so you have
constant time lookup and insertion.

# Tuple

Tuples represent an ordered sequence of fixed length, where each element
in the sequence can be of any type (difference with arrays)

The next example shows a tuple that holds a string and an integer (for
example, to represent the name and age of death of famous writers).

    jane := ("Jane Austen", 41)
    earnest := ("Ernest Hemingway", 61)
    mark := ("Mark Twain", 74)

    print(jane[0], 'died at', jane[1])

# Records

Records, like tuples, can also be used to hold a sequence of values of
different types. However, each value held by a record must be associated
with a name (unlike tuples and arrays which associate values with a
numerical index).

The following defines a record two hold a two-dimensional point.

    Point := record (x Float, y Float)

    p := Point(x=2.0, y=3.0)

# Unions

    MaybeInt := union (None, Some(Int))

    option := MaybeInt.Some(3)

Alternatively, to initialize to None

    option := MaybeInt.None

To determine the value of a union variable, you must use the
destructuring operator match.

    switch option {
        case Some(x) {
            print(x)
        }
        case None {
            print("not found")
        }
    }

This is roughly equivalent to:

    if option == MaybeInt.Some(x) {
        print(x)
    } else {
        print("not found")
    }

Two things are worth noting. First, observe that when using the match
operator it is not necessary to specify the namespace of the union
variable, since this can be inferred by the variable type of option.

# Function definitions

We already learned how to declare new custom variables and define
variables. Functions definitions are similar.

    dist := func (p Point, q Point) -> Float {
        x := p.x - q.x
        y := p.y - q.y
        return sqrt(x*x + y*y)
    }

    p := Point(x=1.0, y=1.0)
    q := Point(x=3.0, y=3.0)

    assert(dist(p,q) == sqrt(8.0))

# Functions that receive references

Mamba treats the input parameters received by functions as read-only.
Restricting functions to not have side effects on their input parameters
generally leads to cleaner code. For those rare cases when modifying the
input parameters is actually what you want, Mamba provides references.
A variable is declared to be a reference by prepending its type with the
`&` operator. Variables must be declared as references both in the
function definition and during function invocation.

    increment: = fun (x &Int) -> None {
        x = x + 1
    }

    &a := 1
    assert(a == 1)
    increment(&a)
    assert(a == 2)
    increment(&a)
    assert(a == 3)

# Closures

It is sometimes desirable to define functions which capture variables
available in the scope they were declared in. For instance,

    incrementor := fun (start Int) -> (Int) -> Int {
        return fun [start] (step Int) -> Int {
           return start + step
        }
    }


    f := incrementor(10)
    g := incrementor(20)

    assert(f(1) == 11)
    assert(g(1) == 21)

# Defining interfaces

The following interface defines an empty type constructor. The reserved
type `Self` is a placeholder for whatever type implements the
interface.

    Default := iface (
        default () -> Self
    )

We can now implement the default constructor interface for the custom
type Point as follows:

    Point.default := fun () -> Point {
        return Point(x=0.0, y=0.0)
    }

    p := Point.default()

# Objected oriented programming without objects

Mamba is NOT an object oriented language; as such concepts like
inheritance are not available in the language.

That being said, most of the object oriented programming patterns can be
achieved within mamba through the use of interfaces. The minimal example
below should illustrate how the some of the classic patterns in object
oriented languages can be easily translated to Mamba.

    Shape := iface (
        area (Self) -> Float,
        perim (Self) -> Float
    )

    Shape.describe := fun (self) -> None {
        print!("area is #{self.area()} and perimeter #{self.perim()})
    }

    Rect := record (
        width Float,
        height Float
    )

    Rect.area := fun (self) -> Float {
        return self.width * self.height
    }

    Rect.perim := fun (self) -> Float {
        return self.width*2 + self.height*2
    }

    Circle := record (
        radius Float
    )

    Circle.area := fun (self) -> Float {
        return math.PI*self.radius**2
    }

    Circle.perim := fun (self) -> Float {
        return 2*math.Pi*self.radius
    }

    shapes := [Rect(width=3.0, height=3.0) as Shape, Circle(radius=2.0) as Shape]

    for shape in shapes {
        shape.describe()
    }

# unwrapping unions

Unions are particularly useful to send and receive parameters which may
or may not have a value. For example, suppose you want to find
the largest value in a list. What should you return if a user
attempts to find the largest element of an empty list?

    var list = [1,5,10,3,7,15,8,4]
    var MaybeInt val = find_max(list)

    switch val {
        case None {
            print("list is empty")
        }
        case Some(x) {
            print("largest value is", x)
        }
    }

In general, the Maybe union type can be used in cases where you would
usually use a placeholder value. For instance, here is how the code of a
function that finds the maximum element could look like if using a list.

    findMax := fun (list [Int]) -> MaybeInt {
        &max := None
        for i in list {
            switch max {
                case None {
                    max = Some(i)
                }
                case Some(x) {
                    if i > x {
                        max = Some(i)
                    }
                }
            }
        }
        return max
    }


Of course, that code isn't particularly efficient, but it is good to
illustrate the point. Instead you should write:

    findMAx := fun (list [Int]) -> MaybeInt {
        if list.length == 0 {
            return None
        }
        &max := list[0]
        for i in list[1:] {
            if i > max {
                max = i
            }
        }
        return max
    }

For small lists there will be no difference, but for larger lists you
will save one branch for every item in the array, since we replaced the
match inside the for loop with a single if outside the loop.

# Allocating in heap

All objects are allocated in the stack unless specifically using the '*'
operator, this includes arrays which do not require heap allocation.

    *x := 3
    *y := 4.0
    *p := Point(x=3, y=2)

# Generic function definitions

    max := fun <T Orderable> (x T, y T) -> T {
        if x > y {
            return x
        } else {
            return y
        }
    }

    swap := fun <T Copyable> (&x T, &y T) -> None {
        t := x
        x = y
        y = t
    }

Example usage of generic functions:

    var big = max(3,4)
    var small = min(3,4)
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

    fun Stack<Val>.push (Self &self, Val v) -> None:
        self.raw_data.append(v)

    fun Stack<Val>.pop (Self &self) -> Val:
        var Val v = self.raw_data[-1]
        self.raw_data.pop_back()
        return v

    record <Key, Val> Map:
        MapEntry<Key,Val> test

    fun <K,V> Map.iter (Self self) -> <K,V> MapIterator:
        return MapIterator <K,V> (self)

# Destructuring (optional)

I am not sure I understand how this is useful, but here is how the
syntax could be:

    p := (1, 2)

    (x, y) := p

    q := Point(x=1, y=2)

    {x, y} := q

    Some(x) := optional // panic if not there?

Without destructuring we have:

    x := p[0]
    y := p[1]

    x := q.x
    y := q.y

    x := optional.value()

If we only support tuple destructuring then we could still do:

    x, y := q.x, q.y
