; ModuleID = 'test/test26_ir_classes.ckp'
source_filename = "test/test26_ir_classes.ckp"

%class.Animal = type { i32 }
%class.Dog = type { %class.Animal, i1 }
%class.Tag = type { i32 }
%class.Pet = type { %class.Animal, %class.Tag, i32 }
%class.Counter = type { %struct.Counter.Inner }
%struct.Counter.Inner = type { i32 }

@class_7Counter_5total = global i32 0
@.str = private unnamed_addr constant [10 x i8] c"Animal()\0A\00"
@.str.1 = private unnamed_addr constant [12 x i8] c"Animal(%d)\0A\00"
@.str.2 = private unnamed_addr constant [13 x i8] c"~Animal(%d)\0A\00"
@.str.3 = private unnamed_addr constant [7 x i8] c"Dog()\0A\00"
@.str.4 = private unnamed_addr constant [8 x i8] c"~Dog()\0A\00"
@.str.5 = private unnamed_addr constant [11 x i8] c"dog %d %d\0A\00"
@.str.6 = private unnamed_addr constant [14 x i8] c"pet %d %d %d\0A\00"
@.str.7 = private unnamed_addr constant [15 x i8] c"counter %d %d\0A\00"

declare i32 @printf(ptr, ...)

define i32 @main() {
entry:
  %d = alloca %class.Dog
  %a = alloca %class.Animal
  %pet = alloca %class.Pet
  %heap = alloca ptr
  %c = alloca %class.Counter
  %retval = alloca i32
  call void @class_3Dog_C_v(ptr %d)
  call void @class_3Dog_4bark_v(ptr %d)
  %t0 = getelementptr inbounds %class.Dog, ptr %d, i32 0, i32 0
  call void @class_6Animal_6setAge_i(ptr %t0, i32 5)
  %t1 = getelementptr inbounds [11 x i8], ptr @.str.5, i64 0, i64 0
  %t2 = getelementptr inbounds %class.Dog, ptr %d, i32 0, i32 0
  %t3 = getelementptr inbounds %class.Animal, ptr %t2, i32 0, i32 0
  %t4 = load i32, ptr %t3
  %t5 = getelementptr inbounds %class.Dog, ptr %d, i32 0, i32 1
  %t6 = load i1, ptr %t5
  %t7 = zext i1 %t6 to i32
  %t8 = call i32 (ptr, ...) @printf(ptr %t1, i32 %t4, i32 %t7)
  call void @class_6Animal_C_i(ptr %a, i32 9)
  call void @class_6Animal_D_v(ptr %a)
  call void @class_3Dog_D_v(ptr %d)
  call void @class_3Pet_C_v(ptr %pet)
  %t9 = getelementptr inbounds %class.Pet, ptr %pet, i32 0, i32 0
  %t10 = getelementptr inbounds %class.Animal, ptr %t9, i32 0, i32 0
  store i32 2, ptr %t10
  %t11 = getelementptr inbounds %class.Pet, ptr %pet, i32 0, i32 1
  %t12 = getelementptr inbounds %class.Tag, ptr %t11, i32 0, i32 0
  store i32 77, ptr %t12
  %t13 = getelementptr inbounds %class.Pet, ptr %pet, i32 0, i32 2
  store i32 3, ptr %t13
  %t14 = getelementptr inbounds [14 x i8], ptr @.str.6, i64 0, i64 0
  %t15 = getelementptr inbounds %class.Pet, ptr %pet, i32 0, i32 0
  %t16 = getelementptr inbounds %class.Animal, ptr %t15, i32 0, i32 0
  %t17 = load i32, ptr %t16
  %t18 = getelementptr inbounds %class.Pet, ptr %pet, i32 0, i32 1
  %t19 = getelementptr inbounds %class.Tag, ptr %t18, i32 0, i32 0
  %t20 = load i32, ptr %t19
  %t21 = getelementptr inbounds %class.Pet, ptr %pet, i32 0, i32 2
  %t22 = load i32, ptr %t21
  %t23 = call i32 (ptr, ...) @printf(ptr %t14, i32 %t17, i32 %t20, i32 %t22)
  %t24 = getelementptr %class.Animal, ptr null, i64 1
  %t25 = ptrtoint ptr %t24 to i64
  %t26 = call ptr @malloc(i64 %t25)
  call void @class_6Animal_C_i(ptr %t26, i32 4)
  store ptr %t26, ptr %heap
  %t27 = load ptr, ptr %heap
  call void @class_6Animal_6setAge_i(ptr %t27, i32 6)
  %t28 = load ptr, ptr %heap
  %t29 = icmp eq ptr %t28, null
  br i1 %t29, label %delete.end, label %delete.notnull

delete.notnull:  ; preds = %entry
  call void @class_6Animal_D_v(ptr %t28)
  call void @free(ptr %t28)
  br label %delete.end

delete.end:  ; preds = %entry, %delete.notnull
  %t30 = getelementptr inbounds %class.Counter, ptr %c, i32 0, i32 0
  %t31 = getelementptr inbounds %struct.Counter.Inner, ptr %t30, i32 0, i32 0
  store i32 11, ptr %t31
  %t32 = call i32 @class_7Counter_4next_v()
  %t33 = getelementptr inbounds [15 x i8], ptr @.str.7, i64 0, i64 0
  %t34 = call i32 @class_7Counter_4next_v()
  %t35 = getelementptr inbounds %class.Counter, ptr %c, i32 0, i32 0
  %t36 = getelementptr inbounds %struct.Counter.Inner, ptr %t35, i32 0, i32 0
  %t37 = load i32, ptr %t36
  %t38 = call i32 (ptr, ...) @printf(ptr %t33, i32 %t34, i32 %t37)
  store i32 0, ptr %retval
  call void @class_3Pet_D_v(ptr %pet)
  %t39 = load i32, ptr %retval
  ret i32 %t39
}

