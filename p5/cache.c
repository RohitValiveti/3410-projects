#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "cache.h"
#include "print_helpers.h"

uint clog2(uint n) {
  if (n <= 1) return 0;;
  return 32-__builtin_clz(n-1);
}

cache_t *make_cache(int capacity, int block_size, int assoc, enum protocol_t protocol, bool lru_on_invalidate_f){
  cache_t *cache = malloc(sizeof(cache_t));
  cache->stats = make_cache_stats();
  
  cache->capacity = capacity;      // in Bytes
  cache->block_size = block_size;  // in Bytes
  cache->assoc = assoc;            // 1, 2, 3... etc.

  // first, correctly set these 5 variables. THEY ARE ALL WRONG
  // note: you may find math.h's log2 function useful

  int c = clog2(capacity);
  int b = clog2(block_size);
  int a = clog2(assoc);

  cache->n_cache_line = 1 << (c - b);
  cache->n_set = 1 << (c - b - a);
  cache->n_offset_bit = b;
  cache->n_index_bit = c - b - a;
  cache->n_tag_bit = 32 - cache->n_index_bit - cache->n_offset_bit;

  // next create the cache lines and the array of LRU bits
  // - malloc an array with n_rows
  // - for each element in the array, malloc another array with n_col

  cache->lines = malloc(sizeof(cache_line_t*)*cache->n_set);
  cache->lru_way = malloc(sizeof(int)*cache->n_set);

  // initializes cache tags to 0, dirty bits to false,
  // state to INVALID, and LRU bits to 0
  for (int i = 0; i < cache->n_set; i++) {
    cache->lines[i] = malloc(sizeof(cache_line_t)*cache->assoc);
    cache->lru_way[i] = 0;

    for (int j = 0; j < cache->assoc; j++) {
      cache->lines[i][j].tag = 0;
      cache->lines[i][j].dirty_f = 0;
      cache->lines[i][j].state = INVALID;
    }
  }

  cache->protocol = protocol;
  cache->lru_on_invalidate_f = lru_on_invalidate_f;
  
  return cache;
}

/* Given a configured cache, returns the tag portion of the given address.
 *
 * Example: a cache with 4 bits each in tag, index, offset
 * in binary -- get_cache_tag(0b111101010001) returns 0b1111
 * in decimal -- get_cache_tag(3921) returns 15 
 */
unsigned long get_cache_tag(cache_t *cache, unsigned long addr) {
  return (addr >> (cache->n_index_bit + cache->n_offset_bit)) & ((1 << cache->n_tag_bit) - 1);
}

/* Given a configured cache, returns the index portion of the given address.
 *
 * Example: a cache with 4 bits each in tag, index, offset
 * in binary -- get_cache_index(0b111101010001) returns 0b0101
 * in decimal -- get_cache_index(3921) returns 5
 */
unsigned long get_cache_index(cache_t *cache, unsigned long addr) {
  return (addr >> cache->n_offset_bit) & ((1 << cache->n_index_bit) - 1);
}

/* Given a configured cache, returns the given address with the offset bits zeroed out.
 *
 * Example: a cache with 4 bits each in tag, index, offset
 * in binary -- get_cache_block_addr(0b111101010001) returns 0b111101010000
 * in decimal -- get_cache_block_addr(3921) returns 3920
 */
unsigned long get_cache_block_addr(cache_t *cache, unsigned long addr) {
  return addr & ~((1 << cache->n_offset_bit) - 1);
}


void lru_update(cache_t* cache, int set, int way) {
  cache->lru_way[set] = (way + 1) % cache->assoc;
}

/* this method takes a cache, an address, and an action
 * it proceses the cache access. functionality in no particular order: 
 *   - look up the address in the cache, determine if hit or miss
 *   - update the LRU_way, cacheTags, state, dirty flags if necessary
 *   - update the cache statistics (call update_stats)
 * return true if there was a hit, false if there was a miss
 * Use the "get" helper functions above. They make your life easier.
 */
bool access_cache(cache_t *cache, unsigned long addr, enum action_t action) {
  int set = get_cache_index(cache, addr);
  int tag = get_cache_tag(cache, addr);
  cache_line_t* cl;

  bool hit = 0, wb = 0, upg = 0;

  int way;
  for (way = 0; way < cache->assoc; ++way) {
    if (tag == cache->lines[set][way].tag
      && (cache->lines[set][way].state != INVALID)) {
      goto hit;
    }
  }

// fail
  way = cache->lru_way[set];
hit:
  cl = &cache->lines[set][way];
  log_set(set);
  log_way(way);

  switch (cache->protocol) {
    case NONE:
      switch ((cl->tag != tag) ? INVALID : cl->state) {
        case INVALID:
          switch (action) {
            case LOAD:
            case STORE:
              // Evict old data
              wb = cl->dirty_f && cl->state == VALID;
    
              cl->dirty_f = action == STORE;
              cl->state = VALID;
              cl->tag = tag;
              lru_update(cache, set, way);
              break;
          }
          break;
        case VALID:
          switch (action) {
            case STORE:
              cl->dirty_f = 1;
            case LOAD:
              hit = 1;
              lru_update(cache, set, way);
              break;
          }
          break;
      }
      break;
    case VI:
      switch ((cl->tag != tag) ? INVALID : cl->state) {
        case INVALID:
          switch (action) {
            case LOAD:
            case STORE:
              // Evict old data
              wb = cl->dirty_f && cl->state == VALID;

              cl->dirty_f = action == STORE;
              cl->state = VALID;
              cl->tag = tag;
              lru_update(cache, set, way);
              break;
          }
          break;
        case VALID:
          switch (action) {
            case STORE:
              cl->dirty_f = 1;
            case LOAD:
              hit = 1;
              lru_update(cache, set, way);
              break;
            case LD_MISS:
            case ST_MISS:
              wb = cl->dirty_f;
              cl->state = INVALID;
              hit = 1;
              break;
          }
          break;
      }
      break;
    case MSI:
      switch ((cl->tag != tag) ? INVALID : cl->state) {
        case INVALID:
          switch (action) {
            case STORE:
            case LOAD:
              // Evict old data
              wb = cl->state == MODIFIED;

              cl->state = (action == LOAD) ? SHARED : MODIFIED;
              cl->dirty_f = action == STORE;
              cl->tag = tag;
              lru_update(cache, set, way);
              break;
          }
          break;
        case SHARED:
        case MODIFIED:
          switch (action) {
            case STORE:
              upg = cl->state == SHARED;
              cl->state = MODIFIED;
              cl->dirty_f = 1;
            case LOAD:
              hit = !upg;
              lru_update(cache, set, way);
              break;
            case LD_MISS:
            case ST_MISS:
              wb = cl->state == MODIFIED;
              cl->state = (action == LD_MISS) ? SHARED : INVALID;
              cl->dirty_f = 0;
              hit = 1;
              break;
          }
          break;
      }
      break;
  }

  update_stats(cache->stats, hit, wb, upg, action);
  return hit;
}