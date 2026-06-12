# Velo MVP 0.6 release notes draft
Velo MVP 0.6 focuses on maps and object literals.

It adds map types, map literals, runtime map values, map indexing, map assignment,
and the first map builtin.

## Highlights
- map type syntax
- map semantic type model
- map literals
- empty map literals with expected type
- runtime map values
- deep-copy support for maps
- map indexing read
- field access after map indexing
- array indexing after map indexing
- map element assignment
- map element insert assignment
- nested map element assignment
- mixed map/array index assignment
- `std::map` module
- `map::len`

## Language
### Map type syntax
Maps use `map<K, V>` syntax.

For MVP 0.6, map keys must be `string`.

```velo
map<string, int>
map<string, string>
map<string, bool>
map<string, User>
map<string, []int>
[]map<string, int>
```

Example:
```velo
struct User {
    id: int;
    name: string;
}

struct Store {
    scores: map<string, int>;
    users: map<string, User>;
    grouped: map<string, []int>;
}
```

### Map literals
Map literals use explicit `map { ... }` syntax:
```velo
let scores: map<string, int> = map {
    "Alex": 10,
    "Bob": 20
};
```

Empty map literals require an expected map type:
```velo
let scores: map<string, int> = map {};
```

Map literal values must have consistent value types.

### Map indexing
Maps can be indexed with string keys:
```velo
return scores["Bob"];
```

Maps of structs support field access after indexing:
```velo
return users["bob"].id;
```

Maps with array values support repeated indexing:
```velo
return grouped["b"][1];
```

Missing keys are runtime errors.

### Map element assignment
Map entries can be assigned through mutable local map values:
```velo
var scores: map<string, int> = map {
    "Alex": 10
};

scores["Alex"] = 42;
```

Assigning a missing leaf key inserts it:
```velo
scores["Bob"] = 20;
```

Nested map paths are supported when intermediate keys already exist:
```velo
nested["outer"]["inner"] = 42;
```

Mixed map/array index paths are supported:
```velo
grouped["a"][0] = 99;
```

### `std::map`
MVP 0.6 adds the `std::map` module.

```velo
use std::map;
```

### `map::len`
```velo
map::len(values)
```
Returns the number of entries in a map.

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

## Runtime
MVP 0.6 adds:
- `MapValuePtr`
- runtime map values
- recursive map cloning in `Runtime::cloneValue(...)`
- map printing as `<map len=N>`
- map indexing through `LoadIndex`
- map assignment through generic `StoreIndexPath`
- `map::len`

## IR and bytecode
MVP 0.6 adds or extends:
- `BuildMap`
- `LoadIndex` support for maps
- `StoreIndexPath` support for maps
- bytecode compiler support for `BuildMap`
- bytecode VM support for map values
- bytecode disassembler support for `BuildMap`
- bytecode file format support for `BuildMap`

## Examples
New examples:
```text
examples/map_literal/main.velo
examples/map_indexing/main.velo
examples/map_assignment/main.velo
examples/map_builtin/main.velo
```

## Tests
MVP 0.6 adds tests for:
- parser map type syntax
- parser map literals
- parser map indexing
- parser map assignment
- semantic map type validation
- semantic map literal validation
- semantic map indexing
- semantic map assignment
- duplicate map literal keys
- runtime map cloning
- map indexing runtime behavior
- map assignment runtime behavior
- `map::len`
- bytecode VM map behavior
- driver examples and smoke tests

## Known limitations
MVP 0.6 does not include:
- non-string map keys
- field assignment through map index, for example `users["bob"].id = 3`
- `map::has`
- `map::keys`
- `map::values`
- `map::remove`
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

## Next
The next milestone should be planned after tagging `v0.6.0`.

Suggested candidate:
```text
MVP 0.7     JSON foundation / serialization
```