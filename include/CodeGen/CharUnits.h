//===-------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CharUnits.h - AST Variable
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//
//
//  This file defines the CharUnits class
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef LLVM_CLANG_AST_CHARUNITS_H
#define LLVM_CLANG_AST_CHARUNITS_H

#include "llvm/ADT/DenseMapInfo.h"
#include "llvm/Support/Alignment.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/MathExtras.h"

namespace fly {

  /// CharUnits - This is an opaque type for sizes expressed in character units.
  /// Instances of this type represent a quantity as a multiple of the size
  /// of the standard C type, char, on the target architecture. As an opaque
  /// type, CharUnits protects you from accidentally combining operations on
  /// quantities in bit units and character units.
  ///
  /// In both C and C++, an object of type 'char', 'signed char', or 'unsigned
  /// char' occupies exactly one byte, so 'character unit' and 'byte' refer to
  /// the same quantity of storage. However, we use the term 'character unit'
  /// rather than 'byte' to avoid an implication that a character unit is
  /// exactly 8 bits.
  ///
  /// For portability, never assume that a target character is 8 bits wide. Use
  /// CharUnit values wherever you calculate sizes, offsets, or alignments
  /// in character units.
  class CharUnits {
    public:
      typedef int64_t QuantityType;

    private:
      QuantityType Quantity = 0;

      explicit CharUnits(QuantityType C) : Quantity(C) {}

    public:

      /// CharUnits - A default constructor.
      CharUnits() = default;

      /// Zero - Construct a CharUnits quantity of zero.
      static CharUnits Zero() {
        return CharUnits(0);
      }

      /// One - Construct a CharUnits quantity of one.
      static CharUnits One() {
        return CharUnits(1);
      }

      /// fromQuantity - Construct a CharUnits quantity from a raw integer type.
      static CharUnits fromQuantity(QuantityType Quantity) {
        return CharUnits(Quantity);
      }

      // Compound assignment.
      CharUnits& operator+= (const CharUnits &Other) {
        Quantity += Other.Quantity;
        return *this;
      }
      CharUnits& operator++ () {
        ++Quantity;
        return *this;
      }
      CharUnits operator++ (int) {
        return CharUnits(Quantity++);
      }
      CharUnits& operator-= (const CharUnits &Other) {
        Quantity -= Other.Quantity;
        return *this;
      }
      CharUnits& operator-- () {
        --Quantity;
        return *this;
      }
      CharUnits operator-- (int) {
        return CharUnits(Quantity--);
      }

      // Comparison operators.
      bool operator== (const CharUnits &Other) const {
        return Quantity == Other.Quantity;
      }
      bool operator!= (const CharUnits &Other) const {
        return Quantity != Other.Quantity;
      }

      // Relational operators.
      bool operator<  (const CharUnits &Other) const {
        return Quantity <  Other.Quantity;
      }
      bool operator<= (const CharUnits &Other) const {
        return Quantity <= Other.Quantity;
      }
      bool operator>  (const CharUnits &Other) const {
        return Quantity >  Other.Quantity;
      }
      bool operator>= (const CharUnits &Other) const {
        return Quantity >= Other.Quantity;
      }

      // Other predicates.

      /// isZero - Test whether the quantity equals zero.
      bool isZero() const     { return Quantity == 0; }

      /// isOne - Test whether the quantity equals one.
      bool isOne() const      { return Quantity == 1; }

      /// isPositive - Test whether the quantity is greater than zero.
      bool isPositive() const { return Quantity  > 0; }

      /// isNegative - Test whether the quantity is less than zero.
      bool isNegative() const { return Quantity  < 0; }

      /// isPowerOfTwo - Test whether the quantity is a power of two.
      /// Zero is not a power of two.
      bool isPowerOfTwo() const {
        return (Quantity & -Quantity) == Quantity;
      }

