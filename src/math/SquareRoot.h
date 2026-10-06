// Spec: ISO C 7.12.7.5 sqrt and F.9.4.5; IEEE 754 5.4.1: the square root correctly rounded - an
// integer root guessed in floating point, settled exactly in integers, and rounded by its remainder
// (a square root is never halfway between two doubles).
#ifndef RTS6X_SQUARE_ROOT_H
#define RTS6X_SQUARE_ROOT_H

namespace rts6x {

class SquareRoot {
public:
    // sqrt: -0 is -0, +inf +inf, a NaN itself; below zero a domain error (EDOM, NaN).
    static double root(double x);
};

}  // namespace rts6x

#endif
