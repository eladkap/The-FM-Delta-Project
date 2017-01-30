#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PACKET_MAX_LEN 100 // max length in bytes

// definitions

typedef struct packet_t{
	int length;
	char array[PACKET_MAX_LEN];
}Packet;



// declarations

int min(int a, int b);

int getFileSize(FILE* fp);

int calcBitmapSizeInBits(int f1_size, int f2_size, int block_size);

void setBitmap(Packet file1, Packet file2, int* bitmap, int bitmap_size);

void showBitmap(int* bitmap, int bitmap_size);

void writeFileToArray(char* filename, Packet* packets, int packets_num);

void convertBytesArrayToBitsArray(int* bytesArray, int bytes, int* bitsArray);

void writeArraysToFile(char* filename,Packet* deltas, int packets_num);

int deltaEncode(Packet file1, Packet file2, Packet* delta, int* bitmap, int bitmap_size);

int deltaDecode(Packet file1, Packet delta, Packet* file2);



// implementation

int min(int a, int b){
	if (a<b){
		return a;
	}
	else{
		return b;
	}
}

int getFileSize(FILE* fp){
	fseek(fp, 0L, SEEK_END);
	int sz = ftell(fp);
	return sz;
}

/*
block_size: 1 or 2 or3 or 4 bytes: the size of block in bytes to be compared for bitmap
*/
int calcBitmapSizeInBits(int f1_size, int f2_size, int block_size){
	int bitmap_size=0;
	int min_file_size;
	min_file_size=min(f1_size,f2_size);

	while (bitmap_size<min_file_size){
		bitmap_size+=8;
	}

	return bitmap_size/block_size;
}



void setBitmap(Packet file1, Packet file2, int* bitmap, int bitmap_size){
	int c=0;
	int bitmap_index=0;
	char byte1;
	char byte2;
	int len1=file1.length;
	int len2=file2.length;
		

	// scan files
	int i=0;
	int j=0;
	while(i<len1 && j<len2){
		byte1=file1.array[i];
		byte2=file2.array[j];
		
		// bytes are identical
		if (byte1==byte2){
			bitmap[bitmap_index]=1;
		}
		else{
			bitmap[bitmap_index]=0;
		}
		bitmap_index++;
		i++;
		j++;
	}

	// padding bitmap with zeros
	while(bitmap_index<bitmap_size){
		bitmap[bitmap_index]=0;
		bitmap_index++;
	}

}

void showBitmap(int* bitmap, int bitmap_size){
	for (int i=0; i<bitmap_size; i++){
		printf("%d ",bitmap[i]);
	}
	printf("\n");
}

void showArray(char* file, int size){
	for (int i=0; i<size; i++){
		printf("%c",file[i]);
	}
	printf("\n");
}

void writeFileToArray(char* filename, Packet* packets, int packets_num){
	FILE* fp;
	int counter=0; // counts the number of bytes in the packets file
	int i=0; // index of the current byte of the packet
	int packet_index=0;
	char byte;
	int len; // saves the length of the current packet
	int is_new_packet=1; // flag indicates if the current byte is the packet len (first byte of the packet)
	

	// open packets file
	fp = fopen(filename, "r");
	if (!fp){
		fprintf(stderr,"\nerror: file not found\n");
		return;
	}
	//printf("writing...\n");
	while(!feof(fp) && packet_index<packets_num){
		fread(&byte,sizeof(char),1,fp);
		counter++;
		//printf("byte = %x\n",byte);
		if (is_new_packet==1){
			i=0;
			is_new_packet=0;
			//len=byte-'0'; // for checking txt files
			len=byte;
			//printf("packet index=%d\n",packet_index);
			packets[packet_index].length=len;
		}
		else{
			packets[packet_index].array[i]=byte;	
			i++;
			len--;
			if (len==0){
				is_new_packet=1;
				packets[packet_index].array[i]=EOF;
				i=0;
				packet_index++;
			}
		}
		
	}
	//printf("counter: %d\n",counter);
	fclose(fp);
	//printf("Done.\n");
}

void writeArraysToFile(char* filename, Packet* deltas, int packets_num){
	FILE* dp;
	
	// open packets file
	dp = fopen(filename, "w");
	if (!dp){
		fprintf(stderr,"\nerror: file not found\n");
		return;
	}	

	for (int d=0; d<packets_num; d++){
		int len=deltas[d].length;
		//fprintf(dp,"%c",len+'0'); // for cheching txt files
		fprintf(dp,"%c",len);
		for (int i=0; i<len; i++){
			char byte=deltas[d].array[i];
			fprintf(dp,"%c",byte); // write byte to delta file
		}
	}

	fclose(dp);
}

