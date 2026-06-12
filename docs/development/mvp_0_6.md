# Velo MVP 0.6 status
MVP 0.6 focuses on maps and object literals.

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
0.6.3   map indexing read
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

## Current rules
Map keys must be `string`:
```velo
map<string, int>
```

This is rejected:
```velo
map<int, int>
```

Map values cannot be `void`.

Not implemented in 0.6.1
- map/object literals
- runtime map values
- map indexing
- map element assignment
- map builtins

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

Not implemented in 0.6.2:
- map indexing
- map element assignment
- map builtins

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

Not implemented in 0.6.3:
- map element assignment
- map builtins