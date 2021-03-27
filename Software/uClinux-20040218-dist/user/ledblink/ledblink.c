#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

volatile int a=0;

int main() {
	int i;

	while (1) {
		*(unsigned char*)0x07E000 = 0xF0;
		for (i=0; i<1000; i++) { a++; }
		*(unsigned char*)0x07E000 = 0x0F;
		for (i=0; i<1000; i++) { a++; }
	}
}
