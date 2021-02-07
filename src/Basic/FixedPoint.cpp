//===- FixedPoint.cpp - Fixed point constant handling -----------*- C++ -*-===//
//
// Part of the Fly Project, under the Apache License v2.0
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Defines the implementation for the fixed point number interface.
//
//===----------------------------------------------------------------------===//

#include "Basic/FixedPoint.h"

namespace fly {

APFixedPoint APFixedPoint::convert(const FixedPointSemantics &DstSema,
                                   bool *Overflow) const {
  llvm::APSInt NewVal = Val;
  unsigned DstWidth = DstSema.getWidth();
  unsigned DstScale = DstSema.getScale();
  bool Upscaling = DstScale > getScale();
  if (Overflow)
    *Overflow = false;

  if (Upscaling) {
    NewVal = NewVal.extend(NewVal.getBitWidth() + DstScale - getScale());
    NewVal <<= (DstScale - getScale());
  } else {
    NewVal >>= (getScale() - DstScale);
  }

  auto Mask = llvm::APInt::getBitsSetFrom(
      NewVal.getBitWidth(),
      std::min(DstScale + DstSema.getIntegralBits(), NewVal.getBitWidth()));
  llvm::APInt Masked(NewVal & Mask);

  // Change in the bits above the sign
  if (!(Masked == Mask || Masked == 0)) {
    // Found overflow in the bits above the sign
    if (DstSema.isSaturated())
      NewVal = NewVal.isNegative() ? Mask : ~Mask;
    else if (Overflow)
      *Overflow = true;
  }

  // If the dst semantics are unsigned, but our value is signed and negative, we
  // clamp to zero.
  if (!DstSema.isSigned() && NewVal.isSigned() && NewVal.isNegative()) {
    // Found negative overflow for unsigned result
    if (DstSema.isSaturated())
      NewVal = 0;
    else if (Overflow)
      *Overflow = true;
  }

  NewVal = NewVal.extOrTrunc(DstWidth);
  NewVal.setIsSigned(DstSema.isSigned());
  return APFixedPoint(NewVal, DstSema);
}

int APFixedPoint::compare(const APFixedPoint &Other) const {
  llvm::APSInt ThisVal = getValue();
  llvm::APSInt OtherVal = Other.getValue();
  bool ThisSigned = Val.isSigned();
  bool OtherSigned = OtherVal.isSigned();
  unsigned OtherScale = Other.getScale();
  unsigned OtherWidth = OtherVal.getBitWidth();

  unsigned CommonWidth = std::max(Val.getBitWidth(), OtherWidth);

  // Prevent overflow in the event the widths are the same but the scales differ
  CommonWidth += getScale() >= OtherScale ? getScale() - OtherScale
                                          : OtherScale - getScale();

  ThisVal = ThisVal.extOrTrunc(CommonWidth);
  OtherVal = OtherVal.extOrTrunc(CommonWidth);

  unsigned CommonScale = std::max(getScale(), OtherScale);
  ThisVal = ThisVal.shl(CommonScale - getScale());
  OtherVal = OtherVal.shl(CommonScale - OtherScale);

  if (ThisSigned && OtherSigned) {
    if (ThisVal.sgt(OtherVal))
      return 1;
    else if (ThisVal.slt(OtherVal))
      return -1;
  } else if (!ThisSigned && !OtherSigned) {
    if (ThisVal.ugt(OtherVal))
      return 1;
    else if (ThisVal.ult(OtherVal))
      return -1;
  } else if (ThisSigned && !OtherSigned) {
    if (ThisVal.isSignBitSet())
      return -1;
    else if (ThisVal.ugt(OtherVal))
      return 1;
    else if (ThisVal.ult(OtherVal))
      return -1;
  } else {
    // !ThisSigned && OtherSigned
    if (OtherVal.isSignBitSet())
      return 1;
    else if (ThisVal.ugt(OtherVal))
      return 1;
    else if (ThisVal.ult(OtherVal))
      return -1;
  }

  return 0;
}

APFixedPoint APFixedPoint::getMax(const FixedPointSemantics &Sema) {
  bool IsUnsigned = !Sema.isSigned();
  auto Val = llvm::APSInt::getMaxValue(Sema.getWidth(), IsUnsigned);
  if (IsUnsigned && Sema.hasUnsignedPadding())
    Val = Val.lshr(1);
  return APFixedPoint(Val, Sema);
}

APFixedPoint APFixedPoint::getMin(const FixedPointSemantics &Sema) {
  auto Val = llvm::APSInt::getMinValue(Sema.getWidth(), !Sema.isSigned());
  return APFixedPoint(Val, Sema);
}

FixedPointSemantics FixedPointSemantics::getCommonSemantics(
    const FixedPointSemantics &Other) const {
  unsigned CommonScale = std::max(getScale(), Other.getScale());
  unsigned CommonWidth =
      std::max(getIntegralBits(), Other.getIntegralBits()) + CommonScale;

  bool ResultIsSigned = isSigned() || Other.isSigned();
  bool ResultIsSaturated = isSaturated() || Other.isSaturated();
  bool ResultHasUnsignedPadding = false;
  if (!ResultIsSigned) {
    // Both are unsigned.
    ResultHasUnsignedPadding = hasUnsignedPadding() &&
                               Other.hasUnsignedPadding() && !ResultIsSaturated;
  }

  // If the result is signed, add an extra bit for the sign. Otherwise, if it is
  // unsigned and has unsigned padding, we only need to add the extra padding
  // bit back if we are not saturating.
  if (ResultIsSigned || ResultHasUnsignedPadding)
    CommonWidth++;

  return FixedPointSemantics(CommonWidth, CommonScale, ResultIsSigned,
                             ResultIsSaturated, ResultHasUnsignedPadding);
}

APFixedPoint APFixedPoint::add(const APFixedPoint &Other,
                               bool *Overflow) const {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  llvm::APSInt ThisVal = ConvertedThis.getValue();
  llvm::APSInt OtherVal = ConvertedOther.getValue();
  bool Overflowed = false;

  llvm::APSInt Result;
  if (CommonFXSema.isSaturated()) {
    Result = CommonFXSema.isSigned() ? ThisVal.sadd_sat(OtherVal)
                                     : ThisVal.uadd_sat(OtherVal);
  } else {
    Result = ThisVal.isSigned() ? ThisVal.sadd_ov(OtherVal, Overflowed)
                                : ThisVal.uadd_ov(OtherVal, Overflowed);
  }

  if (Overflow)
    *Overflow = Overflowed;

  return APFixedPoint(Result, CommonFXSema);
}

void APFixedPoint::toString(llvm::SmallVectorImpl<char> &Str) const {
  llvm::APSInt Val = getValue();
  unsigned Scale = getScale();

  if (Val.isSigned() && Val.isNegative() && Val != -Val) {
    Val = -Val;
    Str.push_back('-');
  }

  llvm::APSInt IntPart = Val >> Scale;

  // Add 4 digits to hold the value after multiplying 10 (the radix)
  unsigned Width = Val.getBitWidth() + 4;
  llvm::APInt FractPart = Val.zextOrTrunc(Scale).zext(Width);
  llvm::APInt FractPartMask = llvm::APInt::getAllOnesValue(Scale).zext(Width);
  llvm::APInt RadixInt = llvm::APInt(Width, 10);

  IntPart.toString(Str, /*Radix=*/10);
  Str.push_back('.');
  do {
    (FractPart * RadixInt)
        .lshr(Scale)
        .toString(Str, /*Radix=*/10, Val.isSigned());
    FractPart = (FractPart * RadixInt) & FractPartMask;
  } while (FractPart != 0);
}

APFixedPoint APFixedPoint::negate(bool *Overflow) const {
  if (!isSaturated()) {
    if (Overflow)
      *Overflow =
          (!isSigned() && Val != 0) || (isSigned() && Val.isMinSignedValue());
    return APFixedPoint(-Val, Sema);
  }

  // We never overflow for saturation
  if (Overflow)
    *Overflow = false;

  if (isSigned())
    return Val.isMinSignedValue() ? getMax(Sema) : APFixedPoint(-Val, Sema);
  else
    return APFixedPoint(Sema);
}

llvm::APSInt APFixedPoint::convertToInt(unsigned DstWidth, bool DstSign,
                                        bool *Overflow) const {
  llvm::APSInt Result = getIntPart();
  unsigned SrcWidth = getWidth();

  llvm::APSInt DstMin = llvm::APSInt::getMinValue(DstWidth, !DstSign);
  llvm::APSInt DstMax = llvm::APSInt::getMaxValue(DstWidth, !DstSign);

  if (SrcWidth < DstWidth) {
    Result = Result.extend(DstWidth);
  } else if (SrcWidth > DstWidth) {
    DstMin = DstMin.extend(SrcWidth);
    DstMax = DstMax.extend(SrcWidth);
  }

  if (Overflow) {
    if (Result.isSigned() && !DstSign) {
      *Overflow = Result.isNegative() || Result.ugt(DstMax);
    } else if (Result.isUnsigned() && DstSign) {
      *Overflow = Result.ugt(DstMax);
    } else {
      *Overflow = Result < DstMin || Result > DstMax;
    }
  }

  Result.setIsSigned(DstSign);
  return Result.extOrTrunc(DstWidth);
}

APFixedPoint APFixedPoint::getFromIntValue(const llvm::APSInt &Value,
                                           const FixedPointSemantics &DstFXSema,
                                           bool *Overflow) {
  FixedPointSemantics IntFXSema = FixedPointSemantics::GetIntegerSemantics(
      Value.getBitWidth(), Value.isSigned());
  return APFixedPoint(Value, IntFXSema).convert(DstFXSema, Overflow);
}

}  // namespace clang
