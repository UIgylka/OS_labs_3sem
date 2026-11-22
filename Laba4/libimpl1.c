#include <math.h>
#include "contracts.h"

float sin_integral(float a, float b, float e) {
    float sum = 0.0f;
    for (float x = a; x < b; x += e) {
        float step = (x + e > b) ? (b - x) : e;
        sum += sinf(x) * step;
    }
    return sum;
}

int gcd(int a, int b) {
    while (b != 0) {
        int t = b;
        b = a % b;
        a = t;
    }
    return a;
}