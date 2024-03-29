//===--------------------------------------------------------------------------------------------------------------===//
// include/Basic/FixedPoint.h - Fixed point constant handling
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//
//
/// \file
/// Defines the fixed point number interface.
/// This is a class for abstracting various operations performed on fixed point
/// types described in ISO/IEC JTC1 SC22 WG14 N1169 starting at clause 4.
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef LLVM_FLY_BASIC_FIXEDPOINT_H
#define LLVM_FLY_BASIC_FIXEDPOINT_H

#include "llvm/ADT/APSInt.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/raw_ostream.h"

namespace fly {

class ASTContext;
class QualType;

/// The fixed point semantics work similarly to llvm::fltSemantics. The width
/// specifies the whole bit width of the underlying scaled integer (with padding
/// if any). The scale represents the number of fractional bits in this type.
/// When HasUnsignedPadding is true and this type is unsigned, the first bit
/// in the value this represents is treated as padding.
class FixedPointSemantics {
public:
  FixedPointSemantics(unsigned Width, unsigned Scale, bool IsSigned,
                      bool IsSaturated, bool HasUnsignedPadding)
      : Width(Width), Scale(Scale), IsSigned(IsSigned),
        IsSaturated(IsSaturated), HasUnsignedPadding(HasUnsignedPadding) {
    assert(Width >= Scale && "Not enough room for the scale");
    assert(!(IsSigned && HasUnsignedPadding) &&
           "Cannot have unsigned padding on a signed type.");
  }

  unsigned getWidth() const { return Width; }
  unsigned getScale() const { return Scale; }
  bool isSigned() const { return IsSigned; }
  bool isSaturated() const { return IsSaturated; }
  bool hasUnsignedPadding() const { return HasUnsignedPadding; }

  void setSaturated(bool Saturated) { IsSaturated = Saturated; }

  /// Return the number of integral bits represented by these semantics. These
  /// are separate from the fractional bits and do not include the sign or
  /// padding bit.
  unsigned getIntegralBits() const {
    if (IsSigned || (!IsSigned && HasUnsignedPadding))
      return Width - Scale - 1;
    else
      return Width - Scale;
  }

  /// Return the FixedPointSemantics that allows for calculating the full
  /// precision semantic that can precisely represent the precision and ranges
  /// of both input values. This does not compute the resulting semantics for a
  /// given binary operation.
  FixedPointSemantics
  getCommonSemantics(const FixedPointSemantics &Other) const;

  /// Return the FixedPointSemantics for an integer type.
  static FixedPointSemantics GetIntegerSemantics(unsigned Width,
                                                 bool IsSigned) {
    return FixedPointSemantics(Width, /*Scale=*/0, IsSigned,
                               /*IsSaturated=*/false,
                               /*HasUnsignedPadding=*/false);
  }

private:
  unsigned Width          : 16;
  unsigned Scale          : 13;
  unsigned IsSigned       : 1;
  unsigned IsSaturated    : 1;
  unsigned HasUnsignedPadding : 1;
};

/// The APFixedPoint class works similarly to APInt/APSInt in that it is a
/// functional replacement for a scaled integer. It is meant to replicate the
/// fixed point types proposed in ISO/IEC JTC1 SC22 WG14 N1169. The class carries
/// info about the fixed point type's width, sign, scale, and saturation, and
/// provides different operations that would normally be performed on fixed point
/// types.
///
/// Semantically this does not represent any existing C type other than fixed
/// point types and should eventually be moved to LLVM if fixed point types gain
/// native IR support.
class APFixedPoint {
public:
  APFixedPoint(const llvm::APInt &Val, const FixedPointSemantics &Sema)
      : Val(Val, !Sema.isSigned()), Sema(Sema) {
    assert(Val.getBitWidth() == Sema.getWidth() &&
           "The value should have a bit width that matches the Sema width");
  }

  APFixedPoint(uint64_t Val, const FixedPointSemantics &Sema)
      : APFixedPoint(llvm::APInt(Sema.getWidth(), Val, Sema.isSigned()),
                     Sema) {}

  // Zero initialization.
  APFixedPoint(const FixedPointSemantics &Sema) : APFixedPoint(0, Sema) {}

