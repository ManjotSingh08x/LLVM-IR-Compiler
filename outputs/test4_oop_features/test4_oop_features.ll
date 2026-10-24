; ModuleID = 'test/test4_oop_features.ckp'
source_filename = "test/test4_oop_features.ckp"

%class.Animal = type { i32, i32, i32 }
%class.Dog = type { %class.Animal, i1 }
%class.Cat = type { %class.Animal, i1 }
%class.Trackable = type { i32 }
%class.Tagged = type { %class.Animal, i32 }
%class.Registered = type { %class.Animal, %class.Trackable, i32 }
%struct.BaseRec = type { i32 }
%struct.DerivedRec = type { %struct.BaseRec, i32 }
%class.Outer = type { %struct.Outer.Inner, %class.Outer.Nested }
%struct.Outer.Inner = type { i32 }
%class.Outer.Nested = type { i32 }

@class_6Animal_7created = global i32 0

define i32 @main() {
entry:
  %a = alloca ptr
  %d = alloca %class.Dog
  %c = alloca %class.Cat
  %retval = alloca i32
  %t0 = getelementptr %class.Animal, ptr null, i64 1
  %t1 = ptrtoint ptr %t0 to i64
  %t2 = call ptr @malloc(i64 %t1)
  call void @class_6Animal_C_v(ptr %t2)
  store ptr %t2, ptr %a
  %t3 = load ptr, ptr %a
  call void @class_6Animal_6setAge_i(ptr %t3, i32 3)
  %t4 = load ptr, ptr %a
  %t5 = icmp eq ptr %t4, null
  br i1 %t5, label %delete.end, label %delete.notnull

delete.notnull:  ; preds = %entry
  call void @class_6Animal_D_v(ptr %t4)
  call void @free(ptr %t4)
  br label %delete.end

delete.end:  ; preds = %entry, %delete.notnull
  call void @class_3Dog_C_v(ptr %d)
  call void @class_3Dog_4bark_v(ptr %d)
  %t6 = getelementptr inbounds %class.Dog, ptr %d, i32 0, i32 0
  call void @class_6Animal_6setAge_i(ptr %t6, i32 5)
  call void @class_3Cat_C_v(ptr %c)
  store i32 0, ptr %retval
  call void @class_3Cat_D_v(ptr %c)
  call void @class_3Dog_D_v(ptr %d)
  %t7 = load i32, ptr %retval
  ret i32 %t7
}

define i32 @global_12buildObjects_v() {
entry:
  %plain = alloca %class.Animal
  %withOne = alloca %class.Animal
  %withTwo = alloca %class.Animal
  %tagged = alloca %class.Tagged
  %registered = alloca %class.Registered
  %outer = alloca %class.Outer
  %retval = alloca i32
  call void @class_6Animal_C_v(ptr %plain)
  call void @class_6Animal_C_i(ptr %withOne, i32 3)
  call void @class_6Animal_C_ii(ptr %withTwo, i32 3, i32 4)
  call void @class_6Tagged_C_v(ptr %tagged)
  call void @class_10Registered_C_v(ptr %registered)
  %t0 = getelementptr inbounds %class.Outer, ptr %outer, i32 0, i32 0
  %t1 = getelementptr inbounds %struct.Outer.Inner, ptr %t0, i32 0, i32 0
  store i32 1, ptr %t1
  %t2 = getelementptr inbounds %class.Outer, ptr %outer, i32 0, i32 1
  %t3 = getelementptr inbounds %class.Outer.Nested, ptr %t2, i32 0, i32 0
  store i32 2, ptr %t3
  %t4 = getelementptr inbounds %class.Animal, ptr %withOne, i32 0, i32 0
  %t5 = load i32, ptr %t4
  %t6 = getelementptr inbounds %class.Animal, ptr %withTwo, i32 0, i32 0
  %t7 = load i32, ptr %t6
  %t8 = add i32 %t5, %t7
  %t9 = getelementptr inbounds %class.Tagged, ptr %tagged, i32 0, i32 1
  %t10 = load i32, ptr %t9
  %t11 = add i32 %t8, %t10
  %t12 = getelementptr inbounds %class.Registered, ptr %registered, i32 0, i32 2
  %t13 = load i32, ptr %t12
  %t14 = add i32 %t11, %t13
  store i32 %t14, ptr %retval
  call void @class_10Registered_D_v(ptr %registered)
  call void @class_6Tagged_D_v(ptr %tagged)
  call void @class_6Animal_D_v(ptr %withTwo)
  call void @class_6Animal_D_v(ptr %withOne)
  call void @class_6Animal_D_v(ptr %plain)
  %t15 = load i32, ptr %retval
  ret i32 %t15
}

define void @class_6Animal_C_v(ptr %this) {
entry:
  %this.addr = alloca ptr
  store ptr %this, ptr %this.addr
  %t0 = load ptr, ptr %this.addr
  ret void
}

