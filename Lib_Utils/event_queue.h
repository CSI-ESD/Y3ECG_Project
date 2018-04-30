
#ifndef EVENT_QUEUE_H_
#define EVENT_QUEUE_H_

#define SIZE_OF_EVENT_Q 8

typedef struct
{
    int event;
    int param1;
    int param2;
    int param3;
} Event;

typedef struct
{
    int size;
    int head;
    int tail;
    Event queue[SIZE_OF_EVENT_Q];
} EventQueue;

void initQ(EventQueue *q);
int readQ(EventQueue *q, Event *e);
int writeQ(EventQueue *q, Event e);

#endif /* EVENT_QUEUE_H_ */
