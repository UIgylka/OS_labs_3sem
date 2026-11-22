#include <math.h>
#include "contracts.h"

float sin_integral(float a, float b, float e) {
    float sum = 0.0f;
    for (float x = a; x < b; x += e) {
        float step = (x + e > b) ? (b - x) : e;
        sum += ((sinf(x) + sinf(x + step)) / 2.0f) * step;
    }
    return sum;
}

int gcd(int a, int b) {
    int mn = (a < b) ? a : b;
    int result = 1;
    for (int i = 1; i <= mn; i++) {
        if (a % i == 0 && b % i == 0) result = i;
    }
    return result;
}