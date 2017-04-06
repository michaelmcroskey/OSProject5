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
int FIFO_INDEX;

/*
 * Frame Table Struct Creation
 * --------------------
 */

// Frame Table Struct
struct frame_table {
    int* time_stamps;
    int* frames;
    int* pages;
    int* permissions;
};

/*
 * Function:  print_frame_table()
 * --------------------
 * Prints out the current state of the global
 * frame_table
 */
void print_frame_table(){
    int i;
    printf("Frame\t|\tData\t| \n");
    for (i = 0; i < NFRAMES; i++){
        printf("%d\t\t%d\n",
               i,
               FT.frames[i]);
    }
}

int num_elements_in_frame_table(){
    int i;
    int count = 0;
    for (i = 0; i < NFRAMES; i++){
        if (FT.frames[i] == 1){
            count++;
        }
    }
    return count;
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
    
    int mem_count;
    mem_count = num_elements_in_frame_table();
    printf("Mem count is:  %d\n", mem_count);
    if (mem_count == NFRAMES){
        return true;
    }
    return false;
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
    if (FT.frames[frame_number] == 0){
        return true;
    }
    else {
        return false;
    }
}
/*
 * Function:  update_frame
 * --------------------
 * Inserts a new frame into the frame table; updates the value
 * in the grame table to be one to indicate that it's inserted
 * Also updates the time stamps table
 *
 *  frame:      index of frame to insert
 *  p:          permissions of frame being inserted
 */
void update_frame(int frame, int p, int p_n){
    // printf("update_frame:  entered func \n");
    FT.frames[frame] = 1;
    FT.permissions[frame] = p;
    FT.pages[frame] = p_n;
    int mem_count;
    mem_count = num_elements_in_frame_table();
    FT.time_stamps[mem_count] = frame;
    // printf("update_frame:  inserted new frame at index: %d with time_stamp_index: %d \n", frame, FT.phys_mem_count);
    // printf("update_frame:  exited func \n");
}

/*
 * Function:  evict frame
 * --------------------
 * Evicts a frame from the frame table
 *
 *  frame:      index of frame to evict
 */
void evict_frame(int frame){
    // printf("evict_frame:  entered func \n");
    FT.frames[frame] = 0;
    FT.permissions[frame] = 0;
    FT.pages[frame] = 0;
    // printf("evict_frame:  evicted frame and now number of elements in frame is %d \n", FT.phys_mem_count);
    // printf("evict_frame:  exited func \n");
}

/*
 * Function:  rand_func
 * --------------------
 * Finds random position in physical memory
 *
 *  returns: n:    page number of memory with free frame
 */
