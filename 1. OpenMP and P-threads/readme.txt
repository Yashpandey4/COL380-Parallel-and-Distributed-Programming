To compile both codes, write:
./compile.sh

To run the compiled files for all the matrix sizes and threads using OpenMP, use:
chmod 777 run1.sh
./run1.sh size num_threads input_file <rand> 		/* rand=1 if randomized initialization, otherwise 0 */

To run the compiled files for all the matrix sizes and threads using P-threads, use:
chmod 777 run2.sh
./run2.sh size num_threads input_file <rand>		/* rand=1 if randomized initialization, otherwise 0 */

The output will be written on log files saved in the same directory concurrently.
