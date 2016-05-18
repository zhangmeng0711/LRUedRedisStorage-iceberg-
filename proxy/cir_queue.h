/*
 * \File
 * cir_queue.h
 * \Brief
 * circular queue
 */
#ifndef __CIR_QUEUE_H__
#define __CIR_QUEUE_H__

#define QUE_SIZE 4096
#define MAX_LEN 128
typedef char* DataType;
typedef struct cir_queue_t
{
  DataType data[QUE_SIZE];
  int front;
  int rear;
  int count;
}cir_queue_t;

extern sem_t queue_sem; 

void init_cir_queue(cir_queue_t* q);
int is_empty_cir_queue(cir_queue_t* q);
int is_full_cir_queue(cir_queue_t* q);
void push_cir_queue(cir_queue_t* q, DataType x);
DataType pop_cir_queue(cir_queue_t* q);
DataType top_cir_queue(cir_queue_t* q);
void destroy_cir_queue(cir_queue_t* q);
void print_queue(cir_queue_t* q);
void clear_queue(cir_queue_t *q);
#endif
