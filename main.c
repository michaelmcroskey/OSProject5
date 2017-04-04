/*
Main program for the virtual memory project.
Make all of your modifications to this file.
You may add or rearrange any code or data as you need.
The header files page_table.h and disk.h explain
how to use the page table and disk interfaces.
*/

// Custom header files
#include "page_table.h"
#include "disk.h"
#include "program.h"

// Standard includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// Page fault handler
void page_fault_handler( struct page_table *pt, int page )
{
	printf("page fault on page #%d\n",page);
	exit(1);
}

// Main execution
int main( int argc, char *argv[] )
{
	if(argc!=5) {
		printf("use: virtmem <npages> <nframes> <rand|fifo|custom> <sort|scan|focus>\n");
		return 1;
	}
	
	// Process command line arguments
	int npages = atoi(argv[1]);
	int nframes = atoi(argv[2]);
	const char *page_replacement_type = argv[3];
	const char *program = argv[4];

	// Create virtual disk
	struct disk *disk = disk_open("myvirtualdisk",npages);
	if(!disk) {
		fprintf(stderr,"couldn't create virtual disk: %s\n",strerror(errno));
		return 1;
	}

	// Initialize page_table
	struct page_table *pt = page_table_create( npages, nframes, page_fault_handler );
	if(!pt) {
		fprintf(stderr,"couldn't create page table: %s\n",strerror(errno));
		return 1;
	}

	// Create virual and physical memory space
	char *virtmem = page_table_get_virtmem(pt);
	char *physmem = page_table_get_physmem(pt);
	
	// Page replacement type case structure
	if(!strcmp(page_replacement_type,"rand")) {
		printf("Selected rand");

	} else if(!strcmp(page_replacement_type,"fifo")) {
		printf("Selected fifo");

	} else if(!strcmp(page_replacement_type,"custom")) {
		printf("Selected custom");

	} else {
		fprintf(stderr,"unknown page replacement type: %s\n",argv[3]);
		return 1;
	}
	
	// Program case structure
	if(!strcmp(program,"sort")) {
		sort_program(virtmem,npages*PAGE_SIZE);

	} else if(!strcmp(program,"scan")) {
		scan_program(virtmem,npages*PAGE_SIZE);

	} else if(!strcmp(program,"focus")) {
		focus_program(virtmem,npages*PAGE_SIZE);

	} else {
		fprintf(stderr,"unknown program: %s\n",argv[4]);
		return 1;
	}

	page_table_delete(pt);
	disk_close(disk);

	return 0;
}