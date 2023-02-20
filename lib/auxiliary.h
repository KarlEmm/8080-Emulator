//
// Created by KarlE on 2/13/2023.
//

#ifndef CPU8080_AUXILIARY_H
#define CPU8080_AUXILIARY_H

template <typename T>
constexpr int popcount(T x)
{
    int count = 0;
    while (x > 0)
    {
        count += x & 1;
        x >>= 1;
    }
    return count;
}

#endif //CPU8080_AUXILIARY_H
