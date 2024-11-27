/**
 * File              : crc.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 25.11.2024
 * Last Modified Date: 25.11.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */
#ifndef TG_CRC_H
#define TG_CRC_H

#include "../libtg.h"

extern uint32_t tg_crc32_(uint32_t crc, const void * buf, size_t size);
extern buf_t tg_crc_crc32(const buf_t);

#endif /* defined(TG_CRC_H) */
