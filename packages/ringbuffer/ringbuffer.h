/**
 * @file board.h
 * @brief 档案简易说明
 *
 * @copyright
 *  Copyright (c) 2022 Yanshi Inc.
 *  All Rights Reserved.This computer program includes Confidential,
 *  Proprietary Information and is a Trade Secret of Yanshi Inc.
 *  All use, disclosure, and/or reproduction is prohibited unless authorized in
 *  writing.
 *
 * @details
 *  详细说明.
 *
 * @author zengyisong@zhmtek.com
 * @date 2023/5/26 10:55
 *
 **/
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "stm32l0xx_hal.h"
#define RT_ALIGN_SIZE 4
#define RT_ALIGN_DOWN(size, align)      ((size) & ~((align) - 1))
    
/* ring buffer */
struct ringbuffer
{
    uint8_t *buffer_ptr;
    /* use the msb of the {read,write}_index as mirror bit. You can see this as
     * if the buffer adds a virtual mirror and the pointers point either to the
     * normal or to the mirrored buffer. If the write_index has the same value
     * with the read_index, but in a different mirror, the buffer is full.
     * While if the write_index and the read_index are the same and within the
     * same mirror, the buffer is empty. The ASCII art of the ringbuffer is:
     *
     *          mirror = 0                    mirror = 1
     * +---+---+---+---+---+---+---+|+~~~+~~~+~~~+~~~+~~~+~~~+~~~+
     * | 0 | 1 | 2 | 3 | 4 | 5 | 6 ||| 0 | 1 | 2 | 3 | 4 | 5 | 6 | Full
     * +---+---+---+---+---+---+---+|+~~~+~~~+~~~+~~~+~~~+~~~+~~~+
     *  read_idx-^                   write_idx-^
     *
     * +---+---+---+---+---+---+---+|+~~~+~~~+~~~+~~~+~~~+~~~+~~~+
     * | 0 | 1 | 2 | 3 | 4 | 5 | 6 ||| 0 | 1 | 2 | 3 | 4 | 5 | 6 | Empty
     * +---+---+---+---+---+---+---+|+~~~+~~~+~~~+~~~+~~~+~~~+~~~+
     * read_idx-^ ^-write_idx
     *
     * The tradeoff is we could only use 32KiB of buffer for 16 bit of index.
     * But it should be enough for most of the cases.
     *
     * Ref: http://en.wikipedia.org/wiki/Circular_buffer#Mirroring */
    uint16_t read_mirror : 1;
    uint16_t read_index : 15;
    uint16_t write_mirror : 1;
    uint16_t write_index : 15;
    /* as we use msb of index as mirror bit, the size should be signed and
     * could only be positive. */
    int16_t buffer_size;
};

enum ringbuffer_state
{
    RINGBUFFER_EMPTY,
    RINGBUFFER_FULL,
    /* half full is neither full nor empty */
    RINGBUFFER_HALFFULL,
};

/**
 * RingBuffer for DeviceDriver
 *
 * Please note that the ring buffer implementation of RT-Thread
 * has no thread wait or resume feature.
 */
void ringbuffer_init(struct ringbuffer *rb, uint8_t *pool, int16_t size);
void ringbuffer_reset(struct ringbuffer *rb);
uint32_t ringbuffer_put(struct ringbuffer *rb, const uint8_t *ptr, uint16_t length);
uint32_t ringbuffer_get(struct ringbuffer *rb, uint8_t *ptr, uint16_t length);
uint32_t ringbuffer_peek(struct ringbuffer *rb, uint8_t *ptr, uint16_t length);
uint32_t ringbuffer_data_len(struct ringbuffer *rb);
uint32_t ringbuffer_rem_data_len(struct ringbuffer *rb);


uint32_t ringbuffer_put_force(struct ringbuffer *rb, const uint8_t *ptr, uint16_t length);
uint32_t ringbuffer_putchar(struct ringbuffer *rb, const uint8_t ch);
uint32_t ringbuffer_putchar_force(struct ringbuffer *rb, const uint8_t ch);
uint32_t ringbuffer_peak(struct ringbuffer *rb, uint8_t **ptr);
uint32_t ringbuffer_getchar(struct ringbuffer *rb, uint8_t *ch);


#ifdef RT_USING_HEAP
struct ringbuffer* ringbuffer_create(uint16_t length);
void ringbuffer_destroy(struct ringbuffer *rb);
#endif

static __inline uint16_t ringbuffer_get_size(struct ringbuffer *rb)
{
    assert_param(rb != NULL);
    return rb->buffer_size;
}

/** return the size of empty space in rb */
#define ringbuffer_space_len(rb) ((rb)->buffer_size - ringbuffer_data_len(rb))


#ifdef __cplusplus
}
#endif
