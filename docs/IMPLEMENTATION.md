<h1 align="center">The Pallene Tracer</h1>
<p align="center">This document records the specifications of Pallene Tracer and its API</p>
<p>
<div align= "center">
    <a href= "#overview">Overview</a>
    •
    <a href= "#implementation">Implementation</a>
    •
    <a href= "#mechanism">Mechanism</a>
    •
    <a href= "#api">API</a>
</div>

## 1. Overview
The Lua call-stack is great for tracing and debugging Lua calls. But when it comes to tracing C function calls, the scenario is different. The builtin Lua call-stack has no features regarding storing line number information for C call-frames and traces of C functions called from another C function is entirely missing from the Lua call-stack.

Pallene Tracer aims to fix that issue without patching the Lua internals. This is done so by maintaining a separate call-stack of our own, the **`Pallene Tracer call-stack`**.

This section provides a high-level overview on how Pallene Tracer works. For precise details regarding how it works in execution level, please go to the <a href="#implementation">Implementation</a> section.

 > **Note:** These assumptions are enforced to reduce redundancy and best reading experience.
### \*Noteworthy Assumptions:
 - Any non-explicit mention of **call-stack** should be thought of as the **Pallene Tracer call-stack**.
 - Standard C functions distinct from Lua C function (`lua_CFunction`) signature are referred to as Generic/Normal C functions.
 - C function calls having traces in both Lua and Pallene Tracer call-stack are referred to as **Lua interface frames** (or black frames), generally the Lua C function calls.
 - C function calls having traces only in call-stack are referred to as **C interface frames** (or white frames). These are generic C functions only traced by Pallene Tracer.
 - The sign `->` is contextualized. If used against functions in such convention: `caller() -> callee()` (e.g. `a() -> b()`), it would denote `caller` function has called the `callee` function. Here the sign should be read as **_called_**, e.g. `a()` called `b()`.\
 \
 If used against environments in such convention: `Caller enviornment -> Callee environment`, it would denote situations when any function of caller environment is calling any function from callee environment such that we were to do something with this information. E.g. `Lua -> C` would mean situation when any Lua function is calling any C function. Here the sign should be read as **_to_**, e.g. Lua to C or C to C.
 - Untracked frames are referred to frames present in Lua call-stack but absent in call-stack. Any mention of "**Tracked frame**" is synonymous to Lua interface frame. In simple terms, any Lua C call-frame traced by Pallene Tracer is a Tracked frame. It is untracked otherwise.
### 1.1 How it works
In **`pallene-tracer`**, we have a separate call-stack maintaining only C call-frames. The call-stack is used synchronously alongside with the Lua call-stack to generate a better stack-trace consisting of both Lua and C/Pallene frame traces.

To understand how it all fits, let's assume there is a Lua C module named `some_mod` with Pallene Tracer tracebacks enabled. A Lua `main.lua` script uses that module.

In the module and script combined:  
 - Six functions in `some_mod` module: `some_mod_c_fn()`, `some_mod_inner_a()`, `some_mod_inner_b()`, `some_mod_inner_c()`, `some_mod_recurse()` and  `some_mod_exit()`
 - Two functions in `main.lua` script: `some_lua_fn()` and `status_print()`
 - One untracked C function: `untracked_c_fn()`

The module and function names are obvious for simplicity sake. Suppose, from a high-level perspective the function calls look like this:

`Lua <main> chunk` &nbsp;&nbsp;&nbsp;&nbsp;&nbsp; -> &nbsp; `some_lua_fn()` &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; ->\
`some_mod_c_fn()` &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; -> &nbsp; `some_mod_inner_a()` ->\
`some_mod_inner_b()` &nbsp; -> &nbsp; `some_mod_inner_c()` ->\
`untracked_c_fn()` &nbsp;&nbsp;&nbsp;&nbsp;&nbsp; -> &nbsp; `some_mod_recurse()` ->\
`some_mod_exit()` &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; -> &nbsp; `status_print()`

Say a runtime error has been triggered in `status_print()` which needs to display the stack-trace. The Lua call-stack would look alike to the left stack of _Figure 1_. Using that call-stack, Lua would generate an error as following: 

> **Note:** The line numbers and module filename are assumed.
```
main.lua:69: Some random error to concern ourselves
stack traceback:
	main.lua:69: in function 'status_print'
	[C]: in function 'some_mod.some_mod_recurse'
	[C]: in ?
	[C]: in function 'some_mod.some_mod_c_fn'
	main.lua:37: in function 'some_lua_fn'
	main.lua:113: in main chunk
	[C]: in ?
```
The traceback above is far from great. The C calls are missing line number information and some are missing function name. Some C call traces are downright missing.

