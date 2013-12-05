//parser.cpp


#include "parser.h"
#include <stack>
#include <cstring>
#include <algorithm>
#include <iomanip>

const string GLOBAL_SCOPE = "g_";
const string MAIN_SCOPE = "main_";
DeclaredVariable NOT_KNOWN( "_____THISVARIABLEISNOTKNOWN________", false, "asdfasdfasdfasdfasdlkajsdflaskdfj", -1 );
DeclaredFunction NOT_KNOWN_F(false, "_____THISVARIABLEISNOTKNOWN________");

//another unfortunate result of poor design
const int FUNCTION_CALL_OPERATOR_PRECEDENCE = 2;

//this is currently only used for comparison against the NOT_KNOWN constant
bool operator == ( const DeclaredVariable& l, const DeclaredVariable&r ){
	return l.variableName == r.variableName;
}
//this is currently only used for comparison against the NOT_KNOWN constant
bool operator != ( const DeclaredVariable& l, const DeclaredVariable&r ){
	return l.variableName != r.variableName;
}

//this is currently only used for comparison against the NOT_KNOWN constant
bool operator != ( const DeclaredFunction& l, const DeclaredFunction&r ){
	return l.functionName != r.functionName;
}

//this is currently only used for comparison against the NOT_KNOWN constant
bool operator == ( const DeclaredFunction& l, const DeclaredFunction&r ){
	return l.functionName == r.functionName;
}

Parser::Parser(vector<Token*>& t, const char* fileName) : tny( (string(fileName) + ".tny" ).c_str() ), tokens(t){
	srand((unsigned int)time(NULL));
	cout << "parsing..." << endl;
	tny << "jmp main\n" << endl;
	tny << "\n; variables:\n" << endl;
	
	temp = randomLabel();
	tnyHeader << temp << ": ds 1 ; TEMP "<< endl;
	temp2 = randomLabel();
	tnyHeader << temp2 << ": ds 1 ; TEMP2 "<< endl;

	//true, false
	literals.push_back("0");
	literals.push_back("1");

	try{
		parse();
	}
	catch(...){
		for( auto p : garbage )
		delete p;

		declareLiterals();
		tny << tnyHeader.str();
		tny << tnyCode.str();
		tny << "stop";
		tny.close();
		throw;
	}

}

Parser::~Parser(){

	for( auto p : garbage )
		delete p;

	declareLiterals();
	tny << tnyHeader.str();
	tny << tnyCode.str();
	tny << "stop";
	tny.close();
}

DeclaredVariable& Parser::fetch( const string& variableName, const string& scope ){
	auto avs = availableScopes(scope);
	for( DeclaredVariable& dv : allDeclaredVariables ){
		for( string s : avs ){
			if( dv.variableName == variableName && dv.scope == s ){
				return dv;
			}
		}
	}
	return NOT_KNOWN;
}

DeclaredFunction& Parser::fetchFunction( const string& functionName ){
	for( DeclaredFunction& dv : allDeclaredFunctions ){
		if( dv.functionName == functionName ){
				return dv;
		}
	}
	return NOT_KNOWN_F;
}

vector<string> Parser::availableScopes	( const string& scope ){
	//tokenize scope string which looks like g_main_if1else_

	vector<string> result;
	size_t startLooking = 0;
	while(  scope.find( "_", startLooking) < string::npos ){
		startLooking = scope.find( "_", startLooking) + 1;

		result.push_back( string(scope.begin(), scope.begin()+startLooking) );
	}

	reverse( result.begin(), result.end() );

	return result;
}

queue<Token*> Parser::getNextStatement( vector<Token*>& tokens, const string& scope, const string& delim = ";", const string& delim2 = "" ){

	queue<Token*> currentStatement;

	auto itr = tokens.begin(); 

	do{
		currentStatement.push(*itr);
		itr++;
	}while( ptr(itr)-> value() != delim && ( delim2 == ""? true : ptr(itr)-> value() != delim2) );

	currentStatement.push( *(itr) );
	
	return currentStatement;
}

queue<Token*> Parser::getNextStatement( queue<Token*>& tokens, const string& scope, const string& delim = ";" ){
	
	queue<Token*> currentStatement;

	do{
		currentStatement.push( tokens.front() );
		tokens.pop();
	}while( tokens.front() -> value() != delim );

	currentStatement.push( tokens.front() );
	
	return currentStatement;
}

//TODO: handle functions
void Parser::extractGlobalDeclarations( vector<Token*>& tokens ){
	while( tokens[1] -> value() != "main" ){

		if( tokens[2] -> value() == "(" ){
			bool isVoid = tokens.front() -> value() == "void";
			clearFirstToken();
			string functionName = tokens.front() -> value();
			clearFirstToken();
			parseFunction( isVoid , functionName );
		}
		else{
			queue<Token*> currentStatement = getNextStatement( tokens, GLOBAL_SCOPE );
			int statementLength = currentStatement.size();
			handleAllDeclarations(currentStatement, GLOBAL_SCOPE, tnyCode );

			for( int i = 0; i < statementLength ; ++i ){
				clearFirstToken();
			}
		}

	}
}

void Parser::handlePutn( string& currentScope, stringstream& out ){
	clearFirstToken();

	if( tokens.front() -> value() != "(" ){
		throw ExpectedOpenParenthesesException( tokens.front() -> lineNumber, tokens.front() -> value() );
	}

	clearFirstToken();

	string s = tokens.front() -> value();

	if( dynamic_cast<IntLiteral_Token*>( tokens.front()->ptr() ) || dynamic_cast<CharLiteral_Token*>( tokens.front()->ptr() )){
				
		literals.push_back(s);
		out << "\nld # " << s << " ; putn( " << s <<  " );" << endl;
		out << "call printn" << " ; \n" << endl;

	}
	else{
		// must be a previously declared variable
		auto dv = fetch( s, currentScope );
		if( dv != NOT_KNOWN ){
			out << "\nld " << dv.scope << dv.variableName << " ; putn( " << s <<  " );" << endl;
			out << "call printn" << " ; \n" << endl;
		}
		else{
			throw VariableNotDeclaredInThisScopeException( tokens.front() -> lineNumber, s );
		}
	}

	clearFirstToken();

	if( tokens.front() -> value() != ")" ){
		throw ExpectedClosedParenthesesException( tokens.front() -> lineNumber, tokens.front() -> value() );
	}

	clearFirstToken();

	if( tokens.front() -> value() != ";" ){
		throw ExpectedSemicolonException( tokens.front() -> lineNumber, tokens.front() -> value() );
	}

}