int rand_func(){
    // printf("rand_func:  getting random position \n");
    int n;
    n = rand() % NFRAMES;
    printf("rand_func:  random number = %d \n", n);
    return n;
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
    int first_in = FT.time_stamps[FIFO_INDEX];
    printf("fifo:  first-in element that should be removed %d\n", first_in);
    page_table_set_entry(pt, first_in, 0, 0);
    int mem_count;
    mem_count = num_elements_in_frame_table();
    if (mem_count > 1){
        evict_frame(first_in);
    }
    FIFO_INDEX++;
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
    // printf("get_new_frame_num:  entered func \n");
    int n;
    if (strcmp(PAGE_REPLACEMENT_TYPE, "rand") == 0){
        // printf("get_new_frame_num:  replacement type is rand \n");
        n = rand_func();
    }
    else if (strcmp(PAGE_REPLACEMENT_TYPE, "fifo") == 0){
        // printf("get_new_frame_num:  replacement type is fifo \n");
        //n = fifo(pt);
        int num_elements = num_elements_in_frame_table();
        n = num_elements % NFRAMES;
    }
    else if (strcmp(PAGE_REPLACEMENT_TYPE, "custom") == 0){
        // printf("ERROR:  get_new_frame_num:  Custom has not been implemented yet \n");
        exit(1);
    }
    // printf("get_new_frame_num:  exited func \n");
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
    
    // printf("\n\n************************PAGE FAULT PROBLEM # %d ************************ \n", NUM_PAGE_FAULTS);
    // printf("page_fault_handler: entered func \n");
    NUM_PAGE_FAULTS++;
    
    int bits;
    //int frame_bits;
    int fn; // Stores frame num associated with a specific page
    int new_fn;
    // Get the page table entry
    // printf("page_fault_handler:  page_table_get_entry starting... \n");
    page_table_get_entry(pt, page, &fn, &bits);
    // printf("page_fault_handler:  page_table_get_entry completed \n");
    printf("**GET ENTRY SUMMARY**\n");
    printf("PAGE NUMBER:\t%d \n", page);
    printf("FRAME NUMBER:\t%d \n", fn);
    printf("BITS:\t\t%d \n", bits);
    // printf("Current Size of Frame Table: %d \n", FT.phys_mem_count);
    // print_frame_table();
    // Trivial Example
    if (NFRAMES == NPAGES){
        page_table_set_entry(pt,page,page,PROT_READ|PROT_WRITE);
        return;
    }
    
    
    
    // Reading or writing permissions issue
    else {
        
        printf("page_fault_handler: Reading or writing permissions problem\n");
        printf("page_fault_handler: Page table printout: \n");
        page_table_print(pt);
        if ((bits == 0) && (!(frame_is_full()))){  // No permissions
            printf("page_fault_handler: Inside if statement for no permissions \n");
            
            new_fn = get_new_frame_num(pt);
            page_table_set_entry(pt, page, new_fn, PROT_READ);
            update_frame(new_fn, 1, page);
            print_frame_table();
            disk_read(DISK, page, &PHYSMEM[new_fn*FRAME_SIZE]);
            NUM_DISK_READS++;
            printf("page_fault_handler: finished adding read permissions  \n");
            printf("page_fault_handler:     Page table printout: \n");
            page_table_print(pt);
            return;
        }
        else if (bits == PROT_READ){ // Only read permissions
            page_table_set_entry(pt, page, fn, (PROT_READ|PROT_WRITE));
            update_frame(fn, 3, page);
            printf("page_fault_handler:  Finished page_table_set_entry & updated to read and write \n");
            printf("page_fault_handler:     Page table printout: \n");
            page_table_print(pt);
            return;
        }
        /*
        else if (bits == (PROT_READ|PROT_WRITE)){ // Read and write permissions
    
            // printf("page_fault_handler:     Bits are read and write \n");
            disk_write(DISK, fn, &PHYSMEM[0*PAGE_SIZE]);
            NUM_DISK_WRITES++;
            update_frame(fn, 1);
            disk_read(DISK, page, &PHYSMEM[0*PAGE_SIZE]);
            page_table_set_entry(pt, page, 0, PROT_READ);
            page_table_set_entry(pt, fn, 0, 0);
            // printf("page_fault_handler:  Bits are read and write case handled \n");
            // printf("page_fault_handler:     Page table printout: \n");
            // page_table_print(pt);
            return;
        }
         */
        // frame_bits = FT.permissions[fn];
        // printf("bits for frame # %d:  %d\n", fn, frame_bits);
        else if(frame_is_full()){
            printf("page_fault_handler:     No more free frames in physical memory \n");
            printf("page_fault_handler:     Frame table printout: \n");
            print_frame_table();
            printf("page_fault_handler:     Page table printout: \n");
            page_table_print(pt);
            
            // Figure out which entry to kick out
            new_fn = get_new_frame_num(pt);
            int page_num;
            page_num = FT.pages[new_fn];
            printf("Page number associated with new_fn is %d \n", page_num);
            evict_frame(new_fn);
            int old_bits;
            // Getting info for entry being kicked out
            page_table_get_entry(pt, page_num, &new_fn, &old_bits);
            
            printf("frame_table after evicting frame %d \n", new_fn);
            print_frame_table();
            update_frame(new_fn, 1, page);
            printf("frame_table after updating frame %d \n", new_fn);
            print_frame_table();
            if (old_bits == 3){
                printf("page_fault_handler:     Bits are read and write \n");
                disk_write(DISK, page_num, &PHYSMEM[0*PAGE_SIZE]);
                NUM_DISK_WRITES++;
            }
            disk_read(DISK, page, &PHYSMEM[0*PAGE_SIZE]);
            page_table_set_entry(pt, page, new_fn, PROT_READ);
            page_table_set_entry(pt, page_num, 0, 0);
            printf("END OF FRAME IS FULL!!!!! page_fault_handler:     Page table printout: \n");
            page_table_print(pt);
            NUM_DISK_READS++;
            
        }
        // There are no more free frames in physical memory
        else{
            printf("page fault on page #%d\n",page);
            exit(1);
        }
    }
    
}

/*
 * Function:  print_summary
 * --------------------
 * Prints the summary of the number of page faults, disk writes, 
 * and disk reads
 */
void print_summary(){
    printf("-------------------------------------------\n");
    printf("                  SUMMARY                  \n");
    printf("-------------------------------------------\n");
    printf("  * NUM_PAGE_FAULTS: %d \n", NUM_PAGE_FAULTS);
    printf("  * NUM_DISK_READS: %d \n", NUM_DISK_READS);
    printf("  * NUM_DISK_WRITES: %d \n", NUM_DISK_WRITES);
    printf("-------------------------------------------\n");
}


/*
 * Function:  print_summary_csv
 * --------------------
 * Prints the summary of the number of page faults, disk writes,
 * and disk reads for the csv output
 */
void print_summary_csv(){
    printf("%d, %d, %d", NUM_PAGE_FAULTS, NUM_DISK_READS, NUM_DISK_WRITES);
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
    FT.frames = (int*)malloc(sizeof(int) * NFRAMES);
    FT.time_stamps = (int*)malloc(sizeof(int) * NFRAMES);
    FT.permissions = (int*)malloc(sizeof(int) * NFRAMES);
    FT.pages = (int*)malloc(sizeof(int) * NFRAMES);
    int i;
    for (i = 0; i < NFRAMES; i++){
        FT.frames[i] = 0; // Initialize all frames to 0; will be equal to 1 if they are filled
        FT.pages[i] = 0; // Initialize all pages to 0
        FT.permissions[i] = 0; // Initialize all permissions to 0; will be set to another num when filled
    }
    FIFO_INDEX = 0;
    
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
    
    print_summary_csv();

	return 0;
}
