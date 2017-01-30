#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILE_LEN 11
#define MAX_NUM_DIG 5


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

void printByteBinary(int byte){
	printf("%c",byte);
}

int count_digits(int num){
	int count=0;
	while (num>0){
		num/=10;
		count++;
	}
	return count;
}

void my_itoa(int num, char str[MAX_NUM_DIG]){
	int len=count_digits(num);
	
	int i=len-1;
	str[len]=0;
	while (num>0){
		int x=num%10;
		num/=10;
		str[i]=x+'0';
		i--;
	}
}

void set_filename(char* filename, int num){
	int i;
	char num_str[MAX_NUM_DIG];
	my_itoa(num,num_str);
	
	int len=count_digits(num);
	for (i=1; i<=len; i++){
		filename[i]=num_str[i-1];
	}
	filename[i]='.';
	filename[i+1]='b';
	filename[i+2]='i';
	filename[i+3]='n';
	filename[i+4]=0;
}

/* args:
	argv[1] - filename
	argv[2] - packets num
	argv[3] - packet file prefix name (one char)
*/
int main(int argc, char **argv){
	FILE* fp;
	FILE* packet_fp;
	int k=0;
	
	char prefix=argv[3][0];
		
	char filename[MAX_FILE_LEN]="p1.bin";
	
	filename[0]=prefix;
	
	fp=fopen(argv[1],"rb");
	if (!fp){
		fprintf(stderr,"\nerror: file not found\n");
		return 0;
	}
	
	int packets_num=atoi(argv[2]);
	
	int packet_len=0;
	int p=1; // packet num
	
	int new_packet_flag=1;
	while(!feof(fp) && p<=packets_num)
	{
		fread(&k,sizeof(char),1,fp);
		
		if (new_packet_flag==1){ // packet length is read
			packet_len=k;
			new_packet_flag=0;
			
			// open new file for writing new packet
			packet_fp = fopen(filename, "w+");
			if (!packet_fp){
				fprintf(stderr,"\nerror: file %d not found\n",p);
				return 0;
			}
			//fprintf(stderr,"file %d created.\n",p);
			
		}
		
		else{ // data byte is read
			fprintf(packet_fp,"%c",k); // writing byte
			packet_len--;
			
			// end of current packet
			if (packet_len==0){
				new_packet_flag=1;
				fclose(packet_fp);
				p++;
				set_filename(filename,p);
			}
		}

	}
	
	
	fclose(fp);
	return 0;
}