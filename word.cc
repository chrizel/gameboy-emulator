#include "word.h"

#include <iomanip>
#include <iostream>

std::ostream & operator<<(std::ostream &cout, byte b)
{
    cout << std::hex << std::setfill('0') << std::setw(2) << (int)b;
    return cout;
}

std::ostream & operator<<(std::ostream &cout, word w)
{
    cout << std::hex << std::setfill('0') << std::setw(4) << w.value();
    return cout;
}
