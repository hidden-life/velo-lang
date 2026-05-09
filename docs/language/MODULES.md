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