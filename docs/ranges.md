for r in collection:
    r.value = test
    test = r.value
    r.erase()


var r = collection.range()
while r.next()
    val = r.get()
    r.erase()

for val in collection:
    if val%1:
        print("odd")
    else:
        print("even")

# modified from andrei alexandrescu

iface Range<T>:
   fun empty(Self) -> Bool
   fun front(Self) -> &T
   fun popFront(&Self) -> None

# with for loop
for val in collection:
    body

# with while loop
var r = collection.range()
while not r.empty():
    var &val = r.front();
    body
    r.popFront()

iface DoubleEndedRange<T>
    fun back(Self) -> &T
    fun popBack(&Self) -> None

generic DER:
    ForwardRange<T>
    BackwardRange<T>

record retro<DER>:
  DER range

fun retro<DER>.empty(Self self) -> Bool:
    return self.range.empty()

fun retro<DER>.front(Self self) -> &T:
    return self.range.back()

fun retro<DER>.popFront(&Self self) -> None:
    self.range.popBack()

fun retro<DER>.back(Self self) -> &T:
    return self.range.front()

fun retro<DER>.popBack(&Self self) -> None:
    self.range.popFront()

fun retro<FiniteRange>.length(Self self) -> ISize:
    return self.range.length

ifc RandomAccessRange<T>:
    fun at(Self, Int) -> &T
    fun slice(Self, Int, Int) -> Self


# should RandomAccessRange just include a length? Do inifnite
# ranges really make sense here? what kind of operations would we
# really want to perform with these?


fun binarySearch(Range r, T elem) -> T?
    if r.empty():
        return None
    var midIdx = r.length/2
    var mid = r.at(midIdx)
    if mid == elem:
        return elem
    else if mid > elem:
        return binarySearch(r.slice(0,mid), elem)
    else
        return binarySearch(r.slice(mid+1,r.length), elem)

fun mergeSort(Range<T> a, Range<T> b) -> Range:
    var [T] out = []
    while not a.empty() and not b.empty():
        if a.front() < b.front()
            out.append(a.front())
            a.popFront()
        else:
            out.append(b.front())
            b.popFront()

    while not a.empty():
        out.append(a.front())
        a.popFront()

    while not b.empty()

record MergedRange<T>:
    [Range<T>] ranges
    Int idx

fun MergedRange<T>.nextIdx(Self) -> T:
    var minIdx = 0
    var minVal = ranges[minIdx].front()
    for i in ranges(1,ranges.length):
        if ranges[i].front() < minVal:
           minIdx = i
           minVal = ranges[i].front()
    return minidx

fun MergedRange<T>.front(Self) -> T:
    return self.ranges[idx]

fun MergedRange<T>.popFront(Self) -> T:
    self.ranges[idx].popFront()
    minIdx=self.nextIdx()
