; ModuleID = 'test/test28_ir_typedef_until.ckp'
source_filename = "test/test28_ir_typedef_until.ckp"

%class.Animal = type { i32 }
%class.Dog = type { %class.Animal, i32 }

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00"
@.str.1 = private unnamed_addr constant [7 x i8] c"%d %d\0A\00"
@.str.2 = private unnamed_addr constant [4 x i8] c"%d \00"
@.str.3 = private unnamed_addr constant [10 x i8] c"%d %d %d\0A\00"
@.str.4 = private unnamed_addr constant [4 x i8] c"%c\0A\00"

declare i32 @printf(ptr, ...)

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

define i32 @global_3mul_ii(i32 %a, i32 %b) {
entry:
  %a.addr = alloca i32
  %b.addr = alloca i32
  store i32 %a, ptr %a.addr
  store i32 %b, ptr %b.addr
  %t0 = load i32, ptr %a.addr
  %t1 = load i32, ptr %b.addr
  %t2 = mul i32 %t0, %t1
  ret i32 %t2
}

define i32 @global_5apply_PFiiiEii(ptr %op, i32 %a, i32 %b) {
entry:
  %op.addr = alloca ptr
  %a.addr = alloca i32
  %b.addr = alloca i32
  store ptr %op, ptr %op.addr
  store i32 %a, ptr %a.addr
  store i32 %b, ptr %b.addr
  %t0 = load ptr, ptr %op.addr
  %t1 = load i32, ptr %a.addr
  %t2 = load i32, ptr %b.addr
  %t3 = call i32 %t0(i32 %t1, i32 %t2)
  ret i32 %t3
}

define i32 @global_3sum_Pi(ptr %values) {
entry:
  %values.addr = alloca ptr
  %total = alloca i32
  %i = alloca i32
  store ptr %values, ptr %values.addr
  store i32 0, ptr %total
  store i32 0, ptr %i
  br label %for.cond

for.cond:  ; preds = %entry, %for.inc
  %t0 = load i32, ptr %i
  %t1 = icmp slt i32 %t0, 3
  br i1 %t1, label %for.body, label %for.end

for.body:  ; preds = %for.cond
  %t2 = load ptr, ptr %values.addr
  %t3 = load i32, ptr %i
  %t4 = getelementptr inbounds i32, ptr %t2, i32 %t3
  %t5 = load i32, ptr %t4
  %t6 = load i32, ptr %total
  %t7 = add i32 %t6, %t5
  store i32 %t7, ptr %total
  br label %for.inc

for.inc:  ; preds = %for.body
  %t8 = load i32, ptr %i
  %t9 = add i32 %t8, 1
  store i32 %t9, ptr %i
  br label %for.cond

for.end:  ; preds = %for.cond
  %t10 = load i32, ptr %total
  ret i32 %t10
}

define i32 @global_7countUp_i(i32 %n) {
entry:
  %n.addr = alloca i32
  store i32 %n, ptr %n.addr
  br label %until.cond

until.cond:  ; preds = %entry, %until.body
  %t0 = load i32, ptr %n.addr
  %t1 = icmp sge i32 %t0, 10
  br i1 %t1, label %until.end, label %until.body

until.body:  ; preds = %until.cond
  %t2 = load i32, ptr %n.addr
  %t3 = add i32 %t2, 3
  store i32 %t3, ptr %n.addr
  br label %until.cond

until.end:  ; preds = %until.cond
  %t4 = load i32, ptr %n.addr
  ret i32 %t4
}

define i32 @global_9neverRuns_v() {
entry:
  %runs = alloca i32
  store i32 0, ptr %runs
  br label %until.cond

until.cond:  ; preds = %entry, %until.body
  br i1 true, label %until.end, label %until.body

until.body:  ; preds = %until.cond
  %t0 = load i32, ptr %runs
  %t1 = add i32 %t0, 1
  store i32 %t1, ptr %runs
  br label %until.cond

until.end:  ; preds = %until.cond
  %t2 = load i32, ptr %runs
  ret i32 %t2
}

