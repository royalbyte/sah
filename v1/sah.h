#ifndef SAH_H
#define SAH_H

/*
 * SAH is implemented both to Linux and Windows
 * to reach your desired version jump or filter
 * between "__unix__" or "_WIN32"
 *
 * current status:
 * Linux	Ready
 * Windows	Ready
 *
 * @creator := royalbyte (0x7C00)
 * */

#ifdef __unix__

/* LIB GLOBAL DEPENDENCIES ON LIBC */
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

/* GLOBAL TYPES, MACROS AND VALUES */
#define MEGABYTE (1024 * 1024)
#define STACK_TINY MEGABYTE
#define STACK_SMALL (2 * MEGABYTE)
#define STACK_MEDIUM (4 * MEGABYTE)
#define STACK_LARGE (8 * MEGABYTE)
#define STACK_HUGE (16 * MEGABYTE)
#define ALIGN(n) (((n) + 15) & ~15)

/* BEGIN SAH PUBLIC API SIGNATURE */

/* BEGIN PUBLIC API TYPES SIGNATURE */

struct _stack_header {
	size_t size;
};

struct sah_stack {
	size_t payload_size;
	uint8_t* bp;
	uint8_t* sp;
}; __attribute__((aligned(64)));

/* END PUBLIC API TYPES SIGNATURE */

/* BEGIN PUBLIC API FUNCTIONS SIGNATURE */

struct sah_stack* screate(size_t); // USED BY BASIC MODULE, CREATES A STACK WITH A FIXED SIZE
void sdestroy(struct sah_stack*); // USED BY BASIC MODULE, DESTROY A STACK
void* spush(struct sah_stack*, size_t); // USED BY BASIC MODULE, PUSHES INTO THE STACK WITH A HEADER
void spop(struct sah_stack*); // USED BY BASIC MODULE, POPS THE STACK ACCORDING TO spush() HEADER, NO NEED FOR MANUAL SIZE TRACKING

/* END PUBLIC API FUNCTIONS SIGNATURE */

/* BEGIN PUBLIC API STATIC FUNCTIONS SIGNATURE */

// USED BY BASIC MODULE, PUSHES INTO THE STACK, REQUIRES MANUAL SIZE TRACKING
static inline void* push(struct sah_stack* s, size_t n)
{
	s->sp -= n;
	return s->sp;
}
// USED BY BASIC MODULE, POPS THE STACK, REQUIRES MANUAL SIZE TRACKING
static inline void pop(struct sah_stack* s, size_t n)
{
	s->sp += n;
}
// USED BY BASIC MODULE, RESETS THE STACK POINTER BACK TO THE BASE POINTER
static inline void sreset(struct sah_stack* s)
{
	s->sp = s->bp;
}

/* END PUBLIC API STATIC FUNCTIONS SIGNATURE */

#ifdef SAH_IMPLEMENTATION

#ifdef BASIC // BASIC MODULE

