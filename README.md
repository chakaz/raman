# Raman: C++ *Ra*nge *Man*ipulation Library

*Dependency free, header-only, low overhead.*

With Raman you can manipulate ranges easily: transform, filter, sort, etc.

Using Raman will make your code *expressive*, *readable* and *short*. Examples:

```cpp
raman::From(input).Where(<lambda>);
raman::From(input).Sort();
raman::From(input).Reverse();
```

Of course, you can combine multiple methods very easily, like:

```cpp
raman::From(input).Where(<lambda>).Sort().Reverse();
```

## Usage

Every usage of Raman begins by wrapping your range (be it a container or a pair
of iterators) with `raman::From()`. You may use any STL or STL-like container /
iterator.

Once wrapped, you may use any of the utility functions below to manipulate
your ranges, like `Where()`, `AddressOf()`, `Sort()`, `Reverse()`, etc.


Raman is designed to be used with range-based for loop, like:

```cpp
for (const auto& i : raman::From(input).Where(<lambda>)) {
  // ...
}
```

Raman also has a convenient implicit-cast operator, so you can copy the
manipulated range to any container, like:

```cpp
vector<int> list_to_vector = raman::From(l);  // l is of type list<int>
```

Raman also supports move-semantics, so the following is safe:

```cpp
for (string s : raman::From(GetStrings()).Sort().Unique().Reverse()) {
  // ...
}
```

In this case Raman will take ownership of the container returned by
`GetStrings()`.

## Getting Started

Start by reading the comment in [raman.hpp](raman.hpp), then the test cases in
[tests.cpp](tests.cpp).

Now that you know roughly how to use Raman, simply `#include "raman.hpp"` and
you're ready to go. No dependencies, no linking.

## How Can I Help?

Feel free to file bugs, ask questions or send pull requests!
