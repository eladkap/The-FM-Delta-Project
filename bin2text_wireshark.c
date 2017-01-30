#include <stdio.h>

#define BUF_SIZE 8


void printByteHex(int byte){
	if (byte>0xf){
		printf("%x ",byte);
	}
	else{
		printf("0%x ",byte);
	}
}

void printByteDec(int byte){
	printf("%d ",byte);
}

void nextAddress(char* address){
	if (address[2]=='f'){
		address[2]='0';
		if (address[1]=='f'){
			address[1]='0';
			address[0]++;
		}
		else address[1]++;
	}
	else{
		address[2]++;
	}
}


// args: argv[1] - fiename
int main(int argc, char **argv){
	FILE* fp;
	int k=0;
	
	fp=fopen(argv[1],"rb");
	if (!fp){
		fprintf(stderr,"\nerror: file not found\n");
		return 0;
	}
	
	int bytes_per_line=0;
	int line=0;
	
	char address[]="0000";
	printf("%s ",address);
	nextAddress(address);
	
	while(!feof(fp))
	{
		fread(&k,sizeof(char),1,fp);
		if (bytes_per_line==16){
			bytes_per_line=1;
			printf("\n%s ",address);
			
			line++;
			nextAddress(address);
			
			printByteHex(k);
		}
		else{
			printByteHex(k);
			bytes_per_line++;
		}

	}
	
	fclose(fp);
	return 0;
}
