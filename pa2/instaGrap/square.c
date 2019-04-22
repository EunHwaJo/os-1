#include <stdio.h>

int main(int argc, char * argv[]) {

	printf("this is square.c\n");
	
	int x = atoi(argv[1]);
	int result = x*x;

	printf("result : %d\n", result);
	return result;
}
