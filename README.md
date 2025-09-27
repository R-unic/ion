# Ion

Statically typed language built for use with Roblox that compiles to Luau.

> [!IMPORTANT]
> This language **does not have a functional prototype yet**. Check out [the roadmap](#roadmap) to see current progress.
> Check out [concept examples](#concept-examples) to see what's in store for the future.

## Roadmap

- [x] Lexer
- [x] Parser
    - [x] Literals & identifiers
    - [x] Binary, unary, postfix unary, ternary, & assignment operations
    - [x] Parenthesized expressions
    - [x] Member access & element access
    - [x] Invocation
    - [x] Variable declarations
    - [x] If/while statements
    - [x] Imports & exports
    - [x] Breaks/continues
    - [x] Returns
    - [x] Blocks
    - [x] Primitive types, type names, & nullable types

## Concept Examples

### Events

```rs
event data_changed<T>(T);

fn on_data_change<T>(new_data: T): void {
  print("New data: {new_data}");
}

data_changed += on_data_change;

let data = 69;
// do something

data = 420;
data_changed!(data);
```

### Instance creation

```swift
instance my_folder: Folder {
  Name: "MyFolder",
  Parent: ReplicatedStorage
}
```
