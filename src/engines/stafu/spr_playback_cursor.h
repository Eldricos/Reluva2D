// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Record-local cursor over indexed SPR events. No timing, rendering or decoding.
#include "engines/stafu/spr_sequence_index.h"
#include <cstddef>

namespace reluva::stafu {
class SprPlaybackCursor {
public:
    // The source index must outlive this cursor and must not be rebuilt while bound.
    // bind() fails without changing the current position when the record is invalid.
    bool bind(const SprSequenceIndex& index,size_t recordNumber) {
        if(recordNumber>=index.records().size())return false;
        record_=&index.records()[recordNumber];
        position_=0;
        return true;
    }
    void unbind() {record_=nullptr;position_=0;}
    void restart() {position_=0;}
    bool bound() const {return record_!=nullptr;}
    bool atEnd() const {return !record_||position_>=record_->events.size();}
    size_t position() const {return position_;}
    size_t size() const {return record_?record_->events.size():0;}
    // Non-owning pointer to the current event; null at end or when unbound.
    const SprSequenceIndex::Event* current() const {
        return atEnd()?nullptr:&record_->events[position_];
    }
    // Advances by one event. Returns false at end, leaving position unchanged.
    bool advance() {
        if(atEnd())return false;
        ++position_;
        return true;
    }
private:
    const SprSequenceIndex::Record* record_=nullptr;
    size_t position_=0;
};
}
