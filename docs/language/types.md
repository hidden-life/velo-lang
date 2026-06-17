# Velo types
Velo currently supports a small set of built-in types.

## Supported types
- `int`
- `string`
- `bool`
- `void`

## int
Integer value.
```velo
let value: int = 42;
```

Supported operations:
```velo
1 + 2
5 - 3
2 * 4
8 / 2
7 % 3
```

## string
String value.
```velo
let text: string = "hello";
```
Current string functionality is provided by `std::string`.
```velo
use std::string as str;

let length: int = str::len("hello"); // 5
```

## bool
Boolean value.
```velo
let enabled: bool = true;
let disabled: bool = false;
```
Supported logical operations:
```velo
!enabled
enabled && true
disabled || true
```

## void
`void` means no value.

It is valid only as a function return type:
```velo
fn log(): void {
    return;
}
```

Invalid:
```velo
fn bad(value: void): int {
    return 0;
}
```
Invalid:
```velo
let value: void = 0;
```

### Equality support
- `int` supports `==` and `!=`
- `string` supports `==` and `!=`
- `bool` supports `==` and `!=`

Current limitations:
- struct equality is not implemented yet
- string ordering comparisons are not implemented yet

### Local variable scopes

Function parameters and top-level function locals live in the function body scope.

`if`, `else`, and `while` bodies create nested local scopes.

```velo
let value: int = 1;

if (true) {
    let value: int = 2;
}
```

The inner `value` is a different local variable and shadows the outer one only inside the block.

Duplicate local declarations are rejected only inside the same scope. Shadowing from an outer scope is allowed.

## Declared type validation
The semantic analyzer validates declared types in:
- function return types
- function parameters
- local variable declarations

Unknown types produce semantic diagnostics.

Example:
```velo
fn broken(value: mystery): int {
    return 0;
}
```

This is invalid because `mystery` is not a known type.

## User-defined struct types
Struct declarations are supported.

Example:
```velo
struct Profile {
    id: int;
}

struct User {
    profile: Profile;
    name: string;
}
```

A struct field can reference another declared struct type.

Forward references between struct declarations are supported:
```velo
struct User {
    profile: Profile;
}

struct Profile {
    id: int;
}
```

Current support:
- struct declarations
- struct field types can reference declared structs
- function parameters can use struct types
- function return types can use struct types
- semantic type checking understands specific struct names
- struct literals can create runtime struct values
- field access can read fields from struct values
- field assignment can update fields through mutable local struct values

Example:
```velo
struct User {
    id: int;
    name: string;
}

fn identity(u: User): User {
    return u;
}

fn main(): int {
    let user: User = User {
        id: 42,
        name: "Alex"
    };

    let copy: User = identity(user);

    return copy.id;
}
```
Current limitation:
- field assignment is not implemented yet
- methods are not implemented yet
- arrays/maps/generics are not implemented yet

### Struct value semantics
Velo structs use value semantics at the language level.

Example:
```velo
let a: User = User {
    id: 1
};

let b: User = a;
```
`a` and `b` are independent struct values.

This is important for future field assignment support:
```velo
b.id = 2;
```
The intended behavior is:
```text
a.id == 1
b.id == 2
```
The runtime may internally use pointers for struct storage, but the interpreter
clones struct values at value boundaries to preserve language-level value semantics.

Field assignment preserves value semantics:
```velo
let a: User = User {
    id: 1
};

var b: User = a;

b.id = 2;
```
After this code:
```text
a.id == 1
b.id == 2
```

## Current limitations
Velo does not yet support:
- methods are not implemented yet
- array/maps/generics are not implemented yet

## Array types
Velo supports array type syntax in type positions.

```velo
[]int
[]string
[]bool
[]User
[][]int
```

Arrays can currently be used in:
- function parameters
- function return types
- local variable declarations
- struct fields

Example:
```velo
struct User {
    id: int;
}

struct Group {
    users: []User;
}

fn count(ids: []int): int {
    return 0;
}
```

Nested array types are supported:
```velo
struct Matrix {
    values: [][]int
}
```

Array literals create runtime array values:
```velo
let ids: []int = [1, 2, 3];
let names: []string = ["Alex", "Bob"];
```

Array element types must be consistent:
```velo
let ids: []int = [1, 2, 3];
```

This is rejected:
```velo
let ids: []int = [1, "bad"];
```

Array indexing returns the element type:
```velo
let ids: []int = [10, 20, 30];
let value: int = ids[1];
```

For arrays of structs, indexing returns the struct value:
```velo
let users: []User = [
    User {id: 1, name: "Alex"}
];

let user: User = users[0];
```

Array element assignment must preserve the element type:

```velo
var ids: []int = [1, 2, 3];

ids[0] = 42;
```

This is rejected:

```velo
var ids: []int = [1, 2, 3];

ids[0] = "bad";
```

Array values preserve value semantics:

```velo
let original: []int = [1, 2, 3];
var copy: []int = original;

copy[0] = 42;
```

After this code:

```text
original[0] == 1
copy[0] == 42
```

Array length can be read with `array::len` from `std::array`:
```velo
use std::array;

let ids: []int = [1, 2, 3];

let count: int = array::len(ids);
```

`array::len` accepts arrays of any element type and returns `int`.

Current limitations:
- mixed field/index assignment paths are not implemented yet
- array mutation builtins are not implemented yet
- iterators are not implemented yet
- `for` loops are not implemented yet

