; ModuleID = 'compiler_module'
source_filename = "compiler_module"

@x = local_unnamed_addr global i32 0
@0 = private unnamed_addr constant [9 x i8] c"Out = %d\00", align 1

; Function Attrs: nofree nounwind
declare noundef i32 @printf(i8* nocapture noundef readonly, ...) local_unnamed_addr #0

; Function Attrs: mustprogress nofree norecurse nosync nounwind readnone willreturn
define i32 @__user_hello(i32 %abc) local_unnamed_addr #1 {
entry:
  %addtmp = add i32 %abc, 50
  ret i32 %addtmp
}

; Function Attrs: nofree nounwind
define i32 @__user_main() local_unnamed_addr #0 {
entry:
  %x_val.pr = load i32, i32* @x, align 4
  %cmptmp14 = icmp slt i32 %x_val.pr, 10
  br i1 %cmptmp14, label %while.body, label %while.after

while.body:                                       ; preds = %entry, %while.cond.backedge
  %printf_call = tail call i32 (i8*, ...) @printf(i8* nonnull dereferenceable(1) getelementptr inbounds ([9 x i8], [9 x i8]* @0, i64 0, i64 0), i32 3)
  %x_val2 = load i32, i32* @x, align 4
  switch i32 %x_val2, label %ifcont11 [
    i32 1, label %while.cond.backedge
    i32 8, label %while.after
  ]

while.cond.backedge:                              ; preds = %while.body, %ifcont11
  %storemerge = phi i32 [ %addtmp13, %ifcont11 ], [ 3, %while.body ]
  store i32 %storemerge, i32* @x, align 4
  %cmptmp = icmp slt i32 %storemerge, 10
  br i1 %cmptmp, label %while.body, label %while.after

ifcont11:                                         ; preds = %while.body
  %addtmp13 = add i32 %x_val2, 1
  br label %while.cond.backedge

while.after:                                      ; preds = %while.cond.backedge, %while.body, %entry
  ret i32 0
}

attributes #0 = { nofree nounwind }
attributes #1 = { mustprogress nofree norecurse nosync nounwind readnone willreturn }
