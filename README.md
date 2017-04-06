# Operating Systems Project 5
A simple but fully functional demand paged virtual memory

A project by [Michael McRoskey](http://michaelmcroskey.com/) (mmcrosk1) and [Maggie Thomann](http://maggiethomann.com/) (mthomann)

Project Overview
--------

- [Project Requirements](http://www3.nd.edu/~dthain/courses/cse30341/spring2017/project5/project5.html)

<img src="images/vm1.gif" width="500">

Operating Systems Project 5 is a page fault handler tool that will trap page faults and identify the correct course of action, which generally means updating the page table, and moving data back and forth between the disk and physical memory.  The OS course at Notre Dame provided us with code that implements a "virtual" page table and a "virtual" disk and so our job was to handle any page fault errors that could result.  A user will run `./virtmem NUM_PAGES NUM_FRAMES PAGE_REPLACEMENT_ALGORITHM PROGRAM` and the output will report (1) the number of page faults, (2) the number of disk writes, (3) the number of disk reads and the (4) result of the `PROGRAM` chosen.

## Files
1. **`site-tester.cpp`**: Given a configuration file argument, it uses producer/consumer threads to fetch websites and parse fetched content to count the occurrences of defined queries in real time
2. **`Makefile`**: Running the command `make` in this directory will properly compile `site-tester.cpp`
3. **`ConfigFile.h`**: Loads in configuration parameters and sets defaults if necessary
4. **`LibCurl.h`**: Grabs a webpage via [libcurl](https://curl.haxx.se/libcurl/) and stores into a C++ string
5. **`Config.txt`**: Example configuration plain text file. Lists arguments for `site-tester` in the form `PARAMETER=VALUE`
6. **`Search.txt`**: Example search terms plain text file with each term on its own line
7. **`Sites.txt`**: Example sites plain text file with each `http://`-prefixed URL on its own line
8. **`README.md`**: Describes how to build, run, and configure code
9. **`EC1.txt`**: Extra credit description
10. **`html/`**
  + **`1.html`**: Webpage to view results of `1.csv`. Similarly, `n.html` where *n* is an integer will show the results of `n.csv`
  + **`append.txt`**: file with minified html that gets appended to output to form `1.html`, `2.html`, etc.
  + **`styles.css`**: CSS styles for html output
14. **`csv/`**
  + **`1.csv`**: first CSV file -- all generated CSV files will go here
16. **`images/`**
  + **`ec_screenshot.png`**: screenshot of extra credit at [http://localhost:8000/html/1.html]()

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

### Extra Credit
1. Run steps 1-5 above.
2. `$ cd root_project_directory`
3. `$ python -m SimpleHTTPServer`
4. Navigate to [http://localhost:8000/html/1.html]() in a web browser and replace `1.html` with any html that's been generated.

## File Requirements

**`Config.txt`**
*See `Config.txt` for working example*
```
PERIOD_FETCH=<int::period>
NUM_FETCH=<int::number_of_fetch_threads>
NUM_PARSE=<int::number_of_parse_threads>
SEARCH_FILE=<string::search_terms_filename>
SITE_FILE=<string::sites_filename>
```

**`Search.txt`**
*See `Search.txt` for working example*
```
<string::search_term_1>
<string::search_term_2>
...
<string::search_term_n>
```

**`Sites.txt`**
*See `Sites.txt` for working example*
```
<string::url_to_fetch_1>
<string::url_to_fetch_2>
...
<string::url_to_fetch_3>
```

## What's Going On

Below is a pseudocode version of the `main()` function in `site-tester.cpp`

```python
int main():

	catch_signals()
	initialize_config_parameters()
	config.display()
	
	num = 1
	# Every period
	while(1):
		
		fetch_queue.lock()
		fetch_queue.push(urls[])
		fetch_queue.unlock()
		
		# Fetch Threads
		for thread in num_fetch_threads:
			create_thread(fetch())
			
		stop_fetching(); # use condition variable to stop fetching
					
		for thread in num_fetch_threads:
			join_thread()
			
		delete fetch_threads
		
		search_terms[] = load_search_terms()
		
		# Parse Threads
		for thread in num_parse_threads:
			create_thread(parse())
			
		stop_parsing(); # use condition variable to stop fetching
					
		for thread in num_parse_threads:
			join_thread()
			
		delete parse_threads
		
		output_to_file(to_string(num++) + ".csv");
		
		sleep_this_thread(config.period);
```

The `fetch()` and `parse()` functions each make use of `unique_lock<mutex>` to lock and unlock queues and two `condition_variable` variables to alert the other theads when they can attempt to acquire the mutex again.

**Rubric Qualifications**

| Feature                        | Description                                                             |
|--------------------------------|-------------------------------------------------------------------------|
| Correct implementation of demand paging with any arbitrary access pattern and amount of virtual and physical memory.   | Good! (at least this README is well documented!)                        |
| A lab report which is clearly written using correct English, contains an appropriate description of your experiments, contains correct results that are clearly presented, and draws appropriate conclusions.  | Yes (gives warning for unknown params, sets defaults)                   |
| Thorough attention to and handling of all possible error conditions, including user error.     | Passes                                                                  |
| Good coding style, including clear formatting, sensible variable names, and useful comments.   | Passes                                                                  |