## Map types
Velo supports map type syntax in type positions.
```velo
map<string, string>
map<string, int>
map<string, bool>
map<string, User>
map<string, []int>
[]map<string, int>
```

Map keys must currently be `string`.

Examples:
```velo
struct User {
    id: int;
}

struct Store {
    scores: map<string, int>;
    users: map<string, User>;
    snapshots: []map<string, int>;
}
```

Map literals create runtime map values:
```velo
let scores: map<string, int> = map {
    "Alex": 10,
    "Bob": 20,
};
```

Map literal values must have consistent value types.

This is rejected:
```velo
let scores: map<string, int> = map {
    "Alex": 10,
    "Bob": "bad",
};
```

Map indexing returns the map value type:
```velo
let scores: map<string, int> = map {
    "Alex": 10
};

let score: int = scores["Alex"];
```

For maps of structs, indexing returns the struct value:
```velo
let user: User = users["Alex"];
```

For maps with array values, indexing returns the array value:
```velo
let values: []int = groupped["a"];
```

Map element assignment must preserve the map value type:
```velo
var scores: map<string, int> = map {
    "Alex": 10
};

scores["Alex"] = 42;
```

This is rejected:
```velo
scores["Alex"] = "bad";
```

Assigning a missing leaf key inserts the key:
```velo
scores["Bob"] = 20;
```

Map length can be read with `map::len` from `std::map`.

```velo
use std::map;

let scores: map<string, int> = map {
    "Alex": 10,
    "Bob": 20
};

let count: int = map::len(scores)
```

`map::len` accepts maps of any value type and returns `int`.

Current limitations:
- no `map::has`
- no `map::keys`
- no `map::values`
- no `map::remove`

## JSON serialization
`json::stringify` serializes supported Velo values to JSON text.

In MVP 0.7.1, supported types are:
- `int`
- `string`
- `bool`
- arrays of supported values
- maps with string keys and supported values
- structs with supported fields

Examples:
```velo
let a: string = json::stringify(42);
let b: string = json::stringify("Alex");
let c: string = json::stringify(true);

let ids: []int = [1, 2, 3];
let idsJson: string = json::stringify(ids);

let scores: map<string, int> = map {
    "Alex": 10,
    "Bob": 20
};

let scoresJson: string = json::stringify(scores);
```

Nested collections are supported when all nested values are supported:
```velo
let grouped: map<string, []int> = map {
    "a": [1, 2],
    "b": [3, 4]
};

let groupedJson: string = json::stringify(grouped);
```

Struct example:
```velo
struct User {
    id: int;
    name: string;
}

let user: User = User {
    id: 1,
    name: "Alex"
};

let jsonText: string = json::stringify(user);
```

Arrays and maps of structs are supported:
```velo
let users: []User = [
    User { id: 1, name: "Alex" }
];

let usersByName: map<string, User> = map {
    "alex": User { id: 1, name: "Alex" }
};
```

## JSON type
Velo has a runtime JSON value type:
```velo
json
```

JSON values are created with `json::parse`:
```velo
let value: json = json::parse("{\"id\":42}");
```

JSON values can be serialized with `json::stringify`:
```velo
let text: string = json::stringify(value);
```

JSON object fields can be read through helper functions:
```velo
let value: json = json::parse("{\"id\":42,\"name\":\"Alex\"}");

let id: int = json::get_int(value, "id");
let name: string = json::get_string(value, "name");
```

Nested JSON values can be kept as `json`:
```velo
let profile: json = json::get_json(value, "profile");
```

Current limitations:
- JSON values cannot be indexed directly yet
- JSON access helpers are not implemented yet
- JSON floats are not implemented yet

## HTTP runtime types
Velo has HTTP runtime foundation types:
```velo
http_request
http_response
```

These types are used to model HTTP request and response values.

HTTP request model:
```text
method: string
path: string
headers: map<string, string>
body: string
```

HTTP response model:
```text
status: int
headers: map<string, string>
body: string
```

In MVP 0.8.1, these types can be used in function signatures:
```velo
fn handle(req: http_request): http_response {
    return fallback(req);
}
```

HTTP response values can be created through `std::http` response builders:
```velo
use std::http;
use std::json;

let raw: http_response = http::response(200, "OK");
let text: http_response = http::text_response(200, "Hello");
let jsonRes: http_response = http::json_response(201, json::parse("{\"ok\":true}"));
```

Response builders return `http_response`.

HTTP response fields can be read through `std::http` helpers:
```velo
let res: http_response = http::text_response(200, "Hello");

let status: int = http::status(res);
let body: string = http::body(res);
let contentType: string = http::header(res, "Content-Type");
```

Header existence can be checked with:
```velo
let exists: bool = http::has_header(res, "Content-Type");
```

HTTP request values can be created through `std::http`.

```velo
use std::http;

let req: http_request = http::request("POST", "/users", "{\"name\":\"Alex\"}");
```

Request fields can be read through helpers:
```velo
let method: string = http::method(req);
let path: string = http::path(req);
let body: string = http::request_body(req);
```

JSON request body can be parsed with:
```velo
let jsonBody: json = http::json_body(req);
```

HTTP values can be passed through normal functions:
```velo
fn create_user(req: http_request): http_response {
    let body: json = http::json_body(req);

    return http::json_response(201, json::parse("{\"ok\":true}"));
}
```

This is the foundation for future HTTP server handlers.