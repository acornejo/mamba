// hash
__hash__

// comparison < <= == >= > !=
__lt__
__le__
__eq__
__ne__
__gt__
__ge__

// unary arithmetic +, -
__pos__
__neg__
// arithmetic +, -, *, /, %, **
__add__
__sub__
__mul__
__div__
__mod__
__pow__

// bitwise: ~, <<, >>, |, &, ^
__inv__
__rshift__
__lshift__
__or__
__and__
__xor__

__iter__// for i in x
__next__// ??
__as__ // x as a
__contains__ // a in x
__delete__ // x out of scope
__call__? // x()
__getitem__ // return x[a]
__setitem__ // x[a] = 3
__delitem__ // del x[a]
__getslice__ // return x[a:b]
__setslice__ // x[a:b] = y

iface Number:
   __add__ = |Self x, Self y| -> Self
   __sub__ = |Self y, Self z| -> Self

bool register_func(name, input_signature, output_signature, impl)

register_func("__add__", "Float,Float", "Float", CreateFadd)