define i32 @global_7skipOdd_v() {
entry:
  %i = alloca i32
  %total = alloca i32
  store i32 0, ptr %i
  store i32 0, ptr %total
  br label %until.cond

until.cond:  ; preds = %entry, %if.then.1, %if.end.1
  br i1 false, label %until.end, label %until.body

until.body:  ; preds = %until.cond
  %t0 = load i32, ptr %i
  %t1 = add i32 %t0, 1
  store i32 %t1, ptr %i
  %t2 = load i32, ptr %i
  %t3 = icmp sgt i32 %t2, 8
  br i1 %t3, label %if.then, label %if.end

if.then:  ; preds = %until.body
  br label %until.end

if.end:  ; preds = %until.body
  %t4 = load i32, ptr %i
  %t5 = srem i32 %t4, 2
  %t6 = icmp eq i32 %t5, 1
  br i1 %t6, label %if.then.1, label %if.end.1

if.then.1:  ; preds = %if.end
  br label %until.cond

if.end.1:  ; preds = %if.end
  %t7 = load i32, ptr %i
  %t8 = load i32, ptr %total
  %t9 = add i32 %t8, %t7
  store i32 %t9, ptr %total
  br label %until.cond

until.end:  ; preds = %until.cond, %if.then
  %t10 = load i32, ptr %total
  ret i32 %t10
}

define i32 @global_11doUntilOnce_v() {
entry:
  %runs = alloca i32
  store i32 0, ptr %runs
  br label %do.body

do.body:  ; preds = %entry, %do.cond
  %t0 = load i32, ptr %runs
  %t1 = add i32 %t0, 1
  store i32 %t1, ptr %runs
  br label %do.cond

do.cond:  ; preds = %do.body
  br i1 true, label %do.end, label %do.body

do.end:  ; preds = %do.cond
  %t2 = load i32, ptr %runs
  ret i32 %t2
}

define i32 @global_5halve_i(i32 %n) {
entry:
  %n.addr = alloca i32
  %steps = alloca i32
  store i32 %n, ptr %n.addr
  store i32 0, ptr %steps
  br label %do.body

do.body:  ; preds = %entry, %do.cond
  %t0 = load i32, ptr %n.addr
  %t1 = sdiv i32 %t0, 2
  store i32 %t1, ptr %n.addr
  %t2 = load i32, ptr %steps
  %t3 = add i32 %t2, 1
  store i32 %t3, ptr %steps
  br label %do.cond

do.cond:  ; preds = %do.body
  %t4 = load i32, ptr %n.addr
  %t5 = icmp sle i32 %t4, 1
  br i1 %t5, label %do.end, label %do.body

do.end:  ; preds = %do.cond
  %t6 = load i32, ptr %steps
  ret i32 %t6
}