**But**, by integrating Pallene Tracer into `some_mod` module and using the `pallene-debug` script on `main.lua`, something like this is achievable: 
```
Runtime error: main.lua:69: Some random error to concern ourselves
Stack traceback:
    main.lua:69: in function 'status_print'
    some_mod.c:11: in function 'some_mod_exit'
    some_mod.c:8: in function 'some_mod_recurse'
    C: in function 'untracked_c_fn'
    some_mod.c:47: in function 'some_mod_inner_c'
    some_mod.c:33: in function 'some_mod_inner_b'
    some_mod.c:21: in function 'some_mod_inner_a'
    some_mod.c:18: in function 'some_mod_c_fn'
    main.lua:37: in function 'some_lua_fn'
    main.lua:113: in <main>
    C: in function 'xpcall'
    C: in function '<?>'
```
Which is more informative and clean. A data-structure is required to store the line number and function name information ourselves due to Luas lack of feature of storing relevant information for C call-frames. That is where Pallene Tracer self-maintained call-stack comes into play.

<figure>
<img src="assets/call-stack-visualization.png" type="image/png" />
<figcaption align="center"><small>Figure 1: Synchronous relation between Lua and Pallene Tracer call-stack</small></figcaption>
</figure>

> **Note:** Black and white frames are concepts solely bounded to Pallene Tracer and Pallene Tracer call-stack regardless of illustrations. The black frames are also illustrated in Lua call-stack primarily for better intuition. 

In the Figure above, Lua and Pallene Tracer call-stack can be seen side by side with call-stack having some extra frames absent in Lua call-stack. There are some boxes (frames) filled with black and also some red lines.

Pallene Tracer call-stack is synchronous to Lua call-stack, meaning that it copes up with Lua call-stack by storing common frames alongside with extra frames traced separately. The black boxes seen in the Figure are black frames (Lua interface frames), the red lines denote connection between Lua C frames and black frames between Lua and Pallene Tracer call-stack. 

The reason behind having two types of frames is because when our Pallene Tracers traceback function starts backtracing, it needs to utilize both of the call-stacks. The black frames act as _hints_ to the traceback function to switch back to Lua call-stack.

All the frames in call-stack are C frames storing information like function name, filename where the function belongs to and the current executing instruction line number. The line number is updated manually by the function in question prior to calling another function or invoking a runtime error.

### 1.2 Working Principle of Traceback Function

The builtin Lua traceback function will not take advantage of the separate self-maintained call-stack that Pallene Tracer have. Therefore, an explicit debug traceback function is used to display the stack-trace. This debug traceback function will mostly be used by the `pallene-debug` script. The function also can be used against `xpcall()` to generate stack-trace as well.

Below is a Figure mostly resembling the figure prior but with curvy red lines, blue dots and some red straight lines at the right.
<figure>
<img src="assets/traceback-visualization.png" type="image/png" />
<figcaption align="center"><small>Figure 2: Working mechanism of Pallene Tracer traceback function</small></figcaption>
</figure>
Unlike the builtin Lua traceback function, Pallene Tracer traceback function needs to deal with two separate but synchronous stacks simultaneously. But here's the elephant in the room. During backtrace how to know which call-stack to use and when?

To solve this problem, the concept of black and white frames is introduced. Frames which are common in both Pallene Tracer and Lua call-stack are black frames or Lua interface frames. The other frames in Pallene Tracer call-stack are referred to as white frames or C interface frames.

> **Intuition:** If a Lua C function compatible with Pallene Tracer gets called, it would create a call-frame in both of the call-stacks. A call-frame would be created in Lua call-stack when the function is invoked (by Lua or any variant of `lua_call`), then a black call-frame would be created in Pallene Tracer call-stack when the function executes. Any Pallene Tracer compatible normal C functions called onwards will create white call-frames only in call-stack during execution.

The traceback mechanism is rather simple. There are two stack pointers pointing at the topmost frame of the respective call-stack. Backtracing shall begin with Lua call-stack. Lua call-frames are printed if encountered. But upon encountering a C call-frame, **_black frame probing_** is done to check traces of the encountered frame in call-stack. If the probing is successful, the frame is tracked.

> **Note:** **_Black frame probing_** is the checking mechanism of a C call-frame of Lua call-stack having any traces in Pallene Tracer call-stack. It is generally the nearest Lua interface frame relative to current stack top. The mechanism is performed in a separate iteration. The black frame probing is denoted by the red straight lines at right of _Figure 2_ facing downwards. In simpler terms, black frame probing checks whether a C function is Tracked.

If the the C call-frame turns out to be tracked, immediate switch to Pallene Tracer call-stack takes place. Stack iteration starts from current stack top denoted by the stack pointer. White frames are printed upon encounter. In case of a black frame, it is printed and call-stack is reswitched to Lua call-stack.