void Parser::handlePuts( string& currentScope, stringstream& out ){
	clearFirstToken();

	if( tokens.front() -> value() != "(" ){
		throw ExpectedOpenParenthesesException( tokens.front() -> lineNumber, tokens.front() -> value() );
	}

	clearFirstToken();

	string s = tokens.front() -> value();

	if( dynamic_cast<StringLiteral_Token*>( tokens.front()->ptr() ) ){
				
		literals.push_back(s);
		tinyPrintStringLiteral(s, out);

	}
	else{
		// must be a previously defined variable
		auto dv = fetch( s, currentScope );
		if( dv != NOT_KNOWN ){

			tinyPrintStringVariable(dv, out);
		}
		else{
			throw VariableNotDeclaredInThisScopeException( tokens.front() -> lineNumber, s );
		}
	}

	clearFirstToken();

	if( tokens.front() -> value() != ")" ){
		throw ExpectedClosedParenthesesException( tokens.front() -> lineNumber, tokens.front() -> value() );
	}

	clearFirstToken();

	if( tokens.front() -> value() != ";" ){
		throw ExpectedSemicolonException( tokens.front() -> lineNumber, tokens.front() -> value() );
	}

}

void Parser::parseFunction( bool isVoid, const string& functionName ){
	assert( functionName != "main" );
	std::cout << "parsing " << functionName << "..." << endl;
	DeclaredFunction newFunction( isVoid, functionName );
	tnyCode << endl << endl << functionName << ":" << endl << endl;
	const string SCOPE = GLOBAL_SCOPE + functionName + "_";
	currentParamNumber = 1;
	assert( tokens.front() -> value() == "(" );
	clearFirstToken();

	if( tokens.front() -> value() == "void" ){
		clearFirstToken();
		if( tokens.front() -> value() != ")" ){
			throw ExpectedClosedParenthesesException( tokens.front() -> lineNumber, tokens.front() -> value() );
		}
		clearFirstToken();
		newFunction.paramCount = 0;
		
	}
	else{	//there are parameters

		int paramCount = 0;

		for( ; tokens.front() -> value() != ")"; ++paramCount ){
			static const char* PARAM_DELIMETER1 = ",";
			static const char* PARAM_DELIMETER2 = ")";
			auto statement = getNextStatement(tokens, SCOPE, PARAM_DELIMETER1, PARAM_DELIMETER2);
			//this handles LDPARAM as well
			handleAllDeclarations( statement, SCOPE, tnyCode, true );
			currentParamNumber++;
			for( unsigned i = 0 ; i < statement.size() && tokens.front() -> value() != ")"; ++i ){
				clearFirstToken();
			}

		}

		clearFirstToken();

		newFunction.paramCount = paramCount;
		
	}

	allDeclaredFunctions.push_back(newFunction);

	OpenCurly_Token* FUNCTION_OPEN_CURLY = dynamic_cast<OpenCurly_Token*> (tokens.front() -> ptr());

	if( !FUNCTION_OPEN_CURLY ){
		throw ExpectedOpenCurlyException( tokens.front() -> lineNumber, tokens.front() -> value() );
	}
	
	Token* last;
	while( tokens.size() > 1 && FUNCTION_OPEN_CURLY -> mate == nullptr ){
		last = tokens.front();
		runtimeParser(SCOPE, tnyCode);
	}

	if( FUNCTION_OPEN_CURLY -> mate == nullptr ){
		throw ExpectedClosedCurlyExpception( last -> lineNumber, last -> value() );
	}

	if( openCurlyStack.size() != 0 ){
		throw ExpectedClosedCurlyExpception( last -> lineNumber, last -> value() );
	}

	ClosedCurly_Token* FUNCTION_CLOSING_CURLY = dynamic_cast<ClosedCurly_Token*> (last -> ptr() );
	assert(FUNCTION_CLOSING_CURLY);

	FUNCTION_CLOSING_CURLY -> mate = FUNCTION_OPEN_CURLY;

	clearFirstToken();

	if( newFunction.isVoid ){
		tnyCode << "ret " << endl;
	}

	tnyCode << endl << " \t\t\t\t\t\t\t\t\t\t ; end of function \"" << newFunction.functionName << '\"' << endl << endl << endl;
}

void Parser::runtimeParser(string currentScope, stringstream& out){
	
	OpenCurly_Token* oc = dynamic_cast<OpenCurly_Token*>(tokens.front() -> ptr() );
	ClosedCurly_Token* cc = dynamic_cast<ClosedCurly_Token*>(tokens.front() -> ptr());
	
	if( oc ){
		openCurlyStack.push( oc );
		clearFirstToken();
	}

	else if( cc ){

		if( openCurlyStack.empty() && cc -> mate == nullptr){
			throw UnexpectedClosedCurlyException( cc -> lineNumber, cc -> value() );
		}

		if( cc -> mate == nullptr ){
			cc -> mate = openCurlyStack.top();
		}

		if( openCurlyStack.top() -> mate == nullptr ){
			openCurlyStack.top() -> mate = cc;
			openCurlyStack.pop();
		}
		//clearFirstToken();
	}

	//if declaring a variable
	else if( dynamic_cast<VariableType_Token*>( ptr(tokens.begin()) ) || dynamic_cast<VariableType_Token*>( ptr(tokens.begin() + 1) )){
		queue<Token*> declaration = getNextStatement( tokens, currentScope );
		bool clear = !handleAllDeclarations( declaration, currentScope, out );

		for( int i = declaration.size(); clear && i > 0; --i ){
			clearFirstToken();
		}

		if( !clear ){
			clearFirstToken();
		}
	}

	// if calling puts(...)
	else if( tokens.front() -> value() == "puts" ){
		handlePuts( currentScope, out );
		clearFirstToken();
	}

	else if( tokens.front() -> value() == "putn" ){
		handlePutn( currentScope, out );
		clearFirstToken();
	}

	else if( tokens.front() -> value() == "for" ){
		handleFor( currentScope , out);
		clearFirstToken();
	}

	else if( tokens.front() -> value() == "while" ){
		handleWhile( currentScope, out );
	}

	else if( tokens.front() -> value() == "else" ){
		//????
	}

	else if( tokens.front() -> value() == "if" ){
		handleIf( currentScope, out );
	}

	else if( tokens.front() -> value() == "return" ){
		clearFirstToken();
		evaluateRuntimePostfix(getPostfixExpression(), currentScope, tnyCode);
		clearFirstToken();

		if( currentScope == "g_main_" ){
			out << "stop" << endl << endl;;
		}
		else{
			//if returning a value, evaluateRuntimePostfix should've loaded it into the acc
			out << "ret" << endl << endl;
		}
	}

	//is expression
	else {
		vector<Token*> postfixExpression = getPostfixExpression();
		evaluateRuntimePostfix( postfixExpression, currentScope , tnyCode);
		clearFirstToken();
	}


		
}

