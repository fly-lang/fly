; ModuleID = 'types.cpp'
source_filename = "types.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @_Z4testv() #0 {
  %1 = alloca i8, align 1
  %2 = alloca i8, align 1
  %3 = alloca i16, align 2
  %4 = alloca i16, align 2
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca i64, align 8
  %8 = alloca i64, align 8
  %9 = alloca float, align 4
  %10 = alloca double, align 8

  ;
  ;to bool
  ;

  ;bool
  %11 = load i8, i8* %1, align 1
  %12 = trunc i8 %11 to i1
  %13 = zext i1 %12 to i8
  store i8 %13, i8* %1, align 1

  ;char
  %14 = load i8, i8* %2, align 1
  %15 = icmp ne i8 %14, 0
  %16 = zext i1 %15 to i8
  store i8 %16, i8* %1, align 1

  ;short
  %17 = load i16, i16* %4, align 2
  %18 = icmp ne i16 %17, 0
  %19 = zext i1 %18 to i8
  store i8 %19, i8* %1, align 1

  ;unsigned short
  %20 = load i16, i16* %3, align 2
  %21 = icmp ne i16 %20, 0
  %22 = zext i1 %21 to i8
  store i8 %22, i8* %1, align 1

  ;int
  %23 = load i32, i32* %6, align 4
  %24 = icmp ne i32 %23, 0
  %25 = zext i1 %24 to i8
  store i8 %25, i8* %1, align 1

  ;unsigned int
  %26 = load i32, i32* %5, align 4
  %27 = icmp ne i32 %26, 0
  %28 = zext i1 %27 to i8
  store i8 %28, i8* %1, align 1

  ;long
  %29 = load i64, i64* %8, align 8
  %30 = icmp ne i64 %29, 0
  %31 = zext i1 %30 to i8
  store i8 %31, i8* %1, align 1

  ;unsigned long
  %32 = load i64, i64* %7, align 8
  %33 = icmp ne i64 %32, 0
  %34 = zext i1 %33 to i8
  store i8 %34, i8* %1, align 1

  ;float
  %35 = load float, float* %9, align 4
  %36 = fcmp une float %35, 0.000000e+00
  %37 = zext i1 %36 to i8
  store i8 %37, i8* %1, align 1

  ;double
  %38 = load double, double* %10, align 8
  %39 = fcmp une double %38, 0.000000e+00
  %40 = zext i1 %39 to i8
  store i8 %40, i8* %1, align 1

  ;
  ;to char
  ;

  ;bool
  %41 = load i8, i8* %1, align 1
  %42 = trunc i8 %41 to i1
  %43 = zext i1 %42 to i8
  store i8 %43, i8* %2, align 1

  ;char
  %44 = load i8, i8* %2, align 1
  store i8 %44, i8* %2, align 1

  ;short
  %45 = load i16, i16* %4, align 2
  %46 = trunc i16 %45 to i8
  store i8 %46, i8* %2, align 1

  ;unsigned short
  %47 = load i16, i16* %3, align 2
  %48 = trunc i16 %47 to i8
  store i8 %48, i8* %2, align 1

  ;int
  %49 = load i32, i32* %6, align 4
  %50 = trunc i32 %49 to i8
  store i8 %50, i8* %2, align 1

  ;unsigned int
  %51 = load i32, i32* %5, align 4
  %52 = trunc i32 %51 to i8
  store i8 %52, i8* %2, align 1

  ;long
  %53 = load i64, i64* %8, align 8
  %54 = trunc i64 %53 to i8
  store i8 %54, i8* %2, align 1

  ;unsigned long
  %55 = load i64, i64* %7, align 8
  %56 = trunc i64 %55 to i8
  store i8 %56, i8* %2, align 1

  ;float
  %57 = load float, float* %9, align 4
  %58 = fptoui float %57 to i8
  store i8 %58, i8* %2, align 1

  ;double
  %59 = load double, double* %10, align 8
  %60 = fptoui double %59 to i8
  store i8 %60, i8* %2, align 1

  ;
  ;to short
  ;

  ;bool
  %61 = load i8, i8* %1, align 1
  %62 = trunc i8 %61 to i1
  %63 = zext i1 %62 to i16
  store i16 %63, i16* %4, align 2

  ;char
  %64 = load i8, i8* %2, align 1
  %65 = zext i8 %64 to i16
  store i16 %65, i16* %4, align 2

  ;short
  %66 = load i16, i16* %4, align 2
  store i16 %66, i16* %4, align 2

  ;unsigned short
  %67 = load i16, i16* %3, align 2
  store i16 %67, i16* %4, align 2

  ;int
  %68 = load i32, i32* %6, align 4
  %69 = trunc i32 %68 to i16
  store i16 %69, i16* %4, align 2

  ;unsigned int
  %70 = load i32, i32* %5, align 4
  %71 = trunc i32 %70 to i16
  store i16 %71, i16* %4, align 2

  ;long
  %72 = load i64, i64* %8, align 8
  %73 = trunc i64 %72 to i16
  store i16 %73, i16* %4, align 2

  ;unsigned long
  %74 = load i64, i64* %7, align 8
  %75 = trunc i64 %74 to i16
  store i16 %75, i16* %4, align 2

  ;float
  %76 = load float, float* %9, align 4
  %77 = fptosi float %76 to i16
  store i16 %77, i16* %4, align 2

  ;double
  %78 = load double, double* %10, align 8
  %79 = fptosi double %78 to i16
  store i16 %79, i16* %4, align 2

  ;
  ;to unsigned short
  ;

  ;bool
  %80 = load i8, i8* %1, align 1
  %81 = trunc i8 %80 to i1
  %82 = zext i1 %81 to i16
  store i16 %82, i16* %3, align 2

  ;char
  %83 = load i8, i8* %2, align 1
  %84 = zext i8 %83 to i16
  store i16 %84, i16* %3, align 2

  ;short
  %85 = load i16, i16* %4, align 2
  store i16 %85, i16* %3, align 2

  ;unsigned short
  %86 = load i16, i16* %3, align 2
  store i16 %86, i16* %3, align 2

  ;int
  %87 = load i32, i32* %6, align 4
  %88 = trunc i32 %87 to i16
  store i16 %88, i16* %3, align 2

  ;unsigned int
  %89 = load i32, i32* %5, align 4
  %90 = trunc i32 %89 to i16
  store i16 %90, i16* %3, align 2

  ;long
  %91 = load i64, i64* %8, align 8
  %92 = trunc i64 %91 to i16
  store i16 %92, i16* %3, align 2

  ;unsigned long
  %93 = load i64, i64* %7, align 8
  %94 = trunc i64 %93 to i16
  store i16 %94, i16* %3, align 2

  ;float
  %95 = load float, float* %9, align 4
  %96 = fptoui float %95 to i16
  store i16 %96, i16* %3, align 2

  ;double
  %97 = load double, double* %10, align 8
  %98 = fptoui double %97 to i16
  store i16 %98, i16* %3, align 2

  ;
  ;to int
  ;

  ;bool
  %99 = load i8, i8* %1, align 1
  %100 = trunc i8 %99 to i1
  %101 = zext i1 %100 to i32
  store i32 %101, i32* %6, align 4

  ;char
  %102 = load i8, i8* %2, align 1
  %103 = zext i8 %102 to i32
  store i32 %103, i32* %6, align 4

  ;short
  %104 = load i16, i16* %4, align 2
  %105 = sext i16 %104 to i32
  store i32 %105, i32* %6, align 4

  ;unsigned short
  %106 = load i16, i16* %3, align 2
  %107 = zext i16 %106 to i32
  store i32 %107, i32* %6, align 4

  ;int
  %108 = load i32, i32* %6, align 4
  store i32 %108, i32* %6, align 4

  ;unsigned int
  %109 = load i32, i32* %5, align 4
  store i32 %109, i32* %6, align 4

  ;long
  %110 = load i64, i64* %8, align 8
  %111 = trunc i64 %110 to i32
  store i32 %111, i32* %6, align 4

  ;unsigned long
  %112 = load i64, i64* %7, align 8
  %113 = trunc i64 %112 to i32
  store i32 %113, i32* %6, align 4

  ;float
  %114 = load float, float* %9, align 4
  %115 = fptosi float %114 to i32
  store i32 %115, i32* %6, align 4

  ;double
  %116 = load double, double* %10, align 8
  %117 = fptosi double %116 to i32
  store i32 %117, i32* %6, align 4

  ;
  ;to unsigned int
  ;

  ;bool
  %118 = load i8, i8* %1, align 1
  %119 = trunc i8 %118 to i1
  %120 = zext i1 %119 to i32
  store i32 %120, i32* %5, align 4

  ;char
  %121 = load i8, i8* %2, align 1
  %122 = zext i8 %121 to i32
  store i32 %122, i32* %5, align 4

  ;short
  %123 = load i16, i16* %4, align 2
  %124 = sext i16 %123 to i32
  store i32 %124, i32* %5, align 4

  ;unsigned short
  %125 = load i16, i16* %3, align 2
  %126 = zext i16 %125 to i32
  store i32 %126, i32* %5, align 4

  ;int
  %127 = load i32, i32* %6, align 4
  store i32 %127, i32* %5, align 4

  ;unsigned int
  %128 = load i32, i32* %5, align 4
  store i32 %128, i32* %5, align 4

  ;long
  %129 = load i64, i64* %8, align 8
  %130 = trunc i64 %129 to i32
  store i32 %130, i32* %5, align 4

  ;unsigned long
  %131 = load i64, i64* %7, align 8
  %132 = trunc i64 %131 to i32
  store i32 %132, i32* %5, align 4

  ;float
  %133 = load float, float* %9, align 4
  %134 = fptoui float %133 to i32
  store i32 %134, i32* %5, align 4

  ;double
  %135 = load double, double* %10, align 8
  %136 = fptoui double %135 to i32
  store i32 %136, i32* %5, align 4

  ;
  ;to long
  ;

  ;bool
  %137 = load i8, i8* %1, align 1
  %138 = trunc i8 %137 to i1
  %139 = zext i1 %138 to i64
  store i64 %139, i64* %8, align 8

  ;char
  %140 = load i8, i8* %2, align 1
  %141 = zext i8 %140 to i64
  store i64 %141, i64* %8, align 8

  ;short
  %142 = load i16, i16* %4, align 2
  %143 = sext i16 %142 to i64
  store i64 %143, i64* %8, align 8

  ;unsigned short
  %144 = load i16, i16* %3, align 2
  %145 = zext i16 %144 to i64
  store i64 %145, i64* %8, align 8

  ;int
  %146 = load i32, i32* %6, align 4
  %147 = sext i32 %146 to i64
  store i64 %147, i64* %8, align 8

  ;unsigned int
  %148 = load i32, i32* %5, align 4
  %149 = zext i32 %148 to i64
  store i64 %149, i64* %8, align 8

  ;long
  %150 = load i64, i64* %8, align 8
  store i64 %150, i64* %8, align 8

  ;unsigned long
  %151 = load i64, i64* %7, align 8
  store i64 %151, i64* %8, align 8

  ;float
  %152 = load float, float* %9, align 4
  %153 = fptosi float %152 to i64
  store i64 %153, i64* %8, align 8

  ;double
  %154 = load double, double* %10, align 8
  %155 = fptosi double %154 to i64
  store i64 %155, i64* %8, align 8

  ;
  ;to unsigned long
  ;

  ;bool
  %156 = load i8, i8* %1, align 1
  %157 = trunc i8 %156 to i1
  %158 = zext i1 %157 to i64
  store i64 %158, i64* %7, align 8

  ;char
  %159 = load i8, i8* %2, align 1
  %160 = zext i8 %159 to i64
  store i64 %160, i64* %7, align 8

  ;short
  %161 = load i16, i16* %4, align 2
  %162 = sext i16 %161 to i64
  store i64 %162, i64* %7, align 8

  ;unsigned short
  %163 = load i16, i16* %3, align 2
  %164 = zext i16 %163 to i64
  store i64 %164, i64* %7, align 8

  ;int
  %165 = load i32, i32* %6, align 4
  %166 = sext i32 %165 to i64
  store i64 %166, i64* %7, align 8

  ;unsigned int
  %167 = load i32, i32* %5, align 4
  %168 = zext i32 %167 to i64
  store i64 %168, i64* %7, align 8

  ;long
  %169 = load i64, i64* %8, align 8
  store i64 %169, i64* %7, align 8

  ;unsigned long
  %170 = load i64, i64* %7, align 8
  store i64 %170, i64* %7, align 8

  ;float
  %171 = load float, float* %9, align 4
  %172 = fptoui float %171 to i64
  store i64 %172, i64* %7, align 8

  ;double
  %173 = load double, double* %10, align 8
  %174 = fptoui double %173 to i64
  store i64 %174, i64* %7, align 8

  ;
  ;to float
  ;

  ;bool
  %175 = load i8, i8* %1, align 1
  %176 = trunc i8 %175 to i1
  %177 = uitofp i1 %176 to float
  store float %177, float* %9, align 4

  ;char
  %178 = load i8, i8* %2, align 1
  %179 = uitofp i8 %178 to float
  store float %179, float* %9, align 4

  ;short
  %180 = load i16, i16* %4, align 2
  %181 = sitofp i16 %180 to float
  store float %181, float* %9, align 4

  ;unsigned short
  %182 = load i16, i16* %3, align 2
  %183 = uitofp i16 %182 to float
  store float %183, float* %9, align 4

  ;int
  %184 = load i32, i32* %6, align 4
  %185 = sitofp i32 %184 to float
  store float %185, float* %9, align 4

  ;unsigned int
  %186 = load i32, i32* %5, align 4
  %187 = uitofp i32 %186 to float
  store float %187, float* %9, align 4

  ;long
  %188 = load i64, i64* %8, align 8
  %189 = sitofp i64 %188 to float
  store float %189, float* %9, align 4

  ;unsigned long
  %190 = load i64, i64* %7, align 8
  %191 = uitofp i64 %190 to float
  store float %191, float* %9, align 4

  ;float
  %192 = load float, float* %9, align 4
  store float %192, float* %9, align 4

  ;double
  %193 = load double, double* %10, align 8
  %194 = fptrunc double %193 to float
  store float %194, float* %9, align 4

  ;
  ;to double
  ;

  ;bool
  %195 = load i8, i8* %1, align 1
  %196 = trunc i8 %195 to i1
  %197 = uitofp i1 %196 to double
  store double %197, double* %10, align 8

  ;char
  %198 = load i8, i8* %2, align 1
  %199 = uitofp i8 %198 to double
  store double %199, double* %10, align 8

  ;short
  %200 = load i16, i16* %4, align 2
  %201 = sitofp i16 %200 to double
  store double %201, double* %10, align 8

  ;unsigned short
  %202 = load i16, i16* %3, align 2
  %203 = uitofp i16 %202 to double
  store double %203, double* %10, align 8

  ;int
  %204 = load i32, i32* %6, align 4
  %205 = sitofp i32 %204 to double
  store double %205, double* %10, align 8

  ;unsigned int
  %206 = load i32, i32* %5, align 4
  %207 = uitofp i32 %206 to double
  store double %207, double* %10, align 8

  ;long
  %208 = load i64, i64* %8, align 8
  %209 = sitofp i64 %208 to double
  store double %209, double* %10, align 8

  ;unsigned long
  %210 = load i64, i64* %7, align 8
  %211 = uitofp i64 %210 to double
  store double %211, double* %10, align 8

  ;float
  %212 = load float, float* %9, align 4
  %213 = fpext float %212 to double
  store double %213, double* %10, align 8

  ;double
  %214 = load double, double* %10, align 8
  store double %214, double* %10, align 8
  ret i32 1
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Debian clang version 11.1.0-++20211011094159+1fdec59bffc1-1~exp1~20211011214627.7"}
