#ifndef PARSER_H
#define PARSER_H
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <regex>
#include <map>
#include <vector>
#include <queue>
#include <stack>
#include "tokens.h"
#include <typeinfo>

struct DeclaredVariable{

	const string variableName;
	string knownValue;
	bool isConstant;
	int arraySize;
	bool isInitialized;
	string scope;

	bool isConst()const{ return isConstant; }
	bool isNotArray()const { return arraySize == -1;}
	DeclaredVariable( const string& s, bool _isConst, const string& knownV = "", int i = -1 ): variableName(s), knownValue(knownV), arraySize(i), isConstant(_isConst), isInitialized(false){};

	
};

struct DeclaredFunction{

	DeclaredFunction( bool isV, const string& name ): isVoid( isV ), functionName( name ), paramCount(-1){};
	const bool isVoid;
	const string functionName;
	int paramCount;
	
};

typedef vector<Token*>::iterator TokenItr;

struct Parser{
	
	Parser(vector<Token*>& tokens, const char* fileName);
	~Parser();

	stringstream tnyHeader; //variable declarations
	stringstream tnyCode;
	stringstream tnyInit;

	string temp;
	string temp2;
	ofstream tny;
	vector< DeclaredVariable > allDeclaredVariables;
	vector< DeclaredFunction > allDeclaredFunctions;
	vector< string > literals;
	vector<Token*> garbage;
	stack<OpenCurly_Token*> openCurlyStack;
	vector<Token*>& tokens;

	//used for hacky bug fix
	int exprTokenCount;

	//hack used for dangling else, etc
	bool accountForElseIf;

	int currentParamNumber;

	void parse();
	void parseFunction						(bool isVoid, const string& functionName);
	void runtimeParser						( string currentScope, stringstream& out );
	void extractGlobalDeclarations			( vector<Token*>& tokens );
	void handlePuts							( string& currentScope, stringstream& out );
	void handleFor							( string& currentScope, stringstream& out );
	void handleWhile						( string& currentScope , stringstream& out);
	void handleIf							( string& currentScope, stringstream& out );
	DeclaredFunction& fetchFunction			( const string& functionName );
	void handleStringDeclaration			( DeclaredVariable& currentDeclaration, queue<Token*>& currentStatement, const string& currentIdentifier, const string& scope, stringstream& out );
	void handleSimpleDeclaration			( DeclaredVariable& currentDeclaration, queue<Token*>& currentStatement, const string& currentIdentifier, const string& scope, stringstream& out );
	void handleArrayDeclaration				( DeclaredVariable& currentDeclaration, queue<Token*>& currentStatement, const string& currentIdentifier, const string& scope, stringstream& out );
	void handleStaticDecInit				( DeclaredVariable& currentDeclaration, queue<Token*>& currentStatement, const string& currentIdentifier, const string& scope, stringstream& out );
	bool handleAllDeclarations				( queue<Token*> currentStatement, const string& scope, stringstream& out, bool isParameter  = false);
	int getStaticValue						( Token* t );
	void tinyPrintStringLiteral				( const string& s , stringstream& out);
	void handlePutn							( string& currentScope, stringstream& out );
	void tinyPrintStringVariable			( const DeclaredVariable& v , stringstream& out);
	int performStaticOperation				( int& opL, int& opR , const Token* operation );
	queue<Token*> getNextStatement			( vector<Token*>& tokens, const string& scope, const string&, const string& );
	queue<Token*> getNextStatement			( queue<Token*>& tokens, const string& scope, const string&  );
	vector<Token*> infixToPostfix			( queue<Token*>& currentExpression, const char* delim = "]" );
	vector<Token*> getPostfixExpression		(string delim = ";");
	int evaluateStaticPostfix				( vector<Token*> postfixExpression, const string& scope );
	void evaluateRuntimePostfix				( vector<Token*> postfixExpression, const string& scope, stringstream& out );
	bool commaFoundAtEndOfDeclaration		( const Token* t );
	DeclaredVariable& fetch					( const string& variableName, const string& scope );
	vector<string> availableScopes			( const string& scope );
	void declareLiterals					();
	//utility function... could have engineered Token class better, oh well
	Terminal*& ptr( TokenItr itr ){ return (*itr) -> ptr();};

	inline void clearFirstToken(){
		if( tokens.size() != 0 ){
			garbage.push_back( tokens.front() );
			tokens.erase( tokens.begin() );
		}
	}

	string randomLabel() {
		static vector<int> hashes;
		hash<string> hash_fn;
		const int maxLabelLength = 10;
		const char pool[] =
			"##abcdefghijklmnopqrstuvwxyz1234567890";
		char randomizedLabel[maxLabelLength + 1];

		bool alreadyUsed = false;
		int resultHash;
		string result;

		//loop combined with hashes vector ensures that we don't accidentally return the same string twice
		do{
			for (int i = 1; i < maxLabelLength; ++i) {
				randomizedLabel[i] = pool[rand() % (sizeof(pool) - 1)];
			}
			randomizedLabel[0] = '#';
			randomizedLabel[maxLabelLength] = '\0';
		
			result = string(randomizedLabel);
			resultHash = hash_fn( result );

			for( int i : hashes ){
				if(resultHash == i){
					alreadyUsed = true;
				}
			}

			for( const auto& dv : allDeclaredVariables ){
				if( dv.variableName == result ){
					alreadyUsed = true;
				}
			}

		}while( alreadyUsed );

		hashes.push_back(hash_fn(result));

		return randomizedLabel;
	}


	//to clear a queue effectively
	void clear( std::queue<Token*> &q ){
		queue<Token*> empty;
		swap( q, empty );
	}
};

#endif