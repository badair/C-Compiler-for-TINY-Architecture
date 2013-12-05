C-Compiler-for-TINY-Architecture
================================

This is a compiler for a subset of C that I wrote for an honors project.

The source code is in /src, info about the target is in /TIDE, and example c programs to compile are in /demo.
Coded in a hurry; ended up being pretty hacky by the end. Still works, though.



What's supported:

char, int, int[], char[], (void), char* (cursory support) types 

functions

literals supported: “ask a sad lass”, 1234, ‘T’

escape sequences supported: “\n\r\t\0”

operators supported: - , +, *, /, &&, ||, ==, !=, >, >=, <, <= 

(no unary operators or modulo)

for-loops, while-loops, if/else

void functions can have an implicit return

output is performed using "puts" for strings, "putn" for ints

relatively thorough compilation errors, including line numbers

Global initialization supports compile-time computation with literals 
and initialized variables




The compiler handles all of the following syntax:


      const int j = 0, k, i = 9 + 3, p = fun(a,b,c);
      const int m = j + i;
      char array[ i ]; 
      fun();
      fun( a + (b * c), 1+2/d );
      i = A[ i+10 ] = 74;
      char* s = "hello world";






What's NOT supported:


floating point numbers, structs, unions, indirection

operators other than those listed ( including % )

unary operators (i.e. negating a variable with - )

do...while loops

implicit int return type

initializer lists: { 1, 2, 3, 4 };

parameters to main()

forward declaration of functions

comments :(

labels, goto

switch/case

console input

and much more...



bugs and quirks:

you must declare something in the first clause of a for-loop header

definitions at global scope are syntactically less flexible than runtime computed expressions,
mostly due to poor design decisions

scope might be buggy - i.e. variables of same name in different places might not compile correctly

you are allowed to use a void function as if it weren’t void

you cannot use an expression /function call as arguments to puts( ) or putn( )

etc...



Coulda, shoulda, woulda:

Shouldn't have passed so many containers by value, out of lazyness.

Probably could've made use of RTTI or something more readable than dynamic_casting every 5 lines for typechecking.

An AST implementation would've been nice to have.

The Token class is a redundant, cumbersome proxy class that shouldn't have existed in the first place.

Would've used a factory, rather than a proxy class, for creating tokens.

Would've favored polymorphism and inheritance over those ridiculous and ultimately crippling macros.

Would've used access specifiers from the start.

The Terminal class would've had the role the Token class ended up with.

Naming conventions were sometimes sloppy and I should've been stricter.

Would've used either exclusively queues or exclusively vectors, rather than foolishly juggling both (probably vectors
out of convenience).

Should've had a stricter look-ahead policy.

Would've used "design by contract" concepts with asserts at the beginning and end of every routine

so that I'd know how the tokens should be queued up without having to debug.

std::smart_ptr and std::weak_ptr would've saved me from rolling my own garbage collection.

Would've overloaded the operator char*()  for the the Token class.

Would've passed a container of delimiters instead of hacking them into place when I needed a new one.
