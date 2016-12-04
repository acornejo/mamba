# try to mimic dlang ranges instead.

In particular an input range has the following methods:

- empty () -> Bool
  Returns false iff there is more data in the range

- front () -> E
  Returns the current element of the range, can only be called if
  empty() returns (or would have returned) false

- popFront () -> None
  Advances to the next element in the range, can ony be called if
  empty() returns (or would have returned) false


Behind the scenes ranges are used to implement our for loops. For
instance, consider the following for loop.

    for elem in list {
        print(elem.toString())
    }

The compiler will translate this into something like the following.

    while let iter = list.iter(); !iter.empty() {
        let elem = iter.front()
        iter.popFront()
        print(elem.toString())
    }

To implement range we do:

    type Range = record (
        start Int,
        end Int,
        step Int,
        offset Int,
    )

    let Range.empty = fun (self) -> Bool {
        return self.offset == self.end
    }

    let Range.front = fun (self) -> Int {
        return self.offset
    }

    let Range.popFront = fun (self) -> None {
        assert(!self.empty(), "called popFront when empty")
        self.offset = self.offset + self.step
    }

    let range = fun (end Int) -> Range {
        let step = cond!(end >= 0, 1, -1)
        return Range(start=0, end=end, step=step, offset=0)
    }

    let range = fun (start Int, end Int) -> Range {
        let step = cond!(start <= end, 1, -1)
        return Range(start=start, end=end, step=step, offset=start)
    }

    let range = fun (start Int, end Int, step Int) -> Range {
        assert(step != 0, "step can't be zero")
        if (start <= end) {
            assert(step > 0, "step should be positive")
        } else {
            assert(step < 0, "step should be negative")
        }
        return Range(start=start, end=end, step=step, offset=start)
    }

This is how range is implemented internally. The asserts will be removed
in the optimized build.

    for i in range(10) {
        print(i)
    }

Random access ranges support slicing, i.e.


   let Range.slice = (start Int, end Int) -> Range ...

Also support at operator

    let Range.at = (offset Int) -> E

And with these we can implement binary search:

    let binarySearch = fun (Range r, T elem) -> T {
        while !r.empty() {
            var midIdx = r.length()/2
            var mid = r.at(midIdx)
            if mid == elem {
                return elem
            } else if mid > elem {
                r = r.slice(0,mid)
            } else {
                r = r.slice(mid+1,r.length())
            }
        }
        return None
    }

We can also implement a mergeSort that combines slices:

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
            out.append(b.front())
            b.popFront()
