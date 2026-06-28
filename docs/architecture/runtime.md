# Runtime and VM

The Velo runtime owns builtin functions, module metadata, and VM execution support.

## Runtime

The runtime owns:

- `BuiltinRegistry`
- `ModuleRegistry`

Builtin functions are registered first.

Then module metadata is built from builtin metadata.

```text
BuiltinRegistry
  -> ModuleRegistry
```

This allows semantic analysis to resolve builtin modules and functions.

## Builtin functions

Builtin functions currently have:

- full name
- parameter types
- return type
- handler

Example signatures:

```text
console::println(any): void
string::len(string): int
int::toString(int): string
bool::toString(bool): string
```

## Builtin return values

Builtins return `ExecutionResult`.

Void builtins leave `returnValue` empty:

```text
console::println(value) -> void
```

Value-returning builtins set `returnValue`:

```text
string::len(value) -> int
```

After successful builtin execution, the interpreter pushes `returnValue` onto the operand stack when it is present.

## Builtin parameter metadata

Builtin functions declare parameter types.

Examples:

- `console::println(any): void`
- `string::len(string): int`
- `int::toString(int): string`
- `bool::toString(bool): string`

The runtime still validates builtin arguments defensively.

Semantic analysis also uses builtin metadata to catch invalid builtin calls before execution.

## Runtime values

Current runtime value type:

```cpp
std::variant<int, std::string, bool, StructValuePtr>
```

Supported runtime values:

- `int`
- `string`
- `bool`
- runtime struct values

Runtime struct values store:
- struct type name
- field values by field name

## Runtime value cloning
Velo structs use value semantics at the language level.

Runtime struct values are represented with `StructValuePtr`, but the interpreter
uses `Runtime::cloneValue(...)` at the value boundaries to avoid accidental aliasing.

`cloneValue(...)` behavior:
- primitive values are copied directly
- struct values are deep-copied recursively
- nested struct fields are also cloned

Important interpreter boundaries:
- `LoadLocal`
- `StoreLocal`
- function arguments
- function return values
- struct literal fields
- field access results

This prepares the runtime for future field assignment support.

Example intended behavior after field assignment is implemented:
```velo
let a: User = User {
    id: 1
};

let b: User = a;
b.id = 2;
```
Expected language-level result:
```text
a.id == 1
b.id == 2
```

## Interpreter

The interpreter executes IR instructions.

It uses:

- operand stack
- local slots
- instruction pointer
- current IR module pointer
- runtime builtin registry

## Locals

Function parameters and local variables are stored in local slots.

IR instructions:

```text
LoadLocal
StoreLocal
```

read and write local slots.

## Struct values
Struct literals are lowered to `BuildStruct`.

At runtime, `BuildStruct` consumes field values from the operand stack and creates a runtime struct value.

Example source:
```velo
let user: User = User {
    id: 42,
    name: "John Doe"
};
```

The resulting runtime value contains:
```text
typeName = User
fields["id"] = 42
fields["name"] = "John Doe"
```

## Field access
Field access is lowered to `LoadField`.

Example source:
```velo
return user.id;
```

Runtime behavior:
1. load the struct value onto the operand stack
2. execute `LoadField id`
3. replace the struct value with the selected field value

Invalid field access should normally be caught be semantic analysis before runtime.

Runtime still checks shape and value type defensively.

## Field assignment

Field assignment is executed through `StoreFieldPath`.

Runtime stack shape before `StoreFieldPath`:

```text
[..., assignedValue, rootStructValue]
```

Runtime behavior:

1. pop root struct value
2. pop assigned value
3. walk the field path inside the root struct value
4. replace the leaf field with a cloned assigned value
5. push the updated root struct value back onto the stack

The following IR shape stores the updated root value back into the local slot:

```text
PushInt 42
LoadLocal local[0]
StoreFieldPath id
StoreLocal local[0]
```

This works with value semantics because `LoadLocal` and `StoreLocal` clone struct values at value boundaries.

## Calls

User-defined function calls use:

```text
CallFunction
```

Builtin calls use:

```text
CallBuiltin
```

Qualified calls are lowered to builtin calls.

Single-segment calls are lowered to user-defined function calls.

## Program exit code

The interpreter returns `ExecutionResult`.

It contains:

- success flag
- exit code
- runtime error message
- optional return value

For `main(): int`, returning an integer produces the process exit code through `DriverResult.exitCode`.

## Integer arithmetic instructions

The VM supports integer arithmetic instructions:

- `AddInt`
- `SubInt`
- `MulInt`
- `DivInt`
- `ModInt`
- `NegInt`

Division and modulo by zero are runtime errors.

## Comparison instructions

The VM supports integer comparison instructions:

- `CompareEqualInt`
- `CompareNotEqualInt`
- `CompareLessInt`
- `CompareGreaterInt`
- `CompareLessEqualInt`
- `CompareGreaterEqualInt`

