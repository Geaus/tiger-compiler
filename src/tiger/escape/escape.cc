#include "tiger/escape/escape.h"
#include "tiger/absyn/absyn.h"

namespace esc {
void EscFinder::FindEscape() { absyn_tree_->Traverse(env_.get()); }
} // namespace esc

namespace absyn {

void AbsynTree::Traverse(esc::EscEnvPtr env) {
  /* TODO: Put your lab5 code here */
  root_->Traverse(env,0);

  return;
}

void SimpleVar::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  esc::EscapeEntry* entry = env->Look(sym_);
  if(entry){
    if (depth > entry->depth_){
     *(entry->escape_) = true;
    }
  }

  return;
}

void FieldVar::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  var_->Traverse(env, depth);
  esc::EscapeEntry* entry = env->Look(sym_);
  if(entry){
    if (depth > entry->depth_){
     *(entry->escape_) = true;
    }
  }

  return;
}

void SubscriptVar::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */

  var_->Traverse(env, depth);
  subscript_->Traverse(env, depth);

  return;
}

void VarExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  var_->Traverse(env, depth);
  return;
}

void NilExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  return;
}

void IntExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  return;
}

void StringExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  return;
}

void CallExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  const auto args = args_->GetList();
  for (auto arg : args){
    arg->Traverse(env, depth);
  }

  return;
}

void OpExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  left_->Traverse(env, depth);
  right_->Traverse(env, depth);

  return;
}

void RecordExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  const auto efields = fields_->GetList();
  for (auto efield : efields){
    efield->exp_->Traverse(env, depth);
  }

  return;
}

void SeqExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  const auto exp_list = seq_->GetList();
  for (auto exp : exp_list) {
    exp->Traverse(env, depth);
  }

  return;
}

void AssignExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  var_->Traverse(env, depth);
  exp_->Traverse(env, depth);

  return;
}

void IfExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  test_->Traverse(env, depth);
  then_->Traverse(env, depth);

  if(elsee_ != nullptr){
    elsee_->Traverse(env, depth);
  }

  return;
}

void WhileExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  test_->Traverse(env, depth);

  if (body_ != nullptr) {
    body_->Traverse(env, depth);
  }

  return;
}

void ForExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  escape_ = false;
  env->Enter(var_, new esc::EscapeEntry(depth, &(escape_)));

  lo_->Traverse(env, depth);
  hi_->Traverse(env, depth);

  if (body_ != nullptr) {
    body_->Traverse(env, depth);
  }

  return;
}

void BreakExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  return;
}

void LetExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  const auto dec_list = decs_->GetList();
  for (auto dec : dec_list){
    dec->Traverse(env, depth);
  }

  if (body_ != nullptr) {
    body_->Traverse(env, depth);
  }

  return;
}

void ArrayExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  size_->Traverse(env, depth);
  init_->Traverse(env, depth);

  return;
}

void VoidExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  return;
}

void FunctionDec::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  const auto fun_list= functions_->GetList();
  for (auto func : fun_list) {
    env->BeginScope();

    const auto param_list = func->params_->GetList();
    int cur_depth = depth + 1;

    for (auto param : param_list) {
      param->escape_ = false;
      env->Enter(param->name_, new esc::EscapeEntry(cur_depth, &param->escape_));
    };

    if(func->body_ != nullptr){
      func->body_->Traverse(env, cur_depth);
    }

    env->EndScope();
  }

  return;
}

void VarDec::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  init_->Traverse(env, depth);

  escape_ = false;
  env->Enter(var_, new esc::EscapeEntry(depth, &escape_));

  return;
}

void TypeDec::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  return;
}

} // namespace absyn