  llvm::APSInt getValue() const { return llvm::APSInt(Val, !Sema.isSigned()); }
  inline unsigned getWidth() const { return Sema.getWidth(); }
  inline unsigned getScale() const { return Sema.getScale(); }
  inline bool isSaturated() const { return Sema.isSaturated(); }
  inline bool isSigned() const { return Sema.isSigned(); }
  inline bool hasPadding() const { return Sema.hasUnsignedPadding(); }
  FixedPointSemantics getSemantics() const { return Sema; }

  bool getBoolValue() const { return Val.getBoolValue(); }

  // Convert this number to match the semantics provided. If the overflow
  // parameter is provided, set this value to true or false to indicate if this
  // operation results in an overflow.
  APFixedPoint convert(const FixedPointSemantics &DstSema,
                       bool *Overflow = nullptr) const;

  // Perform binary operations on a fixed point type. The resulting fixed point
  // value will be in the common, full precision semantics that can represent
  // the precision and ranges of both input values. See convert() for an
  // explanation of the Overflow parameter.
  APFixedPoint add(const APFixedPoint &Other, bool *Overflow = nullptr) const;
  APFixedPoint sub(const APFixedPoint &Other, bool *Overflow = nullptr) const;
  APFixedPoint mul(const APFixedPoint &Other, bool *Overflow = nullptr) const;
  APFixedPoint div(const APFixedPoint &Other, bool *Overflow = nullptr) const;

  /// Perform a unary negation (-X) on this fixed point type, taking into
  /// account saturation if applicable.
  APFixedPoint negate(bool *Overflow = nullptr) const;

  APFixedPoint shr(unsigned Amt) const {
    return APFixedPoint(Val >> Amt, Sema);
  }

  APFixedPoint shl(unsigned Amt) const {
    return APFixedPoint(Val << Amt, Sema);
  }

  /// Return the integral part of this fixed point number, rounded towards
  /// zero. (-2.5k -> -2)
  llvm::APSInt getIntPart() const {
    if (Val < 0 && Val != -Val) // Cover the case when we have the min val
      return -(-Val >> getScale());
    else
      return Val >> getScale();
  }

  /// Return the integral part of this fixed point number, rounded towards
  /// zero. The value is stored into an APSInt with the provided width and sign.
  /// If the overflow parameter is provided, and the integral value is not able
  /// to be fully stored in the provided width and sign, the overflow parameter
  /// is set to true.
  ///
  /// If the overflow parameter is provided, set this value to true or false to
  /// indicate if this operation results in an overflow.
  llvm::APSInt convertToInt(unsigned DstWidth, bool DstSign,
                            bool *Overflow = nullptr) const;

  void toString(llvm::SmallVectorImpl<char> &Str) const;
  std::string toString() const {
    llvm::SmallString<40> S;
    toString(S);
    return std::string(S.str());
  }

  // If LHS > RHS, return 1. If LHS == RHS, return 0. If LHS < RHS, return -1.
  int compare(const APFixedPoint &Other) const;
  bool operator==(const APFixedPoint &Other) const {
    return compare(Other) == 0;
  }
  bool operator!=(const APFixedPoint &Other) const {
    return compare(Other) != 0;
  }
  bool operator>(const APFixedPoint &Other) const { return compare(Other) > 0; }
  bool operator<(const APFixedPoint &Other) const { return compare(Other) < 0; }
  bool operator>=(const APFixedPoint &Other) const {
    return compare(Other) >= 0;
  }
  bool operator<=(const APFixedPoint &Other) const {
    return compare(Other) <= 0;
  }

  static APFixedPoint getMax(const FixedPointSemantics &Sema);
  static APFixedPoint getMin(const FixedPointSemantics &Sema);

  /// Create an APFixedPoint with a value equal to that of the provided integer,
  /// and in the same semantics as the provided target semantics. If the value
  /// is not able to fit in the specified fixed point semantics, and the
  /// overflow parameter is provided, it is set to true.
  static APFixedPoint getFromIntValue(const llvm::APSInt &Value,
                                      const FixedPointSemantics &DstFXSema,
                                      bool *Overflow = nullptr);

private:
  llvm::APSInt Val;
  FixedPointSemantics Sema;
};

inline llvm::raw_ostream &operator<<(llvm::raw_ostream &OS,
                                     const APFixedPoint &FX) {
  OS << FX.toString();
  return OS;
}

}  // namespace fly

#endif