void Parser::handleWhile( string& currentScope, stringstream& out  ){
	assert( tokens.front() -> value() == "while" );
	clearFirstToken();
	if( tokens.front() -> value() != "(" ){
		throw ExpectedOpenParenthesesException( tokens.front() -> lineNumber, tokens.front() -> value() );
	}
	clearFirstToken();
	string randomString = randomLabel();
	string newScope = currentScope + randomString + "_";

	string beginLabel = randomString + "_while_begin";
	string endLabel = randomString + "_end";
	out << endl << beginLabel << ": ; \t\t while_begin" << endl << endl;
	evaluateRuntimePostfix( getPostfixExpression("{"), newScope , out );
	out << "je " << endLabel << endl;
	stringstream endOfLoopCode;
	endOfLoopCode << "jmp " << beginLabel << endl;
	endOfLoopCode << endLabel << ": ; \t\t while_end" << endl << endl;

	if( tokens.front() -> value() == "{" ){
		OpenCurly_Token* myCurly = dynamic_cast<OpenCurly_Token*>(tokens.front() -> ptr() );
		
		do{
			runtimeParser(newScope, out);
		}while( myCurly -> mate == nullptr );
		clearFirstToken();
	}
	else{
		runtimeParser(newScope, out);
	}

	out << endOfLoopCode.str();
}

void Parser::handleIf( string& currentScope, stringstream& out ){
	assert( tokens.front() -> value() == "if" );
	clearFirstToken();
	if( tokens.front() -> value() != "(" ){
		throw ExpectedOpenParenthesesException( tokens.front() -> lineNumber, tokens.front() -> value() );
	}
	clearFirstToken();
	string randomString = randomLabel();
	string newScope = currentScope + randomString + "_";

	string ifLabel = randomString + "_begin";
	string elseLabel = randomString + "_else";
	string endLabel = randomString + "_if_end";
	string elseEndLabel = randomString + "_else_end";
	
	if( accountForElseIf ){
		out << "jg " << elseEndLabel << endl;
		out << "jl " << elseEndLabel << endl;
		accountForElseIf = false;
	}

	out << endl << ifLabel << ": \t\t\t; if_begin" << endl << endl;
	auto pf = getPostfixExpression("{");
	evaluateRuntimePostfix( pf, newScope , out );
	out << "je " << elseLabel << endl;

	if( tokens.front() -> value() == "{" ){
		OpenCurly_Token* ifCurly = dynamic_cast<OpenCurly_Token*>(tokens.front() -> ptr() );

		do{
			runtimeParser(newScope, out);
		}
		while( ifCurly -> mate == nullptr );
		clearFirstToken();
	}
	else{
		runtimeParser(newScope, out);
	}

	//when entering an else, the acc MUST HAVE 0 INT IT
	//loading 1 prevents falling through elses after an if
	out << "ld #1" << endl;
	out << "jmp " << elseEndLabel << endl;
	out << elseLabel << ": \t\t\t; else" << endl;

	if( tokens.front() -> value() == "else" ){
		clearFirstToken();
		if( tokens.front() -> value() == "{" ){

			OpenCurly_Token* elseCurly = dynamic_cast<OpenCurly_Token*>(tokens.front() -> ptr() );

			while( elseCurly -> mate == nullptr ){
				runtimeParser(newScope, out);
			}
			clearFirstToken();
		}
		else{
			if( tokens.front() -> value() == "if" ){
				accountForElseIf = true;
				runtimeParser(newScope, out);
			}
		}
	}

	out << elseEndLabel << ": " << endl << endl;
	

	

}

void Parser::parse(){
	accountForElseIf = false;
	cout << "extracting global declarations..." << endl;
	extractGlobalDeclarations( tokens );

	std::cout << "parsing main..." << endl;
	
	if( !dynamic_cast<IntType_Token*>(tokens.front() -> ptr() ) ){
		cout << "Error: main can only return an int." << endl;
		throw MalformedMainException( tokens.front() -> lineNumber, tokens.front() -> value() );
	}

	clearFirstToken();

	assert( tokens.front() -> value() == "main" );
	
	garbage.push_back( tokens.front() );
	tokens.erase( tokens.begin() );	

	if( tokens[0] -> value() != "(" || tokens[1] -> value() != "void" || tokens[2] -> value() != ")" || tokens[3] -> value() != "{" ){
		throw MalformedMainException( tokens[3] -> lineNumber, tokens[3] -> value() );
	}

	OpenCurly_Token* mainOpenCurly = dynamic_cast<OpenCurly_Token*>(tokens[3]-> ptr());
	openCurlyStack.push(mainOpenCurly);

	clearFirstToken();
	clearFirstToken();
	clearFirstToken();
	clearFirstToken();

	//scope string has all parent scopes encoded within
	string currentScope = GLOBAL_SCOPE + MAIN_SCOPE;
	tnyCode << endl << endl << endl << "main: " << endl;
	
	Token* last;
	while( tokens.size() > 1 ){
		last = tokens.front();
		runtimeParser(currentScope, tnyCode);
	}

	if( openCurlyStack.empty() && mainOpenCurly -> mate == nullptr ){
		throw UnexpectedClosedCurlyException(last -> lineNumber, last -> value() );
	}

	if(!openCurlyStack.empty() && openCurlyStack.top() != mainOpenCurly || openCurlyStack.size() > 1 && mainOpenCurly -> mate == nullptr ){
		throw ExpectedClosedCurlyExpception(last -> lineNumber, last -> value() );
	}
}

