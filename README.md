# Ion

Statically typed language built for use with Roblox that compiles to Luau.

> [!IMPORTANT]
> This language **does not have a functional prototype yet**. Check out [the roadmap](#roadmap) to see current progress.
> Check out [concept examples](#concept-examples) to see what's in store for the future.

## Roadmap

- [x] Lexer
- [x] Parser
  - [x] Literals & identifiers
  - [x] Binary, unary, & assignment operations
  - [x] Member access
  - [x] Invocation
  - [x] Variable declarations
  - [x] If/while statements
  - [x] Imports

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
