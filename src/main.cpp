#include <iostream>
#include <fstream>
#include <vector>
#include <cstddef>
#include "lexer.h"
#include "parser.h"

using std::ifstream;
using std::cout;
using std::cerr;
using std::vector;
using std::string;

void deleteTokens(vector<Token*>& v);


int main(int argc, char *argv[]){
	
	ifstream fin(argv[1]);
	vector<Token*> tokens;

	try{
		Lexer lexer( fin );
		tokens = lexer.tokenizedCode();
		Parser parser( tokens, argv[1] );
	}
	catch( TinyCException& e ){
		cout << e.what();
		cout << endl << "Failed to compile file \'" << argv[1] << "\'.";

		deleteTokens(tokens);
		//cin.get();
		return 1;
	}

	for( auto p : tokens ){
		//cout << "\"" << p -> value() << "\" is a " << p -> name();
		//cin.get();
	}

	deleteTokens(tokens);
	cout << endl << argv[1] << " successfully compiled.";
	cin.get();
	return 0;
}

void deleteTokens(vector<Token*>& v){
	int i = 0;
	for( auto p : v ){
		if( p != nullptr ) delete p;
		else cout << "hmm... token at index " << i << " in main::tokens is null";
		++i;
	}
}