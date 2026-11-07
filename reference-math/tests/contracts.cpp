// SPDX-License-Identifier: LicenseRef-EVEngine-Dual
// Adapter code: see LICENSE, LICENSE-OPENSOURCE and LICENSE-COMMERCIAL.
// Vendored numerical sources retain their original licenses.
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include "eve/reference_math.h"
namespace {
bool same(double a, double b) {
    uint64_t x, y;
    std::memcpy(&x, &a, 8);
    std::memcpy(&y, &b, 8);
    return x == y;
}
}  // namespace
int main() {
    const double inf = std::numeric_limits<double>::infinity(), nan = std::numeric_limits<double>::quiet_NaN();
    const bool   tests[] = {same(eve_reference_sin(7.1018323405351049), .73022213828290972),
                            same(eve_reference_atan2(2, 13), .15264932839526518),
                            same(eve_reference_pow(.56835412160712617, 1.3), .47973866018851202),
                            same(eve_reference_pow(-0., 3), -0.),
                            same(eve_reference_pow(-0., 2), 0.),
                            std::isnan(eve_reference_pow(1, nan)),
                            std::isnan(eve_reference_pow(-1, inf)),
                            eve_reference_pow(nan, 0) == 1,
                            eve_reference_pow(-inf, .5) == inf,
                            same(eve_reference_sin(-0.), -0.),
                            same(eve_reference_atan(-0.), -0.),
                            eve_reference_cos(0) == 1,
                            eve_reference_exp(0) == 1,
                            eve_reference_asin(0) == 0,
                            eve_reference_acos(1) == 0,
                            std::isnan(eve_reference_asin(2)),
                            eve_reference_log(1) == 0,
                            eve_reference_log2(8) == 3,
                            eve_reference_cbrt(-8) == -2,
                            same(eve_reference_tan(-0.), -0.),
                            same(eve_reference_cbrt(-0.), -0.),
                            std::isnan(eve_reference_log(-1)),
                            eve_reference_log(0) == -inf,
                            eve_reference_log2(std::numeric_limits<double>::denorm_min()) == -1074};
    for (unsigned i = 0; i < sizeof(tests) / sizeof(*tests); ++i)
        if (!tests[i]) {
            std::fprintf(stderr, "reference math contract %u failed\n", i);
            return 1;
        }
    std::puts("24 reference math API contracts passed");
    return 0;
}