define i32 @main() {
entry:
  %x = alloca i32
  %p = alloca ptr
  %t = alloca [3 x i32]
  %r = alloca [4 x i32]
  %op = alloca ptr
  %c = alloca %class.Animal
  %d = alloca %class.Dog
  %heap = alloca ptr
  %letter = alloca i8
  %back = alloca i32
  store i32 7, ptr %x
  store ptr %x, ptr %p
  %t0 = load ptr, ptr %p
  %t1 = load i32, ptr %t0
  %t2 = add i32 %t1, 1
  %t3 = load ptr, ptr %p
  store i32 %t2, ptr %t3
  %t4 = getelementptr inbounds [4 x i8], ptr @.str, i64 0, i64 0
  %t5 = load i32, ptr %x
  %t6 = call i32 (ptr, ...) @printf(ptr %t4, i32 %t5)
  %t7 = getelementptr inbounds [3 x i32], ptr %t, i64 0, i64 0
  store i32 1, ptr %t7
  %t8 = getelementptr inbounds [3 x i32], ptr %t, i64 0, i64 1
  store i32 2, ptr %t8
  %t9 = getelementptr inbounds [3 x i32], ptr %t, i64 0, i64 2
  store i32 3, ptr %t9
  %t10 = getelementptr inbounds [4 x i8], ptr @.str, i64 0, i64 0
  %t11 = getelementptr inbounds [3 x i32], ptr %t, i64 0, i64 0
  %t12 = call i32 @global_3sum_Pi(ptr %t11)
  %t13 = call i32 (ptr, ...) @printf(ptr %t10, i32 %t12)
  %t14 = getelementptr inbounds [4 x i32], ptr %r, i64 0, i64 0
  store i32 4, ptr %t14
  %t15 = getelementptr inbounds [4 x i32], ptr %r, i64 0, i64 1
  store i32 5, ptr %t15
  %t16 = getelementptr inbounds [4 x i32], ptr %r, i64 0, i64 2
  store i32 6, ptr %t16
  %t17 = getelementptr inbounds [4 x i32], ptr %r, i64 0, i64 3
  store i32 7, ptr %t17
  %t18 = getelementptr inbounds [7 x i8], ptr @.str.1, i64 0, i64 0
  %t19 = getelementptr inbounds [4 x i32], ptr %r, i64 0, i64 0
  %t20 = getelementptr inbounds i32, ptr %t19, i32 0
  %t21 = load i32, ptr %t20
  %t22 = getelementptr inbounds [4 x i32], ptr %r, i64 0, i64 0
  %t23 = getelementptr inbounds i32, ptr %t22, i32 3
  %t24 = load i32, ptr %t23
  %t25 = call i32 (ptr, ...) @printf(ptr %t18, i32 %t21, i32 %t24)
  store ptr @global_3add_ii, ptr %op
  %t26 = getelementptr inbounds [4 x i8], ptr @.str.2, i64 0, i64 0
  %t27 = load ptr, ptr %op
  %t28 = call i32 @global_5apply_PFiiiEii(ptr %t27, i32 2, i32 3)
  %t29 = call i32 (ptr, ...) @printf(ptr %t26, i32 %t28)
  store ptr @global_3mul_ii, ptr %op
  %t30 = getelementptr inbounds [4 x i8], ptr @.str, i64 0, i64 0
  %t31 = load ptr, ptr %op
  %t32 = call i32 @global_5apply_PFiiiEii(ptr %t31, i32 2, i32 3)
  %t33 = call i32 (ptr, ...) @printf(ptr %t30, i32 %t32)
  call void @class_6Animal_C_i(ptr %c, i32 4)
  call void @class_3Dog_C_i(ptr %d, i32 9)
  %t34 = call i32 @class_3Dog_4bark_v(ptr %d)
  %t35 = getelementptr inbounds [10 x i8], ptr @.str.3, i64 0, i64 0
  %t36 = call i32 @class_6Animal_6getAge_v(ptr %c)
  %t37 = getelementptr inbounds %class.Dog, ptr %d, i32 0, i32 0
  %t38 = call i32 @class_6Animal_6getAge_v(ptr %t37)
  %t39 = call i32 @class_3Dog_4bark_v(ptr %d)
  %t40 = call i32 (ptr, ...) @printf(ptr %t35, i32 %t36, i32 %t38, i32 %t39)
  %t41 = getelementptr %class.Animal, ptr null, i64 1
  %t42 = ptrtoint ptr %t41 to i64
  %t43 = call ptr @malloc(i64 %t42)
  call void @class_6Animal_C_i(ptr %t43, i32 11)
  store ptr %t43, ptr %heap
  %t44 = getelementptr inbounds [4 x i8], ptr @.str, i64 0, i64 0
  %t45 = load ptr, ptr %heap
  %t46 = call i32 @class_6Animal_6getAge_v(ptr %t45)
  %t47 = call i32 (ptr, ...) @printf(ptr %t44, i32 %t46)
  %t48 = load ptr, ptr %heap
  call void @free(ptr %t48)
  store i8 65, ptr %letter
  %t49 = getelementptr inbounds [4 x i8], ptr @.str.4, i64 0, i64 0
  %t50 = load i8, ptr %letter
  %t51 = sext i8 %t50 to i32
  %t52 = call i32 (ptr, ...) @printf(ptr %t49, i32 %t51)
  store i32 65, ptr %back
  %t53 = getelementptr inbounds [4 x i8], ptr @.str, i64 0, i64 0
  %t54 = load i32, ptr %back
  %t55 = call i32 (ptr, ...) @printf(ptr %t53, i32 %t54)
  %t56 = getelementptr inbounds [10 x i8], ptr @.str.3, i64 0, i64 0
  %t57 = call i32 @global_7countUp_i(i32 1)
  %t58 = call i32 @global_9neverRuns_v()
  %t59 = call i32 @global_7skipOdd_v()
  %t60 = call i32 (ptr, ...) @printf(ptr %t56, i32 %t57, i32 %t58, i32 %t59)
  %t61 = getelementptr inbounds [7 x i8], ptr @.str.1, i64 0, i64 0
  %t62 = call i32 @global_11doUntilOnce_v()
  %t63 = call i32 @global_5halve_i(i32 40)
  %t64 = call i32 (ptr, ...) @printf(ptr %t61, i32 %t62, i32 %t63)
  ret i32 0
}

