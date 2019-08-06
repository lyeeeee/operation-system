#include <stdio.h>

int main(){
	int a = 0;
	int (*i) = &a;
	*i = 1;
	printf("a = %d\n",a);
	return 0;
}
