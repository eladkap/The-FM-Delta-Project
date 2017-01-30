#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "cfm.h"
#include "bfd.h"

#define FILE_NAME_MAX_LEN 20

FILE* createFile(int MEPs, int MEs, int MACs, long* MAC_address_array, char* file_name, int arrange);


FILE* createFile(int MEPs, int MEs, int MACs, long* MAC_address_array, char* file_name, int arrange){
    FILE* write_fp;
	Packet_cfm* packet_cfm;
	Packet_bfd* packet_bfd;
	
	int connections_num=MEs;
	int M=MEs;
	
	int num_of_mes_connected;
	int range=MEs/MEPs; // we assume that MEs > MEPs
	
	// array of MEP ids
	int* mep_id_array = (int*)malloc(sizeof(int)*MEPs);
	
	// array that each element is number of connections (MEs) to each MEP
	int* meps_per_med_array = (int*)malloc(sizeof(int)*MEPs);
	
	// array that each element is group id of each MEP 
	int* meps_group_id_array = (int*)malloc(sizeof(int)*MEPs);
	
	int groups_num;
	
	// array of current meg_id per MEG
	int* meg_curr_med_id_array;
	
	// array of MEGs
	Meg_id_struct** meg_structs_array;
	
	// array of MEG levels
	int* meg_levels_array;
	
	// first MEP id to start with
	int MEP_id_source_start;
	
	// indicates protocol
	int packet_type;
	
   	write_fp = fopen(file_name, "w+"); // elad - w+ instead of wb
	
	
	printf("MEs = %d\n",MEs);
   	 for(int i = 0; i<MEPs; i++){
		
		// randomize number of MEs connected to MED i from 1 to range
		num_of_mes_connected=(rand() % range) + 1;
		meps_per_med_array[i]=num_of_mes_connected;
		MEs-=num_of_mes_connected;
		
	}
		
	if (MEs>0){
		for (int i=0; i<MEs; i++){
			int mep_index=rand()%MEPs;
			meps_per_med_array[mep_index]++;
		}
	}
		
	for(int i = 0; i<MEPs; i++){
		printf("MEP %d:		connections = %d \n",i,meps_per_med_array[i]);
	}
	
	// randomize num of groups between 1 to #MEPs
	groups_num=(rand() % (MEPs-1+1)) + 1; // rand() % (B-A+1)+A
	printf("\ngroups num: %d\n",groups_num);	
	
	// create <groups_num> different MEG_id array
	meg_structs_array=(Meg_id_struct**)malloc(sizeof(struct meg_id_t)*groups_num);
	meg_levels_array=(int*) malloc(sizeof(struct meg_id_t)*groups_num);
	
	for (int i=0; i<groups_num; i++){
		meg_structs_array[i]=create_random_meg_id();
		meg_levels_array[i] = rand() % 8;
	}
	
	for (int i=0; i<MEPs; i++){
		// randomize number between 0 to groups_num
		meps_group_id_array[i]=rand()%groups_num;
	}
	
	// randomize starting MEP id
	MEP_id_source_start = rand() % 8192;

	// initialize each group's starting MEP id
	meg_curr_med_id_array=(int*)malloc(sizeof(int)*groups_num);
	for (int i=0; i<groups_num; i++){
		meg_curr_med_id_array[i]=MEP_id_source_start;
	}
	
	int packets_num=1;
	for(int i = 0; i<MEPs; i++){
		printf("-------------------------------------------------\n");
		int packets_num_per_mep=meps_per_med_array[i]; // packets number per MEP
		int N=meps_per_med_array[i];

		printf("MEP %d\n",i);
		int group_index=meps_group_id_array[i];
		int group_id=meps_group_id_array[i];
		
		// go to the next mep id in the group
		meg_curr_med_id_array[group_index]++;
		
		// randomize source mac
		int source_mac_index = rand() % MACs; // randomize MAC address from MAC pool
		long source_mac_address = MAC_address_array[source_mac_index];			

		// randomize source ip
		int source_ip[4];
		randomize_ip_address(source_ip);

		// randomize 2 vlans
		Vlan_tag* vlans[MAX_VLANS_NUM];	
		create_random_vlans(2,vlans);
		
		int vlans_num;

		for (int p=0; p<meps_per_med_array[i]; p++){
			printf("packet %d:\n",p);
			
			int mep_id_source=meg_curr_med_id_array[group_index];
			mep_id_array[i]=mep_id_source;
			
			// arranged DB
			if (arrange==1){
				
				// arrange packets by vlans and protocol
				if (packets_num>=1 && packets_num<=M/6){
					vlans_num=0;
					packet_type=0; // CFM	
				}
				else if (packets_num>M/6 && packets_num<=2*M/6){
					vlans_num=1;
					packet_type=0; // CFM	
				}
				else if (packets_num>2*M/6 && packets_num<=3*M/6){
					vlans_num=2;
					packet_type=0; // CFM	
				}
				else if (packets_num>3*M/6 && packets_num<=4*M/6){
					vlans_num=0;
					packet_type=1; // BFD	
				}
				else if (packets_num>4*M/6 && packets_num<=5*M/6){
					vlans_num=1;
					packet_type=1; // BFD
				}
				else{
					vlans_num=2;
					packet_type=1; // BFD	
				}
			}
				
			// scattered DB
			else{
				packet_type = rand() % 2;
				vlans_num = rand() % 3;
			}

			
			packets_num++;
			

			if (packet_type==0){ // CFM packet
				packet_cfm=createPacket_cfm(mep_id_source,meg_structs_array[group_index],source_mac_address,meps_group_id_array[i],meg_levels_array[group_index],vlans,vlans_num);
				write_cfm_packet_fields_to_file(write_fp,packet_cfm);
				free(packet_cfm);
			}
			else{ // BFD packet
				packet_bfd=createPacket_bfd(source_mac_address,source_ip,vlans, vlans_num);
				write_bfd_packet_fields_to_file(write_fp,packet_bfd);
				free(packet_bfd);
			}
		}
	}   
	
	// show all elements - for testing
	printf("\n\n***********************************************************\n\n");
	for (int i=0; i<groups_num; i++){
		printf("group #%d:     level: %d     \n",i,meg_levels_array[i]);
		printf("MEPs:  \n");
		for (int j=0; j<MEPs; j++){
			if (meps_group_id_array[j]==i){
				printf("%d  ",mep_id_array[j]);
			}
		}
		printf("\n");
	}
	printf("\n\n***********************************************************\n\n");
	
	 
	// destroy data structures
	free(meps_per_med_array);
	free(meps_group_id_array);
	destroy_meg_structs_array(meg_structs_array,groups_num);
	free(meg_levels_array);
	free(meg_curr_med_id_array);
	free(mep_id_array);
    return write_fp;
}




