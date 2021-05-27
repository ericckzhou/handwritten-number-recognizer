#include "knn.h"

/****************************************************************************/
/* For all the remaining functions you may assume all the images are of the */
/*     same size, you do not need to perform checks to ensure this.         */
/****************************************************************************/

/**************************** A1 code ****************************************/

/* Same as A1, you can reuse your code if you want! */
double distance(Image *a, Image *b) {
  // TODO: Return correct distance
  double sum = 0;
  for (int i = 0; i < a->sx * a->sy; i++){
    sum += pow(a->data[i] - b->data[i], 2.0);
  }
  return sqrt(sum);
}

/* Same as A1, you can reuse your code if you want! */
int knn_predict(Dataset *data, Image *input, int K) {
  // TODO: Replace this with predicted label (0-9)
  int predicted[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  double dist[K];
  int indices[K];

  double d = 0;
  int label = 0;
  int k = 0;

  double max_d = 0;
  int max_index = 0;
  for (int i = 0; i < data->num_items; i++){
      d = distance(&data->images[i], input);
      if (k == K && d < max_d){
        dist[max_index] = d;
        indices[max_index] = i;

        max_d = 0;
        for (int j = 0; j < k; j++){
            if (dist[j] > max_d){
              max_d = dist[j];
              max_index = j;
            }
        }
      }
      if (k < K){
        dist[k] = d;
        if (max_d == 0 || d > max_d){
          max_d = d;
          max_index = k;
        }
        indices[k] = i;
        k++;
      }
  }
  for (int i = 0; i < k; i++){
    label = data->labels[indices[i]];
    predicted[label] ++;
  }
  int predict = 0;
  int count = 0;
  for (int i = 0; i < 10; i++){
    if (predicted[i] > count){
      count = predicted[i];
      predict = i;
    }
  }
  return predict;
}

/**************************** A2 code ****************************************/

/* Same as A2, you can reuse your code if you want! */
Dataset *load_dataset(const char *filename) {
  // TODO: Allocate data, read image data / labels, return
  FILE *fp = fopen(filename, "rb");
  if (fp == NULL){
    fprintf(stderr, "\nError: File could not be opened!\n");
    return NULL;
  }
  Dataset *dataset = malloc(sizeof(Dataset));
  int size;
  if (fread(&size, sizeof(int), 1, fp) != 1){ //first 4 bytes is size of dataset => int 4x8=32 bits
    fprintf(stderr, "fread error\n");
  }
  //printf("%d\n", size); 
  dataset->num_items = size;
  dataset->images = malloc(size * sizeof(Image));
  dataset->labels = malloc(size * sizeof(unsigned char));

  //next byte is label then followed by 784 bytes of pixels
  unsigned char val;
  int count_imgs = 0;
  int count = 1;
  if (fread(&dataset->labels[count_imgs], sizeof(unsigned char), 1, fp) != 1){
    fprintf(stderr, "fread error\n");
  }
  dataset->images[count_imgs].sx = 28;
  dataset->images[count_imgs].sy = 28;
  dataset->images[count_imgs].data = malloc((28 * 28) * sizeof(unsigned char));

  while (fread(&val, sizeof(val), 1, fp) == 1)
  {
    if (count % (784 + 1) == 0){
      count = 0;
      count_imgs++;
      dataset->labels[count_imgs] = val;
      dataset->images[count_imgs].sx = 28;
      dataset->images[count_imgs].sy = 28;
      dataset->images[count_imgs].data = malloc((28 * 28) * sizeof(unsigned char));
    }
    else {
      dataset->images[count_imgs].data[count - 1] = val;
    }
    count++;
  }
  fclose(fp);
  return dataset;
}

/* Same as A2, you can reuse your code if you want! */
void free_dataset(Dataset *data) {
  // TODO: free data
  if (data == NULL){
    return;
  }
  for (int i = 0; i < data->num_items; i++){
    free(data->images[i].data);
  }
  free(data->labels);
  free(data->images);
  free(data);
  return;
}


/************************** A3 Code below *************************************/

/**
 * NOTE ON AUTOTESTING:
 *    For the purposes of testing your A3 code, the actual KNN stuff doesn't
 *    really matter. We will simply be checking if (i) the number of children
 *    are being spawned correctly, and (ii) if each child is recieving the 
 *    expected parameters / input through the pipe / sending back the correct
 *    result. If your A1 code didn't work, then this is not a problem as long
 *    as your program doesn't crash because of it
 */

/**
 * This function should be called by each child process, and is where the 
 * kNN predictions happen. Along with the training and testing datasets, the
 * function also takes in 
 *    (1) File descriptor for a pipe with input coming from the parent: p_in
 *    (2) File descriptor for a pipe with output going to the parent:  p_out
 * 
 * Once this function is called, the child should do the following:
 *    - Read an integer `start_idx` from the parent (through p_in)
 *    - Read an integer `N` from the parent (through p_in)
 *    - Call `knn_predict()` on testing images `start_idx` to `start_idx+N-1`
 *    - Write an integer representing the number of correct predictions to
 *        the parent (through p_out)
 */
void child_handler(Dataset *training, Dataset *testing, int K, 
                   int p_in, int p_out) {
  // TODO: Compute number of correct predictions from the range of data 
  //      provided by the parent, and write it to the parent through `p_out`.
  int start_idx, N;
  int predicted;
  int total_correct = 0;
  if (read(p_in, &start_idx, sizeof(int)) == -1){
    perror("read");
  }
  if (read(p_in, &N, sizeof(int)) == -1){
    perror("read");
  }
  if (close(p_in) == -1){
    perror("close");
  }
  for (int i = start_idx; i < start_idx + N; i++){
    predicted = knn_predict(training, &testing->images[i], K);
    if (testing->labels[i] == predicted){
      total_correct++;
    }
  }
  if (write (p_out, &total_correct, sizeof(int)) == -1){
    perror("write");
  }
  if (close(p_out) == -1){
    perror("close");
  }
  //FREEING COPIES OF ADDRESSES TO TRAINING AND TESTING DATA FROM FORK 
  free_dataset(training);
  free_dataset(testing);
  return;
  //1193 @ 1250
  //2390 @ 2500
}//3586 @ 3750