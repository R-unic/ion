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
    - [x] Range literals (see examples)
    - [ ] Vector literals (see examples)
    - [ ] Color literals (see examples)
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
    - [x] Instance constructors (see examples)
        - [x] Name declarator
        - [x] Tag declarator
        - [x] Attribute declarator
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
    - [ ] Shorthand attributes (see examples)

## Examples

### Events

```rs
event data_changed<T>(T);

fn on_data_change<T>(new_data: T): void {
  print("New data: {new_data}");
}

data_changed += on_data_change;

let data = 420;
data_changed!(data);

data_changed -= on_data_change
```

This is equivalent to the following in Luau:

```luau
local data_changed = Signal.new()
function on_data_change<T>(new_data: T): ()
    print(`New data: {new_data}`)
end

local on_data_change_conn = data_changed:Connect(on_data_change)

local data = 420
data_changed:Fire(data)

on_data_change_conn:Disconnect()
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

```luau
local my_part = Instance.new("Part")
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
let health = zombie_model@Health
print(health)
zombie_model@Health -= 10
```

This is equivalent to the following in Luau:

```luau
local health = zombie_model:GetAttribute("Health")
print(health)
zombie_model:SetAttribute("Health", zombie_model:GetAttribute("Health") - 10)
```

### Enums

```rs
enum Abc {
    A
    B
    C
    D = 69
    E
}

print(Abc::A, Abc::B, Abc::C, Abc::D, Abc::E)
```

This is equivalent to the following in Luau:

```luau
print(0, 1, 2, 69, 70)
```