int deltaEncode(Packet file1, Packet file2, Packet* delta, int* bitmap, int bitmap_size){
	char byte1;
	char byte2;
	char byte;

	int len1=file1.length;
	int len2=file2.length;

	int delta_index=0;
	
	// write file2 size in bytes
	//printf("##### %d\n",file2.length);
	delta->array[delta_index++]=file2.length;

	byte=0;
	int bitmap_index=0;
	// writing bitmap to delta array
	while (bitmap_index<bitmap_size){
		byte=0;
		for (int shifts_left=7; shifts_left>=0; shifts_left--){
			char tmp=0;
			if (bitmap[bitmap_index]==1){
				tmp=tmp|0x1;
			}
			
			tmp=tmp<<shifts_left;
			bitmap_index++;
			byte=byte+tmp;
		}
		
		delta->array[delta_index++]=byte; // writing byte to delta file
	}
	//printf("Done.\n");

	
	int i=0;
	int j=0;
	// writing different bytes from file2
	while(i<len1 && j<len2){
		byte1=file1.array[i];
		byte2=file2.array[j];
		
		// bytes are not identical
		if (byte1!=byte2){
			delta->array[delta_index++]=byte2; // writing byte to delta file
		}
		bitmap_index++;
		i++;
		j++;
	}

	// write tail or padding of file2
	while(j<len2){
		byte2=file2.array[j];
		//fprintf(stderr,"put %d , ",byte2);
		delta->array[delta_index++]=byte2; // writing byte to delta file
		j++;
		//fprintf(dp,"#");
	}


	delta->length=delta_index;
	//printf("delta len=%d\n",delta_index);		
	
	
	return 1; // success
}


void convertBytesArrayToBitsArray(int* bytesArray, int bytes, int* bitsArray){
	int j=0;
	char byte;
	unsigned int bitmask; // mask = 1000 000 B
	for (int i=0;i<bytes; i++){
		bitmask=128;
		for (int shifts_right=7; shifts_right>=0; shifts_right--){
			//printf("bitmask %x\n",bitmask);
			byte = bytesArray[i];
			//printf("# %x\n",byte);			
			
			byte = byte & bitmask;
			//printf("## %x\n",byte);
		
			byte = byte >> shifts_right;
			//printf("### %x\n",byte);
			
			bitsArray[j]=byte;
			//printf("#### %x\n",byte);
			j++;
			bitmask/=2;
			//printf("\n");

		}
		//printf("\n");
		
	}
	//printf("\n");
}

int deltaDecode(Packet file1, Packet delta, Packet* file2){
	FILE* fp2;
	int bitmap_size;
	int* bitmap_bytes;
	int* bitmap_bits;
	int min_size;
	int i,j;
	char byte;
	int file2_size;
	int delta_index=0;
	int block_size=1; // 1 bytes

	// get the bitmap length in bytes
	file2_size=delta.array[delta_index++];
	//printf("$$$$%d  %d\n",file1.length,file2_size);
	bitmap_size=calcBitmapSizeInBits(file1.length,file2_size, block_size)/8;
	
	j=0; // index of file2
	//file2->array[j++]=file2_size;
	file2->length=file2_size;

	// get min file size
	min_size=min(file1.length,file2_size);
	
	// allocate bitmap array
	bitmap_bytes=(int*)malloc(sizeof(int)*bitmap_size);
	if (!bitmap_bytes){
		fprintf(stderr,"memory error\n");
		return 0;
	}
	bitmap_bits=(int*)malloc(sizeof(int)*bitmap_size*8);
	if (!bitmap_bits){
		//free(bitmap_bytes);
		fprintf(stderr,"memory error\n");
		return 0;
	}

	// write to bitmap bytes from the delta file array
	for (i=0; i<bitmap_size; i++){
		bitmap_bytes[i]=delta.array[delta_index++];
	}

	// create bitmap array in bits
	convertBytesArrayToBitsArray(bitmap_bytes,bitmap_size,bitmap_bits);
	//showBitmap(bitmap_bits,bitmap_size*8); // for check

	int b=0; // index of bitmap array
	i=0; // index of file1
	

	for (b=0; b<min_size; b++){
		// the two bytes of the two files are different
		if (bitmap_bits[b]==0){
			file2->array[j++]=delta.array[delta_index++];
			i++;
		}
		// the two bytes of the two files are identical
		else{
			file2->array[j++]=file1.array[i];
			i++;
		}
	}
	

	
	// write tail of delta to file2
	for (; j<file2_size; j++){
		file2->array[j]=delta.array[delta_index++];
	}


	free(bitmap_bytes);
	free(bitmap_bits);

	

	return 1; // success
}



void showPackets(Packet* packets, int packets_num){
	for (int p=0; p<packets_num; p++){
		int i=0;
		printf("len=%d\n",packets[p].length);
		while(i<packets[p].length){
			printf("%c",packets[p].array[i]);
			i++;
		}
		printf("\n");
	}
}


