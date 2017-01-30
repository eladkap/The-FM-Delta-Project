#ifndef CFM_H_
#define CFM_H_

#include "utilities.h"

#define MAX_MES_PER_MED 9

#define MEG_NAME_LEN 15
#define MEG_ID_FORMAT 33

#define INIT_BYTES_NUM_CFM 89
#define MAX_SEQ 10000


int packets_cfm_total_length=0;

/*--------------------------- Data types --------------------------------*/

typedef struct meg_id_t{
	int reserved; // 0-255
	int format; // 0-255
	int length; // 0-15
	char name[MEG_NAME_LEN]; // MEG name consists of 15 characters (capital letters)- unique name globally
}Meg_id_struct;


/* CFM packet */
/* CFM packet length: 89 - 97 bytes */

struct packet_cfm_t{
	int packet_length; // 0-255 1 byte remove this field in the textfile
	
	long destination_mac_address; // 48 bits = 6 bytes
	long source_mac_address; // 48 bits = 6 bytes
	
	int ethertype; // 2 bytes
	
	int vlans_num; // not a field in the packet itself
	Vlan_tag* vlans[MAX_VLANS_NUM]; // each vlan: 32 bits = 4 bytes
	
	int MEG_level; // 3 bits - values: 0-7
  	  int version; // 5 bits - values: 0-31

	int opcode; // 1 bytes - value=1 for CCM packet

   	 int flags_rdi; // 1 bits - values: 0-1
   	 int flags_reserved; // 4 bits - values: 0-15
   	 int flags_period; // 3 bits - values: 0-7

	int tlv_offset; // 1 bytes - value=70 for CCM packet

	char sequence_number[4]; // 4 bytes

	int MEP_id_source; // 2 bytes - 000 | (13 bits) - values: 0-8191 

    Meg_id_struct MEG_id; /* 48 bytes:
	1st octet: Reserved 0-255
	2st octet: Format 0-255
	3rd octet: Length 0-15
	4th - 18th octets: Name 
	19th - 48th: Unused - All zeros
	*/
	
	int group_id; // for testing
	
	/*unused fields - 17 bytes
	TxFCf - 2 bytes
	TxFCf - 2 bytes
	RxFCb - 2 bytes
	RxFCb - 2 bytes
	TxFCb - 2 bytes
	TxFCb - 2 bytes
	reserved (0) - 2 bytes
	reserved (0) - 2 bytes
	end_tlv (0) - 1 byte
	*/
};
typedef struct packet_cfm_t Packet_cfm;




/*------------------------Functions declarations-------------------------------------------*/
long* create_mac_address_array(int size);

void destroy_mac_address_array(long* MAC_address_list);

void destroy_meg_structs_array(Meg_id_struct** meg_structs_array, int groups_num);

void show_mac_address_table(long* MAC_address_array, int num_of_MACs);

/* creates the first octet */
char convert_1st_octet(int MEG_level, int version);

/* creates the FLAGS octet */
char convert_flags_octet(int flags_rdi, int flags_reserved, int flags_period);

void showMegId(Meg_id_struct meg_id_struct);

/* creates vlans for packet */
void createVlansForPacket_cfm(Packet_cfm* packet, Vlan_tag* vlans[MAX_VLANS_NUM], int vlans_num);

void show_cfm_packet(Packet_cfm* packet);

void write_vlans_to_file_cfm(FILE* file,Packet_cfm* packet);

/* create random packet */
Packet_cfm* createPacket_cfm(int MEP_id_source, Meg_id_struct* MEG_id_struct, long source_mac, int group_id, int meg_level, Vlan_tag* vlans[MAX_VLANS_NUM], int vlans_num);

int calc_packet_length(Packet_cfm* packet);

void write_meg_id_to_file(FILE* file, Meg_id_struct meg_id_struct);

void write_cfm_packet_fields_to_file(FILE* file, Packet_cfm* packet);

Meg_id_struct* create_random_meg_id();




/*------------------------Functions Implementation-------------------------------------------*/
long* create_mac_address_array(int num_of_MACs){
	long* MAC_address_array = malloc(sizeof(long)*num_of_MACs);
	if (!MAC_address_array){
		return NULL;
	}
	long mac_address=randomize_mac_address();
	MAC_address_array[0]=mac_address++;
	for (int i=1; i<num_of_MACs; i++){
		MAC_address_array[i]=mac_address;
		mac_address++;
	}
	return MAC_address_array;
}

