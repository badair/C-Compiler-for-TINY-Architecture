C-Compiler-for-TINY-Architecture
================================

C subset compiler I wrote for an honors project.

In the words of Steve McConnell, writing my first compiler was a "wicked problem". 
I didn't know how to approach the task until I was 1/3 of the way done. I should've 
redesigned it early on, but I was coding as fast I could ( about 50 hours over a 2 
week period ). Poor design decisions early on turned this into a ghastly, hacked up
pile of code that I'm *almost* embarassed to call my own. It's cryptic, complex, and
sometimes monolithic. Nothing is private, and everything is tightly bound. At least 
it works, by and large.





What's supported:

char, int, int[], char[], (void), char* (cursory support) types 

functions

literals supported: “ask a sad lass”, 1234, ‘T’

escape sequences supported: “\n\r\t\0”

operators supported: - , +, *, /, &&, ||, ==, !=, >, >=, <, <= 

(no unary operators)

void functions may have an implicit return

for-loops, while-loops, if/else

return statements

output is performed using "puts" for strings, "putn" for ints

relatively thorough compilation errors






Global  declaration and initialization:

  compile-time computation with literals and known variables

The compiler handles all of the following syntax:


      const int j = 0, k, i = 9 + 3, p = fun(a,b,c);
      const int m = j + i;
      char array[ i ]; 
      fun();
      fun( a + (b * c), 1+2/d );
      i = A[ i+10 ] = 74;
      char* s = "hello world";






What's NOT supported:


floating point numbers, structs, unions, pointer variables

operators other than those listed ( including % )

unary operators (i.e. negating a variable with - )

do...while loops

implicit return types of int ( C allows this )

initializer lists: { 1, 2, 3, 4 };

parameters to main()

forward declaration of functions

comments :(

labels, goto

switch/case

and much, much more...



bugs and quirks:

you must declare something in the first clause of a for-loop header

the compile-time operations at global scope are syntactically less flexible than run-time operations

scope might be buggy - i.e. variables of same name in different places might not compile correctly

you are allowed to use a void function as if it weren’t void

you cannot use an expression /function call as arguments to puts( ) or putn( )

etc...
