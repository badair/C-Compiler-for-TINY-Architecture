const int good1 = 10, bad1, bad2, good2 = 7;
const int i = 3 + 4*5/6;
const int j = (300+23)*(43-21)/(84+7);
char character = 'T';
int cint = character;
const int constK = j;
int notConstK = constK;

int another = ((3 - j )*(2+2))/(4+8)*(6-5);

const char * constStr = "this is a const \n string Im declaring.";
char * s = "this is a string Im declaring.";
const char* eol = "\n";

const char arrayA[ good1 ];
char arrayB [ bad1 ];
char arrayC [ good2 ];

int main(){
	putn( good1 );
	puts( "\n" );
	
	putn( good2 );
	puts( "\n" );
	
	putn( bad1 );
	puts(eol);
	
	putn( bad2 );
	puts( "\n" );
	
	putn( i );
	puts("\n");
	
	putn( j );
	puts("\n");
	
	badFunction();

	return 0;
}

