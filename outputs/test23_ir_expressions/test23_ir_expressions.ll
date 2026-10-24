; ModuleID = 'test/test23_ir_expressions.ckp'
source_filename = "test/test23_ir_expressions.ckp"

@.str = private unnamed_addr constant [16 x i8] c"%d %d %d %d %d\0A\00"
@.str.1 = private unnamed_addr constant [7 x i8] c"%d %d\0A\00"
@.str.2 = private unnamed_addr constant [4 x i8] c"%d\0A\00"
@.str.3 = private unnamed_addr constant [10 x i8] c"%d %d %d\0A\00"
@.str.4 = private unnamed_addr constant [7 x i8] c"%c %f\0A\00"

declare i32 @printf(ptr, ...)

define i32 @main() {
entry:
  %a = alloca i32
  %b = alloca i32
  %t = alloca i1
  %and.result = alloca i1
  %f = alloca i1
  %or.result = alloca i1
  %bigger = alloca i32
  %cond.result = alloca ptr
  %c = alloca i32
  %i = alloca i32
  %post = alloca i32
  %pre = alloca i32
  %letter = alloca i8
  %half = alloca float
  %sum = alloca double
  store i32 17, ptr %a
  store i32 5, ptr %b
  %t0 = getelementptr inbounds [16 x i8], ptr @.str, i64 0, i64 0
  %t1 = load i32, ptr %a
  %t2 = load i32, ptr %b
  %t3 = add i32 %t1, %t2
  %t4 = load i32, ptr %a
  %t5 = load i32, ptr %b
  %t6 = sub i32 %t4, %t5
  %t7 = load i32, ptr %a
  %t8 = load i32, ptr %b
  %t9 = mul i32 %t7, %t8
  %t10 = load i32, ptr %a
  %t11 = load i32, ptr %b
  %t12 = sdiv i32 %t10, %t11
  %t13 = load i32, ptr %a
  %t14 = load i32, ptr %b
  %t15 = srem i32 %t13, %t14
  %t16 = call i32 (ptr, ...) @printf(ptr %t0, i32 %t3, i32 %t6, i32 %t9, i32 %t12, i32 %t15)
  %t17 = getelementptr inbounds [16 x i8], ptr @.str, i64 0, i64 0
  %t18 = load i32, ptr %a
  %t19 = load i32, ptr %b
  %t20 = and i32 %t18, %t19
  %t21 = load i32, ptr %a
  %t22 = load i32, ptr %b
  %t23 = or i32 %t21, %t22
  %t24 = load i32, ptr %a
  %t25 = load i32, ptr %b
  %t26 = xor i32 %t24, %t25
  %t27 = load i32, ptr %a
  %t28 = shl i32 %t27, 2
  %t29 = load i32, ptr %a
  %t30 = ashr i32 %t29, 1
  %t31 = call i32 (ptr, ...) @printf(ptr %t17, i32 %t20, i32 %t23, i32 %t26, i32 %t28, i32 %t30)
  %t32 = load i32, ptr %a
  %t33 = load i32, ptr %b
  %t34 = icmp sgt i32 %t32, %t33
  br i1 %t34, label %land.rhs, label %land.short

land.short:  ; preds = %entry
  store i1 false, ptr %and.result
  br label %land.end

land.rhs:  ; preds = %entry
  %t35 = load i32, ptr %b
  %t36 = icmp ne i32 %t35, 0
  store i1 %t36, ptr %and.result
  br label %land.end

land.end:  ; preds = %land.short, %land.rhs
  %t37 = load i1, ptr %and.result
  store i1 %t37, ptr %t
  %t38 = load i32, ptr %a
  %t39 = load i32, ptr %b
  %t40 = icmp slt i32 %t38, %t39
  br i1 %t40, label %lor.short, label %lor.rhs

lor.short:  ; preds = %land.end
  store i1 true, ptr %or.result
  br label %lor.end

lor.rhs:  ; preds = %land.end
  %t41 = load i32, ptr %a
  %t42 = icmp eq i32 %t41, 17
  %t43 = xor i1 %t42, true
  store i1 %t43, ptr %or.result
  br label %lor.end

lor.end:  ; preds = %lor.short, %lor.rhs
  %t44 = load i1, ptr %or.result
  store i1 %t44, ptr %f
  %t45 = getelementptr inbounds [7 x i8], ptr @.str.1, i64 0, i64 0
  %t46 = load i1, ptr %t
  %t47 = zext i1 %t46 to i32
  %t48 = load i1, ptr %f
  %t49 = zext i1 %t48 to i32
  %t50 = call i32 (ptr, ...) @printf(ptr %t45, i32 %t47, i32 %t49)
  %t51 = load i32, ptr %a
  %t52 = load i32, ptr %b
  %t53 = icmp sgt i32 %t51, %t52
  br i1 %t53, label %cond.true, label %cond.false

cond.true:  ; preds = %lor.end
  store ptr %a, ptr %cond.result
  br label %cond.end

cond.false:  ; preds = %lor.end
  store ptr %b, ptr %cond.result
  br label %cond.end

cond.end:  ; preds = %cond.true, %cond.false
  %t54 = load ptr, ptr %cond.result
  %t55 = load i32, ptr %t54
  store i32 %t55, ptr %bigger
  %t56 = getelementptr inbounds [4 x i8], ptr @.str.2, i64 0, i64 0
  %t57 = load i32, ptr %bigger
  %t58 = call i32 (ptr, ...) @printf(ptr %t56, i32 %t57)
  store i32 10, ptr %c
  %t59 = load i32, ptr %c
  %t60 = add i32 %t59, 5
  store i32 %t60, ptr %c
  %t61 = load i32, ptr %c
  %t62 = sub i32 %t61, 3
  store i32 %t62, ptr %c
  %t63 = load i32, ptr %c
  %t64 = mul i32 %t63, 2
  store i32 %t64, ptr %c
  %t65 = load i32, ptr %c
  %t66 = sdiv i32 %t65, 4
  store i32 %t66, ptr %c
  %t67 = load i32, ptr %c
  %t68 = srem i32 %t67, 4
  store i32 %t68, ptr %c
  %t69 = getelementptr inbounds [4 x i8], ptr @.str.2, i64 0, i64 0
  %t70 = load i32, ptr %c
  %t71 = call i32 (ptr, ...) @printf(ptr %t69, i32 %t70)
  store i32 3, ptr %i
  %t72 = load i32, ptr %i
  %t73 = add i32 %t72, 1
  store i32 %t73, ptr %i
  store i32 %t72, ptr %post
  %t74 = load i32, ptr %i
  %t75 = add i32 %t74, 1
  store i32 %t75, ptr %i
  %t76 = load i32, ptr %i
  store i32 %t76, ptr %pre
  %t77 = load i32, ptr %i
  %t78 = sub i32 %t77, 1
  store i32 %t78, ptr %i
  %t79 = load i32, ptr %i
  %t80 = sub i32 %t79, 1
  store i32 %t80, ptr %i
  %t81 = getelementptr inbounds [10 x i8], ptr @.str.3, i64 0, i64 0
  %t82 = load i32, ptr %post
  %t83 = load i32, ptr %pre
  %t84 = load i32, ptr %i
  %t85 = call i32 (ptr, ...) @printf(ptr %t81, i32 %t82, i32 %t83, i32 %t84)
  store i8 97, ptr %letter
  %t86 = load i8, ptr %letter
  %t87 = add i8 %t86, 1
  store i8 %t87, ptr %letter
  %t88 = fptrunc double 0x3FE0000000000000 to float
  store float %t88, ptr %half
  %t89 = load float, ptr %half
  %t90 = fpext float %t89 to double
  %t91 = fadd double %t90, 0x3FF4000000000000
  store double %t91, ptr %sum
  %t92 = getelementptr inbounds [7 x i8], ptr @.str.4, i64 0, i64 0
  %t93 = load i8, ptr %letter
  %t94 = sext i8 %t93 to i32
  %t95 = load double, ptr %sum
  %t96 = call i32 (ptr, ...) @printf(ptr %t92, i32 %t94, double %t95)
  ret i32 0
}
