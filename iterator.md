iface Iterator{T}:
    fun hasNext(Self) -> Bool
    fun next(&Self) -> Maybe{T}

for i in iterable:
   body


=>

%temp% = iter(iterable)
while %temp%.hasNext():
    var i = %temp%.next()
    body

loop:
   if not %temp%.hasNext():
       break
    x = %temp%.next()
    body
