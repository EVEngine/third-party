// SPDX-License-Identifier: LicenseRef-EVEngine-Dual
// Consumer validation follows the package license; vendor sources are not included.
#include <math.h>
#include "eve/reference_math.h"

int main(void) {
    const double negative_zero = eve_reference_pow(-0.0, 3.0);
    if (!signbit(negative_zero) || negative_zero != 0.0) return 1;
    if (eve_reference_log2(8.0) != 3.0) return 2;
    if (eve_reference_cbrt(-8.0) != -2.0) return 3;
    if (eve_reference_cos(0.0) != 1.0) return 4;
    if (!isnan(eve_reference_asin(2.0))) return 5;
    return 0;
}
