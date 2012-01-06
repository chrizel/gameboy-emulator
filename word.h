#ifndef WORD_H
#define WORD_H

#include <stdint.h>
#include <iostream>

typedef uint8_t byte;
typedef int8_t signed_byte;
typedef uint16_t word_t;

struct word {
private:
    union {
        word_t w;
        struct {
            byte lo;
            byte hi;
        } b;
    } d;

public:
    word(const word_t v = 0) { d.w = v; };
    word(const byte &lo, const byte &hi) { d.b.lo = lo; d.b.hi = hi; };

    inline byte & loRef() { return d.b.lo; };
    inline byte & hiRef() { return d.b.hi; };

    inline byte lo() const { return d.b.lo; };
    inline byte hi() const { return d.b.hi; };
    inline word_t value() const { return d.w; };

    inline void setlo(byte v) { d.b.lo = v; };
    inline void sethi(byte v) { d.b.hi = v; };

    word operator+(const word &w) const { return word(d.w + w.d.w); };
    word operator-(const word &w) const { return word(d.w - w.d.w); };
    bool operator<(const word &w) const { return d.w < w.d.w; };
    bool operator>=(const word &w) const { return d.w >= w.d.w; };
    bool operator<=(const word &w) const { return d.w <= w.d.w; };
    bool operator==(const word &w) const { return d.w == w.d.w; };
    word operator+=(const word &w) { d.w += w.d.w; return *this; };
    word operator++(int) { return word(d.w++); };
    word operator--(int) { return word(d.w--); };

    void addSignedByte(const byte &b) { d.w = d.w + (int8_t)b; };
};

std::ostream & operator<<(std::ostream &cout, byte b);
std::ostream & operator<<(std::ostream &cout, word w);

#endif
