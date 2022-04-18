# Identifiers

Two basic types: global and local. Global identifiers (functions, global variables) begin with the `@` character. Local identifiers (register names) begin with the `%` character.

Additionally, there are three different formats for identifiers, for different purposes:

- Named values, with regular expression `[%@][-a-zA-Z$._][-a-zA-Z$._0-9]*`.

- Unnamed values, with regular expression `[%@][0-9]+`.

  Unnamed temporaries are numbered sequentially (using a per-function incrementing counter, starting with 0). Note that basic blocks and unnamed function parameters are included in this numbering.

- Constants



# Type System

### Void Type

The void type does not represent any value and has no size.

Syntax:

```
void
```

### Function Type

The function type can be thought of as a function signature. It consists of a return type and a list of formal parameter types. The return type of a function type is a void type or first class type — except for label type.

Syntax:

```
<returntype> (<parameter list>)
```

…where `<parameter list>` is a comma-separated list of type specifiers.

### First Class Type

Values of first class types are the only ones which can be produced by instructions.

#### Single Value Type

These are the types that are valid in registers.

##### Integer Type

The integer type is a very simple type that simply specifies an arbitrary bit width for the integer type desired.

Syntax:

```
iN
```

The number of bits the integer will occupy is specified by the `N` value.

##### Pointer Type

The pointer type is used to specify memory locations. Pointers are commonly used to reference objects in memory.

Syntax:

```
<type>*
```

#### Label Type

The label type represents code labels.

Syntax:

```
lable
```

#### Aggregate Type

Aggregate Types are a subset of derived types that can contain multiple member types.

##### Array Type

The array type is a very simple derived type that arranges elements sequentially in memory. The array type requires a size (number of elements) and an underlying data type.

Syntax:

```
[<# elements> x <elementtype>]
```



# Constants

### Simple Constants

##### Integer Constants

Standard integers (such as `4`) are constants of the integer type.

### Complex Constants

##### Array Constants

Array constants are represented with notation similar to array type definitions, for example `[ i32 42, i32 11, i32 74 ]`. Array constants must have array type, and the number and types of elements must match those specified by the type.

##### Zero Initialization

The string `zeroinitializer` can be used to zero initialize a value to zero of *any* type, including scalar and aggregate types. This is often used to avoid having to print large zero initializers (e.g. for large arrays) and is always exactly equivalent to using explicit zero initializers.

### Global Variable and Function Addresses:

The addresses of global variables and functions are always implicitly valid (link-time) constants. These constants are explicitly referenced when the identifier for the global is used and always have pointer type. For example:

```
@X = global i32 17
@Y = global i32 42
@Z = global [2 x i32*] [ i32* @X, i32* @Y ]
```



# Module

LLVM programs are composed of `Module`’s, each of which is a translation unit of the input programs. Each module consists of some target information, global symbols and other stuff.

The global symbol part consist of some global variables, function declarations and function definitions.

### Global Variables

Global variables define regions of memory allocated at compilation time instead of run-time.

Global variable definitions must be initialized.

A variable may be defined as a global `constant`, which indicates that the contents of the variable will never be modified (enabling better optimization).

As SSA values, global variables define pointer values that are in scope (i.e. they dominate) all basic blocks in the program. Global variables always define a pointer to their “content” type because they describe a region of memory, and all memory objects in LLVM are accessed through pointers.

Syntax:

```
@<GlobalVarName> = <global | constant> <Type> [<InitializerConstant>]
```

### Function Declaration

LLVM function declarations consist of the `declare` keyword, a return type, a function name, and a possibly empty list of arguments.

Syntax:

```
declare <ResultType> @<FunctionName> ([argument list])
```

The argument list is a comma separated sequence of arguments’ type.

### Function Definition

LLVM function definitions consist of the `define` keyword, a return type, a function name, a possibly empty argument list, an opening curly brace, a list of basic blocks, and a closing curly brace.

Syntax:

```
define <ResultType> @<FunctionName> ([argument list]) { ... }
```

The argument list is a comma separated sequence of arguments’ type and name.

> A function definition contains a list of **basic blocks**, forming the CFG (Control Flow Graph) for the function. Each basic block may optionally start with a label (giving the basic block a symbol table entry), contains a list of instructions, and ends with a terminator instruction (such as a branch or function return).
>
> If an explicit label name is not provided, a block is assigned an implicit numbered label, using the next value from the same counter as used for unnamed temporaries. For example, if a function entry block does not have an explicit label, it will be assigned label “%0”, then the first unnamed temporary in that block will be “%1”, etc.
>
> The first basic block in a function is special in two ways: it is immediately executed on entrance to the function, and it is not allowed to have predecessor basic blocks (i.e. there can not be any branches to the entry block of a function).



# Instructions

### Terminator Instructions