But if the frame turns out to be untracked, it is printed as a simple C function without any line number information and next frame is processed. But the function name is printed if found any by <a href="#">global name deduction</a>.

When the traceback function is in action, it would seem like the a single pointer is hopping between frames in both call-stacks, denoted by the curvy lines in the middle of _Figure 2_. For every C frame we find in Lua call-stack, we do black frame probing. A blue dot can be perceived near the C call-frames of Lua call-stack denoting tracked C frames after successful probes for which we switch to Pallene Tracer call-stack.
### 1.3 The Untracked Frames

As aforementioned, upon encountering a C call-frame in Lua call stack, immediately probing is done to check whether the frame is "tracked". During the process, the nearest black frame is approached in Pallene Tracer call-stack to perform a match. If the match fails, the frame in question is "untracked".

This happens when a Lua C function gets called which is not traced by Pallene Tracer at runtime, resulting the call only creating a call-frame only in Lua call-stack.

> **Clarification:** For untracked C frames, the black frame we are comparing with in Pallene Tracer call-stack may represent some other C call-frame in Lua call-stack, which may get encountered in the future during backtrace. As illustrated in _Figure 1_, every black frame of call-stack is connected to a frame in Lua call-stack.
## 2. Implementation

TODO

## 3. Mechanism

TODO

## API
### Data Structures

Data structure of each frame in call-stack: 
```C
/* What type of frame we are dealing with. */
typedef enum frame_type {
    PALLENE_TRACER_FRAME_TYPE_C,
    PALLENE_TRACER_FRAME_TYPE_LUA
} frame_type_t;

typedef struct pt_frame {
    frame_type_t type;             // Frame type
    int line;                      // Current line we are at in the function

    union {
        pt_fn_details_t *details;  // Details for C interface frames
        lua_CFunction c_fnptr;     // The Lua C fn pointer for Lua interface frames
    } shared;
} pt_frame_t;
```

Data structure for holding the stack: 
```C
typedef struct pt_fnstack {
    pt_frame_t *stack;  // Heap allocated stack
    int count;          // Number of entries in the stack
} pt_fnstack_t;
```

### API Functions

```C
pt_fnstack_t *pallene_tracer_init(lua_State *L);
```
**Parameter:** A Lua state (`lua_State`)\
**Return Value:** A `pt_fnstack_t` structure containing the call-stack. A to-be-closed finalizer object is returned through the **Lua value-stack**, essential for Lua interface functions.

Initializes the Pallene Tracer. The initialization refers to creating the heap call-stack if not created, preparing the traceback function and finalizers. This function must only be called from Lua module entry point, `luaopen_*`.

> **Note:** This function may allocate the call-stack in the heap or return pre-allocated call-stack for the same Lua state.
<hr>

```C
void pallene_tracer_frameenter(lua_State *L, pt_fnstack_t *fnstack, pt_frame_t *restrict frame);
```
**Parameters:**
 - `lua_State *L`: The Lua state
 - `pt_fnstack_t *fnstack`: The call-stack
 - `pt_frame_t *restrict frame`: The frame to push to the call-stack

**Return Value:** None

Pushes a frame to the call-stack regardless of frame type. This function may trigger a call-stack overflow error.
<hr>

```C
void pallene_tracer_setline(pt_fnstack_t *fnstack, int line);
```
**Parameters:**
 - `pt_fnstack_t *fnstack`: The call-stack
 - `int line`: The line number to set

**Return Value:** None

Sets the line number to the topmost frame in the call-stack. This function should be called from C interface frames, prior to calling another function (Lua/C call) or triggering Lua runtime error.
<hr>

```C
void pallene_tracer_frameexit(pt_fnstack_t *fnstack);
```

**Parameter:** The call-stack\
**Return Value:** None

Removes the topmost frame from the call-stack.
<hr>

```C
int pallene_tracer_debug_traceback(lua_State *L);
```

The custom Lua C traceback function which will dump a better backtrace using both Pallene Tracer and Lua call-stack simultaneously. This function is meant to be called from Lua and would mostly be used by `pallene-debug` script. However, this function can be passed to `xpcall()` to generate the tracebacks as well.
<hr>

```C
int pallene_tracer_finalizer(lua_State *L);
```

The finalizer Lua C function to be used as to-be-closed finalizer object, essential for Lua interface frames to refrain from Stack corruption.
<hr>

```C
l_noret pallene_tracer_runtime_callstack_overflow_error(lua_State *L);
```

**Parameter:** A Lua state (`lua_State`)\
**Return Value:** None

Triggers the Pallene Tracer call-stack overflow error when frames can no longer be pushed into the stack. This function should get called from `pallene_tracer_frameenter()` function.

TODO: Add API macros.