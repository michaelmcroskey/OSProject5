# Operating Systems Project 5
A simple but fully functional demand paged virtual memory

A project by [Michael McRoskey](http://michaelmcroskey.com/) (mmcrosk1) and [Maggie Thomann](http://maggiethomann.com/) (mthomann)

Project Overview
--------

- [Project Requirements](http://www3.nd.edu/~dthain/courses/cse30341/spring2017/project5/project5.html)

<img src="images/vm1.gif" width="500">

Operating Systems Project 5 is a page fault handler tool that will trap page faults and identify the correct course of action, which generally means updating the page table, and moving data back and forth between the disk and physical memory.  The OS course at Notre Dame provided us with code that implements a "virtual" page table and a "virtual" disk and so our job was to handle any page fault errors that could result.  A user will run `./virtmem NUM_PAGES NUM_FRAMES PAGE_REPLACEMENT_ALGORITHM PROGRAM` (described below) and the output will report (1) the number of page faults, (2) the number of disk writes, (3) the number of disk reads and the (4) result of the `PROGRAM` chosen.

|       Command Line Argument       |                 Description               |
|-----------------------------------|-------------------------------------------|
|  NUM_PAGES		            | # of pages for the page table to have; should be greater than the number of frames to demonstrate page fault functionality |
|  NUM_FRAMES                       | # of frames physical memory will contain |
| PAGE_REPLACEMENT_ALGORITHM        | Options are (1) rand, (2) fifo, or (3) custom; this will determine how to handle page faults |
| PROGRAM			    | Program to run; options are (1) sort, (2) scan, or (3) focus | 

## Files
1. **`main.c`**: This file creates the virtual disk, initializes the page table, creates the frame table, runs the selected `PROGRAM` and handles any page faults that result.  Finally, it prints out a summary of page faults.
2. **`Makefile`**: Running the command `make` in this directory will properly compile the page table, disk, main program, and `PROGRAM` selected.
3. **`program.h`**: Header file for the `PROGRAM` implementation
4. **`program.c`**: Contains functions to run the `PROGRAM` the user specified and outputs the result
5. **`disk.h`**: Header file for the disk implementation
6. **`disk.c`**: Contains the functionality for disk writing, disk reading, opening & closing the disk
7. **`page_table.h`**: Header file for the page table implementation
8. **`page_table.c`**: Contains the functionality for maintaining the status of the page table (setting an entry in the page table, getting an entry in the page table, etc. etc.)
9. **`README.md`**: Describes how to build, run, and configure code

## System Requirements
System should have a `g++` compiler installed and be able to compile with the following flags:
- `-g`
- `-Wall` for errors
- `-c` 

## Usage
1. Decide values for the following parameters you would like to test for virtual memory: (1)`NUM_PAGES`, (2)`NUM_FRAMES`, (3) `PAGE_REPLACEMENT_ALGORITHM`, & (4) `PROGRAM`
2. Run `$ make` to build the executables.
3. Run `$ ./virtmem NUM_PAGES NUM_FRAMES PAGE_REPLACEMENT_ALGORITHM PROGRAM` to simulate virtual memory.
4. The program will output the number of page faults that occured, the number of disk reads and the number of disk writes, and the result of the specific `PROGRAM`.
5. Run `$ make clean` to delete `*.dSYM` files and executables.

### Purpose of the Experiments & Experimental Set-up

### Custom Page Replacement Algorithm Explanation

### Results

### Explanation of Results