define void @class_6Animal_C_v(ptr %this) {
entry:
  %this.addr = alloca ptr
  store ptr %this, ptr %this.addr
  %t0 = load ptr, ptr %this.addr
  %t1 = getelementptr inbounds %class.Animal, ptr %t0, i32 0, i32 0
  store i32 1, ptr %t1
  %t2 = getelementptr inbounds [10 x i8], ptr @.str, i64 0, i64 0
  %t3 = call i32 (ptr, ...) @printf(ptr %t2)
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
  %t3 = getelementptr inbounds [12 x i8], ptr @.str.1, i64 0, i64 0
  %t4 = load i32, ptr %a.addr
  %t5 = call i32 (ptr, ...) @printf(ptr %t3, i32 %t4)
  ret void
}

define void @class_6Animal_D_v(ptr %this) {
entry:
  %this.addr = alloca ptr
  store ptr %this, ptr %this.addr
  %t0 = load ptr, ptr %this.addr
  %t1 = getelementptr inbounds [13 x i8], ptr @.str.2, i64 0, i64 0
  %t2 = getelementptr inbounds %class.Animal, ptr %t0, i32 0, i32 0
  %t3 = load i32, ptr %t2
  %t4 = call i32 (ptr, ...) @printf(ptr %t1, i32 %t3)
  br label %dtor.end

dtor.end:  ; preds = %entry
  ret void
}

define void @class_6Animal_6setAge_i(ptr %this, i32 %a) {
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

define void @class_3Dog_C_v(ptr %this) {
entry:
  %this.addr = alloca ptr
  store ptr %this, ptr %this.addr
  %t0 = load ptr, ptr %this.addr
  %t1 = getelementptr inbounds %class.Dog, ptr %t0, i32 0, i32 0
  call void @class_6Animal_C_v(ptr %t1)
  %t2 = getelementptr inbounds %class.Dog, ptr %t0, i32 0, i32 1
  store i1 false, ptr %t2
  %t3 = getelementptr inbounds [7 x i8], ptr @.str.3, i64 0, i64 0
  %t4 = call i32 (ptr, ...) @printf(ptr %t3)
  ret void
}

define void @class_3Dog_D_v(ptr %this) {
entry:
  %this.addr = alloca ptr
  store ptr %this, ptr %this.addr
  %t0 = load ptr, ptr %this.addr
  %t1 = getelementptr inbounds [8 x i8], ptr @.str.4, i64 0, i64 0
  %t2 = call i32 (ptr, ...) @printf(ptr %t1)
  br label %dtor.end

dtor.end:  ; preds = %entry
  %t3 = getelementptr inbounds %class.Dog, ptr %t0, i32 0, i32 0
  call void @class_6Animal_D_v(ptr %t3)
  ret void
}

define void @class_3Dog_4bark_v(ptr %this) {
entry:
  %this.addr = alloca ptr
  store ptr %this, ptr %this.addr
  %t0 = load ptr, ptr %this.addr
  %t1 = getelementptr inbounds %class.Dog, ptr %t0, i32 0, i32 1
  store i1 true, ptr %t1
  ret void
}

define i32 @class_7Counter_4next_v() {
entry:
  %t0 = load i32, ptr @class_7Counter_5total
  %t1 = add i32 %t0, 1
  store i32 %t1, ptr @class_7Counter_5total
  %t2 = load i32, ptr @class_7Counter_5total
  ret i32 %t2
}

define void @class_3Pet_C_v(ptr %this) {
entry:
  %this.addr = alloca ptr
  store ptr %this, ptr %this.addr
  %t0 = load ptr, ptr %this.addr
  %t1 = getelementptr inbounds %class.Pet, ptr %t0, i32 0, i32 0
  call void @class_6Animal_C_v(ptr %t1)
  %t2 = getelementptr inbounds %class.Pet, ptr %t0, i32 0, i32 1
  ret void
}

declare ptr @malloc(i64)

declare void @free(ptr)

define void @class_3Pet_D_v(ptr %this) {
entry:
  %this.addr = alloca ptr
  store ptr %this, ptr %this.addr
  %t0 = load ptr, ptr %this.addr
  br label %dtor.end

dtor.end:  ; preds = %entry
  %t1 = getelementptr inbounds %class.Pet, ptr %t0, i32 0, i32 1
  %t2 = getelementptr inbounds %class.Pet, ptr %t0, i32 0, i32 0
  call void @class_6Animal_D_v(ptr %t2)
  ret void
}
