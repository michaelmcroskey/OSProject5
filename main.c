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
int TIME_STAMP_SIZE;

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
    printf("Page\t|\tFrame\t|\tData\t|\tPermissions\t|\tTime Stamp Order\t| \n");
    for (i = 0; i < NFRAMES; i++){
        printf("%d\t\t%d\t\t%d\t\t%d\t\t\t%d\n",
               FT.pages[i],
               i,
               FT.frames[i],
               FT.permissions[i],
               FT.time_stamps[i]);
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
    // printf("Mem count is:  %d\n", mem_count);
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
    // printf("(A) updating new frame table:....................\n");
    // print_frame_table();
    
    /*printf("updating........................................\n");
    printf("frame_num:.................................... %d\n", frame);
    printf("page_num:..................................... %d\n", p_n);
    printf("permissions:.................................. %d\n", p);*/
    
    FT.time_stamps[TIME_STAMP_SIZE] = frame;
    TIME_STAMP_SIZE++;
    FT.frames[frame] = 1;
    
    // printf("(B) after `FT.frames[frame] = 1` :....................\n");
    // print_frame_table();
    
    FT.permissions[frame] = p;
    // printf("(C) after `FT.permissions[frame] = p;` :....................\n");
    // print_frame_table();
    
    FT.pages[frame] = p_n;
    // printf("(D) after `FT.pages[frame] = p_n;` :....................\n");
    // print_frame_table();
    
    // printf("(E) updating new frame table:....................\n");
    // print_frame_table();
}

/*
 * Function:  handle_fifo_table
 * --------------------
 * When evicting a page from the frame, this updates the time
 * stamp entry of that page to be -1.
 *
 *  frame:      index of frame to evict
 */
