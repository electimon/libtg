//
//  cmn.c
//  mtx
//
//  Created by Pavel Morozkin on 18.01.14.
//  Copyright (c) 2014 Pavel Morozkin. All rights reserved.
//

#include "fact.h"
#include "rsa.h"
#include "buf.h"

void tl_cmn_fact(uint64_t pq, uint32_t * p, uint32_t * q)
{
  factor(pq, p, q);
}

buf_t cmn_pow_mod(buf_t g, buf_t e, buf_t m)
{
  if (e.size != m.size || e.size != 256 || e.size != 256) {
    printf("can't pow_mod\n");
  }

  buf_t r;

  int l = tl_pow_mod(r.data, g.data, g.size, e.data, e.size, m.data, m.size);

  if (!l) {
    printf("pow_mod failed\n");
  }

  r.size = l;

  return r;
}
