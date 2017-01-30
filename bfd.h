#ifndef BFD_H_
#define BFD_H_

#include "utilities.h"

#define DESC_RANG 1000000
#define INTERVAL 3330
#define IP_HEADER_SIZE 20
#define UDP_HEADER_SIZE 8
#define INIT_BYTES_NUM_BFD 66


/* BFD packet */
/* BFD packet length: 66 - 74 bytes */

int packets_bfd_total_length=0;

struct packet_bfd_t{
	// Ethernet Header -   40-48 bytes (vlan is optional)
	long destination_mac_address; // 6 bytes
	long source_mac_address; // 6 bytes
	
	int ethertype; // 2 bytes
	
	int vlans_num; // not a field in the packet itself
	Vlan_tag* vlans[MAX_VLANS_NUM]; // each vlan: 32 bits = 4 bytes
	
	int source_ip[4]; // source IP is const for each MEP
	
	char ip_header[IP_HEADER_SIZE]; // 20 bytes - constant values
	char udp_header[UDP_HEADER_SIZE]; // 8 bytes - constant values
	// Ethernet Header end
	
	
	// BFD Packet -   24 bytes
	int version; // 3 bits - set to 1
	int diag; // 5 bits
	
	// flags octet
	int state; // 2 bits
	int poll; // 1 bit
	int final; // 1 bit
	int control; // 1 bit
	int authentication; // 1 bit
	int demand; // 1 bit
	int multipoint; // 1 bit
	
	int detect; // 1 byte
	int length; // 1 byte
	
	int my_disc; // 4 bytes
	int your_disc; // 4 bytes
	
	int min_tx_interval; // 4 bytes
	int min_rx_interval; // 4 bytes
	int min_echo_rx_interval; // 4 bytes
	
	// auth sector is not in
	// if authentication==1 then additional fields
	//int auth_type; // 1 byte
	//int auth_len; // 1 byte
	//char auth_data[2]; // 2 bytes
	// BFD Packet end
};
typedef struct packet_bfd_t Packet_bfd;


/*------------------------Functions declarations-------------------------------------------*/

char convert_ver_diag_octet(Packet_bfd* packet);

char convert_flags_bfd_octet(Packet_bfd* packet);

void show_bfd_packet(Packet_bfd* packet);

Packet_bfd* createPacket_bfd(long source_mac, int source_ip[4], Vlan_tag* vlans[MAX_VLANS_NUM], int vlans_num);

void write_vlans_to_file_bfd(FILE* file,Packet_bfd* packet);

void write_int_as_4_bytes(FILE* file, int x);

void write_ethernet_header_to_file(FILE* file, Packet_bfd* packet);

void write_bfd_packet_fields_to_file(FILE* file, Packet_bfd* packet);

void createVlansForPacket_bfd(Packet_bfd* packet, Vlan_tag* vlans[MAX_VLANS_NUM], int vlans_num);

/*------------------------Functions Implementation-------------------------------------------*/

char convert_ver_diag_octet(Packet_bfd* packet){
	char result=packet->version;
	result<<=5;
	result+=packet->diag;
	return result;
}

char convert_flags_bfd_octet(Packet_bfd* packet){
	char result=packet->state;
	result<<=6;
	
	char r1=packet->poll;
	r1<<=5;
	
	char r2=packet->final;
	r2<<=4;
	
	char r3=packet->control;
	r3<<=3;
	
	char r4=packet->authentication;
	r4<<=2;
	
	char r5=packet->demand;
	r5<<=1;
	
	char r6=packet->multipoint;
	result+=r1+r2+r3+r4+r5+r6;
	return result;
}


void show_IP_address(int ip[4]){
	printf("source IP: %d.%d.%d.%d\n",ip[0],ip[1],ip[2],ip[3]);
}