void handle_fifo_table(int frame){
    int i;
    for (i = 0; i < TIME_STAMP_SIZE; i++){
        if (FT.time_stamps[i] == frame){
            FT.time_stamps[i] = -1;
            break;
        }
    }
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
    // printf("rand_func:  random number = %d \n", n);
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
int fifo(struct page_table *pt, int page){
    
    // Find the first in frame
    int each_frame;
    int first_in_frame;
    for (each_frame = 0; each_frame < NFRAMES; each_frame++){
        if (FT.time_stamps[each_frame] == 0){
            first_in_frame = each_frame;
        }
    }
    
    // Update the other frames to now be one less
    int current_time;
    int new_time;
    for (each_frame = 0; each_frame < NFRAMES; each_frame++){
        current_time = FT.time_stamps[each_frame];
        new_time = current_time - 1;
        FT.time_stamps[each_frame] = new_time;
    }
    return first_in_frame;
}

/*
 * Function:  get_initial_frame()
 * --------------------
 * Uses a linear search to find new number
 *
 * returns: free_frame:  index of new frame number that's free
 *
 */

int get_initial_frame(){
    int index = 0;
    int free_frame = FT.frames[index];
    while (free_frame == 1){
        index++;
        free_frame = FT.frames[index];
    }
    return index;
}

/*
 * Function:  custom()
 * --------------------
 * Searches for a clean entry that can be evicted without writing
 * anything back to disk.  Returns the fifo frame if it can't
 * find anything.
 *
 *  pt:     pointer to the page table
 *  page:   page associated with the page fault
 *
 * returns: clean_frame:  index of frame that's clean
 *
 */
int custom(struct page_table *pt, int page){
    int clean_frame;
    for (clean_frame = 0; clean_frame < NFRAMES; clean_frame++){
        if (FT.permissions[clean_frame] == 1){
            return clean_frame;
        }
    }
    clean_frame = fifo(pt, page);
    return clean_frame;
}


/*
 * Function:  get_new_frame_num()
 * --------------------
 * Determines a new frame number depending on the page
 * replacement algorithm selected
 *
 *  pt:     pointer to the page table
 *  page:   page associated with the page fault
 *
 * returns: n:  index of new frame number
 *
 */
int get_new_frame_num(struct page_table *pt, int page){
    int n;
    if (strcmp(PAGE_REPLACEMENT_TYPE, "rand") == 0){
        n = rand_func();
    }
    else if (strcmp(PAGE_REPLACEMENT_TYPE, "fifo") == 0){
        n = fifo(pt, page);
    }
    else if (strcmp(PAGE_REPLACEMENT_TYPE, "custom") == 0){
        n = custom(pt, page);
    }
    return n;
}

/*
 * Function:  evict_frame()
 * --------------------
 * Continously calls get_new_frame_num() until it finds an
 * entry that HAS data in it to evict.  In other words, it
 * will return an entry that is not free.
 *
 *  pt:     pointer to the page table
 *  page:   page associated with the page fault
 *
 * returns: n:  index of new FREE frame number
 *
 */
int get_evict_frame(struct page_table *pt, int page){
    int frame_to_evict;
    frame_to_evict = get_new_frame_num(pt, page);
    while (FT.frames[frame_to_evict] == 0){
        frame_to_evict = get_new_frame_num(pt, page);
    }
    return frame_to_evict;
}

/*
 * Function:  evict_frame()
 * --------------------
 * Actually evict the frame
 *
 *  frame_to_evict:     Frame number to evict
 *
 */
void evict_frame(int frame_to_evict){
    // printf("Evicting frame %d ... \n", frame_to_evict);
    FT.frames[frame_to_evict] = 0;
    FT.permissions[frame_to_evict] = 0;
    FT.pages[frame_to_evict] = 0;
    FT.time_stamps[frame_to_evict] = 0;
    TIME_STAMP_SIZE--;
    
    
    // print_frame_table();
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
    /*printf("page_fault_handler:  page_table_get_entry completed \n");
    printf("**GET ENTRY SUMMARY**\n");
    printf("PAGE NUMBER:\t%d \n", page);
    printf("FRAME NUMBER:\t%d \n", fn);
    printf("BITS:\t\t%d \n", bits);*/
    // printf("Current Size of Frame Table: %d \n", FT.phys_mem_count);
    // // // print_frame_table();
    // Trivial Example
    if (NFRAMES == NPAGES){
        page_table_set_entry(pt,page,page,PROT_READ|PROT_WRITE);
        return;
    }
        
    
    else if ((bits == 0) && (!(frame_is_full()))){  // No permissions so its a new element
        //printf("page_fault_handler: Inside if statement for no permissions \n");
        
        // Get new frame number
        int new_fn;
        new_fn=get_initial_frame();
        
        // Set the entry in the page table
        page_table_set_entry(pt, page, new_fn, PROT_READ);
        
        // Update the global frame table
        FT.frames[new_fn] = 1;
        FT.permissions[new_fn] = 1;
        FT.pages[new_fn] = page;
        FT.time_stamps[new_fn] = TIME_STAMP_SIZE;
        TIME_STAMP_SIZE++;
        
        // print_frame_table();
        
        // Handle the disk
        disk_read(DISK, page, &PHYSMEM[new_fn*FRAME_SIZE]);
        NUM_DISK_READS++;
        
        
        /*printf("page_fault_handler: finished adding read permissions  \n");
        printf("page_fault_handler:     Page table printout: \n");
        // page_table_print(pt);
        printf("page_fault_handler:     Frame table printout: \n");
        // print_frame_table();*/
        return;
    }
    else if (bits == PROT_READ){ // Only read permissions
        // printf("page_fault_handler:     Frame table printout AT THE BEGINNING OF ONLY READ PERMISSIONS: \n");
        // print_frame_table();
        
        // Set the entry in the page table to be read and write
        page_table_set_entry(pt, page, fn, (PROT_READ|PROT_WRITE));
        // printf("page_fault_handler:    page table after updating to read and write \n");
        // page_table_print(pt);
        
        
        // Update the frame table permissions for that frame to be 3 (read & write)
        FT.permissions[fn] = 3;
        // printf("page_fault_handler:     frame table after updating to 3 \n");
        // print_frame_table();
        
        return;
    }
    
    else if(frame_is_full()){
        // FIX THIS ONE NOW!!!!!!!!!!!!
        
        /*printf("page_fault_handler:     No more free frames in physical memory \n");
        printf("page_fault_handler:     Frame table printout: \n");
        // print_frame_table();
        printf("page_fault_handler:     Page table printout: \n");
        // page_table_print(pt);*/
        
        // Get the frame to evict
        new_fn = get_evict_frame(pt, page);
        // printf("Kicking out frame %d\n", new_fn);
        int page_num, old_bits;
        page_num = FT.pages[new_fn];
        old_bits = FT.permissions[new_fn];
        // Getting info for entry being kicked out
        /*printf("**ENTRY BEING KICKED OUT SUMMARY WITH OLD BITS**\n");
        printf("PAGE NUMBER:\t%d \n", page_num);
        printf("FRAME NUMBER:\t%d \n", new_fn);
        printf("BITS:\t\t%d \n", old_bits);*/
        
        
        // Evicting the frame
        evict_frame(new_fn);
        /*
        if (!strcmp(PAGE_REPLACEMENT_TYPE,"fifo")){
            handle_fifo_table(new_fn);
        }*/
        
        
        if (old_bits == 3){
            /*printf("************************************ \n");
            printf("WRITE IS HAPPENING HERE ************ \n");
            printf("************************************ \n");
            printf("page_fault_handler:     Bits are read and write \n");*/
            disk_write(DISK, page_num, &PHYSMEM[0*PAGE_SIZE]);
            NUM_DISK_WRITES++;
        }
        
        
        
        // printf("frame_table after evicting frame %d \n", new_fn);
    
        
        // printf("*****************FRAME TABLE BEFORE UPDATE*****************\n");
        // print_frame_table();
        
        FT.frames[new_fn] = 1;
        FT.permissions[new_fn] = 1;
        FT.pages[new_fn]=page;
        FT.time_stamps[new_fn] = TIME_STAMP_SIZE;
        TIME_STAMP_SIZE++;
        
        // printf("*****************FRAME TABLE AFTER UPDATE*****************\n");
        // print_frame_table();
    
        disk_read(DISK, page, &PHYSMEM[0*PAGE_SIZE]);
        page_table_set_entry(pt, page, new_fn, PROT_READ);
        page_table_set_entry(pt, page_num, 0, 0);
        /*printf("***********END OF FRAME IS FULL!!!!! page_fault_handler:     Page table printout: \n");
        // page_table_print(pt);
        printf("frame_table at end of frame table is full %d \n", new_fn);*/
        // print_frame_table();
        NUM_DISK_READS++;
        
    }
    // There are no more free frames in physical memory
    else{
        printf("page fault on page #%d\n",page);
        exit(1);
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
    TIME_STAMP_SIZE = 0;
    
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
        FT.time_stamps[i] = 0; // Initialize all permissions to 0; will be set to another num when filled
    }
    
    // print_frame_table();
    
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
