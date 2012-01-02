#ifndef WORD_H
#define WORD_H

#include <stdint.h>
#include <iostream>

typedef uint8_t byte;
typedef int8_t signed_byte;

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

    inline void setlo(byte v) { d.b.lo = v; };
    inline void sethi(byte v) { d.b.hi = v; };

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

    void addSignedByte(const byte &b) {
        d.w = d.w + (int8_t)b;
    };

    word operator++(int) {
        return word(d.w++);
    };

    word operator--(int) {
        return word(d.w--);
    };
};

std::ostream & operator<<(std::ostream &cout, byte b);
std::ostream & operator<<(std::ostream &cout, word w);

#endif
