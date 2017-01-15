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
wrappers.

# define type casting

    var x = 1 as Float

# type casting for interfaces too
internally use two pointers, one for the object and the other for the vtable of the interface.

    var c = Circle{radius=2.0} as Shape
