# Ion

Statically typed language built for Roblox that compiles to Luau.

> [!IMPORTANT]
> This language **does not have a functional prototype yet**. Check out [the roadmap](#roadmap) to see current progress.
> Check out [examples](#examples) to see some previews.

## Contributing

Before contributing please read up on how programming language front-ends
work. [Crafting interpreters](https://craftinginterpreters.com/contents.html) is a good start.
Try to adhere to the project's code style as much as possible, and follow general principles of writing clean and
maintainable code.
[Refactoring guru](https://refactoring.guru/refactoring) is a good place to learn principles of writing clean code.

For any bugs please create an issue, describe it descriptively, and include how to reproduce it.

### When contributing code

- Use a commit convention when writing commit names
- Add only one feature per PR, bloated PRs are a lot harder to review and accept.

## Roadmap

- [ ] Unit testing
- [x] Diagnostics
    - [x] Errors
    - [x] Warnings
    - [x] Colors
    - [x] Line info/code view
- [x] Lexer
- [x] Parser
    - [x] Skip semicolons
    - [x] Warn when `if a = b` or `while a = b` is detected
    - [x] Warn if any statements come after a `return` statement
    - [x] Primitive literals
    - [x] Extended number literals (see examples)
    - [x] Array literals
    - [ ] Object literals
    - [x] Range literals (see examples)
    - [x] Vector literals (see examples)
    - [x] Color literals (see examples)
        - [x] RGB
        - [x] HSV
    - [x] Identifiers
    - [x] Binary, unary, postfix unary, ternary, & assignment operations
    - [x] Null coalescing
    - [ ] Optional member access
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
        - [x] `clone` clause
    - [x] Enum declarations
    - [x] Interface declarations
    - [x] Basic type parameters
        - [x] Defaults
    - [x] If statements
    - [x] For loops (see examples)
    - [x] While loops
    - [x] Repeat loops
    - [ ] Match statements & expressions (see examples)
    - [x] Imports & exports (see examples)
    - [x] After statements (see examples)
    - [x] Every statements (see examples)
    - [ ] Destructuring (see examples)
    - [x] Async/await
    - [x] Breaks/continues
    - [x] Returns
    - [x] Blocks
    - [x] Primitive types, type names, & nullable types
    - [x] Union & intersection types
    - [x] Literal types
    - [x] Array types
    - [x] Tuple types
    - [ ] Function types
    - [x] Type parameters
    - [x] Type arguments for calls and type names
    - [x] Type alias declarations
    - [x] `typeof`
    - [x] `nameof` (see examples)
        - [ ] On member access
    - [x] String interpolation
    - [x] Shorthand attributes (see examples)
    - [x] Constant variables and fields
- [ ] Scope resolution
- [ ] Symbol binding
- [ ] Type solving/checking
- [ ] Luau transpilation (Ion AST -> Luau AST)
- [ ] Luau rendering (Luau AST -> Luau source)

## Examples

### Events

```rs
event data_changed<T>(T)

fn on_data_change<T>(new_data: T): void {
  print("New data: %{new_data}")
}

data_changed += on_data_change

let data = 420
data_changed!(data)

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
  Position: <(0, 10, 0)>
  Color: rgb<(255, 0, 0)>
  
  #Lava
  @LavaKind: "very very hot"
} -> game.Workspace
```

```luau
local my_part = Instance.new("Part")
my_part.Name = "MyPart"
my_part.Size = Vector3.one
my_part.Position = Vector3.new(0, 10, 0)
my_part.Color = Color3.fromRGB(255, 0, 0)
my_part:SetAttribute("LavaKind", "very very hot")
my_part.Parent = game.Workspace
my_part:AddTag("Lava")
```

You can also clone instances using this syntax.

```php
instance zombie_model: Model clone ReplicatedStorage.ZombieModel -> game.Workspace
```

```luau
local zombie_model = ReplicatedStorage.ZombieModel:Clone()
my_part.Parent = game.Workspace
```

### Shorthand attributes

```rs
let const health = zombie_model@Health
print(health)
zombie_model@Health -= 10
```

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

```luau
print(0, 1, 2, 69, 70)
```

### Imports/exports

```zig
export let const x = 69;
```

```luau
local x = 69

return {
  x = x
}
```

### Improved for loops

For loops can be used in combination with range literals, and when used with range literals they emit a Luau for-i loop.

```rs
for i : 1..10
  print(i)
```

```luau
for i = 1, 10 do
  print(i)
end
```

### Extended number literals

```rs
let const time = 10s
let const cooldown = 50ms
let const hour = 1h
let const update_rate = 20hz
let const transparency = 50%
```

```luau
local time = 10
local cooldown = 0.05
local hour = 3600
local update_rate = 0.333333333
local transparency = 0.5
```

### `after` statements

`after` statements are a direct syntactic equivalent to `task.delay()`

```swift
after 100ms
  print("delayed result")
```

```luau
task.delay(0.1, function()
  print("delayed result")
end)
```

### `every` statements

`every` statements are a direct syntactic equivalent to an async loop that waits in every iteration

```rs
every 1s
  print("one second passed");
```

```luau
task.spawn(function()
  while true do
    print("one second passed")
    task.wait(1)
  end
end)
```

Using a conditional `every` statement:

```rs
let elapsed = 0;
every 1s while elapsed < 10 {
  print("Elapsed time:", elapsed++);
}
```

```luau
task.spawn(function()
  while elapsed < 10 do
    local _original = elapsed
    elapsed += 1
    print("Elapsed time:", _original)
    task.wait(1)
  end
end)
```

### `match` statements

```rs
enum Abc {
  A
  B
  C
}

let abc = Abc.A;
match abc {
  Abc.A -> print("got a"),
  Abc.B -> print("got b"),
  Abc.C -> print("got c"),
  value -> print("wtf is this:", value)
}
```

```luau
local abc = 0
if abc == 0 then
  print("got a")
else if abc == 1 then
  print("got b")
else if abc == 2 then
  print("got c")
else
  local value = abc
  print("wtf is this:", value)
end
```

### `match` expressions

```rs
enum Abc {
  A
  B
  C
}

let abc = Abc.A;
let message = abc match {
  Abc.A -> "got a",
  Abc.B -> "got b",
  Abc.C -> "got c",
  value -> "wtf is this: " + value
}
```

```luau
local abc = 0
local message
if abc == 0 then
  message = "got a"
else if abc == 1 then
  message = "got b"
else if abc == 2 then
  message = "got c"
else
  local value = abc
  message = "wtf is this: " .. value
end
```

### `nameof` expression

`nameof` returns the text of the token it is given.

```rs
let abc = 69
fn foo -> null
interface FooBar {}
enum Abc {}

print(nameof(abc))
print(nameof(foo))
print(nameof(FooBar))
print(nameof(Abc))
```

```luau
local abc = 69
local function foo()
  return nil
end

print("abc")
print("foo")
print("FooBar")
print("Abc")
```

In the future

```rs
enum Abc { A B C }

print(Abc)
print(Abc::A)
print(Abc::B)
print(Abc::C)
```

```luau
print("Abc")
print("A")
print("B")
print("C")
```

### Destructuring

Syntactic sugar for assigning variable names to properties/indexes of objects/arrays

```rs
let const {my_field} = my_obj
let const [one, two] = my_arr
```

```luau
local my_field = my_obj.my_field
local one = my_arr[1]
local two = my_arr[2]
```
