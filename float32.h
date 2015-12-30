interface {
    // hashing
    __hash__


    // comparison < <= == >= > !=
    __lt__
    __le__
    __eq__
    __ne__
    __gt__
    __ge__

    // arithmetic +, -, *, /, %, **
    __add__
    __sub__
    __mul__
    __div__
    __mod__
    __pow__
    // unary arithmetic +, -
    __pos__
    __neg__

    // bitwise: ~, <<, >>, |, &, ^
    __inv__
    __rshift__
    __lshift__
    __or__
    __and__
    __xor__

    // iterator
    __iter__
    __next__

    // dictionaries and lists
    __getitem__
    __setitem__
    __delitem__
}