void Parser::handleFor( string& currentScope , stringstream& out){
	assert( tokens.front() -> value() == "for" );
	clearFirstToken();
	if( tokens.front() -> value() != "(" ){
		throw ExpectedOpenParenthesesException( tokens.front() -> lineNumber, tokens.front() -> value() );
	}
	clearFirstToken();
	string randomString = randomLabel();
	string newScope = currentScope + randomString + "_";

	//extract declaration clause -- should've probably used runtimeParser instead
	if( dynamic_cast<VariableType_Token*>( ptr(tokens.begin()) ) || dynamic_cast<VariableType_Token*>( ptr(tokens.begin() + 1) )){
		queue<Token*> declaration = getNextStatement( tokens, newScope );
		bool clear = !handleAllDeclarations( declaration, newScope , out);

		for( int i = declaration.size(); clear && i > 0; --i ){
			clearFirstToken();
		}

		if( !clear ){
			clearFirstToken();
		}
	}
	else{
		if( tokens.front() -> value() != ";" ){
			throw ExpectedSemicolonException( tokens.front() -> lineNumber, tokens.front() -> value() );
		}
	}
	string beginLabel = randomString + "_begin";
	string endLabel = randomString + "_end";
	out << beginLabel << ":" << endl;
	evaluateRuntimePostfix( getPostfixExpression(), newScope , out );
	out << "je " << endLabel << endl;
	if( tokens.front() -> value() != ";" ){
		throw ExpectedSemicolonException( tokens.front() -> lineNumber, tokens.front() -> value() );
	}
	clearFirstToken();
	stringstream endOfLoopCode;
	auto endOfLoopPostfix = getPostfixExpression("{");
	evaluateRuntimePostfix( endOfLoopPostfix, newScope, endOfLoopCode );
	endOfLoopCode << "jmp " << beginLabel << endl;
	endOfLoopCode << endLabel << ":" << endl;

	if( tokens.front() -> value() == "{" ){
		OpenCurly_Token* myCurly = dynamic_cast<OpenCurly_Token*>(tokens.front() -> ptr() );

		do{
			runtimeParser(newScope, out);
		}while( myCurly -> mate == nullptr );

	}
	else{
		runtimeParser(newScope, out);
	}

	out << endOfLoopCode.str();


	


	
}

bool Parser::handleAllDeclarations( queue<Token*> currentStatement , const string& scope, stringstream& out, bool isParameter ) {
	bool runTimeDecAssign = false;	
	int sizeOfStatement = currentStatement.size();
	bool isStillDeclaring = true;
	bool alreadyDeclaredOne = false;
	string currentType;
	while( isStillDeclaring ){
		isStillDeclaring = false;

		bool isConst = currentStatement.front() -> value() == "const";
		if( isConst ){ currentStatement.pop(); }

		if( !alreadyDeclaredOne ){
			currentType = currentStatement.front() ->  value();

			if( !dynamic_cast<VariableType_Token*>( currentStatement.front() -> ptr() ) ){
				throw ExpectedDeclarationException( currentStatement.front() -> lineNumber, currentStatement.front() -> value() );
			}

			currentStatement.pop();
		
			if( currentStatement.front() -> value() == "*" ){
				currentType += "*";
				currentStatement.pop();
			}

		}

		// now we should be left with an identifier at the front of our tokens
		const string currentIdentifier = currentStatement.front() -> value();

		if( !dynamic_cast<UserDefinedName_Token*>( currentStatement.front() -> ptr() ) ){
			throw ExpectedIdentifierException( currentStatement.front() -> lineNumber, currentStatement.front() -> value() );
		}
		auto dv = fetch( currentIdentifier, scope );
		if  ( dv != NOT_KNOWN ){
			if( dv.scope == scope ){
				throw VariableAlreadyDeclaredException( currentStatement.front() -> lineNumber, currentStatement.front() -> value() );
			}
		}

		currentStatement.pop();

		DeclaredVariable currentDeclaration( currentIdentifier, isConst );
		currentDeclaration.scope = scope;
		bool isCharPointer = currentType == "char*";

		if( isParameter ){
			if( currentStatement.front() -> value() != "," &&  currentStatement.front() ->value() != ")" ){
				throw ExpectedEndOfDeclarationException( currentStatement.front() -> lineNumber, currentStatement.front() -> value() );
			}
		}

		bool isSimpleDeclaration = currentStatement.front() -> value() == ";" || currentStatement.front() -> value() == "," || (isParameter && currentStatement.front() -> value() == ")");
		bool isArrayDeclaration  = currentStatement.front() -> value() == "[";
		bool isDeclarationAssignment = currentStatement.front() -> value() == "=";
		bool successfullyParsedDeclaration = false;

		if( isCharPointer && isDeclarationAssignment ){
			assert( !isParameter );
			handleStringDeclaration( currentDeclaration, currentStatement, currentIdentifier, scope, out );
			successfullyParsedDeclaration = true;
				
		}

		else if( isSimpleDeclaration ){
			handleSimpleDeclaration( currentDeclaration, currentStatement, currentIdentifier, scope , out );
			successfullyParsedDeclaration = true;
			if( isParameter ){
				out << "ldparam "<< currentParamNumber << endl;
				out << "st " << currentDeclaration.scope << currentDeclaration.variableName << endl;
			}
		}

		//not supporting initializer list syntax
		else if( isArrayDeclaration ){
			handleArrayDeclaration( currentDeclaration, currentStatement, currentIdentifier, scope , out );
			successfullyParsedDeclaration = true;
			if( isParameter ){
				out << "ldparam " << currentParamNumber << endl;
				out << "st " << currentDeclaration.scope << currentDeclaration.variableName << endl;
			}
		}

		else if( isDeclarationAssignment ){
			assert( !isParameter );
			if( scope == GLOBAL_SCOPE ){
				handleStaticDecInit ( currentDeclaration, currentStatement, currentIdentifier, scope , out );
			}
			else{

				runTimeDecAssign = true;
				tnyHeader << scope << currentDeclaration.variableName << ": ds 1" << endl;
				currentDeclaration.isInitialized = true;
				allDeclaredVariables.push_back( currentDeclaration );
				clearFirstToken();
				vector<Token*> pf;

				//need to know how many tokens to throw away
				//since NOW the function args are collapsed before they are passed back from getPostFixExpression, we 
				//need getPostFixExpression to count them for us... this is a bad solution but it is an easy fix
				exprTokenCount = 1;

				if( isParameter ){
					pf = getPostfixExpression(",");
				}
				else{
					pf = getPostfixExpression(";");
				}
				auto l = pf.size();

				evaluateRuntimePostfix( pf, scope , out);

				//stupid queues!!!
				for( unsigned i = 1; i < l + (exprTokenCount == 1? 0 : exprTokenCount); ++i ){
					currentStatement.pop();
				}

			}

			successfullyParsedDeclaration = true;
		}

		isStillDeclaring = !isParameter && commaFoundAtEndOfDeclaration( currentStatement.front() );
		if( isStillDeclaring ) {
			currentStatement.pop();
			alreadyDeclaredOne = true;
		}
	}

	return runTimeDecAssign;
}

