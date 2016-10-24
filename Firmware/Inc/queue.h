// template-like queue using circular array
//
// based on this queue implementation: http://www.mathcs.emory.edu/~cheung/Courses/171/Syllabus/8-List/array-queue2.html
// and this template emulation approach: http://www.flipcode.com/archives/Faking_Templates_In_C.shtml

#define CREATE_QUEUE_TYPE_H(type) \
typedef struct _##type##Queue{ \
  type data[QUEUE_SIZE]; \
  int in; \
  int out; \
} type##Queue; \
unsigned char type##Queue_Put(type##Queue *pQ, type element); \
void type##Queue_Init(type##Queue *pQ); \
type type##Queue_Get(type##Queue *pQ); \
unsigned char type##Queue_IsEmpty(type##Queue *pQ); 

#define CREATE_QUEUE_TYPE_C(type) \
void type##Queue_Init(type##Queue *pQ) \
{ \
  pQ->in = 0; \
  pQ->out = 0; \
} \
unsigned char type##Queue_Put(type##Queue *pQ, type element) \
{ \
  if ( pQ->out == (( pQ->in + 1 ) % QUEUE_SIZE) ) \
  { \
    return 0; \
  } \
  else \
  { \
    pQ->data[pQ->in] = element; \
    pQ->in = ( pQ->in + 1 ) % QUEUE_SIZE; \
    return 1; \
  } \
} \
type type##Queue_Get(type##Queue *pQ) \
{ \
  type res; \
  if (pQ->in == pQ->out) return res; \
  res = pQ->data[pQ->out]; \
  pQ->out = (pQ->out + 1) % QUEUE_SIZE; \
  return res; \
} \
unsigned char type##Queue_IsEmpty(type##Queue *pQ) \
{ \
  return (pQ->in == pQ->out); \
} 
