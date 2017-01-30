Compression in Networks Project
===============================

Project Files
=============

	- README.txt - this file

	- poster.pptx - project poster
	
	- Mid presentation.pptx
	
	- Final Presentation.pptx
	
	- book compression project final.docx - project report
	
	
	- packets_script.c
	==================
	C program that creates random packets from BFD and CFM protocols
	with option to set them arranged or not arranged
	
	Compilation:
		gcc -g -std=c99 -o packets_script packets_script.c
	Running:
		./packets_script <#MEPs> <#MEs> <#MACs> <filename> <arrangement (0 or 1)>  >  <logfile>
	Example:
		./packets_script 1000 5000 30 packets.bin 1 > script_out
		
	- utilities.h - includes Vlan struct and helper functions
	
	- cfm.h - includes CFM packet struct and helper functions
	
	- bfd.h - includes BFD packet struct and helper functions
	
	
	- bin_multipackets.c
	====================
	C program that disassembles a packets file to many files that each one of them includes
	an individual packet
	
	Compilation:
		gcc -g -std=c99 -o bin_multipackets bin_multipackets.c
	Running:
		./bin_multipackets <packets binary file> <packet_num> <packet_file_name_prefix>
	Example:
		./bin_multipackets packets.bin 100 p
		
		
	- bin2text_wireshark.c
	======================
	C program that converts a BIN file representing a packets file into TXT file in
	Wireshark format
	
	Compilation:
		gcc -g -std=c99 -o bin2text_wireshark bin2text_wireshark.c

	Running:
		./bin2text_wireshark <binary file> > <text file>
		
	Example:
		./bin2text_wireshark p7.bin > p7.txt
		
	
	- delta_encoding.c
	==================
	C program implements Delta Encoding algorithm (compression and decompression)
	
	Compilation:
		gcc -g -std=c99 -o delta_encoding delta_encoding.c
		
	Running compression:
		./delta_encoding -e <packets_file> <compressed_file> <packets_number>
		
	Running decompression:	
		./delta_encoding -d <compressed_file> <source_file> <packets_number>
		
	Example:
		./delta_encoding -e compressed_file.bin source_file.bin 1000
	
	
	- delta_encoding_v2.c
	=====================
	C program (2nd version) of Delta Encoding - adding Block size parameter (in bytes)
	*Note: this program implements only compression function
	
	Compilation:
		gcc -g -std=c99 -o delta_encoding_v2 delta_encoding_v2.c
	
	Running compression:
		./delta_encoding_v2 -e <packets_file> <compressed_file> <packets_number> <block_size>
		
	Example:	
		./delta_encoding_v2 -e compressed_file.bin source_file.bin 1000 2
		
		
	- stats_script_zlib
	===================
	BASH program that creates statistics for ZLIB algorithm (GZIP)
	
	Compilation:
		sudo chmod a+x stats_script_zlib
	
	Running compression:
		./stats_script_zlib <statistic file number>
	
	
	- stats_script_lzss
	===================
	BASH program that creates statistics for LZSS algorithm (LZ77)
	
	Compilation:
		sudo chmod a+x stats_script_lzss
	
	Running compression:
		./stats_script_lzss <statistic file number>	
		
		
	- stats_script_delta
	===================
	BASH program that creates statistics for Delta Encoding algorithm
	
	Compilation:
		sudo chmod a+x stats_script_delta
	
	Running compression:
		./stats_script_delta <statistic file number>	
		
	
	- stats_script_dedup
	===================
	BASH program that creates statistics for Deduplication algorithm
	
	Compilation:
		sudo chmod a+x stats_script_dedup
	
	Running compression:
		./stats_script_dedup <statistic file number>


	- stats_delta_script_v2
	=======================
	BASH program that creates statistics for Delta Encoding algorithm with block size parameter
	
	Compilation:
		sudo chmod a+x stats_script_delta_v2
	
	Running compression:
		./stats_script_delt_v2 <statistic file number>
	
		