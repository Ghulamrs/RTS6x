// Spec: IEEE 754 binary64; Newton's method for 1/d (y <- y(2 - d y)) and for 1/sqrt(w) (y <- y(3 -
// w y^2)/2), each from a polynomial first guess on the significand alone and four steps, so with
// multiplications only: the C674x divides in software, and these need no division at all.
#ifndef RTS6X_NEWTON_ITERATION_H
#define RTS6X_NEWTON_ITERATION_H

namespace rts6x {

class NewtonIteration {
public:
    // 1/d to within about 2 ulps, for 2^-1020 < |d| < 2^1020.
    static double reciprocal(double d);
    // 1/sqrt(w) to within about 2 ulps, for 2^-1020 < w < 2^1020.
    static double inverseSquareRoot(double w);
};

}  // namespace rts6x

#endif
