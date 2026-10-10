# C Naming & Style Conventions


## 1. Function Names

**Format:** `tlk_{component}_{subcomponent(s)}_{action}_{detail}`

- `{component}` - name of the component: `uart`, `spi`, `storage`, `api`, etc.  
  For code belonging to the `system/` folder (mem, storage, task_planner, debug, etc.), use the
  specific subsystem name instead of `sys`/`system` (`mem`, `storage`, `task`, `debug`, etc.).

- `{subcomponent(s)}` - optional functional subdivisions of the component: `tx`, `rx`, `master`, `slave`, etc.

- `{action}` - verb describing what the function does: `init`, `set`, `enable`, `configure`, `reset`, `is_ready`, etc.  
  Keep the verb vocabulary consistent across the whole project (don't mix `enable`/`on`/`activate` for the same meaning).

- `{detail}` - optional extra clarification if the verb alone is not specific enough.

**Rules:**
- A function must be `static` unless it is declared in the header and meant to be
  called from other `.c` files. Maximize `static` usage.


## 2. Variable Names

### 2.1. Global variables

**Format:** `tlk_{component}_{subcomponent(s)}_{purpose}`

- `{component}` - name of the component: `uart`, `spi`, `storage`, `api`, etc.  
  For code belonging to the `system/` folder (mem, storage, task_planner, debug, etc.), use the
  specific subsystem name instead of `sys`/`system` (`mem`, `storage`, `task`, `debug`, etc.).

- `{subcomponent(s)}` - optional functional subdivisions of the component: `tx`, `rx`, `master`, `slave`, etc.

- `{purpose}` - short description of what the variable holds.

**Rules:**
- A variable must be `static` unless it is declared in the header and meant to be
  called from other `.c` files. Maximize `static` usage.
- All global variables must be initialized at declaration.

### 2.2. Local variables

**Notes:**
No mandatory rules. The `tlk_` prefix is ​​not required.

### 2.3. Custom types

**Format:** `tlk_{component}_{subcomponent(s)}_{purpose}_{suffix}`

- `{component}` - name of the component: `uart`, `spi`, `storage`, `api`, etc.  
  For code belonging to the `system/` folder (mem, storage, task_planner, debug, etc.), use the
  specific subsystem name instead of `sys`/`system` (`mem`, `storage`, `task`, `debug`, etc.).

- `{subcomponent(s)}` - optional functional subdivisions of the component: `tx`, `rx`, `master`, `slave`, etc.

- `{purpose}` - short description of what the variable holds.

- `{suffix}` - `s` for `struct`, `e` for `enum`, `u` for `union`.

**Rules:**
- Use `typedef` for all `struct`, `union`, and `enum` types.
- Do not use tags for `struct`, `union`, or `enum`.
- Pass `struct` pointers instead of full struct copies as function arguments.
- Match `enum` parameters to hardware feature support.

**Examples:**
```c
typedef struct
{
  ...
} tlk_clock_config_t;
```

### 2.4. Enum names

**Format:** `TLK_{COMPONENT}_{SUBCOMPONENT(S)}_{PURPOSE}`

- `{COMPONENT}` - name of the component: `uart`, `spi`, `storage`, `api`, etc.  
  For code belonging to the `system/` folder (mem, storage, task_planner, debug, etc.), use the
  specific subsystem name instead of `sys`/`system` (`mem`, `storage`, `task`, `debug`, etc.).

- `{SUBCOMPONENT(S)}` - optional functional subdivisions of the component: `tx`, `rx`, `master`, `slave`, etc.

- `{PURPOSE}` - short description of what the enum element holds.

### 2.5. Boolean variables

**Format:** `tlk_{component}_{subcomponent(s)}_{predicate}_{purpose}`

- `{component}` - name of the component: `uart`, `spi`, `storage`, `api`, etc.  
  For code belonging to the `system/` folder (mem, storage, task_planner, debug, etc.), use the
  specific subsystem name instead of `sys`/`system` (`mem`, `storage`, `task`, `debug`, etc.).

- `{subcomponent(s)}` - optional functional subdivisions of the component: `tx`, `rx`, `master`, `slave`, etc.

- `{predicate}` - boolean predicate describing the meaning of the variable: `is`, `has`, `can`, `should`, `was`, etc.  

- `{purpose}` - short description of what the variable holds.

**Type:** `bool`

**Rules:**
- Use the `bool` type for variables representing logical values (`true` / `false`).
  Do not use integer types (`uint8_t`, `int`, etc.) for boolean values unless required
  by hardware registers.
- Use `true` for 1, `false` for 0.

### 2.6. Integer types

**Type:** `{u}int{len}_t`

- `{u}` - unsigned, signed.
- `{len}` - 8, 16, 32, 64

**Rules:**
- Use fixed-width integer types from `<stdint.h>` instead of native C integer types.


## 3. Defines

**Format:** `TLK_{COMPONENT}_{SUBCOMPONENT(S)}_{PURPOSE}`, all uppercase, words separated by `_`.

- `{COMPONENT}` - name of the component: `uart`, `spi`, `storage`, `api`, etc.  
  For code belonging to the `system/` folder (mem, storage, task_planner, debug, etc.), use the
  specific subsystem name instead of `sys`/`system` (`mem`, `storage`, `task`, `debug`, etc.).

- `{SUBCOMPONENT(S)}` - optional functional subdivisions of the component: `tx`, `rx`, `master`, `slave`, etc.

- `{PURPOSE}` - short description of what the define holds.


## 4. Macro Functions

**Format:** `TLK_{COMPONENT}_{SUBCOMPONENT(S)}_{ACTION}_{DETAIL}`, all uppercase, words separated by `_`.

- `{COMPONENT}` - name of the component: `uart`, `spi`, `storage`, `api`, etc.  
  For code belonging to the `system/` folder (mem, storage, task_planner, debug, etc.), use the
  specific subsystem name instead of `sys`/`system` (`mem`, `storage`, `task`, `debug`, etc.).

- `{SUBCOMPONENT(S)}` - optional functional subdivisions of the component: `tx`, `rx`, `master`, `slave`, etc.

- `{ACTION}` - verb describing what the function does: `init`, `set`, `enable`, `configure`, `reset`, `is_ready`, etc.  
  Keep the verb vocabulary consistent across the whole project (don't mix `enable`/`on`/`activate` for the same meaning).

- `{DETAIL}` - optional extra clarification if the verb alone is not specific enough.


## 5. Header Files (.h)

**Format:**

Structure, top to bottom:
1. Include guard
```c
  #ifndef
  #define _{TOP FOLDER}_{FOLDER}_{FILE_NAME}_H
  #endif
```
2. Doxygen file header comment (`@file`, short description).
3. License header.
4. Includes.
5. Types: `enum` first, then `struct` / `union`.
6. `extern` declarations of exposed global variables.
7. Function prototypes.

**Rules:**
- Doxygen comments (`@brief`, `@param`, `@return`) are required in the header, above every public
  function. This is the single source of documentation for the public API. Doxygen comments are optional
  for variables.


## 6. Source Files (.c)

**Format:**

Structure, top to bottom:
1. Includes.
2. global/static objects (variables, buffers).
3. Sections comment `/* {Section} helper functions */` - all `static` internal functions.
4. Section comment `/* {Component} driver APIs */` - implementations of public functions, in the
   same order as declared in the header.
5. Additional helper functions registered for a specific action. (`after sleep`, `auto config`, `prevent sleep`, etc.).

**Rules:**
- Do not duplicate Doxygen comments for public APIs in .c files - they belong in the header.
  Doxygen comments for internal helper functions are optional when they improve clarity.


## 7. Doxygen Comments

**Format:**

Use Doxygen comments for all public APIs, variables and custom types declared in header files.

- Start with `@brief`.
- Write short, clear descriptions.
- Describe what the object represents or what the function does.
- Use imperative verbs for functions (`Initialize`, `Enable`, `Get`, `Set`, `Configure`, etc.).
- End descriptions without a period.
- Add `@param` for every parameter.
- Add `@return` only if the function returns a value.
- Keep the wording consistent across the project.

**Examples:**

Function:
```c
/**
 * @brief Initialize the clock driver
 *
 * @param cfg Pointer to clock configuration
 * @return true if initialization succeeds
 * @return false otherwise
 */
bool tlk_clock_init(const tlk_clock_config_t *cfg);
```

Variable:
```c
/** @brief Current clock frequency in Hz */
extern uint32_t tlk_clock_frequency;
```

Structure:
```c
/**
 * @brief Clock configuration
 */
typedef struct
{
    uint32_t frequency;
    bool enable_pll;
} tlk_clock_config_t;
```

Enum:
```c
/**
 * @brief Clock source
 */
typedef enum
{
    TLK_CLOCK_SRC_RC,
    TLK_CLOCK_SRC_XTAL,
} tlk_clock_source_t;
```
