#include "tiger/absyn/absyn.h"
#include "tiger/semant/semant.h"

namespace absyn {

void AbsynTree::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                           err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  this->root_->SemAnalyze(venv,tenv,0,errormsg);
}

type::Ty *SimpleVar::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                                int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  env::EnvEntry *entry = venv->Look(sym_);
  if(entry ==nullptr || typeid(*entry) != typeid(env::VarEntry)){
    errormsg->Error(pos_,"undefined variable %s",sym_->Name().data());
  }
  else{
    return (static_cast<env::VarEntry *>(entry))->ty_->ActualTy();
  }
  return type::IntTy::Instance();

}

type::Ty *FieldVar::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                               int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty *ty = var_->SemAnalyze(venv, tenv, labelcount, errormsg)->ActualTy();
  
  if (typeid(*ty) != typeid(type::RecordTy)) {
    errormsg->Error(pos_, "not a record type");
  }
  else{
    type::RecordTy *rec_ty = static_cast<type::RecordTy *>(ty);
    type::FieldList *fields = rec_ty->fields_;

    type::Ty *result;
    bool isExist = false;

    auto iter = fields->GetList().begin();
    int size = fields->GetList().size();

    for (; size>0; size--) {

      if (sym_ == (*iter)->name_) {
        result = (*iter)->ty_;
        isExist = true;
        break;
      }
      iter++;
    }
    if (!isExist) {
      errormsg->Error(pos_, "field %s doesn't exist", sym_->Name().data());
    }
    else {
      return result->ActualTy();
    }
  }
  return type::IntTy::Instance();
}

type::Ty *SubscriptVar::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   int labelcount,
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty *sub_ty = subscript_->SemAnalyze(venv, tenv, labelcount, errormsg);
  type::Ty *var_ty = var_->SemAnalyze(venv, tenv, labelcount, errormsg);

  if (typeid(*sub_ty) != typeid(type::IntTy)) {
    errormsg->Error(pos_, "subscript must be an integer");
    return type::IntTy::Instance();
  }
  if (typeid(*var_ty) != typeid(type::ArrayTy)) {
    errormsg->Error(pos_, "array type required");
    return type::IntTy::Instance();
  } 

  type::ArrayTy *arr = static_cast<type::ArrayTy *>(var_ty);
  type::Ty *result = arr->ty_->ActualTy();

  return result;
}

type::Ty *VarExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty *result = var_->SemAnalyze(venv, tenv, labelcount, errormsg);
  return result;
}

type::Ty *NilExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  return type::NilTy::Instance();
}

type::Ty *IntExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  return type::IntTy::Instance();
}

type::Ty *StringExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                                int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  return type::StringTy::Instance();
}

type::Ty *CallExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                              int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  env::EnvEntry *entry = venv->Look(func_);

  if (entry==nullptr || typeid(*entry) != typeid(env::FunEntry)) {
    errormsg->Error(pos_, "undefined function %s", func_->Name().data());
  }
  else{
    env::FunEntry *fun_entry = static_cast<env::FunEntry *>(entry);

    const auto &decl_list = fun_entry->formals_->GetList();
    const auto &call_list = this->args_->GetList();

    int decl_size = decl_list.size();
    int call_size = call_list.size();
    
    auto decl_it = decl_list.begin();
    auto call_it = call_list.begin();

    int size = decl_size > call_size ? call_size : decl_size;

    for(; size > 0; size--){
      type::Ty* ty = (*call_it)->SemAnalyze(venv, tenv, labelcount, errormsg);
      
      if (!ty->IsSameType(*decl_it)) {
        errormsg->Error(pos_, "para type mismatch");
        return type::IntTy::Instance();
      }
      decl_it++;
      call_it++;
    }

    if (call_size > decl_size) {
      errormsg->Error(pos_, "too many params in function %s", func_->Name().data());
      return type::IntTy::Instance();
    } 
    else if (call_size < decl_size) {
      errormsg->Error(pos_, "too few params in function %s", func_->Name().data());
      return type::IntTy::Instance();
    }
    return fun_entry->result_->ActualTy();
  }

  return type::IntTy::Instance();
}