void Parser::handleStringDeclaration ( DeclaredVariable& currentDeclaration, queue<Token*>& currentStatement, const string& currentIdentifier, const string& scope, stringstream& out ){
			
	//insuring against logical error here
	assert( currentStatement.front() -> value() == "=" );
	currentStatement.pop(); // popping "=" token from queue

	if( dynamic_cast<StringLiteral_Token*>( currentStatement.front() -> ptr() ) ){
		string stringValue = currentStatement.front() -> value();
		currentStatement.pop();
		currentDeclaration.knownValue = stringValue;
		currentDeclaration.arraySize = stringValue.length() + 1;
		string possible_dc = "";

		if( stringValue[0] == '\"' ){

			possible_dc = " dc ";

		}

		tnyHeader << scope << currentIdentifier << ": " << possible_dc << stringValue << "  ;\ndb 0;\n" ;

		//debug
		//std::cout << scope << currentIdentifier << ": dc " << stringValue << "  ;\ndb 0;\n" ;

		currentDeclaration.isInitialized = true;
		allDeclaredVariables.push_back( currentDeclaration );
	}

	else {
		throw ExpectedStringLiteralException( currentStatement.front() -> lineNumber, currentStatement.front() -> value() );
	}
}

void Parser::handleSimpleDeclaration ( DeclaredVariable& currentDeclaration, queue<Token*>& currentStatement, const string& currentIdentifier, const string& scope, stringstream& out ){
	
	tnyHeader << scope << currentIdentifier << ": ds 1 ; " << endl;
			
	//debug
	//std::cout << GLOBAL_SCOPE << currentIdentifier << ": ds 1 ; " << endl;

	currentDeclaration.isInitialized = false;
	allDeclaredVariables.push_back( currentDeclaration );
}

void Parser::handleArrayDeclaration( DeclaredVariable& currentDeclaration, queue<Token*>& currentStatement, const string& currentIdentifier, const string& scope, stringstream& out ){
	
	assert( currentStatement.front() -> value() == "[" );

	currentStatement.pop();

	queue<Token*> arraySizeExpression = getNextStatement( currentStatement, scope, "]" );

	vector< Token* > pf = infixToPostfix( arraySizeExpression );

	int arraySize = evaluateStaticPostfix(pf, scope);
			
	tnyHeader << scope << currentIdentifier << ": ds " << arraySize << "; " << endl;

	//debug
	//cout << scope << currentIdentifier << ": ds " << arraySize << "; " << endl;

	currentDeclaration.isInitialized = true;
	currentDeclaration.arraySize = arraySize;
	allDeclaredVariables.push_back(currentDeclaration);

	if( currentStatement.front() -> value() != "]" ){
		throw ExpectedClosingSquareBracketException( currentStatement.front() -> lineNumber, currentStatement.front() -> value() );
	}

	currentStatement.pop();
}

int Parser::getStaticValue ( Token* t ){

	Token* opR = t;

	int opRValue;

	//get opRValue
	if( !dynamic_cast<Literal_Token*>(opR -> ptr()) ){
		const DeclaredVariable opRVar = fetch( opR -> value(), GLOBAL_SCOPE);
		if ( opRVar == NOT_KNOWN ){
			throw VariableNotDeclaredInThisScopeException(opR -> lineNumber, opR -> value());
		}
		else{

			if( !opRVar.isInitialized ){
				cout << "WARNING at line " << opR -> lineNumber << ": variable \'"<< opR -> value() <<
					"\' is uninitialized; using value of 0." << endl;
				opRValue = 0;
			}

			else if ( opRVar.knownValue != "" ){
				opRValue = stoi(opRVar.knownValue);
			}

		}

	}
	else{
		if( dynamic_cast<CharLiteral_Token*>(opR -> ptr()) ){
			char c = opR -> value()[1];
			opRValue = c;
		}
		else if( dynamic_cast<IntLiteral_Token*>(opR -> ptr()) ){
			opRValue = stoi( opR -> value() );
		}
	}

	return opRValue;

}

void Parser::tinyPrintStringLiteral( const string& s , stringstream& out){
	out << "\nlda " << stringHash(s) << " ; puts( " << stringHash(s) <<  " );" << endl;
	out << "call printc" << " ; \n" << endl;
}

void Parser::tinyPrintStringVariable ( const DeclaredVariable& v , stringstream& out){
	out << "\nlda " << v.scope <<  v.variableName << " ; puts( " << v.variableName << " );" << endl;
	out << "call printc" << " ; \n" << endl;
}

void Parser::declareLiterals(){
	
	std::sort(literals.begin(), literals.end());
	literals.erase(std::unique(literals.begin(), literals.end()), literals.end());
	
	tnyHeader << endl << endl << "; literals:\n" << endl;

	for( auto s : literals ){

		//char to int
		if( s[0] == '\'' && s.length() == 3 ){
			char c = s[1];
			int i = c;
			s = to_string(i);
		}
		else{
			bool isString = s[0] == '\"' || s[0] == '\n' || s[0] == '\\' || s[0] == '\r' || s[0] == '\0' || s[0] == '\t';
			if( isString ){
				string possible_dc = "";
				if( s[0] == '\"' ){
					possible_dc = " dc ";
				}
				tnyHeader << stringHash(s) << ": " << possible_dc << s << "\n" << "db 0 ;\n" << endl;
			}
			else{
				tnyHeader << "#" << s << ": db " << s << endl;
			}
		}
	}
}

