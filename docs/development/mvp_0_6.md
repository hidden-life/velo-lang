# Velo MVP 0.6 status
MVP 0.6 focuses on maps and object literals.

The goal is to add map types, map literals, runtime map values, map indexing,
map assignment, and basic map builtins.

## Roadmap
```text
0.6.1   map type syntax and semantic type model
0.6.2   map/object literals and runtime map values
0.6.3   map indexing read
0.6.4   map element assignment
0.6.5   map builtins and examples
0.6.6   docs/examples/release checklist
```

## Current step
```text
0.6.6   docs/examples/release checklist
```

## 0.6.1 scope
Implemented in this step:
- map type syntax
- map semantic type model
- maps in function parameters
- maps in function return types
- maps in local declarations
- maps in struct fields
- arrays of maps
- maps with array values
- maps with struct values
- parser tests
- semantic tests
- documentation

Supported type syntax:
```velo
map<string, int>
map<string, string>
map<string, bool>
map<string, User>
map<string, []int>
[]map<string, int>
```

Current rules:
- map keys must be `string`
- map values cannot be `void`

## 0.6.2 scope
Implemented in this step:
- map literal expressions
- empty map literals with an expected map type
- semantic validation for map literal value types
- duplicate map literal key diagnostics
- runtime map value
- deep-copy support for runtime maps
- IR lowering for map literals
- interpreter support for building runtime maps
- bytecode support for map build instructions
- parser tests
- semantic tests
- runtime tests
- driver tests
- bytecode VM tests
- example

Example:
```velo
let scores: map<string, int> = map {
    "Alex": 10,
    "Bob": 20,
};
```

Empty map literals require an expected map type:
```velo
let scores: map<string, int> = map {};
```

## 0.6.3 scope
Implemented in this step:
- map index read expressions
- semantic validation for map index type
- runtime map index reads
- missing map key runtime error
- field access after map indexing
- array indexing after map indexing
- bytecode VM support for map index reads
- parser tests
- semantic tests
- driver tests
- bytecode VM tests
- example

Example:
```velo
let scores: map<string, int> = map {
    "Alex": 10,
    "Bob": 20
};

return scores["Bob"];
```

Maps of structs can be indexed and then accessed with field access:
```velo
return users["bob"].id;
```

Maps with array values can be indexed repeatedly:
```velo
return groupped["b"][1];
```

## 0.6.4 scope
Implemented in this step:
- map element assignment
- map element insert assignment
- nested map element assignment
- assignment through map index followed by array index
- generic `StoreIndexPath` runtime behavior for arrays and maps
- generic `StoreIndexPath` bytecode VM behavior for arrays and maps
- parser tests
- semantic tests
- driver tests
- bytecode VM tests
- example

Example:
```velo
var scores: map<string, int> = map {
    "Alex": 10
};

scores["Alex"] = 42;
scores["Bob"] = 20;
```

Mixed map/array index paths are supported:
```velo
var grouped: map<string, []int> = map {
    "a": [1, 2]
};

grouped["a"][0] = 42;
```

Nested map paths are supported when intermediate keys already exist:
```velo
nested["outer"]["inner"] = 42;
```

## 0.6.5 scope
Implemented in this step:
- `std::map` module
- `map::len` builtin
- semantic validation for map builtin argument type
- runtime implementation for map length
- runtime module sync test
- semantic tests
- driver tests
- example
- documentation

Example:
```velo
module app;

use std::map;

fn main(): int {
    let scores: map<string, int> = map {
        "Alex": 10,
        "Bob": 20
    };

    return map::len(scores);
}
```

`map::len` works with maps of any value type:
```velo
map::len(map<string, int>)
map::len(map<string, User>)
map::len(map<string, []int>)
```

## 0.6.6 scope
Implemented in this step:
- MVP 0.6 documentation review
- examples documentation review
- release checklist update
- release notes draft
- README current status update
- architecture docs synchronization
- language docs synchronization
- smoke-test command list
- tag instruction

## MVP 0.6 completed feature set
MVP 0.6 includes:
- all MVP 0.1 language foundation
- all MVP 0.2 data model foundation
- all MVP 0.3 value semantics and block scope foundation
- all MVP 0.4 array foundation
- all MVP 0.5 bytecode foundation
- map type syntax
- map semantic type model
- map literals
- empty map literals with expected type
- runtime map values
- deep-copy support for maps
- map indexing read
- field access after map indexing
- array indexing after map indexing
- map element insert assignment
- nested map element assignment
- mixed map/array index assignment
- `std::map` module
- `map::len`

## MVP 0.6 examples
```text
examples/map_literal/main.velo
examples/map_indexing/main.velo
examples/map_assignment/main.velo
examples/map_builtin/main.velo
```

## MVP 0.6 limitations
Not implemented in MVP 0.6:
- non-string map keys
- field assignment through map index, for example `users["bob"].id = 3`
- `map::has`
- `map::keys`
- `map::values`
- `map::remove`
- stable object literal shorthand without `map`
- JSON parser
- JSON serializer
- nullable types
- generics
- methods
- iterators
- `for` loops
- bytecode optimizer
- bytecode verifier
- standalone `veloc`
- standalone `velovm`
- HTTP runtime

## Next milestone
The next milestone should be planned after tagging `v0.6.0`.

Suggested candidate:
```text
MVP 0.7     JSON foundation / serialization
```