struct sah_stack* screate(size_t psize)
{
	size_t guard = sysconf(_SC_PAGESIZE);
	size_t total = guard + psize;
	uint8_t* mem = mmap(NULL, total, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (mem == MAP_FAILED)
		return NULL;

	mprotect(mem, guard, PROT_NONE);

	struct sah_stack* s = malloc(sizeof(struct sah_stack));
	if (!s) {
		munmap(mem, total);
		return NULL;
	}

	s->payload_size = psize;
	s->bp = mem + total;
	s->sp = s->bp;

	return s;
}

void sdestroy(struct sah_stack* s)
{
	if (s == NULL) return;

	size_t payload = s->payload_size;
	size_t guard = sysconf(_SC_PAGESIZE);
	size_t total = guard + payload;

	uint8_t* mem = s->bp - total;
	
	munmap(mem, total);
	free(s);
}

void* spush(struct sah_stack* s, size_t n)
{
	size_t rtotal = sizeof(struct _stack_header) + n;
	size_t total = ALIGN(rtotal);
	
	s->sp -= total;

	struct _stack_header* hdr = (struct _stack_header*)s->sp;
	hdr->size = n;

	return (void*)(hdr + 1);
}

void spop(struct sah_stack* s)
{
	struct _stack_header* hdr = (struct _stack_header*)s->sp;
	s->sp += ALIGN(sizeof(struct _stack_header) + hdr->size);
}

#endif /* BASIC MODULE */

#ifdef ASYNC // ASYNC MODULE

/* W.I.P, MODULE FOR ASYNC, THREAD-SAFE, MULTITHREAD, GLOBAL, CONCURRENT AND PARALLEL STACKS */

#endif /* ASYNC MODULE */
#endif /* IMPLEMENTATION */

#endif /* __unix__ */

/*
 * still in progress for formating text and code
 * on the windows version of the lib, still
 * follows the old way
 */
#ifdef _WIN32

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <windows.h>

#define MEGABYTE (1024 * 1024)
#define STACK_TINY MEGABYTE
#define STACK_SMALL (2 * MEGABYTE)
#define STACK_MEDIUM (4 * MEGABYTE)
#define STACK_LARGE (8 * MEGABYTE)
#define STACK_HUGE (16 * MEGABYTE)

/* HEADER FOR SPUSH AND SPOP AUTO ALIGNMENT, INTERNAL USE ONLY */
struct _stack_header {
	size_t size;
};

/* STACK CONTROLLER STRUCTURE, ALLOCATED ON THE HEAP AND TRACKS THE SIZE OF THE STACK */
struct sah_stack {
	size_t payload_size;
	uint8_t* bp;
	uint8_t* sp;
}; __attribute__((aligned(64)));

/* BEGIN PUBLIC API FUNCTIONS SIGNATURE */
struct sah_stack* screate(size_t);
void sdestroy(struct sah_stack*);
static inline void* push(struct sah_stack*, size_t);
static inline void pop(struct sah_stack*, size_t);
void* spush(struct sah_stack*, size_t);
void spop(struct sah_stack*);
/* END PUBLIC API FUNCTIONS SIGNATURE */

/* LOWEST LEVEL OPERATIONS */
/* FUNCTION push(), PUSHES INTO THE STACK, MANUAL CONTROL OF SIZE TO PUSH */
static inline void* push(struct sah_stack* s, size_t n)
{
	s->sp -= n;
	return s->sp;
}

/* FUNCTION pop(), POP THE STACK, MANUAL CONTROL OF SIZE TO POP */
static inline void pop(struct sah_stack* s, size_t n)
{
	s->sp += n;
}

/* FUNCTION sreset(), RESET THE STACK MOVING THE SP TO BP, ALLOWING REUSE */
static inline void sreset(struct sah_stack* s)
{
	s->sp = s->bp;
}

#ifdef SAH_IMPLEMENTATION

/* AUTO ALIGNMENT MACRO */
#define ALIGN(n) (((n) + 15) & ~15)

/* FUNCTION screate(), CREATES THE STACK AND RETURNS THE STACK CONTROLLER/HANDLER FOR USE */
struct sah_stack* screate(size_t psize)
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	DWORD dw = si.dwPageSize;

	size_t guard = (size_t)dw;
	size_t total = guard + psize;
	uint8_t* mem = VirtualAlloc(NULL, total, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (!mem)
		return NULL;
	
	DWORD unused;
	VirtualProtect(mem, guard, PAGE_NOACCESS, &unused);

	struct sah_stack* s = malloc(sizeof(struct sah_stack));
	if (!s) {
		VirtualFree(mem, 0, MEM_RELEASE);
		return NULL;
	}

	s->payload_size = psize;
	s->bp = mem + total;
	s->sp = s->bp;
	return s;
}

/* FUNCTION sdestroy(), DESTROY THE STACK GIVEN A STACK CONTROLLER/HANDLER, FREE ALL MEMORY INCLUDING THE STACK CONTROLLER */
void sdestroy(struct sah_stack* s)
{
	if (!s) return;

	SYSTEM_INFO si;
	GetSystemInfo(&si);
	DWORD dw = si.dwPageSize;
	size_t guard = (size_t)dw;
	size_t payload = s->payload_size;
	size_t total = guard + payload;

	uint8_t* mem = s->bp - total;
	VirtualFree(mem, 0, MEM_RELEASE);
	free(s);
}

/* FUNCTION spush(), PUSH INTO THE STACK WITH HEADERS TO TRACK SIZE FOR SPOP */
void* spush(struct sah_stack* s, size_t n)
{
	size_t rtotal = sizeof(struct _stack_header) + n;
	size_t total = ALIGN(rtotal);

	s->sp -= total;

	struct _stack_header* hdr = (struct _stack_header*)s->sp;
	hdr->size = n;

	return (void*)(hdr + 1);
}

/* FUNCTION spop(), POP THE STACK, AUTO TRACKS THE AMOUNT TO POP WITH spush() HEADER */
void spop(struct sah_stack* s)
{
	struct _stack_header* hdr = (struct _stack_header*)s->sp;
	s->sp += ALIGN(sizeof(struct _stack_header) + hdr->size);
}

#endif /* WINDOWS_IMPLEMENTATION */
#endif /* _WIN32 */

#endif /* SAH_H */
