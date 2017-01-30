/*	this file consists utilities for cfm.h and bfd.h
	such as vlan structure and functions
*/

#ifndef UTILITIES_H_
#define UTILITIES_H_

#include <limits.h>

#define MIN_MAC (LONG_MAX-2000000)
#define MAX_MAC (LONG_MAX-1000000)

#define MAX_VLANS_NUM 2

#define BUF_SIZE 16
#define MAX_VLANS_NUM 2
#define MAC_SIZE 6

typedef struct vlan_tag_t{
	int tpid; // Tag Protocol Identifier 0x8100 for first vlan OR 0x88a8 for second vlan - 16 bits
	int priority; // 3 bits;
	int dei; // 1 bit - Drop eligible indicator
	int vid; // vlan id - 12 bits: 1 - 4094 (0,4095 are reserved)
}Vlan_tag;


long randomize_mac_address();
void randomize_ip_address(int ip_address[4]);


// Vlans
void showVlans(Vlan_tag* vlans[MAX_VLANS_NUM]);

void write_vlan_to_file(FILE* file, Vlan_tag* vlan_tag);

Vlan_tag* create_random_vlan(int position, int vlans_num);

short convert_vlan_low_16bits(int priority, int dei, int vid);

unsigned char* intToArray(long value, int bytes);

char get_short_low_byte(short x);

char get_short_high_byte(short x);


// implementation

long randomize_mac_address(){
	long mac_address=(rand() % (MAX_MAC-MIN_MAC+1)) + MIN_MAC; // rand() % (B-A+1)+A
	return mac_address;
}

void randomize_ip_address(int ip_address[4]){
	for (int i=0; i<4; i++){
		ip_address[i]=(rand() % (254-1+1)) + 1; // random 1-254
	}
}


void showVlans(Vlan_tag* vlans[MAX_VLANS_NUM]){
	if (vlans[0]==NULL){
		printf("<No vlans>\n");
		return;
	}
	for (int i=0; i<MAX_VLANS_NUM; i++){
		if (vlans[i]==NULL){
			return;
		}
		printf("vlan %d\n",i);
		printf("VLAN TPID: %x\n",vlans[i]->tpid);
		printf("VLAN priority: %d\n",vlans[i]->priority);
		printf("VLAN DEI: %d\n",vlans[i]->dei);
		printf("VLAN vid: %d\n",vlans[i]->vid);
	}
}

void write_vlan_to_file(FILE* file, Vlan_tag* vlan){
	unsigned char buffer[BUF_SIZE];
	short tpid=vlan->tpid;
	
	buffer[0]=get_short_high_byte(tpid);
	buffer[1]=get_short_low_byte(tpid);
	
	fwrite(buffer,1,2,file);
	
	short vlan_low_16bit=convert_vlan_low_16bits(vlan->priority,vlan->dei,vlan->vid);
	buffer[0]=get_short_high_byte(vlan_low_16bit);
	buffer[1]=get_short_low_byte(vlan_low_16bit);
	fwrite(buffer,1,2,file);
}

// if position=1 then tpid=0x8100, if position=2 then tpid=0x88a8
Vlan_tag* create_random_vlan(int position, int vlans_num){
	Vlan_tag* vlan_tag=(Vlan_tag*)malloc(sizeof(struct vlan_tag_t));
	if (vlans_num==1){
		vlan_tag->tpid=0x8100;
	}
	else{ // vlans_num==2
		if (position==1){
			vlan_tag->tpid=0x88a8; // unique value for Ethernet
		}
		if (position==2){
			vlan_tag->tpid=0x8100; // unique value for Ethernet
		}
	}
	
	vlan_tag->priority=rand() % 8;
	vlan_tag->dei=rand() % 2;
	vlan_tag->vid=(rand() % 4093) + 1; // vid=1...4094
	return vlan_tag;
}


void create_random_vlans(int vlans_num, Vlan_tag* vlans[MAX_VLANS_NUM]){
	int i=0;
	for (i=0; i<vlans_num; i++){
		vlans[i]=create_random_vlan(i+1,vlans_num);
	}
	//for (; i<MAX_VLANS_NUM; i++){
	//	vlans[i]=NULL;
	//}	
}


// short = 2 bytes
short convert_vlan_low_16bits(int priority, int dei, int vid){
	short result=priority;
	result<<=13;
	short dei_short=dei;
	dei_short<<=12;
	short vid_short=vid;
	result+=dei_short;
	result+=vid_short;
	return result;
}





// converts int to real MAC address
unsigned char* intToArray(long value, int bytes){
	unsigned char* buffer=malloc(sizeof(char)*bytes);
	int i=0;
	for (i=0; i<bytes; i++){
		buffer[i]='0';
	}
	i=0;
	while (value>0  && i<bytes){
		int x=value%10;
		value/=10;
		int y=value%10;
		buffer[i++]=y*10+x;
		value/=10;
	}
/*
	for (int i=0; i<bytes/2; i++){
		unsigned char tmp=buffer[i];
		buffer[i]=buffer[bytes-i-1];
		buffer[bytes-i-1]=tmp;
	}
*/
	return buffer;
}

char get_short_low_byte(short x){
	//x = x & 0x0F;
	char result=x;
	return result;
}

char get_short_high_byte(short x){
	x>>=8;
	char result=x;
	return result;
}


#endif