These instructions produce `bool`.

## Logical instructions

The VM supports:

- `LogicalNot`
- `LogicalAnd`
- `LogicalOr`

High-level `&&` and `||` are lowered with short-circuit jumps.

The eager logical opcodes may still be used by low-level tests or future compiler passes.

## Control flow

The VM supports:

- `JumpIfFalse`
- `Jump`

`if`, `while`, `break`, `continue`, `&&`, and `||` are lowered into jumps.

## Short-circuit logical lowering

`a && b`:

```text
a
JumpIfFalse false_branch
b
Jump end
false_branch:
PushBool false
end:
```

`a || b`:

```text
a
JumpIfFalse right_branch
PushBool true
Jump end
right_branch:
b
end:
```

This prevents unnecessary evaluation of the right-hand side.

## CLI run mode

Runtime execution happens only in `run` mode.

These modes do not execute the program:

- `check`
- `ast`
- `ir`

Therefore, runtime errors such as division by zero are not triggered in those modes.

## Equality comparison
Runtime equality is executed by generic equality opcodes:

```text
CompareEqual
CompareNotEqual
```

Supported runtime value pairs:

- `int` with `int`
- `string` with `string`
- `bool` with `bool`

The interpreter rejects mismatched or unsupported runtime 
equality operands defensively, but such cases should normally be caught 
by semantic analysis before execution.

## Runtime array values
Runtime arrays are represented as `ArrayValuePtr`.

A runtime array stores:
- ordered element values
- cloned elements at value boundaries

`Runtime::cloneValue(...)` deep-copies arrays recursively.

This preserves value semantics for arrays in the same way as structs.

## Array indexing
Array indexing is executed by `LoadIndex`.

Runtime stack shape before `LoadIndex`.

```text
[..., arrayValue, indexValue]
```

Runtime behavior:
1. pop index value
2. pop array value
3. validate that index is `int`
4. validate that array value is a runtime array
5. check bounds
6. push a cloned element value

Out-of-range access is runtime error.

## Array element assignment

Array element assignment is executed by `StoreIndexPath`.

Runtime stack shape before `StoreIndexPath`:

```text
[..., assignedValue, rootArrayValue, index1, index2, ...]
```

Runtime behavior:

1. pop index values
2. pop root array value
3. pop assigned value
4. walk nested arrays by index path
5. replace the leaf element with a cloned assigned value
6. push the updated root array value back onto the stack

The following IR shape stores the updated root array back into the local slot:

```text
PushInt 42
LoadLocal local[0]
PushInt 0
StoreIndexPath indexes=1
StoreLocal local[0]
```

Out-of-range assignment is a runtime error.

## Array builtins
Runtime provides the `array::len` builtin.

Runtime behavior:

1. validate one argument
2. validate that argument is `ArrayValuePtr`
3. return array element count as `int`

Example:

```velo
array::len([1, 2, 3])
```

returns:

```text
3
```

## Runtime map values
Runtime maps are represented as `MapValuePtr`.

A runtime map stores:
- string keys
- runtime values

`Runtime::cloneValue(...)` deep-copies maps recursively.

This preserves value semantics for maps in the same way as structs and arrays.

## Map indexing
Map indexing is executed by `LoadIndex`.

Runtime stack shape before `LoadIndex`:
```text
[..., mapValue, keyValue]
```

Runtime behavior:
1. pop key value
2. pop map value
3. validate that key is `string`
4. validate that target is a runtime map
5. look up the key
6. push a cloned map entry value

Missing keys are runtime errors.

## Map element assignment
Map element assignment is executed by `StoreIndexPath`.

`StoreIndexPath` is a generic index-path store operation.

Runtime stack shape before `StoreIndexPath`:
```text
[..., assignedValue, rootValue, index1, index2, ...]
```

Runtime behavior:
1. pop index values
2. pop root value
3. pop assigned value
4. walk the index path
5. use array behavior for `ArrayValuePtr + int`
6. use map behavior for `MapValuePtr + string`
7. replace or insert the leaf value
8. push the updated root value back onto the stack

Leaf map assignment inserts missing keys.

Non-leaf missing map keys are runtime errors.

## Map builtins
Runtime provides the `map::len` builtin.

Runtime behavior:
1. validate one argument
2. validate that argument is `MapValuePtr`
3. return map entry count as `int`

Example:
```velo
map::len(scores)
```
returns the number of keys in the map.

## JSON builtins
Runtime provides the `json::stringify` builtin.

`json::stringify` supports recursive serialization for:
- `int`
- `string`
- `bool`
- arrays
- maps
- structs

Runtime behavior:
1. validate one argument
2. serialize supported primitive value to JSON text
3. return the JSON text as `string`

