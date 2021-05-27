#include "knn.h"

// Makefile included in starter:
//    To compile:               make
//    To decompress dataset:    make datasets
//
// Example of running validation (K = 3, 8 processes):
//    ./classifier 3 datasets/training_data.bin datasets/testing_data.bin 8
// 9705

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
  if (argc != 5){
    fprintf(stderr, "Incorrect # of arguments! Expects only 4 command-line arguments!");
    return 1;
  }
  int K, num_procs;
  char train_file[257], test_file[257];
  K = atoi(argv[1]);
  strcpy(train_file, argv[2]);
  strcpy(test_file, argv[3]);
  num_procs = atoi(argv[4]);
  if (K <= 0 || num_procs < 0){
    fprintf(stderr, "\nError: Invalid K or num_procs!\n");
    return 1;
  }
  Dataset *train_data = load_dataset(train_file);
  Dataset *test_data = load_dataset(test_file);
  if (train_data == NULL || test_data == NULL){
    fprintf(stderr, "\nError: Invalid file path!\n");
    return 1;
  }
  if (num_procs == 0){
    int total_correct = 0;
    int predicted = 0;
    for (int i = 0; i < test_data->num_items; i++){
      predicted = knn_predict(train_data, &test_data->images[i], K);
      if (predicted == test_data->labels[i]){
        total_correct++;
      }
    } 
    printf("%d\n", total_correct);
    free_dataset(train_data);
    free_dataset(test_data);
    return 0;
  }
  // TODO: Spawn `num_procs` children
  int r;
  int fd[num_procs * 2][2];
  for (int i = 0; i < num_procs * 2; i+=2){
    //first pipe used for: child reading from parent, parent writing to child
    //second pipe used for: parent reading from child, child writing to parent.
    if (pipe(fd[i]) == -1 || pipe(fd[i+1]) == -1){
      perror("pipe");
      exit(1);
    }
    r = fork();
    if (r < 0){
      perror("fork");
      exit(1);
    }
    else if (r == 0){ //child
    //closing parent writing to child fd[j][1] and child reading from parent fd[j+1][0] from all prev forked child and current's.
      // child only concerned about reading from parent from fd[i][0] and writing to parent from fd[i+1][1]
      //essentially closing ends from pipe call which copied prev child reading/writing ends as well.     
      for (int j = 0; j <= i; j+=2){
        if (close(fd[j][1]) == -1 || close(fd[j+1][0]) == -1){
          perror("close");
        }
      }
      //calls child_handler
      child_handler(train_data, test_data, K, fd[i][0], fd[i+1][1]);
      exit(0); //num_procs only.
    }
    else { //parent - closing parent reading to child and child writing to parent.
    //parent only concern about fd[i][1] - writing to child, start_idx and N and
    //retrieving results from fd[i+1][0] from child.
      if (close(fd[i][0])== -1 || close(fd[i+1][1]) == -1){
        perror("close");
      }
    }
  }
  //parent writing to child, start_idx and N
  int start_idx = 0;
  int N = ceil((1.0 * test_data->num_items) / num_procs);
  for (int i = 0; i < num_procs * 2; i+=2){
    if (write(fd[i][1], &start_idx, sizeof(int)) == -1){
      perror("write");
    }
    if (start_idx + N >= test_data->num_items){
      N = test_data->num_items - start_idx;
    }
    else{
      start_idx += N;
    }
    if (write(fd[i][1], &N, sizeof(int)) == -1){
      perror("write");
    }
    close(fd[i][1]); //closing write end
  }
  int total_correct = 0;
  int buffer;
  for (int i = 0; i < num_procs * 2; i+=2){
    if (read(fd[i+1][0], &buffer, sizeof(int)) == -1){
      perror("read");
    }
    total_correct += buffer;
    if (close(fd[i+1][0]) == -1){ //closing read end
      perror("close");
    }
  }
  printf("%d\n", total_correct);
  free_dataset(train_data);
  free_dataset(test_data);
  return 0;
}
