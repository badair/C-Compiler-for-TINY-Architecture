//tokens.h
#ifndef TOKENS_H
#define TOKENS_H
#include <cstddef>
#include <sstream>
#include <cstring>
#include <string>
#include <functional>
#include <memory>
#include <cassert>
#include "tinycexceptions.h"

//should've had a tokenFactory function
//Terminal should've been Token, and everything should've inherited from it rather than Terminal

#define declareName(type)  const string name() const override { string s(#type); s.erase( s.length() - 6 ); return s; } string myV; int lineNumber; type(){} type( const string& v, int ln = -1, int prec = -1 ): myV(v), lineNumber(ln) {checkString(); precedence = prec; } const string value() const override {return myV;} 

using namespace std;

inline string stringHash( const string& s ){
	hash<string> hash_fn;
	return "#T" + to_string(hash_fn(s) >> 7);
}

struct Terminal{
	virtual const string name() const = 0;
	//virtual bool isPlaceHolder(){ return false; };

	virtual void checkString(){};
	virtual const string value() const = 0;
	
	virtual ~Terminal(){}
	int precedence;	
};

//TODO account for main, puts, etc... not sure where responsibility lies though

struct Token;
struct ClosedParentheses_Token;
struct OpenParentheses_Token	 : Terminal { declareName(OpenParentheses_Token	)
	ClosedParentheses_Token* mate;
	void checkString() override{ 
		mate = nullptr; 
	}
	
};

struct ClosedParentheses_Token	 : Terminal { declareName(ClosedParentheses_Token	) 
	void checkString() override{ mate = nullptr; };
	OpenParentheses_Token* mate;
};
struct StatementTerminator_Token : Terminal { declareName(StatementTerminator_Token ) }; // ";"
struct OpenSquareBracket_Token	 : Terminal { declareName(OpenSquareBracket_Token	) };
struct ClosedSquareBracket_Token : Terminal { declareName(ClosedSquareBracket_Token	) };

struct ClosedCurly_Token;
struct OpenCurly_Token			 : Terminal { declareName(OpenCurly_Token )  
	ClosedCurly_Token* mate;
	void checkString(){
		mate = nullptr;
	}
};

struct ClosedCurly_Token		 : Terminal { declareName(ClosedCurly_Token) 
	void checkString(){
		mate = nullptr;
	}
	OpenCurly_Token* mate;
};

struct Comma_Token				 : Terminal { declareName(Comma_Token	   ) };

struct Const_Token 				 : Terminal { declareName( Const_Token 	)	 };
struct Do_Token 				 : Terminal { declareName( Do_Token	  	)    };
struct While_Token 				 : Terminal { declareName( While_Token 	) 	 };
struct For_Token 				 : Terminal { declareName( For_Token	) 	 };
struct If_Token 				 : Terminal { declareName( If_Token	  	) 	 };
struct Else_Token 				 : Terminal { declareName( Else_Token  	) 	 };
struct Return_Token 			 : Terminal { declareName( Return_Token ) 	 };

struct  FunctionType_Token 	: Terminal {}; 		//pure
struct  VariableType_Token 	: FunctionType_Token {}; //pure
	struct  IntType_Token 	: VariableType_Token  	{ virtual declareName( IntType_Token  ) };
	struct  CharType_Token 	: VariableType_Token  	{ virtual declareName( CharType_Token ) };
	struct  VoidType_Token 	: FunctionType_Token 	{ virtual declareName( VoidType_Token ) };
typedef VariableType_Token 	  ParameterType_Token; //pure




struct UserDefinedName_Token : Terminal { virtual declareName(UserDefinedName_Token) //not pure
	void checkString() override {
		
		if( !(myV[0] == '_' || (myV[0] >= 'A' && myV[0] <= 'Z') || (myV[0] >= 'a' && myV[0] <= 'z')) ){
			throw ImproperIdentifierException (lineNumber, myV);
		}
		if( myV.length() > MAX_IDENTIFIER_LENGTH ){
			
			cout << "\'" << myV << "\' too long, changing to ";
			
			myV = stringHash(myV);
			cout << "\'" << myV << "\'." << endl;
			assert( myV.length() <= MAX_IDENTIFIER_LENGTH);
			//throw IdentifierTooLongException (lineNumber, myV);
		}



	};

	static const int MAX_IDENTIFIER_LENGTH = 10;
	protected: const string me;
};

	//parser will dynamic_cast from UserDefinedName_Token to one of these
	struct  FunctionName_Token 		 	  : UserDefinedName_Token { virtual declareName(FunctionName_Token)};
	struct  VariableName_Token 		 	  : UserDefinedName_Token { virtual declareName(VariableName_Token)};//int and char will be interchangeable
		struct  MutableVariableName_Token : VariableName_Token 	  { virtual declareName(MutableVariableName_Token)};
		struct  ConstVariableName_Token   : VariableName_Token 	  { virtual declareName(ConstVariableName_Token)};


