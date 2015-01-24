#include "io2d.h"
#include "xio2dhelpers.h"
#include "xcairoenumhelpers.h"
#include <cmath>

using namespace std;
using namespace std::experimental::io2d;

matrix_2d::matrix_2d()
	: _M00{ 1.0 }
	, _M01{ 0.0 }
	, _M10{ 0.0 }
	, _M11{ 1.0 }
	, _M20{ 0.0 }
	, _M21{ 0.0 } {
}

matrix_2d::matrix_2d(matrix_2d&& other)
	: _M00(move(other._M00))
	, _M01(move(other._M01))
	, _M10(move(other._M10))
	, _M11(move(other._M11))
	, _M20(move(other._M20))
	, _M21(move(other._M21)) {
	other._M00 = 0.0;
	other._M01 = 0.0;
	other._M10 = 0.0;
	other._M11 = 0.0;
	other._M20 = 0.0;
	other._M21 = 0.0;
}

matrix_2d& matrix_2d::operator=(matrix_2d&&other) {
	if (this != &other) {
		_M00 = move(other._M00);
		_M01 = move(other._M01);
		_M10 = move(other._M10);
		_M11 = move(other._M11);
		_M20 = move(other._M20);
		_M21 = move(other._M21);

		other._M00 = 0.0;
		other._M01 = 0.0;
		other._M10 = 0.0;
		other._M11 = 0.0;
		other._M20 = 0.0;
		other._M21 = 0.0;
	}
	return *this;
}

matrix_2d::matrix_2d(double m00, double m01, double m10, double m11, double m20, double m21)
	: _M00{ m00 }
	, _M01{ m01 }
	, _M10{ m10 }
	, _M11{ m11 }
	, _M20{ m20 }
	, _M21{ m21 } {
}

matrix_2d matrix_2d::init_identity() {
	return{ 1.0, 0.0, 0.0, 1.0, 0.0, 0.0 };
}

matrix_2d matrix_2d::init_translate(const point& value) {
	return{ 1.0, 0.0, 0.0, 1.0, value.x(), value.y() };
}

matrix_2d matrix_2d::init_scale(const point& value) {
	return{ value.x(), 0.0, 0.0, value.y(), 0.0, 0.0 };
}

matrix_2d matrix_2d::init_rotate(double radians) {
	auto sine = sin(radians);
	auto cosine = cos(radians);
	return{ cosine, sine, -sine, cosine, 0.0, 0.0 };
}

matrix_2d matrix_2d::init_shear_x(double factor) {
	return{ 1.0, 0.0, factor, 1.0, 0.0, 0.0 };
}

matrix_2d matrix_2d::init_shear_y(double factor) {
	return{ 1.0, factor, 0.0, 1.0, 0.0, 0.0 };
}

matrix_2d& matrix_2d::translate(const point& value) {
	*this = init_translate(value) * (*this);
	return *this;
}

matrix_2d& matrix_2d::scale(const point& value) {
	*this = init_scale(value) * (*this);
	return *this;
}

matrix_2d& matrix_2d::rotate(double radians) {
	*this = init_rotate(radians) * (*this);
	return *this;
}

matrix_2d& matrix_2d::shear_x(double factor) {
	*this = init_shear_x(factor) * (*this);
	return *this;
}

matrix_2d& matrix_2d::shear_y(double factor) {
	*this = init_shear_y(factor) * (*this);
	return *this;
}

