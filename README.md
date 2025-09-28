# Ion

Statically typed language built for use with Roblox that compiles to Luau.

> [!IMPORTANT]
> This language **does not have a functional prototype yet**. Check out [the roadmap](#roadmap) to see current progress.
> Check out [concept examples](#concept-examples) to see what's in store for the future.

## Roadmap

- [x] Lexer
- [ ] Diagnostics
    - [x] Errors
    - [ ] Warnings
    - [ ] Colors
    - [x] Line info/code view
- [x] Parser
    - [x] Primitive literals
    - [x] Range literals
    - [ ] Vector literals
    - [ ] Color literals
        - [ ] RGB
        - [ ] HSV
        - [ ] Hex
    - [x] Identifiers
    - [x] Binary, unary, postfix unary, ternary, & assignment operations
    - [x] Parenthesized expressions
    - [x] Member access & element access
    - [x] Invocation
    - [x] Variable declarations
    - [x] Event declarations
    - [x] Function declarations
    - [x] Instance constructors
        - [ ] Attribute declarator
        - [ ] Children declarator
    - [ ] Enum declarations
    - [x] Basic type parameters
        - [ ] Defaults
    - [x] If/while statements
    - [ ] Switch statements
    - [ ] For statements
    - [x] Imports & exports
    - [x] Breaks/continues
    - [x] Returns
    - [x] Blocks
    - [x] Primitive types, type names, & nullable types
    - [x] Type parameters
    - [ ] String interpolation

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

### Instance construction

```swift
instance my_part: Part {
  "MyPart"
  Size: Vector3.one
  Position: <0, 10, 0>
  Color: #ff0000
  
  #Lava
  @LavaKind: "very very hot"
} -> game.Workspace
```

This is equivalent to the following in Luau:

```lua
local my_part = Instance.new("Folder")
my_part.Name = "MyPart"
my_part.Size = Vector3.one
my_part.Position = Vector3.new(0, 10, 0)
my_part.Color = Color3.fromRGB(255, 0, 0) -- #ff0000 is parsed at compile-time to avoid .fromHex()
my_part:SetAttribute("LavaKind", "very very hot")
my_part.Parent = game.Workspace
my_part:AddTag("Lava")
```

### Shorthand attributes

```rs
zombie_model
let health = zombie_model@Health
print(health)
zombie_model@Health = 0
```

This is equivalent to the following in Luau:

```lua
local my_part = Instance.new("Folder")
my_part.Name = "MyPart"
my_part.Size = Vector3.one
my_part.Position = Vector3.new(0, 10, 0)
my_part.Parent = game.Workspace
```