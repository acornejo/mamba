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

// dictionaries and lists
__getitem__(key)           // return x[key]
__setitem__(key, value)    // x[key] = value
__getslice__(i,j)          // return x[i:j]
__setslice__(i,j, value)   // x[i:j] = value
__contains__(value)        // value in x

do not need __delitem__  since we can use "list.erase(key)"    instead of "delete(list, key)"
do not need __len__      since we can use "list.length()"      instead of "len(list)"
do not need __hash__     since we can use "elem.hash()"        instead of "hash(elem)"
do not need __contains__ since we can use "dict.contains(val)" instead of "val in dict"   (maybe worth it?)