Every basic block in a program ends with a “Terminator” instruction, which indicates which block should be executed after the current block is finished. These terminator instructions typically yield a `void` value: they produce control flow, not values.

##### `ret` Instruction

The `ret` instruction is used to return control flow (and optionally a value) from a function back to the caller.

The type of the return value must be a first class type.

Syntax:

```
ret <type> <value>
ret void
```

##### `br` Instruction

The `br` instruction is used to cause control flow to transfer to a different basic block in the current function.

Syntax:

```
br i1 <cond>, lable <iftrue>, lable <iffalse>
br lable <dest>
```

### Binary Operations

Binary operators are used to do most of the computation in a program.

They require two operands of the same type, execute an operation on them, and produce a single value. The result value has the same type as its operands.

##### `add` Instruction

Syntax:

```
<result> = add <ty> <op1>, <op2>
```

##### `sub` Instruction

Syntax:

```
<result> = sub <ty> <op1>, <op2>
```

##### `mul` Instruction

Syntax:

```
<result> = mul <ty> <op1>, <op2>
```

##### `sdiv` Instruction

Syntax:

```
<result> = sdiv <ty> <op1>, <op2>
```

##### `srem` Instruction

Syntax:

```
<result> = srem <ty> <op1>, <op2>
```

### Bitwise Binary Operations

Bitwise binary operators are used to do various forms of bit-twiddling in a program.

They require two operands of the same type, execute an operation on them, and produce a single value. The resulting value is the same type as its operands.

#### `and` Instruction

```
<result> = and <ty> <op1>, <op2>
```

#### `or` Instruction

```
<result> = or <ty> <op1>, <op2>
```

### Memory Access and Addressing Operations

A key design point of an SSA-based representation is how it represents memory. In LLVM, no memory locations are in SSA form, which makes things very simple.

#### `alloca` Instruction

The `alloca` instruction allocates `sizeof(<type>)` bytes of memory on the runtime stack, returning a pointer of the appropriate type to the program.

The allocated memory is uninitialized.

Syntax:

```
<result> = alloca <ty> [, align <alignment>]
```

#### `load` Instruction

The location of memory pointed to is loaded. The type specified must be a first class type of known size.

Syntax:

```
<result> = load <ty>, <ty>* <pointer>
```

#### `store` Instruction

The contents of memory are updated to contain `<value>` at the location specified by the `<pointer>` operand. The type of the `<pointer>` operand must be a pointer to the first class type of the `<value>` operand.

Syntax:

```
store <ty> <value>, <ty>* <pointer>
```

#### `getelementptr` Instruction

The `getelementptr` instruction is used to get the address of a subelement of an aggregate data structure. It performs address calculation only and does not access memory.

The first argument is always a type used as the basis for the calculations. The second argument is always a pointer, and is the base address to start from.

The first index dose NOT change the returning pointer type. and it offsets by the pointee type.

The further indices offset inside aggregate types, and change the returning pointer type by removing one layer of “aggregation”.

Syntax:

```
<result> = getelementptr <ty>, <ty>* <ptrval>{, <ty> <idx>}*
```

### Conversion Operations

The instructions in this category are the conversion instructions (casting) which all take a single operand and a type. They perform various bit conversions on the operand.

#### `zext .. to` Instruction

The `zext` instruction zero extends its operand to type `ty2`. Both types must be of integer types, and the bit size of the `value` must be smaller than the bit size of the destination type `ty2`.

Syntax:

```
<result> = zext <ty> <value> to <ty2>
```

#### `bitcast .. to` Instruction

The `bitcast` instruction converts `value` to type `ty2` without changing any bits.

The `bitcast` instruction takes a value to cast, which must be a non-aggregate first class value, and a type to cast it to, which must also be a non-aggregate first class type. The bit sizes of `value` and the destination type `ty2`, must be identical. If the source type is a pointer, the destination type must also be a pointer of the same size.

Syntax:

```
<result> = bitcast <ty> <value> to <ty2> 
```

### Other Operations

#### `icmp` Instruction

The `icmp` instruction returns a boolean value.

The two operand must be integer or pointer typed. They must also be identical types.

Syntax:

```
<result> = icmp <cond> <ty> <op1>, <op2>	; <cond> = eq | ne | ugt | uge | ule | sgt | sge | slt | sle
```

#### `phi` Instruction

At runtime, the `phi` instruction logically takes on the value specified by the pair corresponding to the predecessor basic block that executed just prior to the current block.

There must be no non-phi instructions between the start of a basic block and the PHI instructions: i.e. PHI instructions must be first in a basic block.

Syntax:

```
<result> = phi <ty> [ <val0>, <label0>], ...
```

#### `call` Instruction

The `call` instruction is used to cause control flow to transfer to a specified function, with its incoming arguments bound to the specified values.

Syntax:

```
<result> = call <ty> <fnptrval>(<function args>)
```