void destroy_mac_address_array(long* MAC_address_array){
	free(MAC_address_array);
}

void destroy_meg_structs_array(Meg_id_struct** meg_structs_array, int groups_num){
	for (int i=0; i<groups_num; i++){
		free(meg_structs_array[i]);
	}
	free(meg_structs_array);
}

// for check
void show_mac_address_table(long* MAC_address_array, int num_of_MACs){
	printf("MAC address table:\n");
	for (int i=0; i<num_of_MACs; i++){
		printf("%ld\n",MAC_address_array[i]);
	}
	printf("\n");
}

char convert_1st_octet(int MEG_level, int version){
	char result=MEG_level;
	result<<=5;
	char ver=version;
	result+=ver;
	return result;
}

char convert_flags_octet(int flags_rdi, int flags_reserved, int flags_period){
	char result=flags_rdi;
	result<<=7;
	char reserved=flags_reserved;
	reserved<<=3;
	char period=flags_period;
	result+=reserved;
	result+=period;
	return result;
}

void showMegId(Meg_id_struct meg_id_struct){
	printf("MEG reserved: %d\n",meg_id_struct.reserved);
	printf("MEG format: %d\n",meg_id_struct.format);
	printf("MEG length: %d\n",meg_id_struct.length);
	printf("MEG name: ");
	for (int i=0; i<meg_id_struct.length; i++){
		printf("%c",meg_id_struct.name[i]);
	}
	printf("\n");
}

void createVlansForPacket_cfm(Packet_cfm* packet,Vlan_tag* vlans[MAX_VLANS_NUM],int vlans_num){
	packet->vlans_num=vlans_num;
	int i=0;
	for (i=0; i<vlans_num; i++){
		packet->vlans[i]=vlans[i];
	}
	for (; i<MAX_VLANS_NUM; i++){
		packet->vlans[i]=NULL;
	}	
}

void show_cfm_packet(Packet_cfm* packet){
	printf("CFM\n");
	printf("packet length: %d\n",packet->packet_length);
	printf("packet number of vlans: %d\n",packet->vlans_num);
	printf("group id: %d\n",packet->group_id);
	printf("MEG_level: %d\n",packet->MEG_level);
	printf("version: %d\n",packet->version);
	printf("opcode: %d\n",packet->opcode);
	printf("flags_rdi: %d\n",packet->flags_rdi);
	printf("flags_reserved: %d\n",packet->flags_reserved);
	printf("flags_period: %d\n",packet->flags_period);
	printf("MEP_id_source: %d\n",packet->MEP_id_source);
	showMegId(packet->MEG_id);
	printf("source_mac_address:	%ld\n",packet->source_mac_address);
	printf("destination_mac_address: %ld\n",packet->destination_mac_address);
	showVlans(packet->vlans);
	printf("\n\n");
}

void write_vlans_to_file_cfm(FILE* file,Packet_cfm* packet){
	for (int i=0; i<packet->vlans_num; i++){
		write_vlan_to_file(file,packet->vlans[i]);
		//free(packet->vlans[i]);
	}
}

Packet_cfm* createPacket_cfm(int MEP_id_source, Meg_id_struct* MEG_id_struct, long source_mac, int group_id, int meg_level, Vlan_tag* vlans[MAX_VLANS_NUM], int vlans_num){
	Packet_cfm* packet=malloc(sizeof(struct packet_cfm_t));
	packet->MEG_level = meg_level;
	packet->version = 1; // CFM version
	packet->opcode = 1; // CCM packet
	packet->flags_rdi = rand() % 2;
	packet->flags_reserved = 0;
	packet->flags_period = 1;
	packet->tlv_offset = 70; // CCM packet
	
	for (int i=0; i<4; i++){
		packet->sequence_number[i] = rand() % 256;
	}
	
	packet->MEP_id_source = MEP_id_source;
	packet->MEG_id = *MEG_id_struct;
	
	long destination_mac_address = randomize_mac_address(); // arbitrary destination MAC address
	packet->source_mac_address = source_mac;
	
	packet->destination_mac_address=destination_mac_address;
	packet->group_id=group_id; // for testing

	createVlansForPacket_cfm(packet,vlans,vlans_num);

	packet->packet_length=calc_packet_length(packet);
	packets_cfm_total_length+=packet->packet_length; // for checking
	show_cfm_packet(packet);
	return packet;
}

