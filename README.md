## Documentation
[English Documentation](Documentation/v1/docs/en_US.md) \
[Documentação em Português](Documentation/v1/docs/pt_BR.md) \
[ドキュメント](Documentation/v1/docs/ja_JP.md)

## Other Languages (README)
[Português](Documentation/v1/readme/pt_BR.md) \
[日本語](Documentation/v1/readme/ja_JP.md)

## Introduction

SAH (Stack as Heap) is a small, low-level C library that provides a stack-backed allocator built on top of virtual memory primitives. It is designed for temporary allocations, arenas, parsers, runtimes, virtual machines, and any performance-critical code that benefits from predictable, fast memory management.

This is not a general-purpose heap replacement.

## Functionality

SAH maps a fixed-size memory region and places a guard page below the stack to catch overflows at the OS level. Allocation and deallocation are O(1) and branch-free. The stack grows downward, following real CPU stack semantics, managed through an explicit base pointer and stack pointer.

Overflow is not caught by the library. If the stack pointer crosses into the guard page, the OS raises a segmentation fault. This is by design — bugs fail fast, and the allocator stays branch-free.

Two allocation interfaces are provided: a raw interface where the caller manages sizes manually, and a structured interface that stores metadata so sizes do not need to be tracked by the caller.

## Layout

SAH maps two contiguous regions. The first is a guard page, placed at the bottom of the allocation, which has no access permissions. The second is the usable stack region, 4096 bytes by default. The stack pointer starts at the top of the usable region and moves downward with each allocation.

```
High address  [ Stack region  - 4096 bytes ]  <-- BP, SP starts here
              [ Guard page   - 1 page      ]  <-- PROT_NONE / PAGE_NOACCESS
Low address
```

## Goals

- Minimal overhead with no bounds checks in hot paths
- Real stack semantics: downward growth, explicit BP and SP
- OS-level overflow detection via guard page
- Simple, predictable behavior with a small API surface
- Suitable for temporary allocations and arena-style usage patterns

## Example

```c
#define SAH_IMPLEMENTATION
#include "sah.h"

int main(void)
{
    struct sah_stack* s = screate();

    /* raw push/pop — caller tracks size */
    int* x = push(s, sizeof(int));
    *x = 123;

    /* structured push/pop — size stored internally */
    char* buf = spush(s, 32);
    /* use buf ... */
    spop(s);              /* frees buf  */
    pop(s, sizeof(int)); /* frees x    */

    sdestroy(s);
    return 0;
}
```

To use SAH, define `SAH_IMPLEMENTATION` in exactly one translation unit before including the header.

## Platform Support

| Platform | Status  |
|----------|---------|
| Linux    | Ready   |
| NT       | Ready   |
| OS X     | Void    |

On Linux, SAH uses `mmap` and `mprotect`. On Windows, SAH uses `VirtualAlloc` and `VirtualProtect`. The API is identical on both platforms.

## License

BSD-3-Clause / Public Domain

## Observation

SAH is a personal project, written for enjoyment and because I wanted a stack based allocator, as of now it can only create normal stacks, but on the future I will implement dynamic stacks, Forth size stacks (only bytes in size/size of a word or instruction) to people who want to make Forth-based programs, virtual stacks which are stacks that only live on a certain context and have weird capabilities like being able to pass the stack via function parameters (yes, this sound strange, but my mind is strange...) and other things. Overall SAH is experimental and should be treated as such. Use in production at your own discretion.
