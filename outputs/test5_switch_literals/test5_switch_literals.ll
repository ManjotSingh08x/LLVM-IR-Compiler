; ModuleID = 'test/test5_switch_literals.ckp'
source_filename = "test/test5_switch_literals.ckp"

@.str = private unnamed_addr constant [12 x i8] c"hello world\00"
@.str.1 = private unnamed_addr constant [22 x i8] c"tab\09here and newline\0A\00"
@.str.2 = private unnamed_addr constant [31 x i8] c"quote \22 and backslash \5C inside\00"
@.str.3 = private unnamed_addr constant [42 x i8] c"not a comment: /* nor this */ // nor this\00"
@.str.4 = private unnamed_addr constant [1 x i8] c"\00"

define i32 @main() {
entry:
  %dec = alloca i32
  %hex = alloca i32
  %oct = alloca i32
  %bin = alloca i32
  %f1 = alloca float
  %f2 = alloca float
  %d1 = alloca double
  %d2 = alloca double
  %d3 = alloca double
  %c1 = alloca i8
  %c2 = alloca i8
  %c3 = alloca i8
  %c4 = alloca i8
  %c5 = alloca i8
  %c6 = alloca i8
  %c7 = alloca i8
  %s1 = alloca ptr
  %s2 = alloca ptr
  %s3 = alloca ptr
  %s4 = alloca ptr
  %s5 = alloca ptr
  %yes = alloca i1
  %no = alloca i1
  store i32 42, ptr %dec
  store i32 31, ptr %hex
  store i32 493, ptr %oct
  store i32 10, ptr %bin
  %t0 = fptrunc double 0x40091EB851EB851F to float
  store float %t0, ptr %f1
  %t1 = fptrunc double 0x3F589374BC6A7EFA to float
  store float %t1, ptr %f2
  store double 0x3FE0000000000000, ptr %d1
  store double 0x4000000000000000, ptr %d2
  store double 0x4202A05F20000000, ptr %d3
  store i8 97, ptr %c1
  store i8 10, ptr %c2
  store i8 9, ptr %c3
  store i8 39, ptr %c4
  store i8 92, ptr %c5
  store i8 65, ptr %c6
  store i8 0, ptr %c7
  %t2 = getelementptr inbounds [12 x i8], ptr @.str, i64 0, i64 0
  store ptr %t2, ptr %s1
  %t3 = getelementptr inbounds [22 x i8], ptr @.str.1, i64 0, i64 0
  store ptr %t3, ptr %s2
  %t4 = getelementptr inbounds [31 x i8], ptr @.str.2, i64 0, i64 0
  store ptr %t4, ptr %s3
  %t5 = getelementptr inbounds [42 x i8], ptr @.str.3, i64 0, i64 0
  store ptr %t5, ptr %s4
  %t6 = getelementptr inbounds [1 x i8], ptr @.str.4, i64 0, i64 0
  store ptr %t6, ptr %s5
  store i1 true, ptr %yes
  store i1 false, ptr %no
  %t7 = load i32, ptr %dec
  switch i32 %t7, label %sw.default [
    i32 1, label %sw.case
    i32 2, label %sw.case.1
    i32 3, label %sw.case.2
    i32 4, label %sw.case.3
  ]

sw.case:  ; preds = %entry
  store i32 100, ptr %dec
  br label %sw.end

sw.case.1:  ; preds = %entry
  store i32 200, ptr %dec
  br label %sw.end

sw.case.2:  ; preds = %entry
  br label %sw.case.3

sw.case.3:  ; preds = %sw.case.2, %entry
  store i32 300, ptr %dec
  br label %sw.end

sw.default:  ; preds = %entry
  store i32 0, ptr %dec
  br label %sw.end

sw.end:  ; preds = %sw.case, %sw.case.1, %sw.case.3, %sw.default
  ret i32 0
}