type::Ty *OpExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                            int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty *left_ty = left_->SemAnalyze(venv, tenv, labelcount, errormsg)->ActualTy();
  type::Ty *right_ty= right_->SemAnalyze(venv, tenv, labelcount, errormsg)->ActualTy();

  if (oper_ == absyn::PLUS_OP || oper_ == absyn::MINUS_OP || 
    oper_ == absyn::TIMES_OP || oper_ == absyn::DIVIDE_OP) {
    if (typeid(*left_ty) != typeid(type::IntTy)) {
      errormsg->Error(left_->pos_,"integer required");
    }
    if (typeid(*right_ty) != typeid(type::IntTy)) {
      errormsg->Error(right_->pos_,"integer required");
    }
    return type::IntTy::Instance();
  }
  else {
    if (!left_ty->IsSameType(right_ty)) {
      errormsg->Error(pos_, "same type required");
      return type::IntTy::Instance();
    }
  }
  return type::IntTy::Instance();
  
}

type::Ty *RecordExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                                int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty* result = tenv->Look(typ_);
  if (result==nullptr) {
    errormsg->Error(pos_, "undefined type %s", typ_->Name().data());
    return type::IntTy::Instance();
  };
  return result;
  
}

type::Ty *SeqExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  const auto &exp_list = seq_->GetList();
  type::Ty *result;
  auto iter = exp_list.begin();
  int size = exp_list.size();

  for (; size > 0; size--) {
    result = (*iter)->SemAnalyze(venv, tenv, labelcount, errormsg);
    iter++;
  }
  return result;
  
}

type::Ty *AssignExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                                int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty *var_ty = var_->SemAnalyze(venv, tenv, labelcount, errormsg);
  type::Ty *exp_ty = exp_->SemAnalyze(venv, tenv, labelcount, errormsg);

  if (typeid(*var_) == typeid(SimpleVar)) {
    env::EnvEntry *entry = venv->Look(static_cast<SimpleVar *>(var_)->sym_);

    if (entry->readonly_==true) {
      errormsg->Error(var_->pos_, "loop variable can't be assigned");
    }

  }
  if (!var_ty->IsSameType(exp_ty)) {
    errormsg->Error(pos_, "unmatched assign exp");
  }

  return type::VoidTy::Instance();
}

type::Ty *IfExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                            int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty *test_ty = test_->SemAnalyze(venv, tenv, labelcount, errormsg);
  type::Ty *then_ty = then_->SemAnalyze(venv, tenv, labelcount, errormsg);

  if (typeid(*test_ty) != typeid(type::IntTy)) {
    errormsg->Error(test_->pos_, "if test must be Integer");
    return type::VoidTy::Instance();
  }
  if (elsee_==nullptr) {

    if(typeid(*then_ty) != typeid(type::VoidTy)){
      errormsg->Error(then_->pos_, "if-then exp's body must produce no value");
      return type::VoidTy::Instance();
    }
  }
  else {
    type::Ty *else_ty = elsee_->SemAnalyze(venv, tenv, labelcount, errormsg);

    if (!then_ty->IsSameType(else_ty)) {
      errormsg->Error(then_->pos_, "then exp and else exp type mismatch");
      return type::VoidTy::Instance();
    }
    return then_ty;
  }

  return type::VoidTy::Instance();
}

type::Ty *WhileExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                               int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty *test_ty = test_->SemAnalyze(venv, tenv, labelcount, errormsg);

  if (typeid(*test_ty) != typeid(type::IntTy)) {
    errormsg->Error(test_->pos_, "while test must be Integer");
    return type::VoidTy::Instance();
  }

  if (body_!=nullptr) {
    type::Ty *body_ty = body_->SemAnalyze(venv, tenv, labelcount + 1, errormsg);
    
    if (typeid(*body_ty) != typeid(type::VoidTy)) {
      errormsg->Error(body_->pos_, "while body must produce no value");
      return type::VoidTy::Instance();
    }
  }

  return type::VoidTy::Instance();
}

