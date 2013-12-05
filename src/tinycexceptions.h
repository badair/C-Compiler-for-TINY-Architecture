#ifndef TINYCEXCEPTIONS_H
#define TINYCEXCEPTIONS_H

#include<exception>
#include <string>
#include <iostream>
#ifndef _MSC_VER
#define NOEXCEPT noexcept
#else
#define NOEXCEPT
#endif

//I would definitely use this macro again
#define decltinycexp(type) : TinyCException { type( int lineNumber, std::string token ):TinyCException(lineNumber, token){} const char* what() const NOEXCEPT override{ return 


struct TinyCException : std::exception {
	TinyCException( int ln, std::string t ): lineNumber(ln), token(t){ 
		std::cout << "Error at line " << ln << "(\'" << t << "\'): ";
	}
	virtual ~TinyCException(){};
private:
	int lineNumber;
	std::string token;

};
 
struct MultiLetterCharLiteralException decltinycexp (MultiLetterCharLiteralException)
"Character literals cannot be more than one character long.";
}};

struct IdentifierTooLongException decltinycexp (IdentifierTooLongException)
"Identifiers may be no more that 10 characters long."; 
}};

struct integerLiteralOutsideSupportedRangeException decltinycexp (integerLiteralOutsideSupportedRangeException)
"Tiny C integer literals must be in the range +/- 99999.";
}};

struct ImproperIdentifierException decltinycexp (ImproperIdentifierException)
"Identifiers must begin with a letter or underscore.";
}};

struct ExpectedDeclarationException decltinycexp (ExpectedDeclarationException)
"Expected a declaration. ";
}};

struct ExpectedIdentifierException decltinycexp (ExpectedIdentifierException)
"Expected an identifier. ";
}};

struct VariableAlreadyDeclaredException decltinycexp (VariableAlreadyDeclaredException)
"This variable has already been declared.";
}};

struct ExpectedEndOfDeclarationException decltinycexp (ExpectedEndOfDeclarationException)
"Expected end of declaration.";
}};

struct ExpectedStringLiteralException decltinycexp (ExpectedStringLiteralException)
"Expected assignment of string literal.";
}};

struct InvalidArraySizeException decltinycexp (InvalidArraySizeException)
"Expected integer literal or known constant in array size specifier.";
}};

struct ExpectedClosingSquareBracketException decltinycexp (ExpectedClosingSquareBracketException)
"Expected \']\' in array declaration.";
}};

struct VariableNotDeclaredInThisScopeException decltinycexp (VariableNotDeclaredInThisScopeException)
"Variable not declared in this scope.";
}};

struct InvalidStaticOperationException decltinycexp (InvalidStaticOperationException)
"Unrecognized operator.";
}};

struct InvalidExpresssionException decltinycexp (InvalidExpresssionException)
"Invalid expression.";
}};

struct MalformedMainException decltinycexp (MalformedMainException)
"\'main\' was not properly defined.";
}};

struct ExpectedOpenParenthesesException decltinycexp(ExpectedOpenParenthesesException)
"Expected a function call beginning with \'(\'.";
}};

struct ExpectedClosedParenthesesException decltinycexp(ExpectedClosedParenthesesException)
"Expected \')\'.";
}};

struct ExpectedSemicolonException decltinycexp(ExpectedSemicolonException)
"Expected \';\'.";
}};

struct ModifyingConstVariableException decltinycexp(ModifyingConstVariableException)
"Cannot modify constant.";
}};

struct ExpectedAssignmentException decltinycexp(ExpectedAssignmentException)
"Expected assignment.";
}};

struct FeatureNotImplementedYetException decltinycexp(FeatureNotImplementedYetException)
"This feature is not yet implemented.";
}};

struct ExpectedArrayException decltinycexp(ExpectedArrayException)
"Expected array before operator \'[\'.";
}};

struct ExpectedClosedCurlyExpception decltinycexp(ExpectedClosedCurlyExpception)
"Expected \'}\' at end of file.";
}};

struct UnexpectedClosedCurlyException decltinycexp(UnexpectedClosedCurlyException)
"Unexpected \'}\'.";
}};

struct ExpectedOpenCurlyException decltinycexp(ExpectedOpenCurlyException)
"Expected a \'{\'";
}};

struct IncorrectNumberOfParametersException decltinycexp(IncorrectNumberOfParametersException)
"Incorrect number of parameters.";
}};

struct UnknownFunctionException decltinycexp(UnknownFunctionException)
"Unknown function.";
}};

#endif