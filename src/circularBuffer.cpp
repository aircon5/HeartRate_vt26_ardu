#include <stdio.h>
#include <limits.h>
#include "circularBuffer.h"



void initCircularBuffer(struct circularBuffer* bufferPtr, int* data, int maxLength) {
  bufferPtr->data = data;
  bufferPtr->head = 0;
  bufferPtr->tail = 0;
  bufferPtr->maxLength = maxLength;
  bufferPtr->size = 0;
  bufferPtr->average = 0;
}

/* 
 * This function should check if the buffer pointed to by bufferPtr
 * contains one or more elements with the value specified by the 
 * 'value' argument.
 *                                                                                                  
 * The function should return:
 *  - 'value' if the an element with the argument value was found in the queue.
 *  - INT_MIN (defined in limits.h) if value was not found.
 */ 
int contains(struct circularBuffer* bufferPtr, int value) 
{
  int startIndex = bufferPtr -> head;

  while (startIndex != bufferPtr->tail)
  {
    if(bufferPtr->data[startIndex] == value) {
      return value;
    } 
    startIndex = (startIndex+1) % (bufferPtr->maxLength);
  }
  return INT_MIN;
}

/*
 * This function should add the value specified by the 'value' 
 * argument at the tail of the buffer.
 *
 * The function should return:
 *  - 'value' if the value was successfully added to the queue.
 *  - INT_MIN (defined in limits.h) if the value was not added.
 */
int addElement(struct circularBuffer* bufferPtr, int value) 
{
  int tailIndex = bufferPtr->tail;
  int headIndex = bufferPtr->head;
  

  if(bufferPtr->size == bufferPtr->maxLength)  // om det är fullt 
  {
    removeHead(bufferPtr);
    bufferPtr->head = (headIndex + 1) % bufferPtr->maxLength;
  } 
  
  bufferPtr->data[tailIndex] = value;
  bufferPtr->tail = (tailIndex + 1) % bufferPtr->maxLength;
  bufferPtr->size++;
  bufferPtr->sum += value;
  return value;

}


/* 
 * Remove the oldest element, which is at the head of the queue. 
 * 
 * The function should return:
 *   - 'value' if the head element was successfully removed
 *   - INT_MIN (defined in limits.h) if no element was removed (i.e., the
 *     queue was empty when the function was called.       
 */
int removeHead(struct circularBuffer* bufferPtr) 
{
  int startIndex = bufferPtr->head;
  int headValue;

  if(bufferPtr->data[startIndex] != INT_MIN) {
    headValue = bufferPtr->data[startIndex];
    bufferPtr->data[startIndex] = INT_MIN;
    bufferPtr->head = (bufferPtr->head + 1) % (bufferPtr->maxLength);
    bufferPtr->size--;
    bufferPtr->sum -= headValue;
    return headValue;
  }
  return INT_MIN;
}

int get(struct circularBuffer* bufferPtr, int i)  
{
  return bufferPtr->data[(bufferPtr->head + i)%bufferPtr->maxLength];
}

/* 
 * Print the elements in the buffer from head to tail. 
 */
void printBuffer(struct circularBuffer* bufferPtr) {
  int startIndex = bufferPtr -> head;
  while (startIndex != bufferPtr->tail)
  {
    printf("%d, ", bufferPtr->data[startIndex]);
    startIndex = (startIndex+1) % (bufferPtr->maxLength);
  }
}

int getSize(struct circularBuffer* bufferPtr) 
{
  return bufferPtr->size;
}

int getSum(struct circularBuffer* bufferPtr) 
{
    printf("---------summan: %d \n", bufferPtr->sum);
    return bufferPtr->sum;

}

int getAverage(struct circularBuffer* bufferPtr) 
{
  return bufferPtr->sum/bufferPtr->size;
}





