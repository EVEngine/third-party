// SPDX-License-Identifier: LicenseRef-EVEngine-Dual
// Adapter code: see LICENSE, LICENSE-OPENSOURCE and LICENSE-COMMERCIAL.
// Vendored numerical sources retain their original licenses.
#include "eve/reference_math.h"
#include <cmath>
#include <limits>
#if defined(V8_USE_LIBM_TRIG_FUNCTIONS) || defined(V8_GN_HEADER)
#error "Reference math requires the pinned standalone V8 numerical implementation"
#endif
#ifdef __FAST_MATH__
#error "Reference math requires IEEE binary64 operations without fast-math"
#endif
#include "src/base/ieee754.h"
extern "C" double cr_pow(double, double);
extern "C" double eve_reference_sin(double x) { return v8::base::ieee754::sin(x); }
extern "C" double eve_reference_cos(double x) { return v8::base::ieee754::cos(x); }
extern "C" double eve_reference_atan(double x) { return v8::base::ieee754::atan(x); }
extern "C" double eve_reference_atan2(double y, double x) { return v8::base::ieee754::atan2(y, x); }
extern "C" double eve_reference_asin(double x) { return v8::base::ieee754::asin(x); }
extern "C" double eve_reference_acos(double x) { return v8::base::ieee754::acos(x); }
extern "C" double eve_reference_exp(double x) { return v8::base::ieee754::exp(x); }
extern "C" double eve_reference_log(double x) { return v8::base::ieee754::log(x); }
extern "C" double eve_reference_log2(double x) { return v8::base::ieee754::log2(x); }
extern "C" double eve_reference_tan(double x) { return v8::base::ieee754::tan(x); }
extern "C" double eve_reference_cbrt(double x) { return v8::base::ieee754::cbrt(x); }
extern "C" double eve_reference_pow(double x, double y) {
    if (std::isnan(y) || (std::isinf(y) && std::abs(x) == 1)) return std::numeric_limits<double>::quiet_NaN();
    return cr_pow(x, y);
}
