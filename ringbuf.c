#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <error.h>

#include "ringbuf.h"

struct ringbuf_t {
	uint8_t * buffer;
	size_t head;
	size_t tail;
	size_t max; //of the buffer
	bool full;
};

static void advance_pointer(struct ringbuf_t* rbuf)
{
	if(rbuf->full)
    {
        rbuf->tail = (rbuf->tail + 1) % rbuf->max;
    }

	rbuf->head = (rbuf->head + 1) % rbuf->max;

	// We mark full because we will advance tail on the next time around
	rbuf->full = (rbuf->head == rbuf->tail);
}

static void retreat_pointer(struct ringbuf_t* rbuf)
{
	rbuf->full = false;
	rbuf->tail = (rbuf->tail + 1) % rbuf->max;
}

struct ringbuf_t* ringbuf_init(uint8_t* buffer, size_t size)
{
	struct ringbuf_t* rbuf = malloc(sizeof(struct ringbuf_t));
	if (rbuf == NULL) {
		perror("malloc failed");
		exit(EXIT_FAILURE);
	}

	rbuf->buffer = buffer;
	rbuf->max = size;
	ringbuf_reset(rbuf);

	return rbuf;
}

void ringbuf_free(struct ringbuf_t* rbuf)
{
	free(rbuf);
}

void ringbuf_reset(struct ringbuf_t* rbuf)
{
    rbuf->head = 0;
    rbuf->tail = 0;
    rbuf->full = false;
}

size_t ringbuf_size(struct ringbuf_t* rbuf)
{
	size_t size = rbuf->max;

	if(!rbuf->full)
	{
		if(rbuf->head >= rbuf->tail)
		{
			size = (rbuf->head - rbuf->tail);
		}
		else
		{
			size = (rbuf->max + rbuf->head - rbuf->tail);
		}

	}

	return size;
}

size_t ringbuf_capacity(struct ringbuf_t* rbuf)
{
	return rbuf->max;
}

void ringbuf_push(struct ringbuf_t* rbuf, uint8_t data)
{
    rbuf->buffer[rbuf->head] = data;

    advance_pointer(rbuf);
}

int ringbuf_push2(struct ringbuf_t* rbuf, uint8_t data)
{
    int r = -1;

    if(!ringbuf_full(rbuf))
    {
        rbuf->buffer[rbuf->head] = data;
        advance_pointer(rbuf);
        r = 0;
    }

    return r;
}

int ringbuf_pop(struct ringbuf_t* rbuf, uint8_t * data)
{
    int r = -1;

    if(!ringbuf_empty(rbuf))
    {
        *data = rbuf->buffer[rbuf->tail];
        retreat_pointer(rbuf);

        r = 0;
    }

    return r;
}

bool ringbuf_empty(struct ringbuf_t* rbuf)
{
    return (!rbuf->full && (rbuf->head == rbuf->tail));
}

bool ringbuf_full(struct ringbuf_t* rbuf)
{
    return rbuf->full;
}
