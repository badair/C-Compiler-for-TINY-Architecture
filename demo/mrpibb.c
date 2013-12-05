

int main(){

	char* limerick = "__ bottles of Mr. Pibb on the wall\n";

	for( int bottlesA = '0' + 9; bottlesA != 31; bottlesA = bottlesA - 1 ){
		
		if( bottlesA == '0'){
			bottlesA = 32;
		}

		limerick[0] = bottlesA;

		for( int bottlesB = '0' + 9; bottlesB >= '0'; bottlesB = bottlesB - 1 ){
			
			limerick[1] = bottlesB;
			puts(limerick);
		}
	}

	return 0;
}