type::Ty *ForExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty *lo_ty = lo_->SemAnalyze(venv, tenv, labelcount, errormsg);
  type::Ty *hi_ty = hi_->SemAnalyze(venv, tenv, labelcount, errormsg);

  if (typeid(*lo_ty) != typeid(type::IntTy) || typeid(*hi_ty) != typeid(type::IntTy)) {
    errormsg->Error(lo_->pos_, "for exp's range type is not integer");
  }

  if (body_!=nullptr) {

    venv->BeginScope();
    tenv->BeginScope();

    venv->Enter(var_, new env::VarEntry(type::IntTy::Instance(), true));
    type::Ty *body_ty = body_->SemAnalyze(venv, tenv, labelcount + 1, errormsg);

    venv->EndScope();
    tenv->EndScope();
  }

  return type::VoidTy::Instance();
}

type::Ty *BreakExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                               int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  if (labelcount == 0) {
    errormsg->Error(pos_, "break is not inside any loop");
  }

  return type::VoidTy::Instance();
}

type::Ty *LetExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  venv->BeginScope();
  tenv->BeginScope();

  for (Dec *dec : decs_->GetList())
      dec->SemAnalyze(venv, tenv, labelcount, errormsg);

  type::Ty *result;
  if (!body_) 
      result = type::VoidTy::Instance();
  else 
      result = body_->SemAnalyze(venv, tenv, labelcount, errormsg);
  
  tenv->EndScope();
  venv->EndScope();
  return result;

}

type::Ty *ArrayExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                               int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
   type::Ty *ty = tenv->Look(typ_)->ActualTy();
  if (ty==nullptr) {
    errormsg->Error(pos_, "undefined type %s", typ_->Name().data());
    return type::VoidTy::Instance();
  } 
  if (typeid(*ty) != typeid(type::ArrayTy)){
    errormsg->Error(pos_, "not an array type");
    return type::VoidTy::Instance();
  }
  else{
    type::ArrayTy *arr_ty = static_cast<type::ArrayTy *>(ty);
    type::Ty *ele_ty = arr_ty->ty_;

    type::Ty *index_ty = size_->SemAnalyze(venv, tenv, labelcount, errormsg);
    type::Ty *value_ty = init_->SemAnalyze(venv, tenv, labelcount, errormsg);

    if (typeid(*index_ty) != typeid(type::IntTy)) {
      errormsg->Error(size_->pos_, "integer required");
      return type::VoidTy::Instance();
    }
    if (!value_ty->IsSameType(ele_ty)) {
      errormsg->Error(init_->pos_, "type mismatch");
      return type::VoidTy::Instance();
    }

    return arr_ty;
  }

  return type::VoidTy::Instance();
}

type::Ty *VoidExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                              int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  return type::VoidTy::Instance();
}

void FunctionDec::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  const auto &func_list = functions_->GetList();
  auto iter = func_list.begin();
  int size = func_list.size();

  for (; size > 0; size--) {

    absyn::FunDec *function = *iter;

    if (venv->Look(function->name_) != nullptr) {
      errormsg->Error(pos_, "two functions have the same name");
    }

    absyn::FieldList *params = function->params_;
    type::TyList *formals = params->MakeFormalTyList(tenv, errormsg);

    if (function->result_ != nullptr) {
      type::Ty *result_ty = tenv->Look(function->result_);
      venv->Enter(function->name_, new env::FunEntry(formals, result_ty));
    } 
    else {
      venv->Enter(function->name_, new env::FunEntry(formals, type::VoidTy::Instance()));
    }

    iter++;
  }

  iter = func_list.begin();
  size = func_list.size();

  for (; size > 0; size--) {

    absyn::FunDec *function = *iter;
    absyn::FieldList *params = function->params_;
    type::TyList *formals = params->MakeFormalTyList(tenv, errormsg);

    venv->BeginScope();

    auto formal_it = formals->GetList().begin();
    auto param_it = params->GetList().begin();

    for (; param_it != params->GetList().end(); formal_it++, param_it++)
      venv->Enter((*param_it)->name_, new env::VarEntry(*formal_it));

    type::Ty *ty;
    ty= function->body_->SemAnalyze(venv, tenv, labelcount, errormsg);

    env::EnvEntry *entry = venv->Look(function->name_);
    if (typeid(*entry) != typeid(env::FunEntry) || ty != static_cast<env::FunEntry *>(entry)->result_->ActualTy()) {
      errormsg->Error(pos_, "procedure returns value");
    }
    venv->EndScope();

    iter++;
  }
}

