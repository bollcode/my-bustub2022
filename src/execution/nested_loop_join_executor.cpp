//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// nested_loop_join_executor.cpp
//
// Identification: src/execution/nested_loop_join_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "execution/executors/nested_loop_join_executor.h"

namespace bustub {

NestedLoopJoinExecutor::NestedLoopJoinExecutor(ExecutorContext *exec_ctx, const NestedLoopJoinPlanNode *plan,
                                               std::unique_ptr<AbstractExecutor> &&left_executor,
                                               std::unique_ptr<AbstractExecutor> &&right_executor)
    : AbstractExecutor(exec_ctx),
    plan_(plan),
    left_executor_(std::move(left_executor)), 
    right_executor_(std::move(right_executor)) {}

void NestedLoopJoinExecutor::Init() { left_executor_->Init(); }

bool NestedLoopJoinExecutor::Next(Tuple *tuple, RID *rid) {
    if (!tmp_results_.empty()) {
    *tuple = tmp_results_.front();
    tmp_results_.pop();
    return true;
  }

  Tuple left_tuple;
  RID left_rid;

  Tuple right_tuple;
  RID right_rid;
   // 获取左表的一个tuple
  if (!left_executor_->Next(&left_tuple, &left_rid)) {
    return false;
  }
  // 重新设置这个右孩子的迭代器指向第一个tuple
  right_executor_->Init();
  // 不断的获取右表的tuple
  while (right_executor_->Next(&right_tuple, &right_rid)) {
    if (plan_->Predicate() == nullptr || plan_->Predicate()
                                             ->EvaluateJoin(&left_tuple, left_executor_->GetOutputSchema(),
                                                            &right_tuple, right_executor_->GetOutputSchema())
                                             .GetAs<bool>()) {
      std::vector<Value> output;
      for (const auto &col : GetOutputSchema()->GetColumns()) {
        output.push_back(col.GetExpr()->EvaluateJoin(&left_tuple, left_executor_->GetOutputSchema(), &right_tuple,
                                                     right_executor_->GetOutputSchema()));
      }
      tmp_results_.push(Tuple(output, GetOutputSchema()));
    }
  }
  return Next(tuple, rid);
}

}  // namespace bustub
