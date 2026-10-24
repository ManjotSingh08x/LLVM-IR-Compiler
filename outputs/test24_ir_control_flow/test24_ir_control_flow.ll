; ModuleID = 'test/test24_ir_control_flow.ckp'
source_filename = "test/test24_ir_control_flow.ckp"

@.str = private unnamed_addr constant [10 x i8] c"%d %d %d\0A\00"
@.str.1 = private unnamed_addr constant [4 x i8] c"%d\0A\00"
@.str.2 = private unnamed_addr constant [13 x i8] c"%d %d %d %d\0A\00"

declare i32 @printf(ptr, ...)

define i32 @global_4sign_i(i32 %n) {
entry:
  %n.addr = alloca i32
  store i32 %n, ptr %n.addr
  %t0 = load i32, ptr %n.addr
  %t1 = icmp slt i32 %t0, 0
  br i1 %t1, label %if.then, label %if.else

if.then:  ; preds = %entry
  %t2 = sub i32 0, 1
  ret i32 %t2

if.else:  ; preds = %entry
  %t3 = load i32, ptr %n.addr
  %t4 = icmp eq i32 %t3, 0
  br i1 %t4, label %if.then.1, label %if.else.1

if.then.1:  ; preds = %if.else
  ret i32 0

if.else.1:  ; preds = %if.else
  ret i32 1
}

define i32 @global_5grade_i(i32 %score) {
entry:
  %score.addr = alloca i32
  %result = alloca i32
  store i32 %score, ptr %score.addr
  store i32 0, ptr %result
  %t0 = load i32, ptr %score.addr
  switch i32 %t0, label %sw.default [
    i32 1, label %sw.case
    i32 2, label %sw.case.1
    i32 3, label %sw.case.2
  ]

sw.case:  ; preds = %entry
  br label %sw.case.1

sw.case.1:  ; preds = %sw.case, %entry
  store i32 10, ptr %result
  br label %sw.end

sw.case.2:  ; preds = %entry
  store i32 30, ptr %result
  br label %sw.end

sw.default:  ; preds = %entry
  %t1 = sub i32 0, 1
  store i32 %t1, ptr %result
  br label %sw.end

sw.end:  ; preds = %sw.case.1, %sw.case.2, %sw.default
  %t2 = load i32, ptr %result
  ret i32 %t2
}

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

define i32 @main() {
entry:
  %odd = alloca i32
  %i = alloca i32
  %n = alloca i32
  %m = alloca i32
  %tries = alloca i32
  %t0 = getelementptr inbounds [10 x i8], ptr @.str, i64 0, i64 0
  %t1 = sub i32 0, 4
  %t2 = call i32 @global_4sign_i(i32 %t1)
  %t3 = call i32 @global_4sign_i(i32 0)
  %t4 = call i32 @global_4sign_i(i32 9)
  %t5 = call i32 (ptr, ...) @printf(ptr %t0, i32 %t2, i32 %t3, i32 %t4)
  %t6 = getelementptr inbounds [10 x i8], ptr @.str, i64 0, i64 0
  %t7 = call i32 @global_5grade_i(i32 2)
  %t8 = call i32 @global_5grade_i(i32 3)
  %t9 = call i32 @global_5grade_i(i32 7)
  %t10 = call i32 (ptr, ...) @printf(ptr %t6, i32 %t7, i32 %t8, i32 %t9)
  %t11 = getelementptr inbounds [4 x i8], ptr @.str.1, i64 0, i64 0
  %t12 = call i32 @global_9factorial_i(i32 5)
  %t13 = call i32 (ptr, ...) @printf(ptr %t11, i32 %t12)
  store i32 0, ptr %odd
  store i32 0, ptr %i
  br label %for.cond

for.cond:  ; preds = %entry, %for.inc
  %t14 = load i32, ptr %i
  %t15 = icmp slt i32 %t14, 10
  br i1 %t15, label %for.body, label %for.end

for.body:  ; preds = %for.cond
  %t16 = load i32, ptr %i
  %t17 = srem i32 %t16, 2
  %t18 = icmp eq i32 %t17, 0
  br i1 %t18, label %if.then, label %if.end

if.then:  ; preds = %for.body
  br label %for.inc

if.end:  ; preds = %for.body
  %t19 = load i32, ptr %i
  %t20 = icmp sgt i32 %t19, 7
  br i1 %t20, label %if.then.1, label %if.end.1

if.then.1:  ; preds = %if.end
  br label %for.end

if.end.1:  ; preds = %if.end
  %t21 = load i32, ptr %i
  %t22 = load i32, ptr %odd
  %t23 = add i32 %t22, %t21
  store i32 %t23, ptr %odd
  br label %for.inc

for.inc:  ; preds = %if.then, %if.end.1
  %t24 = load i32, ptr %i
  %t25 = add i32 %t24, 1
  store i32 %t25, ptr %i
  br label %for.cond

for.end:  ; preds = %for.cond, %if.then.1
  store i32 0, ptr %n
  br label %while.cond

while.cond:  ; preds = %for.end, %while.body
  %t26 = load i32, ptr %n
  %t27 = icmp slt i32 %t26, 5
  br i1 %t27, label %while.body, label %while.end

while.body:  ; preds = %while.cond
  %t28 = load i32, ptr %n
  %t29 = add i32 %t28, 1
  store i32 %t29, ptr %n
  br label %while.cond

while.end:  ; preds = %while.cond
  store i32 0, ptr %m
  br label %do.body

do.body:  ; preds = %while.end, %do.cond
  %t30 = load i32, ptr %m
  %t31 = add i32 %t30, 2
  store i32 %t31, ptr %m
  br label %do.cond

do.cond:  ; preds = %do.body
  %t32 = load i32, ptr %m
  %t33 = icmp slt i32 %t32, 7
  br i1 %t33, label %do.body, label %do.end

do.end:  ; preds = %do.cond
  store i32 0, ptr %tries
  br label %retry

retry:  ; preds = %do.end, %if.then.2
  %t34 = load i32, ptr %tries
  %t35 = add i32 %t34, 1
  store i32 %t35, ptr %tries
  %t36 = load i32, ptr %tries
  %t37 = icmp slt i32 %t36, 3
  br i1 %t37, label %if.then.2, label %if.end.2

if.then.2:  ; preds = %retry
  br label %retry

if.end.2:  ; preds = %retry
  %t38 = getelementptr inbounds [13 x i8], ptr @.str.2, i64 0, i64 0
  %t39 = load i32, ptr %odd
  %t40 = load i32, ptr %n
  %t41 = load i32, ptr %m
  %t42 = load i32, ptr %tries
  %t43 = call i32 (ptr, ...) @printf(ptr %t38, i32 %t39, i32 %t40, i32 %t41, i32 %t42)
  ret i32 0
}