//evaluates and leaves in accumulator
void Parser::evaluateRuntimePostfix( vector<Token*> postfixExpression, const string& scope, stringstream& out ){
	stack<CompositeToken*> localGarbage;
	stack<Token*> operandStack;

	for( auto t : postfixExpression ){

		//if( t is operand )
		if( t -> ptr() -> precedence == -1 ){
			if( dynamic_cast<IntLiteral_Token*>( t -> ptr() ) || dynamic_cast<CharLiteral_Token*>( t -> ptr() )){
				literals.push_back(t -> value() );
				out << "ld #" << t -> value() << " ; \n";
			}

			else if( dynamic_cast<UserDefinedName_Token*>( t -> ptr() ) ){
				
				auto dv = fetch( t -> value(), scope );
				
				if( dv == NOT_KNOWN ){
					throw VariableNotDeclaredInThisScopeException( t -> lineNumber, t -> value() );
				}

				if( dv.isNotArray() ){
					out << "ld " << dv.scope << dv.variableName << " ; \n";
				}
				else{
					out << "lda " << dv.scope << dv.variableName << " ; \n";
				}
			}

			out << "push" << endl;
			operandStack.push(t);
		}
		else if( !!dynamic_cast<FunctionCall_Token*> ( t -> ptr()) ){
			auto fToken = dynamic_cast<FunctionCall_Token*> ( t -> ptr());
			auto df = fetchFunction( fToken -> value() );

			if( df == NOT_KNOWN_F ){
				throw UnknownFunctionException(fToken -> lineNumber, fToken -> value() );
			}

			//string temp = randomLabel();
			//tnyHeader << temp << ": ds 1 ; temp storage while calling \"" << fToken -> value() << "\" function" << endl;
				
			//if ( function has args )
			static Token extraComma( ",", fToken -> lineNumber );
			fToken -> argumentTokens.push_back( &extraComma );

			if( fToken -> argumentTokens.size() > 1 ){
				

				//another consequence of bad design
				//TODO NEXT: need multiple vectors bc of RL PINKY RULE
				vector< vector<Token*> > args;

				auto fArgs = fToken -> argumentTokens;
				for( int i = 0; !fArgs.empty() || i == 0; ++i ){
					queue<Token* > pfe = getNextStatement( fArgs , scope, "," );
					fArgs.erase( fArgs.begin(), fArgs.begin() + pfe.size() );
					args.push_back( vector<Token*> () );
					do{
						args[i].push_back(pfe.front());
						pfe.pop();
					}while( pfe.front() -> value() != ",");
				}

				if( df.paramCount != args.size() ){
					throw IncorrectNumberOfParametersException( fToken -> lineNumber, fToken -> value() );
				}

				//right pinky rule
				reverse< vector<vector<Token*>>::iterator > ( args.begin(), args.end() );

				//at this point, whatever
				for( auto argTokens : args ){
					evaluateRuntimePostfix( argTokens, scope, out );
					out << "push ; passing parameter" << endl;
				}
			}

			out << "call " << fToken -> value() << ";		calling function \"" << fToken -> value() << '\"' << endl;
			//out << "push" << endl;
			operandStack.push( t );
		}
		else { //operator

			if( operandStack.size() < 2 ){
				throw InvalidExpresssionException( t->lineNumber, t->value());
			}

			Token* opR = operandStack.top();
			operandStack.pop();
			Token* opL = operandStack.top();
			operandStack.pop();
			CompositeToken* ct = new CompositeToken( opL -> value() +  " " + t -> value() + " " + opR -> value() );
			localGarbage.push(ct);
			operandStack.push(ct);

			//loading right operand
			if( !dynamic_cast<FunctionCall_Token*>(opR -> ptr() ) ){
				out << "pop ; loading value of "<< opR -> value() << endl; 
			}
			
			out << "st " << temp << endl;
			out << "pop ; loading value of "<< opL -> value() << endl;
			//essentially a big switch -- load result of operation into the acc
			if( t -> value() == "+" ){
				out << "add " << temp << " ; " << opL -> value() << " " << t -> value() << " " << opR -> value() << endl;
			}

			else if( t  -> value() == "-" ){
				out << "sub " << temp << " ; " << opL -> value() << " " << t -> value() << " " << opR -> value() << endl;
			}

			else if( t -> value() == "*" ){
				out << "mul " << temp << " ; " << opL -> value() << " " << t -> value() << " " << opR -> value() << endl;
			}
			else if( t -> value() == "/" ){
				out << "div " << temp << " ; " << opL -> value() << " " << t -> value() << " " << opR -> value() << endl;
			}
			else if( t -> value() == "%" ){
				//not yet supported
				throw FeatureNotImplementedYetException( t -> lineNumber, t -> value() );
			}
			else if( t -> value() == "[" ){
				if( !dynamic_cast<CompositeToken*>(opL) ){
					auto dv = fetch( opL -> value(), scope );
					if( dv.isNotArray() ){
						cout << "WARNING (line " << opL -> lineNumber << "): " << ": \'" << opL -> value() << "\' is not an array. " << endl;
					}
				}
				else{
					throw ExpectedArrayException( opL -> lineNumber, opL -> value() );
				}

				string pointerLocation = randomLabel();
				CompositeToken* dereferenceExpression = dynamic_cast<CompositeToken*> (operandStack.top());
				assert( dereferenceExpression );

				dereferenceExpression -> compositeValue += " ] ";
				dereferenceExpression -> isArrayDereference = true;
				dereferenceExpression -> addressOfPointer = pointerLocation;
				
				tnyHeader << pointerLocation << ": ds 1 ; pointer used for dereferencing in expression " << dereferenceExpression -> value() << endl;

				out << "; just loaded address of " << opL -> value() << endl;
				out << "add " << temp << "; adding offset [" << opR -> value() << "]" << endl;
				out << "st " << pointerLocation << " ; special pointer for dereferencing " << opL -> value() << endl;
				out << "ldi " << pointerLocation << endl;
				

			}
			
			else if( t -> value() == "<" ){
				string trueLabel = randomLabel();
				string endLabel = randomLabel();
				out << "\nsub " << temp << " ; " << opL -> value() << " " << t -> value() << " " << opR -> value() << endl;
				out << "jl " << trueLabel << endl;
				out << "ld #0" << endl;
				out << "jmp " << endLabel << endl;
				out << trueLabel << ":" << endl;
				out << "ld #1" << endl;
				out << endLabel << ":" << endl;
			}
			else if( t -> value() == "<=" ){
				string trueLabel = randomLabel();
				string falseLabel = randomLabel();
				string endLabel = randomLabel();
				out << "\nsub " << temp << " ; " << opL -> value() << " " << t -> value() << " " << opR -> value() << endl;
				out << "jl " << trueLabel << endl;
				out << "je " << trueLabel << endl;
				out << "ld #0" << endl;
				out << "jmp " << endLabel << endl;
				out << trueLabel << ":" << endl;
				out << "ld #1" << endl;
				out << endLabel << ":" << endl;
				
			}
			else if( t -> value() == ">" ){
				string trueLabel = randomLabel();
				string endLabel = randomLabel();
				out << "\nsub " << temp << " ; " << opL -> value() << " " << t -> value() << " " << opR -> value() << endl;
				out << "jg " << trueLabel << endl;
				out << "ld #0" << endl;
				out << "jmp " << endLabel << endl;
				out << trueLabel << ":" << endl;
				out << "ld #1" << endl;
				out << endLabel << ":" << endl;
			}
			else if( t -> value() == ">=" ){
				string trueLabel = randomLabel();
				string endLabel = randomLabel();
				out << "\nsub " << temp << " ; " << opL -> value() << " " << t -> value() << " " << opR -> value() << endl;
				out << "jg " << trueLabel << endl;
				out << "je " << trueLabel << endl;
				out << "ld #0" << endl;
				out << "jmp " << endLabel << endl;
				out << trueLabel << ":" << endl;
				out << "ld #1" << endl;
				out << endLabel << ":" << endl;
			}
			else if( t -> value() == "||" ){
				string trueLabel = randomLabel();
				string endLabel = randomLabel();

				out << "\njg " << trueLabel << " ; " << opL -> value() << " " << t -> value() << " " << opR -> value() << endl;
				out << "jl " << trueLabel << " ; " << endl;
				out << "ld " << temp << endl;
				out << "jl " << trueLabel << endl;
				out << "\njg " << trueLabel << " ; " << endl;
				out << "ld #0" << endl;
				out << "jmp " << endLabel << endl;
				out << trueLabel << ": " << endl;
				out << "ld #1" << endl;
				out << endLabel << ": " << endl;
				
			}
			else if( t -> value() == "&&" ){
				string falseLabel = randomLabel();
				string endLabel = randomLabel();

				out << "\nje " << falseLabel << " ; " << opL -> value() << " " << t -> value() << " " << opR -> value() << endl;
				out << "ld " << temp << endl;
				out << "je " << falseLabel << endl;
				out << "ld #1" << endl;
				out << "jmp " << endLabel << endl;
				out << falseLabel << ": " << endl;
				out << "ld #0" << endl;
				out << endLabel << ": " << endl;
			}
			else if( t -> value() == "!=" ){
				string falseLabel = randomLabel();
				string endLabel = randomLabel();
				out << "\nsub " << temp << " ; " << opL -> value() << " " << t -> value() << " " << opR -> value() << endl;
				out << "je " << falseLabel << endl;
				out << "ld #1" << endl;
				out << "jmp " << endLabel << endl;
				out << falseLabel << ":" << endl;
				out << "ld #0" << endl;
				out << endLabel << ":" << endl;
			}
			else if( t -> value() == "==" ){
				string trueLabel = randomLabel();
				string endLabel = randomLabel();
				out << "\nsub " << temp << " ; " << opL -> value() << " " << t -> value() << " " << opR -> value() << endl;
				out << "je " << trueLabel << endl;
				out << "ld #0" << endl;
				out << "jmp " << endLabel << endl;
				out << trueLabel << ":" << endl;
				out << "ld #1" << endl;
				out << endLabel << ":" << endl;
			}
			else if( t -> value() == "=" ){
				CompositeToken* ctp = dynamic_cast<CompositeToken*>(opL);
				if( ctp ){
					//must be array dereference
					if( ! ctp -> isArrayDereference ){
						throw ExpectedArrayException( opL -> lineNumber, opL -> value() );
					}

					out << "\nld " << temp << " ; "  << opL -> value() << " " << t -> value() << " " << opR -> value() << endl; 
					out << "sti " << ctp -> addressOfPointer << " ; dereferencing and storing... " << endl;
				}
				else{
					//out << "\st " << temp2 << " ; ";
					out << "\nld " << temp << " ; "  << opL -> value() << " " << t -> value() << " " << opR -> value() << endl;
					auto dv = fetch( opL ->value(), scope );
					if( dv == NOT_KNOWN ){
						throw VariableNotDeclaredInThisScopeException( opL -> lineNumber, opL -> value() );
					}
					else if( dv.isConst() ){
						throw ModifyingConstVariableException( opL -> lineNumber, opL -> value() );
					}

					out << "st " << dv.scope << dv.variableName << endl;
				}
			}

			else throw InvalidExpresssionException(t -> lineNumber, t -> value() );

			out << "push" << endl;


		}

	}



	if( operandStack.size() != 1 && !dynamic_cast<FunctionCall_Token*> (postfixExpression.front() -> ptr() )){
		throw InvalidExpresssionException( postfixExpression.front() -> lineNumber, postfixExpression.front() -> value() );
	}

	out << "pop ; " << ( ( !dynamic_cast<FunctionCall_Token*> (postfixExpression.front() -> ptr() ))? "result of expression " + operandStack.top() -> value() : "result of call to " + postfixExpression.front() -> value()  ) << endl;

	while( !localGarbage.empty() ){
		delete localGarbage.top();
		localGarbage.pop();
	}

	

}