void VarDec::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv, int labelcount,
                        err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty *init_ty = init_->SemAnalyze(venv, tenv, labelcount, errormsg);

  if (typ_==nullptr) {
    if (typeid(*init_ty) == typeid(type::NilTy) && typeid(*(init_ty->ActualTy())) != typeid(type::RecordTy)) {
      errormsg->Error(pos_, "init should not be nil without type specified");
      return;
    }
    venv->Enter(var_, new env::VarEntry(init_ty));
  }
  else {
    type::Ty *ty = tenv->Look(typ_);
    if (ty==nullptr) {
      errormsg->Error(pos_, "undefined type of %s", this->typ_->Name().data());
      return;
    }
    if (!ty->IsSameType(init_ty)) {
      errormsg->Error(pos_, "type mismatch");
      return;
    }
    venv->Enter(var_, new env::VarEntry(ty));
  }
}

void TypeDec::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv, int labelcount,
                         err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  const auto &type_list = types_->GetList();
  auto iter = type_list.begin();
  int size = type_list.size();

  for (; size > 0; size--) {

    absyn::NameAndTy *nty = *iter;

    if (tenv->Look((*iter)->name_) != nullptr) {
      errormsg->Error(pos_, "two types have the same name");
    }
    else {
      tenv->Enter((*iter)->name_, new type::NameTy((*iter)->name_, nullptr));
    }

    iter++;
  }

  iter = type_list.begin();
  size = type_list.size();
  for (; size > 0; size--) {

    type::Ty *ty = tenv->Look((*iter)->name_);
    type::NameTy *name_ty = static_cast<type::NameTy *>(ty);
    name_ty->ty_ = (*iter)->ty_->SemAnalyze(tenv, errormsg);

    iter++;
  }

  iter = type_list.begin();
  size = type_list.size();
  bool cycleExist = false;

  for (; size > 0; size--) {

    type::Ty* ty = tenv->Look((*iter)->name_);
    type::Ty* t = static_cast<type::NameTy *>(ty)->ty_;

    while (typeid(*t) == typeid(type::NameTy)) {

      type::NameTy* name_ty = static_cast<type::NameTy *>(t);
      if (name_ty->sym_ == (*iter)->name_) {
        cycleExist = true;
        break;
      }
      t = name_ty->ty_;
    }
    if (cycleExist) {
      break;
    }

    iter++;
  }

  if (cycleExist) {
    errormsg->Error(pos_, "illegal type cycle");
  }
}

type::Ty *NameTy::SemAnalyze(env::TEnvPtr tenv, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty *ty = tenv->Look(name_);
  if (ty==nullptr) {
    errormsg->Error(pos_, "undefined type %s", name_->Name().data());
  } 
  else {
    return ty;
  }
  return type::IntTy::Instance();

}

type::Ty *RecordTy::SemAnalyze(env::TEnvPtr tenv,
                               err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::FieldList* field_list = record_->MakeFieldList(tenv,errormsg);

  return new type::RecordTy(field_list);
}

type::Ty *ArrayTy::SemAnalyze(env::TEnvPtr tenv,
                              err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty *ty = tenv->Look(array_);
  if (ty==nullptr) {
    errormsg->Error(pos_, "undefined type %s", array_->Name().data());
  }
  else {
    return new type::ArrayTy(ty);
  }

  return new type::ArrayTy(type::IntTy::Instance());
}

} // namespace absyn

namespace sem {

void ProgSem::SemAnalyze() {
  FillBaseVEnv();
  FillBaseTEnv();
  absyn_tree_->SemAnalyze(venv_.get(), tenv_.get(), errormsg_.get());
}
} // namespace sem
