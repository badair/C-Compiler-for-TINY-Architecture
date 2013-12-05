//lexer.h
#ifndef LEXER_H
#define LEXER_H
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <regex>
#include <queue>
#include "tokens.h"

#define collapseDoubleWhitespace(s) do { for ( unsigned int f = s.find("  ") ; f < s.length(); f = s.find("  ", f ) ) { 	s.erase(f,1); } }while(0)

class Lexer {


	std::ifstream& fin;
	std::ofstream  fout;
	std::ofstream  fout2;
	std::queue< std::string >  extractedStringLiterals;
	std::vector<Token*> result;
	static const std::string stringLiteralPlaceholder;
	static const std::string linefeedPlaceholder;
public:

	Lexer( std::ifstream& f ) :
		fin(f),
		fout("lexed.c"),
		fout2("lexed2.c")

	{
		cout << "Tokenizing...\n";
		std::string s;



		while( std::getline( fin, s ) ){

			//extract quoted strings from code
			for( 	size_t  firstQuote = s.find("\""),
							lengthOfQuote = s.find("\"", firstQuote + 1) - firstQuote + 1;
					firstQuote < s.length();

					firstQuote = s.find("\"", firstQuote + 1),
					lengthOfQuote = s.find("\"", firstQuote + 1) - firstQuote + 1

					){

				extractedStringLiterals.push( s.substr( firstQuote, lengthOfQuote) );
				s.replace( firstQuote, lengthOfQuote, stringLiteralPlaceholder );
			}


			size_t f;

			//TODO: need to extract escape sequence characters (cause crash!)

			//code structure symbols
			for ( f = s.find(";") ; f < s.length(); f = s.find(";", f + 2 ) ) {
				s.erase(f,1).insert( f, " ; " );
			}

			for ( f = s.find("(") ; f < s.length(); f = s.find("(", f + 2 ) ) {
				s.erase(f,1).insert( f, " ( " );
			}

			for ( f = s.find(")") ; f < s.length(); f = s.find(")",  f + 2 ) ) {
				s.erase(f,1).insert( f, " ) " );
			}

			for ( f = s.find("}") ; f < s.length(); f = s.find("}",  f + 2 ) ) {
				s.erase(f,1).insert( f, " } " );
			}

			for ( f = s.find("{") ; f < s.length(); f = s.find("{",  f + 2 ) ) {
				s.erase(f,1).insert( f, " { " );
			}

			for ( f = s.find(",") ; f < s.length(); f = s.find(",", f + 2 ) ) {
				s.erase(f,1).insert( f, " , ");
			}

			//square brackets
			for ( f = s.find("[") ; f < s.length(); f = s.find("[",  f + 4 ) ) {
				s.erase(f,1).insert( f, " [ ( " );
			}

			for ( f = s.find("]") ; f < s.length(); f = s.find("]",  f + 4 ) ) {
				s.erase(f,1).insert( f, " ) ] " );
			}
			
			/*
			for ( f = s.find("[") ; f < s.length(); f = s.find("[",  f + 2 ) ) {
				s.erase(f,1).insert( f, " [ " );
			}

			for ( f = s.find("]") ; f < s.length(); f = s.find("]",  f + 2 ) ) {
				s.erase(f,1).insert( f, " ] " );
			}*/


			//arithmetic operators - we don't worry about inc and dec operators
			for ( f = s.find("+") ; f < s.length(); f = s.find("+", f + 2 ) ) {
				s.erase(f,1).insert( f, " + " );
			}

			for ( f = s.find("-") ; f < s.length(); f = s.find("-", f + 2 ) ) {
				s.erase(f,1).insert( f, " - " );
			}

			for ( f = s.find("/") ; f < s.length(); f = s.find("/", f + 2 ) ) {
				s.erase(f,1).insert( f, " / " );
			}

			for ( f = s.find("*") ; f < s.length(); f = s.find("*", f + 2 ) ) {
				s.erase(f,1).insert( f, " * " );
			}



			//boolean operators
			for ( f = s.find("<") ; f < s.length(); f = s.find("<", f + 2 ) ) {
				s.erase(f,1).insert( f, " < " );
			}

			for ( f = s.find(">") ; f < s.length(); f = s.find(">", f + 2 ) ) {
				s.erase(f,1).insert( f, " > " );
			}

			for ( f = s.find("=") ; f < s.length(); f = s.find("=", f + 2 ) ) {
				s.erase(f,1).insert( f, " = " );
			}

			for ( f = s.find("||") ; f < s.length(); f = s.find("||", f + 2 ) ) {
				s.erase(f,2).insert(f, " || ");
			}

			for ( f = s.find("!") ; f < s.length(); f = s.find("!", f + 2 ) ) {
				s.erase(f,1).insert( f, " ! ");
			}



			collapseDoubleWhitespace(s);




			//fix ==, >=, <=
			for ( f = s.find("= =") ; f < s.length(); f = s.find("= =", f + 2 ) ) {
				s.erase(f + 1,1);
			}

			for ( f = s.find("> =") ; f < s.length(); f = s.find("> =", f + 2 ) ) {
				s.erase(f + 1,1);
			}

			for ( f = s.find("< =") ; f < s.length(); f = s.find("< =", f + 2 ) ) {
				s.erase(f + 1,1);
			}

			for ( f = s.find("! =") ; f < s.length(); f = s.find("! =", f + 2 ) ) {
				s.erase(f + 1,1);
			}



			collapseDoubleWhitespace(s);


			f = s.find("/ /");
			if( f < s.length() ) s.erase(f + 1, 1);

			for ( f = s.find("/ *") ; f < s.length(); f = s.find("/ *", f + 2 ) ) {
				s.erase(f + 1, 1);
			}

			for ( f = s.find("* /"); f < s.length(); f = s.find("* /", f + 2 ) ) {
				s.erase(f + 1, 1);
			}

			collapseDoubleWhitespace(s);
			//put void between all empty parentheses
			for ( f = s.find("( )"); f < s.length(); f = s.find("( )", f + 7 ) ) {
				s.erase(f, 1).insert(f, "( void");
			}

			s += linefeedPlaceholder;

			fout << " " << s << " " << std::endl;

			//debug
			//std::cout << " " << s << " " << std::endl;
		}

	}

