// SPDX-License-Identifier: LicenseRef-EVEngine-Dual
// Adapter code: see LICENSE, LICENSE-OPENSOURCE and LICENSE-COMMERCIAL.
// Vendored numerical sources retain their original licenses.
#ifndef EVE_REFERENCE_MATH_H
#define EVE_REFERENCE_MATH_H

/** @file
 * @brief Stateless binary64 math used by procedural PixelFX.
 * @note Value-only inputs/results; no allocation, retained storage, locks or callbacks.
 * Calls may run concurrently. The caller supplies the normal FE_TONEAREST environment;
 * these functions do not change its rounding mode. Ordinary IEEE exception flags may change.
 * NaN/Infinity are numeric results, not a recoverable operation status. No C++ exceptions escape.
 * The pinned V8 functions reproduce the selected source implementation, not universally
 * correctly-rounded transcendental values. pow uses CORE-MATH with ECMAScript special cases;
 * selected source Node system pow can differ by an ulp. See README.md and UPSTREAM.json.
 */
#ifdef __cplusplus
extern "C" {
#endif
/** @brief Compute sine in radians. @param x Binary64 argument. @return Sine or IEEE NaN. */
double eve_reference_sin(double x);
/** @brief Compute cosine in radians. @param x Binary64 argument. @return Cosine or IEEE NaN. */
double eve_reference_cos(double x);
/** @brief Compute principal arctangent. @param x Binary64 argument. @return Angle in radians. */
double eve_reference_atan(double x);
/** @brief Compute quadrant-aware arctangent. @param y Numerator. @param x Denominator. @return Angle in radians. */
double eve_reference_atan2(double y, double x);
/** @brief Compute principal arcsine. @param x Binary64 argument. @return Angle in radians or IEEE NaN. */
double eve_reference_asin(double x);
/** @brief Compute principal arccosine. @param x Binary64 argument. @return Angle in radians or IEEE NaN. */
double eve_reference_acos(double x);
/** @brief Compute natural exponential. @param x Binary64 argument. @return Exponential or IEEE NaN/Infinity. */
double eve_reference_exp(double x);
/** @brief Compute natural logarithm. @param x Binary64 argument. @return Logarithm or IEEE NaN/Infinity. */
double eve_reference_log(double x);
/** @brief Compute base-two logarithm. @param x Binary64 argument. @return Logarithm or IEEE NaN/Infinity. */
double eve_reference_log2(double x);
/** @brief Compute tangent in radians. @param x Binary64 argument. @return Tangent or IEEE NaN. */
double eve_reference_tan(double x);
/** @brief Compute real cube root. @param x Binary64 argument. @return Cube root preserving signed zero. */
double eve_reference_cbrt(double x);
/** @brief Compute ECMAScript-compatible power with canonical CORE-MATH rounding.
 * @param x Base. @param y Exponent. @return Power, preserving signed zero, or IEEE NaN/Infinity.
 * @note NaN exponents and infinite exponents of bases +/-1 produce NaN, as required by ECMAScript.
 */
double eve_reference_pow(double x, double y);
#ifdef __cplusplus
}
#endif
#endif