void show_bfd_packet(Packet_bfd* packet){
	printf("BFD\n");
	printf("packet length: %d\n",packet->length);
	printf("packet number of vlans: %d\n",packet->vlans_num);
	printf("version: %d\n",packet->version);
	printf("diag: %d\n",packet->diag);
	
	show_IP_address(packet->source_ip);
	
	printf("source_mac_address:	%ld\n",packet->source_mac_address);
	printf("destination_mac_address: %ld\n",packet->destination_mac_address);
	showVlans(packet->vlans);
	printf("\n\n");
}

Packet_bfd* createPacket_bfd(long source_mac, int source_ip[4],Vlan_tag* vlans[MAX_VLANS_NUM], int vlans_num){
	Packet_bfd* packet=malloc(sizeof(struct packet_bfd_t));
	
	// Ethernet Header
	long destination_mac_address = randomize_mac_address(); // arbitrary destination MAC address
	packet->source_mac_address = source_mac;
	packet->destination_mac_address = destination_mac_address;
	
	createVlansForPacket_bfd(packet,vlans,vlans_num);
	
	// set IP header
	packet->ip_header[0]=0x45; // IP version = 4
	
	packet->ip_header[2]=0; // total length including data
	packet->ip_header[3]=packet->vlans_num*4+66;
	
	packet->ip_header[4]=rand() % 256; // identification
	packet->ip_header[5]=rand() % 256;
	
	packet->ip_header[6]=0; // flag, fragement offset
	packet->ip_header[7]=0;
	
	packet->ip_header[8]=255; // TTL>0
	
	packet->ip_header[9]=17; // protocol = 17 (UDP)
	
	 
	
	for (int i=1; i<IP_HEADER_SIZE; i++){
		if (i!=9 && i!=8 && i!=2 && i!=3 && i!=4 && i!=5 && i!=6 && i!=7){
			packet->ip_header[i] = rand() % 256;
		}
	}

	
	packet->udp_header[4]=0; // length = udp header+data (BFD packet)
	packet->udp_header[5]=8+24; 
	packet->udp_header[2]=0x0e; // destination port = 3784 (for BFD encapsulation)
	packet->udp_header[3]=0xc8;
	
	// set random source ip address
	for (int i=0; i<4; i++){
		packet->source_ip[i]=source_ip[i];
		packet->ip_header[i+12]=packet->source_ip[i];
	}
	
	
	
	// set UDP header
	for (int i=0; i<UDP_HEADER_SIZE; i++){
		if (i!=4 && i!=5 && i!=2 && i!=3){
			packet->udp_header[i] = rand() % 256;
		}
	}
	
	
	
	// BFD Packet
	packet->version = 1;
	packet->diag = rand() % 32;
	packet->state = rand() % 4;
	
	packet->poll = rand() % 2;
	packet->final = rand() % 2;
	packet->control = rand() % 2;
	packet->authentication = 0; // no authentication
	packet->demand = rand() % 2;
	packet->multipoint = rand() % 2;
	
	packet->detect = rand() % 256;
	
	packet->my_disc = rand() % DESC_RANG;
	packet->your_disc = rand() % DESC_RANG;
	
	packet->min_tx_interval= INTERVAL;
	packet->min_rx_interval= INTERVAL;
	packet->min_echo_rx_interval = INTERVAL;
	
	//packet->auth_type = rand() % 256;
	//packet->auth_len = rand() % 256;
	//packet->auth_data[0] = rand() % 256;
	//packet->auth_data[1] = rand() % 256;
	
	packet->length = 24; // bfd packet length
	
	packets_bfd_total_length+=packet->length; // for checking
	
	show_bfd_packet(packet);
	
	return packet;
}

void write_int_as_4_bytes(FILE* file, int x){
	unsigned char buffer[BUF_SIZE];
	int bytes[4];
	int bits_shift=24;
	
	for (int i=0; i<4; i++){
		bytes[i]=x;
		bytes[i]>>=bits_shift;
		bits_shift-=8;
		buffer[i]=bytes[i];
	}
	
	fwrite(buffer,1,4,file);
}

