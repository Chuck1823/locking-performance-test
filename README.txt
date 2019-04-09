Charles Antoine Malenfant (0616332)
CS444 - Spring 2019
Grid Assignment

FILE GUIDE

This assignment has a total of 5 files including this README.txt file. The first one, gridapp.c contains the
source code for the assignment and all necessary changes are clearly and thoroughly commented directly in the
code directly.

The second file, gridAssignment_data is an Excel spreadsheet with five tabs containing all the raw data from
the 228 tests ran. The first four tab, NONE, CELL, ROW, and GRID aggregate the raw data by type of locking
granularity. All rows and columns are clearly labeled. Each tab is separated by a dark blue delimiter that
segregates the tests ran with a constant grid size of ten (on the left of the delimiter) and the tests ran
with a constant thread count of ten (on the right side of the delimiter). Copies of the graphs used in the analysis
are also present in each corresponding tab. The last and fifth tab called COMPREHENSIVE GRAPHS contains two
graphs that include data from the other four tabs.

The third file, gridAssignment_writeup is a pdf file that contains the writeup associated with this assignment.
The writeup starts with a description of the locking and deadlock prevention strategy used for the assignment.
For each locking granularity, a depiction of the strategy is given along with its deadlock prevention approach,
if needed. The next five pages comprise all of the necessary graphs used in the analysis of the test results.
Each graph is defined clearly. The last part is the analysis and every part of the analysis is self-explanatory.
Questions that were in green font on the assignment description at: https://people.clarkson.edu/~jmatthew/os/assignments/grid/index.html
are in a dark blue font in the analysis section of the writeup.

The fourth file is the makefile used to build and compile the gridapp.c file.

EXTRAS

I did not implement any extra features in the gridapp.c file per say but you may have noted that I was extremely
thorough in my analysis, graphs, material in general. It is for you to judge if this is worthy of extra credit.

BUGS AND POSSIBLE IMPROVEMENTS

*** If the program is to be compiled using the makefile (not manually), the makefile itself needs to be
modified to include either -DUNIX (for Linux machines) or -DWINDOWS (for Windows machines) under CF flags. By
default, I have included -DUNIX in the makefile submitted with this assignment. This needs to be changed to
-DWINDOWS if run on Windows. If this is not done, none of the #ifdef statements will be recognized and this
entire program will not run properly (all the locking will be avoided!).***

I did not focus much on having efficient code runtime wise. For example, in my Init_Locks function, a nested
for loop is used to initialize the two-dimensional array for the cell locking scheme. Furthermore, all locks
for all granularities are initialized no matter what the chosen scheme is. The nested ifs for deadlock prevention
could probably be more beautifully written as well. All in all, there are a couple of tweaks here and there
that could be done to have the code be more organized and efficient.

HOW TO RUN

Assuming that the gridapp.c will not be built and compiled manually... Decompress the tar.gz file and navigate
to the directory containing the gridapp.c file and the makefile. Run the following commands:

$ make gridapp
$ ./gridapp gridsize thread_count -granularity

NOTES AND COMMENTS

Please note that all of my tests were run on the opsys.clarkson.edu server. Although I am close to 100% certain
that this did not skew my results, I thought I would mention it. My tests were also run with my code having
some print statements that have since been removed. I did this because I liked to see the progression of the
execution of the threads.