void write_meg_id_to_file(FILE* file, Meg_id_struct meg_id_struct){
	unsigned char buffer[BUF_SIZE];
	
	buffer[0]=meg_id_struct.reserved;
	fwrite(buffer,1,1,file);
	
	buffer[0]=meg_id_struct.format;
	fwrite(buffer,1,1,file);
	
	buffer[0]=meg_id_struct.length;
	fwrite(buffer,1,1,file);
	
	for (int i=0; i<meg_id_struct.length; i++){
		buffer[i]=meg_id_struct.name[i];
	}
	fwrite(buffer,1,meg_id_struct.length,file);
	
	// write zeros
	for (int i=19; i<=48; i++){
		buffer[0]=0;
		fwrite(buffer,1,1,file);
	}
	
}

// calculates the packet length
int calc_packet_length(Packet_cfm* packet){
	int sum=INIT_BYTES_NUM_CFM;
	
	// packet length changed by vlan field
	sum+=4*packet->vlans_num;
	
	return sum;
}

void write_cfm_packet_fields_to_file(FILE* file, Packet_cfm* packet){
	unsigned char buffer[BUF_SIZE];
	unsigned char* buffer2;
	char octet;
	
	// packet length octet
	buffer[0]=packet->packet_length;
	fwrite(buffer,1,1,file); // for self use only - we remove it for Wireshark
	
	//Ethernet Header begin
	// destination_MAC_address - 6 octets
	buffer2=intToArray(packet->destination_mac_address,MAC_SIZE);
	fwrite(buffer2,1,6,file);
	free(buffer2);
	
	// source_MAC_address - 6 octets
	buffer2=intToArray(packet->source_mac_address,MAC_SIZE);
	fwrite(buffer2,1,6,file);
	free(buffer2);
	
	// vlan tag
	write_vlans_to_file_cfm(file,packet);
	
	// ethertype=0x8902
	buffer[0]=0x89;
	buffer[1]=0x02;
	fwrite(buffer,1,2,file);
	//Ethernet Header end
	
	
	// 1st octet
	octet = convert_1st_octet(packet->MEG_level,packet->version);
	buffer[0]=octet;
	fwrite(buffer,1,1,file);
	
	buffer[0]=packet->opcode;
	fwrite(buffer,1,1,file);
	
	// flags octet
	octet = convert_flags_octet(packet->flags_rdi,packet->flags_reserved,packet->flags_period);
	buffer[0]=octet;
	fwrite(buffer,1,1,file);
	
	// tlv_offset
	buffer[0]=packet->tlv_offset;
	fwrite(buffer,1,1,file);
	
	// sequence number - 4 octets
	for (int i=0; i<4; i++){
		octet=packet->sequence_number[i];
		buffer[0]=octet;
		fwrite(buffer,1,1,file);
	}
	
	
	// MEP_id_source - 2 octets
	
	short mep_id=packet->MEP_id_source;
	octet=get_short_high_byte(mep_id);
	buffer[0]=octet;
	fwrite(buffer,1,1,file);
	
	octet=get_short_low_byte(mep_id);
	buffer[0]=octet;
	fwrite(buffer,1,1,file);
	
	
	// MEG_ID - 48 octets
	write_meg_id_to_file(file,packet->MEG_id);
	
	// unused fields - 17 bytes
	buffer[0]=0;
	for (int i=0; i<17; i++){
		fwrite(buffer,1,1,file);
	}
}




Meg_id_struct* create_random_meg_id(){
	int i;
	Meg_id_struct* meg_id=malloc(sizeof(struct meg_id_t));

	meg_id->reserved = 0;
	meg_id->format = MEG_ID_FORMAT; // const 33
	meg_id->length = MEG_NAME_LEN;
	for (i=0; i<meg_id->length; i++){
		int n = rand() % 26; // randomize a letter between A-Z
		meg_id->name[i] = n + 'A';
	}
	
	return meg_id;
}



#endif
