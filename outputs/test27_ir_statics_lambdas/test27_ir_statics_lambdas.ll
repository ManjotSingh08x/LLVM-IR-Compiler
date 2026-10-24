; ModuleID = 'test/test27_ir_statics_lambdas.ckp'
source_filename = "test/test27_ir_statics_lambdas.ckp"

%class.Box = type { i32 }
%closure.23.22 = type { ptr }
%closure.36.19 = type {}
%closure.37.20 = type { i32 }
%closure.38.18 = type { ptr }
%closure.45.21 = type { i32, i32 }
%closure.46.19 = type { ptr }

@global_9instances = internal global i32 0
@global_5limit = internal global i32 10
@global_5calls_v.count.0 = internal global i32 0
@.str = private unnamed_addr constant [7 x i8] c"%d %d\0A\00"
@.str.1 = private unnamed_addr constant [10 x i8] c"%d %d %d\0A\00"
@.str.2 = private unnamed_addr constant [4 x i8] c"%d\0A\00"

declare i32 @printf(ptr, ...)

define internal i32 @global_6helper_i(i32 %x) {
entry:
  %x.addr = alloca i32
  store i32 %x, ptr %x.addr
  %t0 = load i32, ptr %x.addr
  %t1 = mul i32 %t0, 2
  ret i32 %t1
}

define i32 @global_5calls_v() {
entry:
  %t0 = load i32, ptr @global_5calls_v.count.0
  %t1 = add i32 %t0, 1
  store i32 %t1, ptr @global_5calls_v.count.0
  %t2 = load i32, ptr @global_5calls_v.count.0
  ret i32 %t2
}

define i32 @main() {
entry:
  %base = alloca i32
  %square = alloca %closure.36.19
  %byValue = alloca %closure.37.20
  %byRef = alloca %closure.38.18
  %a = alloca i32
  %b = alloca i32
  %allValue = alloca %closure.45.21
  %allRef = alloca %closure.46.19
  %box = alloca %class.Box
  %t0 = call i32 @global_5calls_v()
  %t1 = call i32 @global_5calls_v()
  %t2 = load i32, ptr @global_5limit
  %t3 = call i32 @global_6helper_i(i32 %t2)
  store i32 %t3, ptr @global_9instances
  %t4 = getelementptr inbounds [7 x i8], ptr @.str, i64 0, i64 0
  %t5 = call i32 @global_5calls_v()
  %t6 = load i32, ptr @global_9instances
  %t7 = call i32 (ptr, ...) @printf(ptr %t4, i32 %t5, i32 %t6)
  store i32 10, ptr %base
  %t8 = getelementptr inbounds %closure.37.20, ptr %byValue, i32 0, i32 0
  %t9 = load i32, ptr %base
  store i32 %t9, ptr %t8
  %t10 = getelementptr inbounds %closure.38.18, ptr %byRef, i32 0, i32 0
  store ptr %base, ptr %t10
  call void @__lambda_38_18(ptr %byRef, i32 5)
  %t11 = getelementptr inbounds [10 x i8], ptr @.str.1, i64 0, i64 0
  %t12 = call i32 @__lambda_36_19(ptr %square, i32 4)
  %t13 = call i32 @__lambda_37_20(ptr %byValue, i32 1)
  %t14 = load i32, ptr %base
  %t15 = call i32 (ptr, ...) @printf(ptr %t11, i32 %t12, i32 %t13, i32 %t14)
  store i32 1, ptr %a
  store i32 2, ptr %b
  %t16 = getelementptr inbounds %closure.45.21, ptr %allValue, i32 0, i32 0
  %t17 = load i32, ptr %a
  store i32 %t17, ptr %t16
  %t18 = getelementptr inbounds %closure.45.21, ptr %allValue, i32 0, i32 1
  %t19 = load i32, ptr %b
  store i32 %t19, ptr %t18
  %t20 = getelementptr inbounds %closure.46.19, ptr %allRef, i32 0, i32 0
  store ptr %a, ptr %t20
  call void @__lambda_46_19(ptr %allRef)
  %t21 = getelementptr inbounds [7 x i8], ptr @.str, i64 0, i64 0
  %t22 = call i32 @__lambda_45_21(ptr %allValue)
  %t23 = load i32, ptr %a
  %t24 = call i32 (ptr, ...) @printf(ptr %t21, i32 %t22, i32 %t23)
  %t25 = getelementptr inbounds %class.Box, ptr %box, i32 0, i32 0
  store i32 21, ptr %t25
  %t26 = getelementptr inbounds [4 x i8], ptr @.str.2, i64 0, i64 0
  %t27 = call i32 @class_3Box_7doubled_v(ptr %box)
  %t28 = call i32 (ptr, ...) @printf(ptr %t26, i32 %t27)
  ret i32 0
}

