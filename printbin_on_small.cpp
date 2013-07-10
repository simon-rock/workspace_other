#include <iostream>
#include <string.h>
using namespace std;

template<typename T>
void printbin_small(T t)
{
	int bufsize = sizeof(T);
	char *buf = new char[bufsize];
	memcpy(buf, (void*)(&t), bufsize);

	for(int i = bufsize - 1; i >=0; i--)
	{
		for(int j = 7; j >= 0; j--) 
		{
			printf("%d", (buf[i]>> j)&0x01);
		}
	}
	printf("\n");
}

union   
{  
	int number;  
	char s;  
}test;  

bool testBigEndin()  
{  
	test.number=0x01000002;  
	return (test.s==0x01);  
} 

int main()
{
	if (testBigEndin())    
		cout<<"big"<<endl;  
	else   
		cout<<"small"<<endl;  

	int ipara = 1;
	double dpara = 30.345673423123454;
	float fpara = dpara;
	printbin_small<int>(ipara);
	printbin_small<double>(dpara);
	printf("   ");
	printbin_small<float>(fpara);
	double dpara2 = -30.345673423123454;
	printbin_small<double>(dpara2);

	return 0;
}