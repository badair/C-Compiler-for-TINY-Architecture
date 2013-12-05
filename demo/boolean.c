int fun( int a, int b, int c , int d ){
	
	int result = 0 - 1;
	
	if( a == b && b == c && c == d ){
		return 99999 - a;
	}
	
	else if( a >= b && a >= c && a >= d ){
		result = a;
	}
	
	else if( d - c - b - a == 0 || a - b - c - d == 0 ){
		result = 0;
	}
	
	return result;
}

int main(){

	int i = fun( 1, 1, 1, 1 );
	
	char* s = "\n";
	
	putn( i );
	puts( "\n" );
	
	i = fun( 6, 6, 5, 1 );
	
	putn(i);
	puts(s);
	
	i = fun( 10, 5, 3, 2 );
	
	putn(i);
	puts(s);
	
	i = fun( 2, 3, 5, 10);
	
	putn(i);
	puts(s);
	
	i = fun( 0, 10, 123, 14 );
	
	putn(i);
	puts(s);	
	
	return 0;
}