define void @class_6Animal_C_v(ptr %this) {
entry:
  %this.addr = alloca ptr
  store ptr %this, ptr %this.addr
  %t0 = load ptr, ptr %this.addr
  %t1 = getelementptr inbounds %class.Animal, ptr %t0, i32 0, i32 0
  store i32 0, ptr %t1
  ret void
}

define void @class_6Animal_C_i(ptr %this, i32 %a) {
entry:
  %this.addr = alloca ptr
  %a.addr = alloca i32
  store ptr %this, ptr %this.addr
  %t0 = load ptr, ptr %this.addr
  store i32 %a, ptr %a.addr
  %t1 = load i32, ptr %a.addr
  %t2 = getelementptr inbounds %class.Animal, ptr %t0, i32 0, i32 0
  store i32 %t1, ptr %t2
  ret void
}

define i32 @class_6Animal_6getAge_v(ptr %this) {
entry:
  %this.addr = alloca ptr
  store ptr %this, ptr %this.addr
  %t0 = load ptr, ptr %this.addr
  %t1 = getelementptr inbounds %class.Animal, ptr %t0, i32 0, i32 0
  %t2 = load i32, ptr %t1
  ret i32 %t2
}

define void @class_3Dog_C_i(ptr %this, i32 %a) {
entry:
  %this.addr = alloca ptr
  %a.addr = alloca i32
  store ptr %this, ptr %this.addr
  %t0 = load ptr, ptr %this.addr
  store i32 %a, ptr %a.addr
  %t1 = getelementptr inbounds %class.Dog, ptr %t0, i32 0, i32 0
  call void @class_6Animal_C_v(ptr %t1)
  %t2 = load i32, ptr %a.addr
  %t3 = getelementptr inbounds %class.Dog, ptr %t0, i32 0, i32 0
  %t4 = getelementptr inbounds %class.Animal, ptr %t3, i32 0, i32 0
  store i32 %t2, ptr %t4
  %t5 = getelementptr inbounds %class.Dog, ptr %t0, i32 0, i32 1
  store i32 0, ptr %t5
  ret void
}

define i32 @class_3Dog_4bark_v(ptr %this) {
entry:
  %this.addr = alloca ptr
  store ptr %this, ptr %this.addr
  %t0 = load ptr, ptr %this.addr
  %t1 = getelementptr inbounds %class.Dog, ptr %t0, i32 0, i32 1
  %t2 = load i32, ptr %t1
  %t3 = add i32 %t2, 1
  %t4 = getelementptr inbounds %class.Dog, ptr %t0, i32 0, i32 1
  store i32 %t3, ptr %t4
  %t5 = getelementptr inbounds %class.Dog, ptr %t0, i32 0, i32 1
  %t6 = load i32, ptr %t5
  ret i32 %t6
}

declare ptr @malloc(i64)

declare void @free(ptr)
