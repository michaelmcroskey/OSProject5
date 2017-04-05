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
#include <time.h>
#include <errno.h>
#include <stdbool.h>

// Globals
int NFRAMES;
int NPAGES;
struct frame_table FT;
const char *PAGE_REPLACEMENT_TYPE;
const char *PROGRAM;
struct disk *DISK;
char *PHYSMEM;
#ifndef FRAME_SIZE
#define FRAME_SIZE 4096
#endif
int NUM_PAGE_FAULTS;
int NUM_DISK_READS;
int NUM_DISK_WRITES;

/*
 * Frame Table and Frame Struct Creation
 * --------------------
 */

// Frame Struct
typedef struct{
    int frame;
    char *bits;
} frame;

// Frame Table Struct
struct frame_table {
    frame* frames;
    int* time_stamps;
    int phys_mem_count;
};

/*
 * Function:  print_frame_table()
 * --------------------
 * Prints out the current state of the global
 * frame_table
 */
void print_frame_table(){
    int i;
    printf("Page\t|\tFrame\t|\tBits \n");
    for (i = 0; i < NFRAMES; i++){
        printf("%d\t\t%d\t\t%s\n",
               i,
               FT.frames[i].frame,
               FT.frames[i].bits);
    }
}



/*
 * Function:  get_permissions
 * --------------------
 * Gives the permissions for a physical frame number
 *
 *  frame_number: # of frame to query
 *
 *  returns: 1 :    No permissions
 *           2 :    Read permissions
 *           3 :    Read & write permissions
 *          -1 :    Error
 */
int get_permissions(int frame_number){
    printf("get_permissions:  entered func looking for frame number %d\n", frame_number);
    printf("get_permissions:  bits for frame_number: %s\n", FT.frames[frame_number].bits);
    if ((strcmp(FT.frames[frame_number].bits, "-") == 0) && (FT.frames[frame_number].frame == 0)){
        printf("get_permissions:  exited func with val 1\n");
        return 1;
    }
    else if (strcmp(FT.frames[frame_number].bits, "R") == 0){
        printf("get_permissions:  exited func with val 2\n");
        return 2;
    }
    else if (strcmp(FT.frames[frame_number].bits, "RW") == 0){
        printf("get_permissions:  exited func with val 3\n");
        return 3;
    }
    else {
        printf("Error:  has_no_permissions():  Permissions unknown ");
        return -1;
    }
    
}


/*
 * Function:  is_frame_free
 * --------------------
 * Determines if a specific frame is free in physical memory
 *
 *  frame_number: # of frame to query
 *
 *  returns: True:  Frame is free
 *           False: Frame is not free
 */
bool is_frame_free(int frame_number){
    if (strcmp(FT.frames[frame_number].bits, "-") == 0
        && (FT.frames[frame_number].frame == 0)){
        return true;
    }
    else {
        return false;
    }
}


/*
 * Function:  next_free_frame
 * --------------------
 * Uses a linear search to get the next free place in
 * the frame table
 *
 *  returns: fn:    int = to the next free frame
 *           -1:    Error
 */
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


/*
 * Function:  insert_frame
 * --------------------
 * Inserts a new frame into the frame table
 *
 *  page_num:  index of frame to insert
 *  free:       value of frame to insert (0 = Free)
 *  bits:       string indicating permissions ("-" = None)
 */
void insert_frame(int page_num, int frame, char* bits){
    printf("insert_frame:  entered func \n");
    FT.frames[page_num].frame = frame;
    FT.frames[page_num].bits = bits;
    FT.time_stamps[FT.phys_mem_count] = page_num;
    FT.phys_mem_count++;
    printf("insert_frame:  exited func \n");
}

/*
 * Function:  frame_is_full
 * --------------------
 * Checks to see if the physical memory is full
 *
 *  returns:  True:  Frame is full
 *            False: Frame is not full
 */
bool frame_is_full(){
    print_frame_table();
    int i;
    int frame_full_count;
    frame_full_count = 0;
    for (i = 0; i < NFRAMES; i++){
        if ((get_permissions(i) == 2) || (get_permissions(i) == 3)){
            frame_full_count++;
        }
    }
    if (frame_full_count == NFRAMES){
        return true;
    }
    return false;
}


/*
 * Function:  rand_func
 * --------------------
 * Finds random free position in physical memory
 *
 *  returns: n:    page number of memory with free frame
 */
int rand_func(){
    printf("rand_func:  entered func \n");
    /*int n = rand() % NFRAMES;
    while(!is_frame_free(n)){
        n = rand() % NFRAMES;
    }*/
    printf("rand_func:  exited func \n");
    return rand() % NFRAMES;
}


/*
 * Function:  fifo
 * --------------------
 * Performs FIFO behavior
 *
 *  pt:  pointer to the page table
 *
 *  returns: first_in:  index of page that was
 *                      removed
 */
int fifo(struct page_table *pt){
    int first_in = FT.time_stamps[0];
    page_table_set_entry(pt, first_in, 0, 0);
    insert_frame(first_in, 0, "-");
    memmove(FT.time_stamps, FT.time_stamps+1, sizeof FT.time_stamps - sizeof *FT.time_stamps);
    FT.phys_mem_count--;
    return first_in;
}





/*
 * Function:  get_new_frame_num()
 * --------------------
 * Determines a new frame number depending on the page
 * replacement algorithm selected
 *
 *  pt:  pointer to the page table
 * 
 * returns: n:  index of new frame number that's free
 *
 */
