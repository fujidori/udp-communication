#ifndef RINGBUF_H_
#define RINGBUF_H_

/// Pass in a storage buffer and size, returns a ring buffer handle
/// Requires: buffer is not NULL, size > 0
/// Ensures: rbuf has been created and is returned in an empty state
struct ringbuf_t* ringbuf_init(uint8_t* buffer, size_t size);

/// Free a ring buffer structure
/// Requires: rbuf is valid and created by ringbuf_init
/// Does not free data buffer; owner is responsible for that
void ringbuf_free(struct ringbuf_t* rbuf);

/// Reset the ring buffer to empty, head == tail. Data not cleared
/// Requires: rbuf is valid and created by ringbuf_init
void ringbuf_reset(struct ringbuf_t* rbuf);

/// Push version 1 continues to add data if the buffer is full
/// Old data is overwritten
/// Requires: rbuf is valid and created by ringbuf_init
void ringbuf_push(struct ringbuf_t* rbuf, uint8_t data);

/// Push Version 2 rejects new data if the buffer is full
/// Requires: rbuf is valid and created by ringbuf_init
/// Returns 0 on success, -1 if buffer is full
int ringbuf_push2(struct ringbuf_t* rbuf, uint8_t data);

/// Retrieve a value from the buffer
/// Requires: rbuf is valid and created by ringbuf_init
/// Returns 0 on success, -1 if the buffer is empty
int ringbuf_pop(struct ringbuf_t* rbuf, uint8_t * data);

/// CHecks if the buffer is empty
/// Requires: rbuf is valid and created by ringbuf_init
/// Returns true if the buffer is empty
bool ringbuf_empty(struct ringbuf_t* rbuf);

/// Checks if the buffer is full
/// Requires: rbuf is valid and created by ringbuf_init
/// Returns true if the buffer is full
bool ringbuf_full(struct ringbuf_t* rbuf);

/// Check the capacity of the buffer
/// Requires: rbuf is valid and created by ringbuf_init
/// Returns the maximum capacity of the buffer
size_t ringbuf_capacity(struct ringbuf_t* rbuf);

/// Check the number of elements stored in the buffer
/// Requires: rbuf is valid and created by ringbuf_init
/// Returns the current number of elements in the buffer
size_t ringbuf_size(struct ringbuf_t* rbuf);

//TODO: int ringbuf_get_range(ringbuf_t rbuf, uint8_t *data, size_t len);
//TODO: int ringbuf_put_range(ringbuf_t rbuf, uint8_t * data, size_t len);

#endif //RINGBUF_H_
