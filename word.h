#ifndef WORD_H
#define WORD_H

#include <stdint.h>

typedef uint8_t byte;

struct word {
public:
    union {
        uint16_t w;
        struct {
            byte lo;
            byte hi;
        } b;
    } d;

    word(uint16_t v = 0) { d.w = v; };
    word(byte lo, byte hi) { d.b.lo = lo; d.b.hi = hi; };

    inline byte & lo() { return d.b.lo; };
    inline byte & hi() { return d.b.hi; };
    inline uint16_t & value() { return d.w; };

    inline byte lo() const { return d.b.lo; };
    inline byte hi() const { return d.b.hi; };
    inline uint16_t value() const { return d.w; };

    operator uint16_t() const {
        return d.w;
    };

    word operator+(const word &w) {
        return word(d.w + w.d.w);
    };

    word operator+=(const word &w) {
        d.w += w.d.w;
        return *this;
    };

    word operator+=(const byte &b) {
        d.b.lo += b;
        return *this;
    };

    word operator+(const byte &b) {
        return word(d.b.lo + b, d.b.hi);
    };

    word operator++(int) {
        return word(d.w++);
    };

    word operator--(int) {
        return word(d.w--);
    };
};

#endif