int Parser::evaluateStaticPostfix( vector<Token*> postfixExpression, const string& scope ){
	stack<int> operandStack;

	for( auto t : postfixExpression ){

		//lazy way of saying if( t is operand )
		if( t -> ptr() -> precedence == -1 ){
			int operandValue = getStaticValue(t);
			operandStack.push(operandValue);
		}

		else{ //operator
			if( operandStack.size() < 2 ){
				throw InvalidExpresssionException( t->lineNumber, t->value());
			}

			int opR = operandStack.top();
			operandStack.pop();
			int opL = operandStack.top();
			operandStack.pop();

			int resultOfOperation = performStaticOperation( opL, opR, t );
			operandStack.push( resultOfOperation );


		}

	}

	if( operandStack.size() != 1 ){
		throw InvalidExpresssionException( postfixExpression.front() -> lineNumber, postfixExpression.front() -> value() );
	}

	return operandStack.top();
	
}

void  Parser::handleStaticDecInit( DeclaredVariable& currentDeclaration, queue<Token*>& currentStatement, const string& currentIdentifier, const string& scope, stringstream& out ){
	assert( currentStatement.front() -> value() == "=" );

	currentStatement.pop();

	int resultOfAssignmentExpression = evaluateStaticPostfix( infixToPostfix (currentStatement), scope );
	
	tnyHeader << scope << currentDeclaration.variableName << ": db " << resultOfAssignmentExpression << "; \n";

	//debug
	//cout << scope << currentDeclaration.variableName << ": db " << resultOfAssignmentExpression << "; \n";

	currentDeclaration.isInitialized = true;
	currentDeclaration.knownValue = to_string( resultOfAssignmentExpression );

	allDeclaredVariables.push_back( currentDeclaration );

}

int Parser::performStaticOperation( int& opL, int& opR  , const Token* operation ){

	if( operation -> value() == "+" ){
		return opL + opR;
	}
	else if( operation  -> value() == "-" ){
		return opL - opR;
	}
	else if( operation -> value() == "*" ){
		return opL * opR;
	}
	else if( operation -> value() == "/" ){
		return opL / opR;
	}
	else if( operation -> value() == "%" ){
		return opL % opR;
	}
	else if( operation -> value() == "<" ){
		return opL < opR;
	}
	else if( operation -> value() == "<=" ){
		return opL <= opR;
	}
	else if( operation -> value() == ">" ){
		return opL > opR;
	}
	else if( operation -> value() == ">=" ){
		return opL >= opR;
	}
	else if( operation -> value() == "||" ){
		return opL || opR;
	}
	else if( operation -> value() == "&&" ){
		return opL && opR;
	}
	else if( operation -> value() == "!=" ){
		return opL != opR;
	}
	else if( operation -> value() == "==" ){
		return opL == opR;
	}

	throw InvalidStaticOperationException(operation -> lineNumber, operation -> value() );

	return 0;


}

