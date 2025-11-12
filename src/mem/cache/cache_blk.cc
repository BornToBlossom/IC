/*
 * Copyright (c) 2012-2013 ARM Limited
 * All rights reserved
 *
 * The license below extends only to copyright in the software and shall
 * not be construed as granting a license to any other intellectual
 * property including but not limited to intellectual property relating
 * to a hardware implementation of the functionality of the software
 * licensed hereunder.  You may use the software subject to the license
 * terms below provided that you ensure that this notice is replicated
 * unmodified and in its entirety in all distributions of the software,
 * modified or unmodified, in source code or in binary form.
 *
 * Copyright (c) 2007 The Regents of The University of Michigan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "mem/cache/cache_blk.hh"
#include <algorithm>
#include <cstdint>  
#include "base/cprintf.hh"

// --- Implement clearTouchBitmap ---
void
CacheBlk::clearTouchBitmap()
{
    if (!touch_bitmap.empty()) {
        std::fill(touch_bitmap.begin(), touch_bitmap.end(), 0ULL);
    }
    bytes_read_total = 0;
    bytes_written_total = 0;
    unique_bytes_read = 0;
    unique_bytes_written = 0;
}

// --- Implement markBytesTouched ---
// Marks byte range [offset, offset+len) in touch_bitmap.
// Returns the number of bytes newly marked in this call.
unsigned
CacheBlk::markBytesTouched(unsigned offset, unsigned len)
{
    if (len == 0 || touch_bitmap.empty())
        return 0;

    unsigned blkBytes = touch_bitmap.size() * 64;
    if (offset >= blkBytes)
        return 0;

    unsigned end = offset + len;
    if (end > blkBytes)
        end = blkBytes;

    unsigned wstart = offset >> 6;       // offset / 64
    unsigned wend = (end - 1) >> 6;      // inclusive

    unsigned newly_set = 0;

    for (unsigned w = wstart; w <= wend; ++w) {
        unsigned word_base = w << 6; // w * 64
        unsigned a = std::max<unsigned>(offset, word_base);
        unsigned b = std::min<unsigned>(end, word_base + 64);
        unsigned off = a - word_base;
        unsigned n = b - a; // number of bytes in this word to set

        uint64_t mask;
        if (n == 0) {
            continue;
        } else if (off == 0 && n == 64) {
            mask = ~0ULL;
        } else {
            // safe guard for n >= 64 (shouldn't happen due to above)
            mask = ((n >= 64) ? ~0ULL : ((1ULL << n) - 1ULL)) << off;
        }

        uint64_t old = touch_bitmap[w];
        uint64_t nw = old | mask;
        uint64_t newly = nw & ~old; // bits that were 0 and are now 1
        if (newly) {
            newly_set += __builtin_popcountll(newly);
            touch_bitmap[w] = nw;
        }
    }

    return newly_set;
}

// --- Implement computeBitmapPopCount ---
unsigned
CacheBlk::computeBitmapPopCount() const
{
    if (touch_bitmap.empty()) return 0;
    unsigned total = 0;
    for (uint64_t w : touch_bitmap)
        total += __builtin_popcountll(w);
    return total;
}
void
CacheBlk::initTouchBitmap(unsigned blkSize)
{
    // number of 64-bit words needed to cover blkSize bytes
    unsigned words = (blkSize + 63) / 64;
    touch_bitmap.assign(words, 0ULL);

    // reset counters
    bytes_read_total = 0;
    bytes_written_total = 0;
    unique_bytes_read = 0;
    unique_bytes_written = 0;
}



void
CacheBlk::insert(const Addr tag, const bool is_secure,
                 const int src_master_ID, const uint32_t task_ID)
{
    // Set block tag
    this->tag = tag;

    // Set source requestor ID
    srcMasterId = src_master_ID;

    // Set task ID
    task_id = task_ID;

    // Set insertion tick as current tick
    tickInserted = curTick();

    // Insertion counts as a reference to the block
    refCount = 1;

    // Set secure state
    if (is_secure) {
        status = BlkSecure;
    } else {
        status = 0;
    }
}

void
CacheBlkPrintWrapper::print(std::ostream &os, int verbosity,
                            const std::string &prefix) const
{
    ccprintf(os, "%sblk %c%c%c%c\n", prefix,
             blk->isValid()    ? 'V' : '-',
             blk->isWritable() ? 'E' : '-',
             blk->isDirty()    ? 'M' : '-',
             blk->isSecure()   ? 'S' : '-');
}