      /// Test whether this is a multiple of the other value.
      ///
      /// Among other things, this promises that
      /// self.alignTo(N) will just return self.
      bool isMultipleOf(CharUnits N) const {
        return (*this % N) == 0;
      }

      // Arithmetic operators.
      CharUnits operator* (QuantityType N) const {
        return CharUnits(Quantity * N);
      }
      CharUnits &operator*= (QuantityType N) {
        Quantity *= N;
        return *this;
      }
      CharUnits operator/ (QuantityType N) const {
        return CharUnits(Quantity / N);
      }
      CharUnits &operator/= (QuantityType N) {
        Quantity /= N;
        return *this;
      }
      QuantityType operator/ (const CharUnits &Other) const {
        return Quantity / Other.Quantity;
      }
      CharUnits operator% (QuantityType N) const {
        return CharUnits(Quantity % N);
      }
      QuantityType operator% (const CharUnits &Other) const {
        return Quantity % Other.Quantity;
      }
      CharUnits operator+ (const CharUnits &Other) const {
        return CharUnits(Quantity + Other.Quantity);
      }
      CharUnits operator- (const CharUnits &Other) const {
        return CharUnits(Quantity - Other.Quantity);
      }
      CharUnits operator- () const {
        return CharUnits(-Quantity);
      }


      // Conversions.

      /// getQuantity - Get the raw integer representation of this quantity.
      QuantityType getQuantity() const { return Quantity; }

      /// getAsAlign - Returns Quantity as a valid llvm::Align,
      /// Beware llvm::Align assumes power of two 8-bit bytes.
      llvm::Align getAsAlign() const { return llvm::Align(Quantity); }

      /// alignTo - Returns the next integer (mod 2**64) that is
      /// greater than or equal to this quantity and is a multiple of \p Align.
      /// Align must be non-zero.
      CharUnits alignTo(const CharUnits &Align) const {
        return CharUnits(llvm::alignTo(Quantity, Align.Quantity));
      }

      /// Given that this is a non-zero alignment value, what is the
      /// alignment at the given offset?
      CharUnits alignmentAtOffset(CharUnits offset) const {
        assert(Quantity != 0 && "offsetting from unknown alignment?");
        return CharUnits(llvm::MinAlign(Quantity, offset.Quantity));
      }

      /// Given that this is the alignment of the first element of an
      /// array, return the minimum alignment of any element in the array.
      CharUnits alignmentOfArrayElement(CharUnits elementSize) const {
        // Since we don't track offsetted alignments, the alignment of
        // the second element (or any odd element) will be minimally
        // aligned.
        return alignmentAtOffset(elementSize);
      }


  }; // class CharUnit
} // namespace clang

inline fly::CharUnits operator* (fly::CharUnits::QuantityType Scale,
                                   const fly::CharUnits &CU) {
  return CU * Scale;
}

namespace llvm {

template<> struct DenseMapInfo<fly::CharUnits> {
  static fly::CharUnits getEmptyKey() {
    fly::CharUnits::QuantityType Quantity =
      DenseMapInfo<fly::CharUnits::QuantityType>::getEmptyKey();

    return fly::CharUnits::fromQuantity(Quantity);
  }

  static fly::CharUnits getTombstoneKey() {
    fly::CharUnits::QuantityType Quantity =
      DenseMapInfo<fly::CharUnits::QuantityType>::getTombstoneKey();

    return fly::CharUnits::fromQuantity(Quantity);
  }

  static unsigned getHashValue(const fly::CharUnits &CU) {
    fly::CharUnits::QuantityType Quantity = CU.getQuantity();
    return DenseMapInfo<fly::CharUnits::QuantityType>::getHashValue(Quantity);
  }

  static bool isEqual(const fly::CharUnits &LHS,
                      const fly::CharUnits &RHS) {
    return LHS == RHS;
  }
};

} // end namespace llvm

#endif // LLVM_CLANG_AST_CHARUNITS_H
