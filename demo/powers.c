
int powers(){

	int result = 2;
	int counter = 1;
	while( result < 4096 ){
		result = result * 2;
		puts( "power of 2 #");
		putn( counter );
		puts( ": ");
		putn(result);
		puts("\n");
		counter = counter + 1;
	}

	return result * 2;
}

int main(){
	int lastPower = powers();
	puts( "last power of 2: ");
	putn(lastPower);
	
	return 0;
}