int get_new_frame_num(struct page_table *pt){
    printf("get_new_frame_num:  entered func \n");
    int n;
    if (strcmp(PAGE_REPLACEMENT_TYPE, "rand") == 0){
        printf("get_new_frame_num:  replacement type is rand \n");
        n = rand_func();
    }
    else if (strcmp(PAGE_REPLACEMENT_TYPE, "fifo") == 0){
        printf("get_new_frame_num:  replacement type is fifo \n");
        n = fifo(pt);
    }
    else if (strcmp(PAGE_REPLACEMENT_TYPE, "custom") == 0){
        printf("ERROR:  get_new_frame_num:  Custom has not been implemented yet \n");
        exit(1);
    }
    printf("get_new_frame_num:  exited func \n");
    return n;
}


/*
 * Function:  page_fault_handler
 * --------------------
 * Handles all page faults
 *
 *  pt:     pointer to the page table
 *  page:   page number that is faulty
 */
void page_fault_handler( struct page_table *pt, int page )
{
    printf("page_fault_handler: entered func \n");
    NUM_PAGE_FAULTS++;
    int fn, p;
    // Trivial Example
    if (NFRAMES == NPAGES){
        page_table_set_entry(pt,page,page,PROT_READ|PROT_WRITE);
        return;
    }
    // There are no more free frames in physical memory
    else if(frame_is_full()){
        printf("page_fault_handler:     No more free frames in physical memory \n");
        printf("page_fault_handler:     Page table printout: \n");
        page_table_print(pt);
        fn = get_new_frame_num(pt);
        insert_frame(page, fn, "R");
        disk_write(DISK, fn, &PHYSMEM[0*PAGE_SIZE]);
        disk_read(DISK, page, &PHYSMEM[0*PAGE_SIZE]);
        page_table_set_entry(pt, page, 0, PROT_READ);
        page_table_set_entry(pt, fn, 0, 0);
        NUM_DISK_READS++;
        NUM_DISK_WRITES++;
    }
    
    // Reading or writing permissions issue
    else {
        printf("\npage_fault_handler: Reading or writing permissions problem\n");
        printf("page_fault_handler: Page table printout: \n");
        page_table_print(pt);
        p = get_permissions(page);
        printf("page_fault_handler: Current permissions of the page: %d\n", p);
        if (p == 1){  // No permissions
            printf("page_fault_handler: Inside if statement for no permissions \n");
            fn = get_new_frame_num(pt);
            page_table_set_entry(pt, page, fn, PROT_READ);
            insert_frame(page, fn, "R");
            disk_read(DISK, page, &PHYSMEM[fn*FRAME_SIZE]);
            NUM_DISK_READS++;
            return;
        }
        else if (p == 2){ // Only read permissions
            insert_frame(page, 0, "RW");
            page_table_set_entry(pt, page, 0, PROT_READ|PROT_WRITE);
            printf("page_fault_handler:  Finished page_table_set_entry & updated to read and write \n");
            
            return;
        }
        else{
            printf("page fault on page #%d\n",page);
            exit(1);
        }
    }
    
}

/*
 * Function:  page_fault_handler
 * --------------------
 * Handles all page faults
 *
 *  pt:     pointer to the page table
 *  page:   page number that is faulty
 */
void print_summary(){
    printf("-------------------------------------------\n");
    printf("                  SUMMARY                  \n");
    printf("-------------------------------------------\n");
    printf("  * NUM_PAGE_FAULTS: %d \n", NUM_PAGE_FAULTS);
    printf("  * NUM_DISK_READS: %d \n", NUM_DISK_READS);
    printf("  * NUM_PAGE_WRITES: %d \n", NUM_DISK_WRITES);
    printf("-------------------------------------------\n");
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
	PAGE_REPLACEMENT_TYPE = argv[3];
	PROGRAM = argv[4];
    
    // Set global variables
    NUM_PAGE_FAULTS = 0;
    NUM_DISK_READS = 0;
    NUM_DISK_WRITES = 0;

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
    FT.frames = (frame*)malloc(sizeof(frame) * NPAGES);
    FT.time_stamps = (int*)malloc(sizeof(int) * NPAGES);
    int i;
    for (i = 0; i < NPAGES; i++){
        FT.frames[i].frame = 0;
        FT.frames[i].bits = "-";
    }
    FT.phys_mem_count = 0;
    
	// Create virual and physical memory space
	char *virtmem = page_table_get_virtmem(pt);
	PHYSMEM = page_table_get_physmem(pt);
	
	// Page replacement type case structure
	if(!strcmp(PAGE_REPLACEMENT_TYPE,"rand")) {
		printf("Selected rand \n");

	} else if(!strcmp(PAGE_REPLACEMENT_TYPE,"fifo")) {
		printf("Selected fifo \n");

	} else if(!strcmp(PAGE_REPLACEMENT_TYPE,"custom")) {
		printf("Selected custom \n");

	} else {
		fprintf(stderr,"unknown page replacement type: %s\n",argv[3]);
		return 1;
	}
	
	// Program case structure
	if(!strcmp(PROGRAM,"sort")) {
		sort_program(virtmem,NPAGES*PAGE_SIZE);

	} else if(!strcmp(PROGRAM,"scan")) {
		scan_program(virtmem,NPAGES*PAGE_SIZE);

	} else if(!strcmp(PROGRAM,"focus")) {
		focus_program(virtmem,NPAGES*PAGE_SIZE);

	} else {
		fprintf(stderr,"unknown program: %s\n",argv[4]);
		return 1;
	}

	page_table_delete(pt);
	disk_close(DISK);
    
    print_summary();

	return 0;
}
