; ModuleID = 'types.cpp'
source_filename = "types.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @_Z4testv() #0 {
  %1 = alloca i8, align 1
  %2 = alloca i16, align 2
  %3 = alloca i16, align 2
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i64, align 8
  %7 = alloca i64, align 8
  %8 = alloca float, align 4
  %9 = alloca double, align 8

  ;to bool

  ;bool to bool
  %10 = load i8, i8* %1, align 1
  %11 = trunc i8 %10 to i1
  %12 = zext i1 %11 to i8
  store i8 %12, i8* %1, align 1

  ;short to bool
  %13 = load i16, i16* %3, align 2
  %14 = icmp ne i16 %13, 0
  %15 = zext i1 %14 to i8
  store i8 %15, i8* %1, align 1

  ;unsigned short
  %16 = load i16, i16* %2, align 2
  %17 = icmp ne i16 %16, 0
  %18 = zext i1 %17 to i8
  store i8 %18, i8* %1, align 1

  ;int
  %19 = load i32, i32* %5, align 4
  %20 = icmp ne i32 %19, 0
  %21 = zext i1 %20 to i8
  store i8 %21, i8* %1, align 1

  ;unsigned int
  %22 = load i32, i32* %4, align 4
  %23 = icmp ne i32 %22, 0
  %24 = zext i1 %23 to i8
  store i8 %24, i8* %1, align 1

  ;long
  %25 = load i64, i64* %7, align 8
  %26 = icmp ne i64 %25, 0
  %27 = zext i1 %26 to i8
  store i8 %27, i8* %1, align 1

  ;unsigned long
  %28 = load i64, i64* %6, align 8
  %29 = icmp ne i64 %28, 0
  %30 = zext i1 %29 to i8
  store i8 %30, i8* %1, align 1

  ;float
  %31 = load float, float* %8, align 4
  %32 = fcmp une float %31, 0.000000e+00
  %33 = zext i1 %32 to i8
  store i8 %33, i8* %1, align 1

  ;double
  %34 = load double, double* %9, align 8
  %35 = fcmp une double %34, 0.000000e+00
  %36 = zext i1 %35 to i8
  store i8 %36, i8* %1, align 1

  ;to short

  ;bool
  %37 = load i8, i8* %1, align 1
  %38 = trunc i8 %37 to i1
  %39 = zext i1 %38 to i16
  store i16 %39, i16* %3, align 2

  ;short
  %40 = load i16, i16* %3, align 2
  store i16 %40, i16* %3, align 2

  ;unsigned short
  %41 = load i16, i16* %2, align 2
  store i16 %41, i16* %3, align 2

  ;int
  %42 = load i32, i32* %5, align 4
  %43 = trunc i32 %42 to i16
  store i16 %43, i16* %3, align 2

  ;unsigned int
  %44 = load i32, i32* %4, align 4
  %45 = trunc i32 %44 to i16
  store i16 %45, i16* %3, align 2

  ;long
  %46 = load i64, i64* %7, align 8
  %47 = trunc i64 %46 to i16
  store i16 %47, i16* %3, align 2

  ;unsigned long
  %48 = load i64, i64* %6, align 8
  %49 = trunc i64 %48 to i16
  store i16 %49, i16* %3, align 2

  ;float
  %50 = load float, float* %8, align 4
  %51 = fptosi float %50 to i16
  store i16 %51, i16* %3, align 2

  ;double
  %52 = load double, double* %9, align 8
  %53 = fptosi double %52 to i16
  store i16 %53, i16* %3, align 2

  ;to unsigned short

  ;bool
  %54 = load i8, i8* %1, align 1
  %55 = trunc i8 %54 to i1
  %56 = zext i1 %55 to i16
  store i16 %56, i16* %2, align 2

  ;short
  %57 = load i16, i16* %3, align 2
  store i16 %57, i16* %2, align 2

  ;unsigned short
  %58 = load i16, i16* %2, align 2
  store i16 %58, i16* %2, align 2

  ;int
  %59 = load i32, i32* %5, align 4
  %60 = trunc i32 %59 to i16
  store i16 %60, i16* %2, align 2

  ;unsigned int
  %61 = load i32, i32* %4, align 4
  %62 = trunc i32 %61 to i16
  store i16 %62, i16* %2, align 2

  ;long
  %63 = load i64, i64* %7, align 8
  %64 = trunc i64 %63 to i16
  store i16 %64, i16* %2, align 2

  ;unsigned long
  %65 = load i64, i64* %6, align 8
  %66 = trunc i64 %65 to i16
  store i16 %66, i16* %2, align 2

  ;float
  %67 = load float, float* %8, align 4
  %68 = fptoui float %67 to i16
  store i16 %68, i16* %2, align 2

  ;double
  %69 = load double, double* %9, align 8
  %70 = fptoui double %69 to i16
  store i16 %70, i16* %2, align 2

  ;to int

  ;bool
  %71 = load i8, i8* %1, align 1
  %72 = trunc i8 %71 to i1
  %73 = zext i1 %72 to i32
  store i32 %73, i32* %5, align 4

  ;short
  %74 = load i16, i16* %3, align 2
  %75 = sext i16 %74 to i32
  store i32 %75, i32* %5, align 4

  ;unsigned short
  %76 = load i16, i16* %2, align 2
  %77 = zext i16 %76 to i32
  store i32 %77, i32* %5, align 4

  ;int
  %78 = load i32, i32* %5, align 4
  store i32 %78, i32* %5, align 4

  ;unsigned int
  %79 = load i32, i32* %4, align 4
  store i32 %79, i32* %5, align 4

  ;long
  %80 = load i64, i64* %7, align 8
  %81 = trunc i64 %80 to i32
  store i32 %81, i32* %5, align 4

  ;unsigned long
  %82 = load i64, i64* %6, align 8
  %83 = trunc i64 %82 to i32
  store i32 %83, i32* %5, align 4

  ;float
  %84 = load float, float* %8, align 4
  %85 = fptosi float %84 to i32
  store i32 %85, i32* %5, align 4

  ;double
  %86 = load double, double* %9, align 8
  %87 = fptosi double %86 to i32
  store i32 %87, i32* %5, align 4

  ;to unsigned int

  ;bool
  %88 = load i8, i8* %1, align 1
  %89 = trunc i8 %88 to i1
  %90 = zext i1 %89 to i32
  store i32 %90, i32* %4, align 4

  ;short
  %91 = load i16, i16* %3, align 2
  %92 = sext i16 %91 to i32
  store i32 %92, i32* %4, align 4

  ;unsigned short
  %93 = load i16, i16* %2, align 2
  %94 = zext i16 %93 to i32
  store i32 %94, i32* %4, align 4

  ;int
  %95 = load i32, i32* %5, align 4
  store i32 %95, i32* %4, align 4

  ;unsigned int
  %96 = load i32, i32* %4, align 4
  store i32 %96, i32* %4, align 4

  ;long
  %97 = load i64, i64* %7, align 8
  %98 = trunc i64 %97 to i32
  store i32 %98, i32* %4, align 4

  ;unsigned long
  %99 = load i64, i64* %6, align 8
  %100 = trunc i64 %99 to i32
  store i32 %100, i32* %4, align 4

  ;float
  %101 = load float, float* %8, align 4
  %102 = fptoui float %101 to i32
  store i32 %102, i32* %4, align 4

  ;double
  %103 = load double, double* %9, align 8
  %104 = fptoui double %103 to i32
  store i32 %104, i32* %4, align 4

  ;to long

  ;bool
  %105 = load i8, i8* %1, align 1
  %106 = trunc i8 %105 to i1
  %107 = zext i1 %106 to i64
  store i64 %107, i64* %7, align 8

  ;short
  %108 = load i16, i16* %3, align 2
  %109 = sext i16 %108 to i64
  store i64 %109, i64* %7, align 8

  ;unsigned short
  %110 = load i16, i16* %2, align 2
  %111 = zext i16 %110 to i64
  store i64 %111, i64* %7, align 8

  ;int
  %112 = load i32, i32* %5, align 4
  %113 = sext i32 %112 to i64
  store i64 %113, i64* %7, align 8

  ;unsigned int
  %114 = load i32, i32* %4, align 4
  %115 = zext i32 %114 to i64
  store i64 %115, i64* %7, align 8

  ;long
  %116 = load i64, i64* %7, align 8
  store i64 %116, i64* %7, align 8

  ;unsigned long
  %117 = load i64, i64* %6, align 8
  store i64 %117, i64* %7, align 8

  ;float
  %118 = load float, float* %8, align 4
  %119 = fptosi float %118 to i64
  store i64 %119, i64* %7, align 8

  ;double
  %120 = load double, double* %9, align 8
  %121 = fptosi double %120 to i64
  store i64 %121, i64* %7, align 8

  ;to unsigned long

  ;bool
  %122 = load i8, i8* %1, align 1
  %123 = trunc i8 %122 to i1
  %124 = zext i1 %123 to i64
  store i64 %124, i64* %6, align 8

  ;short
  %125 = load i16, i16* %3, align 2
  %126 = sext i16 %125 to i64
  store i64 %126, i64* %6, align 8

  ;unsigned short
  %127 = load i16, i16* %2, align 2
  %128 = zext i16 %127 to i64
  store i64 %128, i64* %6, align 8

  ;int
  %129 = load i32, i32* %5, align 4
  %130 = sext i32 %129 to i64
  store i64 %130, i64* %6, align 8

  ;unsigned int
  %131 = load i32, i32* %4, align 4
  %132 = zext i32 %131 to i64
  store i64 %132, i64* %6, align 8

  ;long
  %133 = load i64, i64* %7, align 8
  store i64 %133, i64* %6, align 8

  ;unsigned long
  %134 = load i64, i64* %6, align 8
  store i64 %134, i64* %6, align 8

  ;float
  %135 = load float, float* %8, align 4
  %136 = fptoui float %135 to i64
  store i64 %136, i64* %6, align 8

  ;double
  %137 = load double, double* %9, align 8
  %138 = fptoui double %137 to i64
  store i64 %138, i64* %6, align 8

  ;to float

  ;bool
  %139 = load i8, i8* %1, align 1
  %140 = trunc i8 %139 to i1
  %141 = uitofp i1 %140 to float
  store float %141, float* %8, align 4

  ;short
  %142 = load i16, i16* %3, align 2
  %143 = sitofp i16 %142 to float
  store float %143, float* %8, align 4

  ;unsigned short
  %144 = load i16, i16* %2, align 2
  %145 = uitofp i16 %144 to float
  store float %145, float* %8, align 4

  ;int
  %146 = load i32, i32* %5, align 4
  %147 = sitofp i32 %146 to float
  store float %147, float* %8, align 4

  ;unsigned int
  %148 = load i32, i32* %4, align 4
  %149 = uitofp i32 %148 to float
  store float %149, float* %8, align 4

  ;long
  %150 = load i64, i64* %7, align 8
  %151 = sitofp i64 %150 to float
  store float %151, float* %8, align 4

  ;unsigned long
  %152 = load i64, i64* %6, align 8
  %153 = uitofp i64 %152 to float
  store float %153, float* %8, align 4

  ;float
  %154 = load float, float* %8, align 4
  store float %154, float* %8, align 4

  ;double
  %155 = load double, double* %9, align 8
  %156 = fptrunc double %155 to float
  store float %156, float* %8, align 4

  ;to double

  ;bool
  %157 = load i8, i8* %1, align 1
  %158 = trunc i8 %157 to i1
  %159 = uitofp i1 %158 to double
  store double %159, double* %9, align 8

  ;short
  %160 = load i16, i16* %3, align 2
  %161 = sitofp i16 %160 to double
  store double %161, double* %9, align 8

  ;unsigned short
  %162 = load i16, i16* %2, align 2
  %163 = uitofp i16 %162 to double
  store double %163, double* %9, align 8

  ;int
  %164 = load i32, i32* %5, align 4
  %165 = sitofp i32 %164 to double
  store double %165, double* %9, align 8

  ;unsigned int
  %166 = load i32, i32* %4, align 4
  %167 = uitofp i32 %166 to double
  store double %167, double* %9, align 8

  ;long
  %168 = load i64, i64* %7, align 8
  %169 = sitofp i64 %168 to double
  store double %169, double* %9, align 8

  ;unsigned long
  %170 = load i64, i64* %6, align 8
  %171 = uitofp i64 %170 to double
  store double %171, double* %9, align 8

  ;float
  %172 = load float, float* %8, align 4
  %173 = fpext float %172 to double
  store double %173, double* %9, align 8

  ;double
  %174 = load double, double* %9, align 8
  store double %174, double* %9, align 8

  ret i32 1
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Debian clang version 11.1.0-++20211011094159+1fdec59bffc1-1~exp1~20211011214627.7"}
