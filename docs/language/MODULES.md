# Modules
Velo uses `use` declarations to import standard modules.

## Import
```velo
use std::console;
use std::string as str;
```

The visible module name is the last path segment by default:
```velo
use std::console;

console::println("hello");
```

Aliases are supported:
```velo
use std::string as str;

str::len("hello");
```

## Current standard modules
##### std::console
```velo
console::println(value);
```
Prints a runtime value.

##### std::string
```velo
use std::string as str;

str::len("hello"); // int
```
`str::len` expects a `string` argument.

##### std::int
```velo
use std::int as ints;

ints::toString(123);
```
`ints::toString` expects an `int` argument and returns `string`.

##### std::bool
```velo
use std::bool as bools;

bools::toString(true); // string
bools::toString(false); // string
```
`bools::toString` expects `bool` argument and returns `string`.