bool Parser::commaFoundAtEndOfDeclaration( const Token* t ){
	bool isStillDeclaring = false;

	if ( t -> value() == "," ){
		isStillDeclaring = true;
	}

	else if( t -> value() != ";" ) {
		throw ExpectedEndOfDeclarationException( t -> lineNumber, t -> value() );
	}

	return isStillDeclaring;
}

vector<Token*> Parser::infixToPostfix ( queue<Token*>& currentExpression, const char* delim ){
	stack<Token*> operatorStack;
	vector<Token*> result;


	for( auto t  = currentExpression.front(); currentExpression.front() -> value() != ";" && currentExpression.front() -> value() != "," && currentExpression.front() -> value() != delim; t = currentExpression.front() ){
		currentExpression.pop();

		bool isOperand = dynamic_cast<IntLiteral_Token*>( t -> ptr()) || dynamic_cast<CharLiteral_Token*>( t -> ptr()) || dynamic_cast<UserDefinedName_Token*>( t -> ptr());
		bool isOperator =  t -> value() == ")" || t -> value() == "(" || t -> value() == "[" || t -> value() == "]" ||  !!(dynamic_cast<BinaryOperator_Token*>( t -> ptr() ) );
		
		if( isOperand ){

			result.push_back( t );
			assert( !isOperator );

		}

		else if( isOperator ){
			Terminal* cur = t -> ptr();
			assert( t -> ptr() -> precedence != -1 );
			
			if( cur -> value() == ")" ){
																// Might need to make this < ...
				while( !operatorStack.empty() && operatorStack.top() -> ptr() -> value() != "(" ){
					Token* p = operatorStack.top();
					operatorStack.pop();
					result.push_back(p);
				}

				//wont be needing parentheses anymore
				if(!operatorStack.empty()){
					operatorStack.pop();

				}

			}

			else if( cur -> value() == "(" ){
				operatorStack.push(t);
			}
															
			else {
																 // Might need to make this < ...
				while( !operatorStack.empty() && operatorStack.top() -> ptr() -> precedence <= cur -> precedence ){
					Token* p = operatorStack.top();
					operatorStack.pop();
					if( p -> value() != "]" ){
						result.push_back(p);
					}
				}

				operatorStack.push(t);
			}

		}
	}

	while( !operatorStack.empty() ){
		result.push_back( operatorStack.top() );
		operatorStack.pop();
	}

	return result;
}

vector<Token*> Parser::getPostfixExpression ( string delim ){
	stack<Token*> operatorStack;
	vector<Token*> result;


	for( auto t  = tokens.front(); tokens.front() -> value() != ";" && tokens.front() -> value() != delim; t = tokens.front() ){
		//FUNCTION_CALL_OPERATOR_PRECEDENCE

		bool isOperand;
		bool isOperator;

		//function call case -- this code is ERROR PRONE and FRAGILE, very unfortunate consequence of poor design
		if( !!dynamic_cast<UserDefinedName_Token*>( t -> ptr()) && tokens[1] -> value() == "(" ){
			isOperand = true;
			isOperator = false;
			auto call = new FunctionCall_Token( t -> value(), t -> lineNumber, FUNCTION_CALL_OPERATOR_PRECEDENCE );
			Token* newToken = new Token();
			newToken -> lineNumber = call -> lineNumber;
			newToken -> T = call;
			
			//clear function name token from tokens
			clearFirstToken();

			//clear ( token
			stack<OpenParentheses_Token*> openParStack;
			OpenParentheses_Token* firstOpenPar = dynamic_cast<OpenParentheses_Token*>(tokens.front() -> ptr() );
			openParStack.push( firstOpenPar );
			clearFirstToken();

			vector<Token*> args;
			if( tokens.front() -> value() != "void" ){
				int count = 0;
				for( auto itr = tokens.begin(); firstOpenPar -> mate == nullptr && itr < tokens.end() && !openParStack.empty() ; ++itr, ++count ){
					auto o = dynamic_cast<OpenParentheses_Token*>(ptr(itr));
					auto c = dynamic_cast<ClosedParentheses_Token*>(ptr(itr));
					if( !!o ){
						openParStack.push(o);
						call -> argumentTokens.push_back( tokens[count] );
					}
					else if( !!c ){
						openParStack.top() -> mate = c;
						c -> mate = openParStack.top();
						openParStack.pop();
						if( !openParStack.empty() ){
							call -> argumentTokens.push_back( tokens[count] );
						}
					}
					else{
						call -> argumentTokens.push_back( tokens[count] );
					}
					
					
				}

				for( int i = 0; i  < count; ++i ){
					clearFirstToken();
					exprTokenCount++;
				}
			}
			else{
				clearFirstToken();
				exprTokenCount += 2;
				if( tokens.front() -> value() != ")" ){
					throw ExpectedClosedParenthesesException( tokens.front() -> lineNumber, tokens.front() -> value() );
				}
				clearFirstToken();
			}

			t = newToken;
			//tokens.insert( tokens.begin(), newToken );
		}
		
		else{
			clearFirstToken();
			isOperand = dynamic_cast<IntLiteral_Token*>( t -> ptr()) || dynamic_cast<CharLiteral_Token*>( t -> ptr()) || dynamic_cast<UserDefinedName_Token*>( t -> ptr());
			isOperator =  t -> value() == ")" || t -> value() == "(" || t -> value() == "[" || t -> value() == "]" ||  !!(dynamic_cast<BinaryOperator_Token*>( t -> ptr() ) );
		}

		if( isOperand ){

			result.push_back( t );
			assert( !isOperator );

		}

		else if( isOperator ){
			Terminal* cur = t -> ptr();
			assert( t -> ptr() -> precedence != -1 );
			
			if( cur -> value() == ")" ){
																// Might need to make this < ...
				while( !operatorStack.empty() && operatorStack.top() -> ptr() -> value() != "(" ){
					Token* p = operatorStack.top();
					operatorStack.pop();
					result.push_back(p);
				}

				//wont be needing parentheses anymore
				if(!operatorStack.empty()){
					operatorStack.pop();

				}

			}

			else if( cur -> value() == "(" ){
				operatorStack.push(t);
			}
															
			else {
																 // Might need to make this < ...
				while( !operatorStack.empty() && operatorStack.top() -> ptr() -> precedence <= cur -> precedence ){
					Token* p = operatorStack.top();
					operatorStack.pop();
					if( p -> value() != "]" ){
						result.push_back(p);
					}
				}

				operatorStack.push(t);
			}

		}
	}

	while( !operatorStack.empty() ){
		result.push_back( operatorStack.top() );
		operatorStack.pop();
	}

	return result;
}