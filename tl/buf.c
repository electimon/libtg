//
//  buf.c
//  mtx
//
//  Created by Pavel Morozkin on 17.01.14.
//  Copyright (c) 2014 Pavel Morozkin. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "buf.h"

void log_hex(unsigned char * a, uint32_t s)
{
  int m = 16;
  int b = 8;
  int f = 0;

  fprintf(stderr, "size : %d\n", s);

  for (uint32_t i = 0; i < s; i++) {
    if (!i) {
      fprintf(stderr, "%.4x | ", i), f = 1;
    }

    if (!(i % m) && i) {
      fprintf(stderr, "\n%.4x | ", i), f = 1;
    }

    if (!(i % b) && i && !f) {
      fprintf(stderr, " ");
    }

    fprintf(stderr, "%.2x ", a[i]);
    f = 0;
  }

  fprintf(stderr, "\n");
}
void buf_init(buf_t *buf)
{
	buf->aptr = malloc(BUFSIZ + 1); 
	buf->asize = BUFSIZ;
	buf->size = 0;
	buf->data = buf->aptr;
}

void buf_realloc(buf_t *buf, uint32_t size)
{
	if (size > buf->asize){
		void *ptr = realloc(buf->data, size);
		if (ptr){
			buf->aptr = ptr;
			buf->asize = size;
			buf->data = buf->data;
		}
	}
}

buf_t buf_add(uint8_t *data, uint32_t size)
{
  buf_t b;
	buf_init(&b);

  if (size > b.asize) {
		buf_realloc(&b, size + 1);
  }

	uint32_t i;
  for (i = 0; i < size; ++i) {
    b.data[i] = data[i];
  }
	b.data[i] = 0;

  b.size = size;

  return b;
}

buf_t buf_cat(buf_t dest, buf_t src)
{
  uint32_t s = dest.size + src.size;

  if (s > dest.asize) {
		buf_realloc(&dest, s);
  }

  int offset = dest.size;

  for (uint32_t i = 0; i < src.size; ++i) {
    dest.data[i + offset] = src.data[i];
  }

  dest.size = s;

  return dest;
}

void buf_dump(buf_t b)
{
  if (b.size > b.asize) {
    printf("Error: buf_dump: max size reached\n");
  } else if (!b.size) {
    printf("buffer is empty\n");
  }

  log_hex(b.data, b.size);
}

uint8_t buf_cmp(buf_t a, buf_t b)
{
  if (a.size != b.size) {
    printf("Error: buf_cmp: different sizes\n");
  }

  for (uint32_t i = 0; i < a.size; ++i) {
    if (a.data[i] != b.data[i]) {
      return 0;
    }
  }

  return 1;
}

buf_t buf_swap(buf_t b)
{
  unsigned char * lo = (unsigned char *)b.data;
  unsigned char * hi = (unsigned char *)b.data + b.size - 1;
  unsigned char swap;

  while (lo < hi) {
    swap = *lo;
    *lo++ = *hi;
    *hi-- = swap;
  }

  return b;
}

buf_t buf_add_ui32(uint32_t v)
{
  return buf_add((uint8_t *)&v, 4);
}

buf_t buf_add_ui64(uint64_t v)
{
  return buf_add((uint8_t *)&v, 8);
}

uint32_t buf_get_ui32(buf_t b)
{
  return *(uint32_t *)b.data;
}

uint64_t buf_get_ui64(buf_t b)
{
  return *(uint64_t *)b.data;
}

buf_t buf_rand(uint32_t s)
{
  buf_t b = {};
	buf_init(&b);

  srand((unsigned int)time(NULL));

  for (uint32_t i = 0; i < s; i++) {
    b.data[i] = rand() % 256;
  }

  b.size = s;

  return b;
}

buf_t buf_xor(buf_t a, buf_t b)
{
  if (a.size != b.size) {
    printf("Error: buf_cmp: different sizes\n");
  }

  buf_t r;
	buf_init(&r);
  if (a.size > r.asize) {
		buf_realloc(&r, a.size);
  }

  for (uint32_t i = 0; i < a.size; ++i) {
    r.data[i] = a.data[i] ^ b.data[i];
  }

  r.size = a.size;
  
  return r;
}

buf_t buf_cat_ui32(buf_t dest, uint32_t i)
{
	buf_t src = buf_add_ui32(i);
	buf_t buf = buf_cat(dest, src);
	free(src.aptr);
	return buf; 
}

buf_t buf_cat_ui64(buf_t dest, uint64_t i){
	buf_t src = buf_add_ui64(i);
	buf_t buf = buf_cat(dest, src);
	free(src.aptr);
	return buf; 
}

buf_t buf_cat_data(buf_t dest, uint8_t *data, uint32_t len){
	buf_t src = buf_add(data, len);
	buf_t buf = buf_cat(dest, src);
	free(src.aptr);
	return buf; 
}

void buf_free(buf_t b){
	if (b.aptr)
		free(b.aptr);
	b.aptr = NULL;
}