/*------------------------MAIN-------------------------------------------*/

/* 	parameters of main function:
	argv[1] = #MEPs
	argv[2] = #MEs
	argv[3] = #MACs
	argv[4] = output file name
	argv[5] = arrange: 1 - arranged DB , 0 - scattered DB
	*/
int main(int argc, char* argv[]){
	long* MAC_address_array=NULL; // list of MAC addresses, each MAC address consists of 6 bytes
	int MEPs=0;
	int MEs=0;
	int MACs=0;
	char* file_name;
	int arrange;
	
	
	FILE* file_pointer=NULL;
	
	srand(time(NULL));

    /** check parameters validity **/
	if (argc != 6){
		printf("\nError: wrong parameters.\n");
		return -1;
	}

	/** preparing main parameters **/
	MEPs = atoi(argv[1]);
	MEs = atoi(argv[2]);
	MACs = atoi(argv[3]);
	
	
	if (MEs<MEPs){
		printf("\nError: wrong parameters.\n");
		return -1;
	}
	
	file_name=(char*)malloc(sizeof(char)*FILE_NAME_MAX_LEN);
	file_name=argv[4];
	
	arrange = atoi(argv[5]);
	
	printf("\nCreating %s...\n",file_name);

    /** creating MAC address array **/
	MAC_address_array=(long*)create_mac_address_array(MACs);
	if (!MAC_address_array){
		printf("\nMemory error.\n");
		return -1;
	}
	
	show_mac_address_table(MAC_address_array,MACs);
	
    /** creating the file **/
	file_pointer = createFile(MEPs,MEs,MACs,MAC_address_array,file_name,arrange);

	printf("\npackets.bin file was created.\n");
	printf("CFM packets file size: %d bytes\n",packets_cfm_total_length);
	printf("BFD packets file size: %d bytes\n",packets_bfd_total_length);
	
	/** free data structures and close files **/
	destroy_mac_address_array(MAC_address_array);
	//free(file_name);
	fclose(file_pointer);
    return 0;
	
}

