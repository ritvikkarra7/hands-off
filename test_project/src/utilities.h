#include <esp32-hal-log.h>

double expmap(double x, double in_min, double in_max, double out_min, double out_max) {

    double prefactor = log10(out_max - out_min + 1)/ (in_max);
    double exponent = (x - in_min) * prefactor;
    return out_min + (pow(10, exponent) - 1);

}