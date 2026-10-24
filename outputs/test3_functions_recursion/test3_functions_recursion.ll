; ModuleID = 'test/test3_functions_recursion.ckp'
source_filename = "test/test3_functions_recursion.ckp"

%closure.31.19 = type {}

@.str = private unnamed_addr constant [3 x i8] c"%d\00"
@.str.1 = private unnamed_addr constant [10 x i8] c"%d %d %s\0A\00"

declare i32 @printf(ptr, ...)

declare i32 @scanf(ptr, ...)

define i32 @global_9factorial_i(i32 %n) {
entry:
  %n.addr = alloca i32
  store i32 %n, ptr %n.addr
  %t0 = load i32, ptr %n.addr
  %t1 = icmp sle i32 %t0, 1
  br i1 %t1, label %if.then, label %if.end

if.then:  ; preds = %entry
  ret i32 1

if.end:  ; preds = %entry
  %t2 = load i32, ptr %n.addr
  %t3 = load i32, ptr %n.addr
  %t4 = sub i32 %t3, 1
  %t5 = call i32 @global_9factorial_i(i32 %t4)
  %t6 = mul i32 %t2, %t5
  ret i32 %t6
}

define i32 @global_3add_ii(i32 %a, i32 %b) {
entry:
  %a.addr = alloca i32
  %b.addr = alloca i32
  store i32 %a, ptr %a.addr
  store i32 %b, ptr %b.addr
  %t0 = load i32, ptr %a.addr
  %t1 = load i32, ptr %b.addr
  %t2 = add i32 %t0, %t1
  ret i32 %t2
}

define i32 @global_5twice_i(i32 %x) {
entry:
  %x.addr = alloca i32
  store i32 %x, ptr %x.addr
  %t0 = load i32, ptr %x.addr
  %t1 = mul i32 %t0, 2
  ret i32 %t1
}

define i32 @main(i32 %argc, ptr %argv) {
entry:
  %argc.addr = alloca i32
  %argv.addr = alloca ptr
  %input = alloca i32
  %f = alloca i32
  %s = alloca i32
  %fp = alloca ptr
  %square = alloca %closure.31.19
  %buffer = alloca ptr
  %d = alloca ptr
  store i32 %argc, ptr %argc.addr
  store ptr %argv, ptr %argv.addr
  %t0 = getelementptr inbounds [3 x i8], ptr @.str, i64 0, i64 0
  %t1 = call i32 (ptr, ...) @scanf(ptr %t0, ptr %input)
  %t2 = load i32, ptr %input
  %t3 = call i32 @global_9factorial_i(i32 %t2)
  store i32 %t3, ptr %f
  %t4 = load i32, ptr %f
  %t5 = call i32 @global_3add_ii(i32 %t4, i32 10)
  store i32 %t5, ptr %s
  store ptr @global_5twice_i, ptr %fp
  %t6 = load ptr, ptr %fp
  %t7 = load i32, ptr %s
  %t8 = call i32 %t6(i32 %t7)
  store i32 %t8, ptr %s
  %t9 = load i32, ptr %s
  %t10 = call i32 @__lambda_31_19(ptr %square, i32 %t9)
  store i32 %t10, ptr %s
  %t11 = sext i32 10 to i64
  %t12 = getelementptr i32, ptr null, i64 %t11
  %t13 = ptrtoint ptr %t12 to i64
  %t14 = call ptr @malloc(i64 %t13)
  store ptr %t14, ptr %buffer
  %t15 = load i32, ptr %s
  %t16 = load ptr, ptr %buffer
  %t17 = getelementptr inbounds i32, ptr %t16, i32 0
  store i32 %t15, ptr %t17
  %t18 = load ptr, ptr %buffer
  call void @free(ptr %t18)
  %t19 = getelementptr double, ptr null, i64 1
  %t20 = ptrtoint ptr %t19 to i64
  %t21 = call ptr @malloc(i64 %t20)
  store ptr %t21, ptr %d
  %t22 = load ptr, ptr %d
  call void @free(ptr %t22)
  %t23 = getelementptr inbounds [10 x i8], ptr @.str.1, i64 0, i64 0
  %t24 = load i32, ptr %s
  %t25 = load i32, ptr %argc.addr
  %t26 = load ptr, ptr %argv.addr
  %t27 = getelementptr inbounds ptr, ptr %t26, i32 0
  %t28 = load ptr, ptr %t27
  %t29 = call i32 (ptr, ...) @printf(ptr %t23, i32 %t24, i32 %t25, ptr %t28)
  ret i32 0
}

define internal i32 @__lambda_31_19(ptr %__closure, i32 %x) {
entry:
  %x.addr = alloca i32
  store i32 %x, ptr %x.addr
  %t0 = load i32, ptr %x.addr
  %t1 = load i32, ptr %x.addr
  %t2 = mul i32 %t0, %t1
  ret i32 %t2
}

declare ptr @malloc(i64)

declare void @free(ptr)