matrix_2d& matrix_2d::invert() {
	// Get the determinant for this matrix_2d.
	auto det = determinant();
	if (_Almost_equal_relative(det, 0.0)) {
		// A matrix_2d with a determinant of 0.0 is not invertible.
		_Throw_if_failed_cairo_status_t(CAIRO_STATUS_INVALID_MATRIX);
	}

	double
		adjugateM00, adjugateM01,
		adjugateM10, adjugateM11,
		adjugateM20, adjugateM21;

	// Calculates the determinant of a 2x2 row-major matrix.
	auto determinant2x2 = [](double dm00, double dm01, double dm10, double dm11) { return dm00 * dm11 + dm01 * dm10; };

	// The values for the third column of this matrix_2d.
	const double cM02 = 0.0, cM12 = 0.0, cM22 = 1.0;
	adjugateM00 = determinant2x2(_M11, cM12, _M21, cM22);
	adjugateM01 = -determinant2x2(_M01, cM02, _M20, cM22);
	adjugateM10 = -determinant2x2(_M10, cM12, _M20, cM22);
	adjugateM11 = determinant2x2(_M00, cM02, _M20, cM22);
	adjugateM20 = determinant2x2(_M10, _M11, _M20, _M21);
	adjugateM21 = -determinant2x2(_M00, _M01, _M20, _M21);
	auto inverseDeterminant = 1.0 / det;
	assert("inverse M02 should equal 0.0 " && _Almost_equal_relative(inverseDeterminant * determinant2x2(_M01, cM02, _M11, cM12), 0.0));
	assert("inverse M12 should equal 0.0 " && _Almost_equal_relative(inverseDeterminant * -determinant2x2(_M00, cM02, _M10, cM12), 0.0));
	assert("inverse M22 should equal 1.0 " && _Almost_equal_relative(inverseDeterminant * determinant2x2(_M00, _M01, _M10, _M11), 1.0));
	_M00 = inverseDeterminant * adjugateM00;
	_M01 = inverseDeterminant * adjugateM01;
	_M10 = inverseDeterminant * adjugateM10;
	_M11 = inverseDeterminant * adjugateM11;
	_M20 = inverseDeterminant * adjugateM20;
	_M21 = inverseDeterminant * adjugateM21;

	return *this;
}

double matrix_2d::determinant() const {
	if (isnan(_M20) || isnan(_M21)) {
		_Throw_if_failed_cairo_status_t(CAIRO_STATUS_INVALID_MATRIX);
	}
	return _M00 * _M11 - _M01 * _M10;
}

point matrix_2d::transform_distance(const point& dist) const {
	return{ _M00 * dist.x() + _M10 * dist.y(), _M01 * dist.x() + _M11 * dist.y() };
}

point matrix_2d::transform_point(const point& pt) const {
	return transform_distance(pt) + point{ _M20, _M21 };
}

void matrix_2d::m00(double value) {
	_M00 = value;
}

void matrix_2d::m01(double value) {
	_M01 = value;
}

void matrix_2d::m10(double value) {
	_M10 = value;
}

void matrix_2d::m11(double value) {
	_M11 = value;
}

void matrix_2d::m20(double value) {
	_M20 = value;
}

void matrix_2d::m21(double value) {
	_M21 = value;
}

double matrix_2d::m00() const {
	return _M00;
}

double matrix_2d::m01() const {
	return _M01;
}

double matrix_2d::m10() const {
	return _M10;
}

double matrix_2d::m11() const {
	return _M11;
}

double matrix_2d::m20() const {
	return _M20;
}

double matrix_2d::m21() const {
	return _M21;
}

namespace std {
	namespace experimental {
		namespace io2d {
#if _Inline_namespace_conditional_support_test
			inline namespace v1 {
#endif
				matrix_2d operator*(const matrix_2d& lhs, const matrix_2d& rhs) {
					return matrix_2d{
						(lhs.m00() * rhs.m00()) + (lhs.m01() * rhs.m10()),
						(lhs.m00() * rhs.m01()) + (lhs.m01() * rhs.m11()),
						(lhs.m10() * rhs.m00()) + (lhs.m11() * rhs.m10()),
						(lhs.m10() * rhs.m01()) + (lhs.m11() * rhs.m11()),
						(lhs.m20() * rhs.m00()) + (lhs.m21() * rhs.m10()) + lhs.m20(),
						(lhs.m20() * rhs.m01()) + (lhs.m21() * rhs.m11()) + lhs.m21()
					};
				}

				matrix_2d& operator*=(matrix_2d& lhs, const matrix_2d& rhs) {
					lhs = lhs * rhs;
					return lhs;
				}

				bool operator==(const matrix_2d& lhs, const matrix_2d& rhs) {
					return
						lhs.m00() == rhs.m00() &&
						lhs.m01() == rhs.m01() &&
						lhs.m10() == rhs.m10() &&
						lhs.m11() == rhs.m11() &&
						lhs.m20() == rhs.m20() &&
						lhs.m21() == rhs.m21();
				}

				bool operator!=(const matrix_2d& lhs, const matrix_2d& rhs) {
					return !(lhs == rhs);
				}
#if _Inline_namespace_conditional_support_test
			}
#endif
		}
	}
}