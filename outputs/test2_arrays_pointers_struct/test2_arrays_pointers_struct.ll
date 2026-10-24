; ModuleID = 'test/test2_arrays_pointers_struct.ckp'
source_filename = "test/test2_arrays_pointers_struct.ckp"

%struct.Point = type { i32, i32 }

define i32 @main() {
entry:
  %arr = alloca [5 x i32]
  %name = alloca [20 x i8]
  %grid = alloca [3 x [4 x i32]]
  %cube = alloca [2 x [3 x [4 x i32]]]
  %value = alloca i32
  %p = alloca ptr
  %v = alloca i32
  %ref = alloca ptr
  %pt = alloca %struct.Point
  %pp = alloca ptr
  %nothing = alloca ptr
  %t0 = getelementptr inbounds [5 x i32], ptr %arr, i64 0, i64 0
  %t1 = getelementptr inbounds i32, ptr %t0, i32 0
  store i32 1, ptr %t1
  %t2 = getelementptr inbounds [20 x i8], ptr %name, i64 0, i64 0
  %t3 = getelementptr inbounds i8, ptr %t2, i32 0
  store i8 65, ptr %t3
  %t4 = getelementptr inbounds [3 x [4 x i32]], ptr %grid, i64 0, i64 0
  %t5 = getelementptr inbounds [4 x i32], ptr %t4, i32 1
  %t6 = getelementptr inbounds [4 x i32], ptr %t5, i64 0, i64 0
  %t7 = getelementptr inbounds i32, ptr %t6, i32 2
  store i32 7, ptr %t7
  %t8 = getelementptr inbounds [2 x [3 x [4 x i32]]], ptr %cube, i64 0, i64 0
  %t9 = getelementptr inbounds [3 x [4 x i32]], ptr %t8, i32 1
  %t10 = getelementptr inbounds [3 x [4 x i32]], ptr %t9, i64 0, i64 0
  %t11 = getelementptr inbounds [4 x i32], ptr %t10, i32 2
  %t12 = getelementptr inbounds [4 x i32], ptr %t11, i64 0, i64 0
  %t13 = getelementptr inbounds i32, ptr %t12, i32 3
  store i32 9, ptr %t13
  store i32 42, ptr %value
  store ptr %value, ptr %p
  %t14 = load ptr, ptr %p
  %t15 = load i32, ptr %t14
  store i32 %t15, ptr %v
  store ptr %value, ptr %ref
  %t16 = load i32, ptr %v
  %t17 = load ptr, ptr %ref
  store i32 %t16, ptr %t17
  %t18 = getelementptr inbounds %struct.Point, ptr %pt, i32 0, i32 0
  store i32 1, ptr %t18
  %t19 = getelementptr inbounds %struct.Point, ptr %pt, i32 0, i32 1
  store i32 2, ptr %t19
  store ptr %pt, ptr %pp
  %t20 = load ptr, ptr %pp
  %t21 = getelementptr inbounds %struct.Point, ptr %t20, i32 0, i32 0
  store i32 3, ptr %t21
  %t22 = load ptr, ptr %pp
  %t23 = getelementptr inbounds %struct.Point, ptr %t22, i32 0, i32 1
  store i32 4, ptr %t23
  store ptr null, ptr %nothing
  ret i32 0
}