	std::vector<Token*> tokenizedCode(){

		std::ifstream tokenFeed("lexed.c");

		//debug...?
		vector<string> stringTokens;

		std::string s;
		int currentLine = 1;
		int tokensOnLine = 0;

		while( tokenFeed >> s ){

			if( s == linefeedPlaceholder ){
				currentLine++;
				tokensOnLine = 0;

				//debug
				fout2 << std::endl;
				for( const auto& str : stringTokens ){
					fout2  << std::setw(40) << str;
				}

				fout2 << std::endl << std::endl;

				stringTokens.clear();
			}
			else{
				tokensOnLine++;

				
				//TODO - fix comment stuff
				//may want to save these to put in tiny assembly code source file
				size_t lineComment = s.find("//");
				if( lineComment < s.length() ) s.erase(lineComment);

				//size_t blockCommentStart = s.find("/*"); fix?
				//size_t blockCommentEnd = s.find("*/");

				if ( s == stringLiteralPlaceholder ){
					s = extractedStringLiterals.front();
					extractedStringLiterals.pop();
				}

				//debug...?
				stringTokens.push_back(s);

				//debug
				//std::cout << "s in main: " << s << std::endl << "press enter\n";
				//cin.get();



				result.push_back( new Token( s , currentLine) );

				//debug
				fout2 << std::setw(40) << result.back() -> name();
				
			}
		}

		
		tokenFeed.close();



		return result;
	}

	~Lexer(){
		fout.close();
		fout2.close();
	}


};

const std::string Lexer::stringLiteralPlaceholder = "_EXTRACTED_STRING_LITERAL_";
const std::string Lexer::linefeedPlaceholder = "_LF_";

#endif
