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
#include <stdbool.h>

// Globals
int NFRAMES;
int NPAGES;
struct frame_table FT;
struct disk *DISK;
char *PHYSMEM;

/*--FRAME_TABLE Creation & Functions--------------------*/

// Frame Struct
typedef struct{
    int free;
    char *bits;
} frame;

// Frame Table Struct
struct frame_table {
    frame* frames;
};

/*  Returns true if the given frame number in the
    frame_table is free and false if it's not free */
bool is_frame_free(int frame_number){
    if (strcmp(FT.frames[frame_number].bits, "-") == 0
        && (FT.frames[frame_number].free == 0)){
        return true;
    }
    else {
        return false;
    }
}

/*  Returns an int of the next free place in the 
    frame_table by performing a linear search or 
    -1 if no frames are free */
int next_free_frame(){
    int fn;
    for (fn = 0; fn < NFRAMES; fn++){
        if (is_frame_free(fn)){
            return fn;
        }
    }
    printf("ERROR: next_free_frame():  No frames are free. \n");
    return -1;
}

/*  Prints out the current state of the global 
    frame_table */
void print_frame_table(){
    int i;
    printf("Frame\t|\tBits \n");
    for (i = 0; i < NFRAMES; i++){
        printf("%d\t\t%s\n",
               FT.frames[i].free,
               FT.frames[i].bits);
    }
}

/*--PAGE FAULT ERROR HANDLING---------------------------*/

/* Page fault handler handles page_table errors */
void page_fault_handler( struct page_table *pt, int page )
{
    // Trivial Example
    if (NFRAMES == NPAGES){
        page_table_set_entry(pt,page,page,PROT_READ|PROT_WRITE);
        return;
    }
    // Reading a page that doesn't exist in physical memory
    else {
        /*  If there are no permissions, set it to PROT_READ
            and return */
        if (is_frame_free){ // Instead get the permissions for a page
            int fn = next_free_frame();
            page_table_set_entry(pt, page, fn, PROT_READ);
            disk_read(DISK, page, &PHYSMEM[fn*sizeof(frame)]);
            return;
        }
        else{
            printf("page fault on page #%d\n",page);
            exit(1);
        }
    }
    
}


// Main execution
int main( int argc, char *argv[] )
{
	if(argc!=5) {
		printf("use: virtmem <NPAGES> <NFRAMES> <rand|fifo|custom> <sort|scan|focus>\n");
		return 1;
	}
	
	// Process command line arguments
	NPAGES = atoi(argv[1]);
	NFRAMES = atoi(argv[2]);
	const char *page_replacement_type = argv[3];
	const char *program = argv[4];

	// Create virtual disk
	DISK = disk_open("myvirtualdisk",NPAGES);
	if(!DISK) {
		fprintf(stderr,"couldn't create virtual disk: %s\n",strerror(errno));
		return 1;
	}

	// Initialize page_table
	struct page_table *pt = page_table_create( NPAGES, NFRAMES, page_fault_handler );
    
	if(!pt) {
		fprintf(stderr,"couldn't create page table: %s\n",strerror(errno));
		return 1;
	}
    
    // Create frame_table & initialize as empty
    FT.frames = (frame*)malloc(sizeof(frame) * NFRAMES);
    int i;
    for (i = 0; i < NFRAMES; i++){
        FT.frames[i].free = 0;
        FT.frames[i].bits = "-";
    }
    next_free_frame();
    
	// Create virual and physical memory space
	char *virtmem = page_table_get_virtmem(pt);
	PHYSMEM = page_table_get_physmem(pt);
	
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
		sort_program(virtmem,NPAGES*PAGE_SIZE);

	} else if(!strcmp(program,"scan")) {
		scan_program(virtmem,NPAGES*PAGE_SIZE);

	} else if(!strcmp(program,"focus")) {
		focus_program(virtmem,NPAGES*PAGE_SIZE);

	} else {
		fprintf(stderr,"unknown program: %s\n",argv[4]);
		return 1;
	}

	page_table_delete(pt);
	disk_close(DISK);

	return 0;
}
