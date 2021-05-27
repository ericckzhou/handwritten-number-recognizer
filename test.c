#include "knn.h"

// Makefile included in starter:
//    To compile:               make
//    To decompress dataset:    make datasets
//
// Example of running validation (K = 3, 8 processes):
//    ./classifier 3 datasets/training_data.bin datasets/testing_data.bin 8

/*****************************************************************************/
/* This file should only contain code for the parent process. Any code for   */
/*      the child process should go in `knn.c`. You've been warned!          */
/*****************************************************************************/

/**
 * main() takes in 4 command line arguments:
 *   - K:  The K value for kNN
 *   - training_data: A binary file containing training image / label data
 *   - testing_data: A binary file containing testing image / label data
 *   - num_procs: The number of processes to be used in validation
 * 
 * You need to do the following:
 *   - Parse the command line arguments, call `load_dataset()` appropriately.
 *   - Create the pipes to communicate to and from children
 *   - Fork and create children, close ends of pipes as needed
 *   - All child processes should call `child_handler()`, and exit after.
 *   - Parent distributes the testing set among childred by writing:
 *        (1) start_idx: The index of the image the child should start at
 *        (2)    N:      Number of images to process (starting at start_idx)
 *     Each child should gets N = ceil(test_set_size / num_procs) images
 *      (The last child might get fewer if the numbers don't divide perfectly)
 *   - Parent waits for children to exit, reads results through pipes and keeps
 *      the total sum.
 *   - Print out (only) one integer to stdout representing the number of test 
 *      images that were correctly classified by all children.
 *   - Free all the data allocated and exit.
 */
int main(int argc, char *argv[]) {
  // TODO: Handle command line arguments

  // TODO: Spawn `num_procs` children
    int p_in[2], p_out[2], r;
    pipe(p_in);
    pipe(p_out);
    r = fork();
    if (r > 0){
        close(p_in[0]);
        close(p_out[1]);
        write(p_in[1], "hello child", 12);
        close(p_in[1]);
        char buffer[1024];
        read(p_out[0], buffer, 1024);
        printf("read from child: %s\n", buffer);
        close(p_out[0]);
    }
    else if (r == 0){
        close(p_in[1]);
        close(p_out[0]);
        char buffer[1024];
        read(p_in[0], buffer, 1024);
        printf("read from parent: %s\n", buffer);
        close(p_in[0]);
        strcpy(buffer, "hey parents!");
        printf("%d\n", (int)strlen(buffer));
        write(p_out[1], buffer, strlen(buffer));
        close(p_out[1]);
    }
    return 0;
  // TODO: Send information to children

  // TODO: Compute the total number of correct predictions from returned values
  int total_correct = 0;

  // Print out answer
  printf("%d\n", total_correct);
  return 0;
}