define void @class_6Animal_C_i(ptr %this, i32 %arg0) {
entry:
  %this.addr = alloca ptr
  store ptr %this, ptr %this.addr
  %t0 = load ptr, ptr %this.addr
  ret void
}

define void @class_6Animal_C_ii(ptr %this, i32 %arg0, i32 %arg1) {
entry:
  %this.addr = alloca ptr
  store ptr %this, ptr %this.addr
  %t0 = load ptr, ptr %this.addr
  ret void
}

define void @class_6Animal_D_v(ptr %this) {
entry:
  %this.addr = alloca ptr
  store ptr %this, ptr %this.addr
  %t0 = load ptr, ptr %this.addr
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

define i32 @class_6Animal_6hidden_v(ptr %this) {
entry:
  %this.addr = alloca ptr
  store ptr %this, ptr %this.addr
  %t0 = load ptr, ptr %this.addr
  %t1 = getelementptr inbounds %class.Animal, ptr %t0, i32 0, i32 2
  %t2 = load i32, ptr %t1
  %t3 = getelementptr inbounds %class.Animal, ptr %t0, i32 0, i32 1
  %t4 = load i32, ptr %t3
  %t5 = add i32 %t2, %t4
  ret i32 %t5
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
  ret void
}

define void @class_3Dog_C_b(ptr %this, i1 %l) {
entry:
  %this.addr = alloca ptr
  %l.addr = alloca i1
  store ptr %this, ptr %this.addr
  %t0 = load ptr, ptr %this.addr
  store i1 %l, ptr %l.addr
  %t1 = getelementptr inbounds %class.Dog, ptr %t0, i32 0, i32 0
  call void @class_6Animal_C_v(ptr %t1)
  %t2 = load i1, ptr %l.addr
  %t3 = getelementptr inbounds %class.Dog, ptr %t0, i32 0, i32 1
  store i1 %t2, ptr %t3
  ret void
}

define void @class_3Dog_D_v(ptr %this) {
entry:
  %this.addr = alloca ptr
  store ptr %this, ptr %this.addr
  %t0 = load ptr, ptr %this.addr
  %t1 = getelementptr inbounds %class.Dog, ptr %t0, i32 0, i32 1
  store i1 false, ptr %t1
  br label %dtor.end

dtor.end:  ; preds = %entry
  %t2 = getelementptr inbounds %class.Dog, ptr %t0, i32 0, i32 0
  call void @class_6Animal_D_v(ptr %t2)
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

declare ptr @malloc(i64)

declare void @free(ptr)

define void @class_3Cat_C_v(ptr %this) {
entry:
  %this.addr = alloca ptr
  store ptr %this, ptr %this.addr
  %t0 = load ptr, ptr %this.addr
  %t1 = getelementptr inbounds %class.Cat, ptr %t0, i32 0, i32 0
  call void @class_6Animal_C_v(ptr %t1)
  ret void
}

define void @class_3Cat_D_v(ptr %this) {
entry:
  %this.addr = alloca ptr
  store ptr %this, ptr %this.addr
  %t0 = load ptr, ptr %this.addr
  br label %dtor.end

dtor.end:  ; preds = %entry
  %t1 = getelementptr inbounds %class.Cat, ptr %t0, i32 0, i32 0
  call void @class_6Animal_D_v(ptr %t1)
  ret void
}

define void @class_6Tagged_C_v(ptr %this) {
entry:
  %this.addr = alloca ptr
  store ptr %this, ptr %this.addr
  %t0 = load ptr, ptr %this.addr
  %t1 = getelementptr inbounds %class.Tagged, ptr %t0, i32 0, i32 0
  call void @class_6Animal_C_v(ptr %t1)
  ret void
}

define void @class_10Registered_C_v(ptr %this) {
entry:
  %this.addr = alloca ptr
  store ptr %this, ptr %this.addr
  %t0 = load ptr, ptr %this.addr
  %t1 = getelementptr inbounds %class.Registered, ptr %t0, i32 0, i32 0
  call void @class_6Animal_C_v(ptr %t1)
  %t2 = getelementptr inbounds %class.Registered, ptr %t0, i32 0, i32 1
  ret void
}

define void @class_10Registered_D_v(ptr %this) {
entry:
  %this.addr = alloca ptr
  store ptr %this, ptr %this.addr
  %t0 = load ptr, ptr %this.addr
  br label %dtor.end

dtor.end:  ; preds = %entry
  %t1 = getelementptr inbounds %class.Registered, ptr %t0, i32 0, i32 1
  %t2 = getelementptr inbounds %class.Registered, ptr %t0, i32 0, i32 0
  call void @class_6Animal_D_v(ptr %t2)
  ret void
}

define void @class_6Tagged_D_v(ptr %this) {
entry:
  %this.addr = alloca ptr
  store ptr %this, ptr %this.addr
  %t0 = load ptr, ptr %this.addr
  br label %dtor.end

dtor.end:  ; preds = %entry
  %t1 = getelementptr inbounds %class.Tagged, ptr %t0, i32 0, i32 0
  call void @class_6Animal_D_v(ptr %t1)
  ret void
}
