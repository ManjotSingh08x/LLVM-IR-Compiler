; ModuleID = 'test/test12_extra_semicolon.ckp'
source_filename = "test/test12_extra_semicolon.ckp"

define i32 @global_10extracolon_v() {
entry:
  %x = alloca i32
  store i32 3, ptr %x
  %t0 = load i32, ptr %x
  ret i32 %t0
}