Runtime behavior for arrays:
1. validate runtime value is `ArrayValuePtr`
2. serialize each element recursively
3. join elements with commas
4. return JSON array text

Runtime behavior for maps:
1. validate runtime value is `MapValuePtr`
2. serialize each key as a JSON string
3. serialize each value recursively
4. join entries with commas
5. return JSON object text

Runtime behavior for structs:
1. validate runtime value is `StructValuePtr`
2. iterate runtime fields
3. serialize field names as JSON object keys
4. serialize field values recursively
5. return JSON object text

Struct field order follows the runtime field storage order.

## Runtime JSON values
Runtime JSON values are represented as `JsonValuePtr`.

Supported JSON value kinds:
- null
- bool
- string
- int
- array
- object

`Runtime::cloneValue(...)` deep-copies JSON values recursively.

## JSON parse builtin
`json::parse` parses JSON text and returns a runtime JSON value.

Runtime behavior:
1. validate one string argument
2. parse JSON text
3. return `JsonValuePtr`
4. return runtime error or invalid JSON input

The MVP parser supports integers but not floats.

## JSON object access helpers
Runtime JSON object access helpers work on `JsonValuePtr`.

Supported helpers:
- `json::has`
- `json::get_int`
- `json::get_string`
- `json::get_bool`
- `json::get_json`

Runtime behavior:
1. validate target is `JsonValuePtr`
2. validate target kind is object
3. validate key is string
4. locate object field
5. validate field type for typed getters
6. return the corresponding Velo value

`json::get_json` returns a deep copy of the selected JSON subtree.

## Runtime HTTP values
Runtime HTTP request values are represented as `HttpRequestValuePtr`.

Runtime HTTP response values are represented as `HttpResponseValuePtr`.

HTTP request value fields:
```text
method
path
headers
body
```

HTTP response value fields:
```text
status
headers
body
```

`Runtime::cloneValue(...)` deep-copies HTTP request and response values.

Debug string output:
```text
<http_request METHOD PATH>
<http_response STATUS len=BODY_LENGTH>
```

Runtime registers response builders in `std::http`.

Builtins:
```text
http::response
http::text_response
http::json_response
```

`http::response` creates a response with status and raw body.

`http::text_response` creates a response with:
```text
Content-Type: text/plain
```

`http::json_response` serializes a runtime `json` value and creates a response with:
```text
Content-Type: application/json
```

## HTTP response access helpers
Runtime registers response access helpers in `std::http`.

Builtins:
```text
http::status
http::body
http::has_header
http::header
```

Runtime behavior:
- `http::status` returns `HttpResponseValue::status`
- `http::body` returns `HttpResponseValue::body`
- `http::has_header` checks `HttpResponseValue::headers`
- `http::header` returns a header value or reports a runtime error

## HTTP request helpers
Runtime registers request helpers in `std::http`.

Builtins:
```text
http::request
http::method
http::path
http::request_body
http::json_body
```

`http::request` creates an `HttpRequestValuePtr` with method, path, body, and
empty headers.

`http::json_body` parses request body using the runtime JSON parser and returns
a `JsonValuePtr`.

Invalid JSON request body is a runtime error.

## HTTP and JSON integration
MVP 0.8 combines HTTP runtime values with runtime JSON values.

Typical flow:
```text
http_request
    -> http::json_body
    -> json access helpers
    -> http::json_response
    -> http_response
```

This flow does not require a real network server yet.

The runtime can execute handler-like functions that accept `http_request`
and return `http_response`.

## HTTP parser/serializer foundation
MVP 0.9 adds a reusable HTTP message layer.

Namespace:
```text
Velo::Http
```

Core functions:
```text
parseHttpRequest(raw)
serializeHttpResponse(response)
reasonPhraseForStatus(status)
```

The parser converts raw HTTP request text into `HttpRequestValuePtr`.

The serializer converts `HttpResponseValue` into raw HTTP response text.

This layer is intentionally independent from CLI serve mode.

Future entry points should reuse it:
```text
velo serve app.velo
http::listen(...)
```

Current limitations:
- no socket server in 0.9.1
- no chunked transfer encoding
- no keep-alive
- no TLS

## HTTP request pipeline
MVP 0.9.3 introduces a reusable raw HTTP request pipeline.

Namespace:
```text
Velo::Http
```

Core function:
```text
handleRawHttpRequest(interpreter, module, rawRequest, handlerName)
```

The pipeline composes the lower-level HTTP pieces:
```text
raw HTTP request
    -> parseHttpRequest(raw)
    -> executeHttpHandler(...)
    -> serializeHttpResponse(response)
    -> raw HTTP response
```

Malformed raw requests produce a serialized `400 Bad Request` response.

Handler execution failures produce a serialized `500 Internal Server Error` response.

This layer is intentionally independent from sockets and CLI server mode.
The next sub-step will use it from the blocking HTTP server log.