struct Literal_Token : Terminal {}; //pure
	struct StringLiteral_Token 	: Literal_Token { declareName(StringLiteral_Token) 
		void checkString() override {
			
			/*
			while( myV.find("\"") != string::npos ){
				int pos = myV.find("\"");
				myV.erase(pos,1);
				myV.insert( pos , "\'");

			}

			while( myV.find("\'\'") != string::npos ){
				int pos = myV.find("\'\'");
				myV.erase(pos,2);
			}*/

			while( myV.find("\\n") != string::npos ){
				int pos = myV.find("\\n");
				myV.erase(pos,2);
				myV.insert( pos , "\" ; \ndb 13 ; \ndb 10 ;\ndc \"");
			}

			while( myV.find("\\0") != string::npos ){
				int pos = myV.find("\\0");
				myV.erase(pos,2);
				myV.insert( pos , "\" ; \n db 0 ;\ndc \"");
			}

			while( myV.find("\\t") != string::npos ){
				int pos = myV.find("\\t");
				myV.erase(pos,2);
				myV.insert( pos , "\" ; \n db 9 ;\ndc \"");
			}

			while( myV.find("\\r") != string::npos ){
				int pos = myV.find("\\r");
				myV.erase(pos,2);
				myV.insert( pos , "\" ; \n db 13 ;\ndc \"");
			}

			while( myV.find("dc \"\"") != string::npos ){
				int pos = myV.find("dc \"\"");
				myV.erase(pos,8);
			}

			while( myV.find("\"\"") != string::npos ){
				int pos = myV.find("\"\"");
				myV.erase(pos,5);
			}



		}
	};


	struct CharLiteral_Token 	: Literal_Token { declareName(CharLiteral_Token) 
		void checkString() override {
			if( myV.length() != 3 || (myV.length() > 4 && myV[1] != '\\') ){
				throw MultiLetterCharLiteralException (lineNumber, myV);
			}

			myV = to_string((int) myV[1]);
		}
	};

	struct IntLiteral_Token 	: Literal_Token { declareName(IntLiteral_Token) 
		void checkString() override {
			static const int MAX_INT_VALUE = 99999;
			static const int MIN_INT_VALUE = -99999;
			if( stoi(myV) > MAX_INT_VALUE || stoi(myV) < MIN_INT_VALUE ){
				throw integerLiteralOutsideSupportedRangeException (lineNumber, myV);
			}
			
			for( char c : myV ){
				if( !( c >= '0' && c <= '9' ) ){
					throw ImproperIdentifierException (lineNumber, myV);
				}
			}

		}
	};




struct UnaryOperator_Token : Terminal {}; //pure... not yet implemented
struct BinaryOperator_Token : Terminal {
}; //pure

	struct AssignmentOperator_Token : BinaryOperator_Token 	{ declareName( AssignmentOperator_Token )};
	struct AddOperator_Token 		: BinaryOperator_Token 	{ declareName( AddOperator_Token ) 		};
	struct SubOperator_Token 		: BinaryOperator_Token 	{ declareName( SubOperator_Token ) 		};
	struct MulOperator_Token 		: BinaryOperator_Token 	{ declareName( MulOperator_Token ) 		};
	struct DivOperator_Token	 	: BinaryOperator_Token 	{ declareName( DivOperator_Token ) 		};
	struct ModOperator_Token		: BinaryOperator_Token 	{ declareName( ModOperator_Token ) 		};
	struct GTOperator_Token 		: BinaryOperator_Token 	{ declareName( GTOperator_Token  ) 		}; // ">"
	struct LTOperator_Token 		: BinaryOperator_Token 	{ declareName( LTOperator_Token  ) 		}; // "<"
	struct GTEOperator_Token		: BinaryOperator_Token 	{ declareName( GTEOperator_Token ) 		}; // ">="
	struct LTEOperator_Token 		: BinaryOperator_Token 	{ declareName( LTEOperator_Token ) 		}; // "<="
	struct ETOperator_Token 		: BinaryOperator_Token 	{ declareName( ETOperator_Token  ) 		}; // "=="
	struct NETOperator_Token 		: BinaryOperator_Token 	{ declareName( NETOperator_Token ) 		}; // "!="
	struct LogicalAndOperator_Token	: BinaryOperator_Token 	{ declareName( LogicalAndOperator_Token)}; // "&&"
	struct LogicalOrOperator_Token	: BinaryOperator_Token 	{ declareName( LogicalOrOperator_Token )}; // "||"

	//not a terminal, oh well
	struct FunctionCall_Token : BinaryOperator_Token { declareName ( FunctionCall_Token )
		//includes commas, etc. Everything between function call parentheses
		//stupid stupid stupid design
		vector<Token*> argumentTokens;

	};

	