/* 	parameters of main function:
	argv[1] = -e for encode , -d for decode
	argv[2] = packets_file_name (-e)   or    compressed_file_name (-d)
	argv[3] = compressed_file_name (-e)   or   source_file_name (-d)
	argv[4] = packets_number
*/
int main(int argc, char* argv[]){
	FILE* packets_fp; // packets file pointer
	FILE* compressed_fp; // compressed file pointer

	Packet* packets;
	Packet* deltas;

	int packets_num;
	int packets_file_size;

	char* file1_array;
	char* file2_array;
	char* delta_array;

	FILE* fp1; // file1 pointer for encode
	FILE* fp2; // file2 pointer for encode
	FILE* dp; // delta file pointer for encode - output file

	FILE* fp1_decode; // file1 pointer for decode
	FILE* dp_decode; // delta pointer for decode
	FILE* fp2_decode; // file2 file pointer for decode - output file
	
	int f1_size=0; // file1 size in bytes
	int f2_size=0; // file2 size in bytes
	int delta_size=0; // delta file size in bytes

	int bitmap_size=0; // bitmap size in bits
	int* bitmap; // bitmap array
	
	float ratio=0.0; // compression ratio

	int result=0;

	int block_size=1; // 1 bytes

	if (argc != 5){
		fprintf(stderr,"\nError: wrong parameters 1.\n");
		return -1;
	}
	

	// encode
	if (strcmp(argv[1],"-e")==0){

		// get packets number
		packets_num = atoi(argv[4]);
		


		// allocate packets array
		packets=(Packet*)malloc(sizeof(struct packet_t)*packets_num);
		if (!packets){
			fprintf(stderr,"\nerror: memory alloc\n");
			return 1;
		}
		
		// write packets file to packets array
		writeFileToArray(argv[2],packets,packets_num);

		//showPackets(packets,packets_num);


		// allocate deltas array
		deltas=(Packet*)malloc(sizeof(struct packet_t)*packets_num);
		if (!deltas){
			fprintf(stderr,"\nerror: memory alloc\n");
			return 1;
		}

		// copy packet1 to delta0
		deltas[0].length=packets[0].length;
		for (int i=0; i<packets[0].length; i++){
			char ch=packets[0].array[i];
			deltas[0].array[i]=ch;
		}
		

		// create delta arrays
		for (int p=0; p<packets_num-1; p++){
			int len1=packets[p].length;
			int len2=packets[p+1].length;
			bitmap_size=calcBitmapSizeInBits(len1,len2,block_size);
			
			// alocate bitmap array
			bitmap=(int*)malloc(sizeof(int)*bitmap_size);
			if (!bitmap){
				fprintf(stderr,"\nerror: memory alloc\n");
				return 1;
			}
			
			setBitmap(packets[p],packets[p+1],bitmap,bitmap_size);

			//showBitmap(bitmap,bitmap_size);

			deltaEncode(packets[p],packets[p+1],&deltas[p+1],bitmap,bitmap_size);

			free(bitmap);
		}


		// check packets and deltas arrays
		
		//showPackets(deltas,packets_num);



		// write delta arrays to compressed file
		writeArraysToFile(argv[3],deltas,packets_num);

		// free arrays
		free(packets);
		free(deltas);

	} 

	// decode




	else if (strcmp(argv[1],"-d")==0){
		
		// get packets number
		packets_num = atoi(argv[4]);


		// allocate deltas array
		deltas=(Packet*)malloc(sizeof(struct packet_t)*packets_num);
		if (!deltas){
			fprintf(stderr,"\nerror: memory alloc\n");
			return 1;
		}
		
		// write deltas file to deltas array
		writeFileToArray(argv[2],deltas,packets_num);



		// allocate packets array
		packets=(Packet*)malloc(sizeof(struct packet_t)*packets_num);
		if (!packets){
			fprintf(stderr,"\nerror: memory alloc\n");
			return 1;
		}

		// copy delta0 to packet1
		packets[0].length=deltas[0].length;
		for (int i=0; i<deltas[0].length; i++){
			char ch=deltas[0].array[i];
			packets[0].array[i]=ch;
		}


		// create packets arrays
		for (int d=0; d<packets_num-1; d++){
			//printf("%d   %d   \n",packets[d].length,deltas[d+1].length);
			deltaDecode(packets[d],deltas[d+1],&packets[d+1]);
		}
		//showPackets(packets,packets_num);



		// check packets and deltas arrays
		//showPackets(packets,packets_num);
		//showPackets(deltas,packets_num);



		// write delta arrays to compressed file
		writeArraysToFile(argv[3],packets,packets_num);

		// free arrays
		free(packets);
		free(deltas);
	}

	else{
		fprintf(stderr,"\nerror: wrong parameters: -e or -d expected\n");
		return 1;	
	}
	
	
	int c=0x28;
	printf("%c\n",c);

	
	return 0;
}

