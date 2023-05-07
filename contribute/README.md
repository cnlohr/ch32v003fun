# general

## principles
reduce "heavyness" of the compiled program
*	instructions
*	run-time branching
	*	select branches at compiile-time, where possible:
		*	`#define`
		*	`#if defined()`

## styling
tabs, not spaces



# writing a library
translation unit = .c file

## principles
we want the compiler to do all the work it can, including
*	compile-time branching, where possible
	*	you can have the user #define feature selections and then have the compiler decide what code gets built by using #if defined() #elif defined() etc.
*	compile-time math, where possible
	*	-> compiler won't bring expensive math functions onto our tiny mcu

## header-only libraries with optional implementation and static inlinig
This is the structural concept to which we write libraries for the CH32V003.

### benefits
Compared to traditional .h declaration and .c implementation files, the approach offers the following benefits:
*	functions remain separate between translation units
	*	= static
*	functions get baked right into the translation unit that #includes them at the preprocessor stage
	*	= h-file only
    *   simple build process, no linking required
    *   the compiler can see the code all in one translation unit
        *   -> more opportunities for optimization
            *   -> better executables
    small functions get inserted in-place
	*	= inline
        *	-> reduced expense for function calls (flash, instructions to execute)
*	large functions are only selectively available if needed in a translation unit
	*	see large, non-inlined functions in implementation block

### basic structural example
```
//#### include guard
#ifndef MY_LIBRARY_H
#define MY_LIBRARY_H

//#### library function overview: declare all
int large_function(int a, int b, int c);

//#### small function definitions, static inline
static inline int small_function(int a, int b) {
	return a+b;
}

//#### implementation block
#if defined(LIBRARY_IMPLEMENTATION)

//#### define larger functions
int large_function(int a, int b, int c) {
	c += a * a / b;
	...
    return a + b / c;
}

#endif // LIBRARY_IMPLEMENTATION
#endif // MY_LIBRARY_H
```
You can find a more elaborate structure in `ch32v003_library_template.h`, please use it when writing libraries.

to access functions of implementation block in .c file:
```
#define LIBRARY_IMPLEMENTATION
#include "ch32v003_library.h"
```