void write_vlans_to_file_bfd(FILE* file,Packet_bfd* packet){
	for (int i=0; i<packet->vlans_num; i++){
		write_vlan_to_file(file,packet->vlans[i]);
		//free(packet->vlans[i]);
	}
}

void write_ethernet_header_to_file(FILE* file, Packet_bfd* packet){
	unsigned char buffer[BUF_SIZE];
	unsigned char* buffer2;
	char octet;

	// destination_MAC_address - 6 octets
	buffer2=intToArray(packet->destination_mac_address,MAC_SIZE);
	fwrite(buffer2,1,6,file);
	free(buffer2);
	
	// source_MAC_address - 6 octets
	buffer2=intToArray(packet->source_mac_address,MAC_SIZE);
	fwrite(buffer2,1,6,file);
	free(buffer2);
	
	// vlan tag - 0 or 4 bytes or 8 bytes
	write_vlans_to_file_bfd(file,packet);

	// ethertype=0x0800 (IP)
	buffer[0]=0x08;
	buffer[1]=0x00;
	fwrite(buffer,1,2,file);
	//Ethernet Header end
	
	
	// IP header - 20 octets
	for (int i=0; i<IP_HEADER_SIZE; i++){
		octet=packet->ip_header[i];
		buffer[0]=octet;
		fwrite(buffer,1,1,file);
	}
	
	// UDP header - 8 octets
	for (int i=0; i<UDP_HEADER_SIZE; i++){
		octet=packet->udp_header[i];
		buffer[0]=octet;
		fwrite(buffer,1,1,file);
	}
	
}

void write_bfd_packet_fields_to_file(FILE* file, Packet_bfd* packet){
	unsigned char buffer[BUF_SIZE];
	
	// packet length octet
	buffer[0]=packet->vlans_num*4+66;
	fwrite(buffer,1,1,file); // for self use only - we remove it for Wireshark
	
	// Ethernet Header
	write_ethernet_header_to_file(file,packet);
	
	// version,diag
	buffer[0]=convert_ver_diag_octet(packet);
	fwrite(buffer,1,1,file);
	
	// flags
	buffer[0]=convert_flags_bfd_octet(packet);
	fwrite(buffer,1,1,file);
	
	// detect
	buffer[0]=packet->detect;
	fwrite(buffer,1,1,file);
	
	// length
	buffer[0]=packet->length;
	fwrite(buffer,1,1,file);
	
	// my_disc
	write_int_as_4_bytes(file,packet->my_disc);
	
	// your_disc
	write_int_as_4_bytes(file,packet->your_disc);
	
	// min_tx_interval
	write_int_as_4_bytes(file,packet->min_tx_interval);
	
	// min_rx_interval
	write_int_as_4_bytes(file,packet->min_rx_interval);
	
	// min_echo_rx_interval
	write_int_as_4_bytes(file,packet->min_echo_rx_interval);
	
	if (packet->authentication==0){
		return;
	}
	
	// auth_type
	//buffer[0]=packet->auth_type;
	//fwrite(buffer,1,1,file);

	// auth_len
	//buffer[0]=packet->auth_len;
	//fwrite(buffer,1,1,file);
	
	// auth_data
	//for (int i=0; i<2; i++){
	//	buffer[i]=packet->auth_data[i];
	//}
	//fwrite(buffer,1,2,file);
}

void createVlansForPacket_bfd(Packet_bfd* packet,Vlan_tag* vlans[MAX_VLANS_NUM], int vlans_num){
	packet->vlans_num=vlans_num;
	int i=0;
	for (i=0; i<vlans_num; i++){
		packet->vlans[i]=vlans[i];
	}
	for (; i<MAX_VLANS_NUM; i++){
		packet->vlans[i]=NULL;
	}
}

#endif