struct Token{
	Token():lineNumber(-1), T(nullptr){}
	Token ( string v, int ln ) : lineNumber(ln), T(nullptr)  {

		//debug
		//std::cout << "s in Token::Token: " << v << std::endl << "press enter\n";
		//cin.get();

		assert( v != "" );

		if	   ( 	v == "int" 		){ T = new IntType_Token(v, lineNumber); 				}
		else if(	v == "char" 	){ T = new CharType_Token(v, lineNumber); 				}
		else if(	v == "void" 	){ T = new VoidType_Token(v, lineNumber); 				}
		else if( 	v == "do" 		){ T = new Do_Token(v, lineNumber); 					}
		else if( 	v == "while" 	){ T = new While_Token(v, lineNumber); 					}
		else if( 	v == "for" 		){ T = new For_Token(v, lineNumber); 					}
		else if( 	v == "const" 	){ T = new Const_Token(v, lineNumber); 					}
		else if( 	v == "return" 	){ T = new Return_Token(v, lineNumber); 				}
		else if( 	v == ","	 	){ T = new Comma_Token(v, lineNumber); 					}
		else if( 	v == ";" 		){ T = new StatementTerminator_Token(v, lineNumber); 	}
		else if( 	v == "(" 		){ T = new OpenParentheses_Token(v, lineNumber, 20);	}
		else if( 	v == ")" 		){ T = new ClosedParentheses_Token(v, lineNumber, 1);	}
		else if( 	v == "{" 		){ T = new OpenCurly_Token(v, lineNumber); 				}
		else if( 	v == "}" 		){ T = new ClosedCurly_Token(v, lineNumber); 			}
		else if( 	v == "[" 		){ T = new OpenSquareBracket_Token(v, lineNumber, 2);	}
		else if( 	v == "]" 		){ T = new ClosedSquareBracket_Token(v, lineNumber, 2); }
		else if( 	v == "=" 		){ T = new AssignmentOperator_Token(v, lineNumber, 15);	}
		else if( 	v == "+"	 	){ T = new AddOperator_Token(v, lineNumber, 6); 		}
		else if( 	v == "-"	 	){ T = new SubOperator_Token(v, lineNumber, 6); 		}
		else if( 	v == "*"	 	){ T = new MulOperator_Token(v, lineNumber, 5);			} //TODO: account for int* etc?
		else if( 	v == "/"	 	){ T = new DivOperator_Token(v, lineNumber, 5);			}
		else if( 	v == "%"	 	){ T = new ModOperator_Token(v, lineNumber, 5);			}
		else if( 	v == ">"	 	){ T = new GTOperator_Token(v, lineNumber, 8); 			}
		else if( 	v == "<"	 	){ T = new LTOperator_Token(v, lineNumber, 8); 			}
		else if( 	v == ">="	 	){ T = new GTEOperator_Token(v, lineNumber, 8);			}
		else if( 	v == "<="	 	){ T = new LTEOperator_Token(v, lineNumber, 8); 		}
		else if( 	v == "=="	 	){ T = new ETOperator_Token(v, lineNumber, 9); 			}
		else if( 	v == "!="	 	){ T = new NETOperator_Token(v, lineNumber, 9); 		}
		else if( 	v == "&&"	 	){ T = new LogicalAndOperator_Token(v, lineNumber, 13);	}
		else if( 	v == "||"	 	){ T = new LogicalOrOperator_Token(v, lineNumber, 14);	}


		else if( *v.begin() == '\'' && *v.rbegin() == '\'' ){
			T = new CharLiteral_Token( v, lineNumber );
		}

		else if( *v.begin() == '\"' && *v.rbegin() == '\"' ){
			T = new StringLiteral_Token( v, lineNumber );
		}

		else if( *v.begin() >= '0' && *v.begin() <= '9' ){
			T = new IntLiteral_Token( v, lineNumber );
		}
		else if( v == "true" ){
			T = new IntLiteral_Token( "1", lineNumber );
		}
		else if( v == "false" ){
			T = new IntLiteral_Token( "0", lineNumber );
		}
		else{
			T = new UserDefinedName_Token( v, lineNumber );
		}

	}

	virtual ~Token(){
		if( T != nullptr ) delete T;
	}


	Terminal* T;
	int lineNumber;
	virtual const string name() const { return T -> name(); }
	virtual const string value() const { return T -> value();}
	Terminal*& ptr(){return T;}

};

struct CompositeToken : Token {
	CompositeToken( const string& s ) : compositeValue(s), isArrayDereference(false), addressOfPointer(""){}
	string compositeValue;
	bool isArrayDereference;
	string addressOfPointer;
	const string value() const override { return compositeValue;}
	~CompositeToken() override{}
};

struct DeclaredFunction;

#endif