define i32 @class_3Box_7doubled_v(ptr %this) {
entry:
  %this.addr = alloca ptr
  %twice = alloca %closure.23.22
  store ptr %this, ptr %this.addr
  %t0 = load ptr, ptr %this.addr
  %t1 = getelementptr inbounds %closure.23.22, ptr %twice, i32 0, i32 0
  store ptr %t0, ptr %t1
  %t2 = call i32 @__lambda_23_22(ptr %twice)
  ret i32 %t2
}

define internal i32 @__lambda_23_22(ptr %__closure) {
entry:
  %t0 = getelementptr inbounds %closure.23.22, ptr %__closure, i32 0, i32 0
  %t1 = load ptr, ptr %t0
  %t2 = getelementptr inbounds %class.Box, ptr %t1, i32 0, i32 0
  %t3 = load i32, ptr %t2
  %t4 = mul i32 %t3, 2
  ret i32 %t4
}

define internal i32 @__lambda_36_19(ptr %__closure, i32 %v) {
entry:
  %v.addr = alloca i32
  store i32 %v, ptr %v.addr
  %t0 = load i32, ptr %v.addr
  %t1 = load i32, ptr %v.addr
  %t2 = mul i32 %t0, %t1
  ret i32 %t2
}

define internal i32 @__lambda_37_20(ptr %__closure, i32 %v) {
entry:
  %v.addr = alloca i32
  %t0 = getelementptr inbounds %closure.37.20, ptr %__closure, i32 0, i32 0
  store i32 %v, ptr %v.addr
  %t1 = load i32, ptr %t0
  %t2 = load i32, ptr %v.addr
  %t3 = add i32 %t1, %t2
  ret i32 %t3
}

define internal void @__lambda_38_18(ptr %__closure, i32 %v) {
entry:
  %v.addr = alloca i32
  %t0 = getelementptr inbounds %closure.38.18, ptr %__closure, i32 0, i32 0
  store i32 %v, ptr %v.addr
  %t1 = load ptr, ptr %t0
  %t2 = load i32, ptr %t1
  %t3 = load i32, ptr %v.addr
  %t4 = add i32 %t2, %t3
  %t5 = load ptr, ptr %t0
  store i32 %t4, ptr %t5
  ret void
}

define internal i32 @__lambda_45_21(ptr %__closure) {
entry:
  %t0 = getelementptr inbounds %closure.45.21, ptr %__closure, i32 0, i32 0
  %t1 = getelementptr inbounds %closure.45.21, ptr %__closure, i32 0, i32 1
  %t2 = load i32, ptr %t0
  %t3 = load i32, ptr %t1
  %t4 = add i32 %t2, %t3
  ret i32 %t4
}

define internal void @__lambda_46_19(ptr %__closure) {
entry:
  %t0 = getelementptr inbounds %closure.46.19, ptr %__closure, i32 0, i32 0
  %t1 = load ptr, ptr %t0
  store i32 100, ptr %t1
  ret void
}
