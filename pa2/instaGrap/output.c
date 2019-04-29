#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void main() {
	int x = 0;
	char c[10] = {0x0, };
	int result = 0;
	scanf("%s", c);

	int len = strlen(c);
	c[len] = 0x0;	
	x = atoi(c);
	result = x*x;
	printf("%d\n", result);
}
