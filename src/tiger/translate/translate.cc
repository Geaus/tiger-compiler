#include "tiger/translate/translate.h"

#include <tiger/absyn/absyn.h>

#include "tiger/env/env.h"
#include "tiger/errormsg/errormsg.h"
#include "tiger/frame/x64frame.h"
#include "tiger/frame/temp.h"
#include "tiger/frame/frame.h"

extern frame::Frags *frags;
extern frame::RegManager *reg_manager;


namespace tr {

Access *Access::AllocLocal(Level *level, bool escape) {
  /* TODO: Put your lab5 code here */
  auto tr_access = new tr::Access(level, level->frame_->AllocLocal(escape));
  return tr_access;
}

Level *Level::NewLevel(Level *parent, temp::Label *name, std::list<bool> formals) {
  formals.push_front(true);
  frame::Frame *frame = frame::Frame::NewFrame(name, formals);
  return new Level(frame, parent);
}
tree::Exp *staticLink(Level *current, Level *target){
  tree::Exp *framePtr = new tree::TempExp(reg_manager->FramePointer());
  while(current != target){
    framePtr = current->frame_->formals_->front()->ToExp(framePtr);
    current = current -> parent_;
  }
  return framePtr;
}
class Cx {
public:
  PatchList trues_;
  PatchList falses_;
  tree::Stm *stm_;
  Cx() {}
  Cx(PatchList trues, PatchList falses, tree::Stm *stm)
      : trues_(trues), falses_(falses), stm_(stm) {}
};

class Exp {
public:
  [[nodiscard]] virtual tree::Exp *UnEx() = 0;
  [[nodiscard]] virtual tree::Stm *UnNx() = 0;
  [[nodiscard]] virtual Cx UnCx(err::ErrorMsg *errormsg) = 0;
};

class ExpAndTy {
public:
  tr::Exp *exp_;
  type::Ty *ty_;

  ExpAndTy(tr::Exp *exp, type::Ty *ty) : exp_(exp), ty_(ty) {}
};

class ExExp : public Exp {
public:
  tree::Exp *exp_;

  explicit ExExp(tree::Exp *exp) : exp_(exp) {}

  [[nodiscard]] tree::Exp *UnEx() override {
    /* TODO: Put your lab5 code here */
    return exp_;
  }
  [[nodiscard]] tree::Stm *UnNx() override {
    /* TODO: Put your lab5 code here */
    return new tree::ExpStm(exp_);
  }
  [[nodiscard]] Cx UnCx(err::ErrorMsg *errormsg) override {
    /* TODO: Put your lab5 code here */
    tree::CjumpStm *stm = new tree::CjumpStm(tree::NE_OP, exp_, new tree::ConstExp(0),
                                             nullptr, nullptr);
    PatchList trues({&(stm->true_label_)});
    PatchList falses({&(stm->false_label_)});
    Cx ret(trues, falses, stm);
    return ret;
  }
};

class NxExp : public Exp {
public:
  tree::Stm *stm_;

  explicit NxExp(tree::Stm *stm) : stm_(stm) {}

  [[nodiscard]] tree::Exp *UnEx() override {
    /* TODO: Put your lab5 code here */
    return new tree::EseqExp(stm_, new tree::ConstExp(0));
  }
  [[nodiscard]] tree::Stm *UnNx() override {
    /* TODO: Put your lab5 code here */
    return stm_;
  }
  [[nodiscard]] Cx UnCx(err::ErrorMsg *errormsg) override {
    /* TODO: Put your lab5 code here */
  }
};

class CxExp : public Exp {
public:
  Cx cx_;

  CxExp(PatchList trues, PatchList falses, tree::Stm *stm)
      : cx_(trues, falses, stm) {}
  
  [[nodiscard]] tree::Exp *UnEx() override {

    temp::Label *t = temp::LabelFactory::NewLabel();
    temp::Label *f = temp::LabelFactory::NewLabel();
    cx_.trues_.DoPatch(t);
    cx_.falses_.DoPatch(f);

    temp::Temp *reg = temp::TempFactory::NewTemp();

    auto ret =
        new tree::EseqExp(new tree::MoveStm(new tree::TempExp(reg), new tree::ConstExp(1)),
  new tree::EseqExp(cx_.stm_,
  new tree::EseqExp(new tree::LabelStm(f),
  new tree::EseqExp(new tree::MoveStm(new tree::TempExp(reg), new tree::ConstExp(0)),
  new tree::EseqExp(new tree::LabelStm(t), new tree::TempExp(reg))))));

    return ret;

  }
  [[nodiscard]] tree::Stm *UnNx() override {
  }
  [[nodiscard]] Cx UnCx(err::ErrorMsg *errormsg) override { 
    return cx_;
  }
};

void ProgTr::Translate() {
  /* TODO: Put your lab5 code here */
  FillBaseVEnv();
  FillBaseTEnv();
  
  frame::Frame * main_frame = new frame::X64Frame;
  main_frame->name_ = temp::LabelFactory::NamedLabel("tigermain");
  main_level_->frame_ = main_frame;
  main_level_->parent_ = nullptr;

  tr::ExpAndTy *main_trans = absyn_tree_->Translate(venv_.get(), tenv_.get(),
                                                    main_level_.get(), nullptr, errormsg_.get());

  tree::Stm *ret = main_trans->exp_->UnNx();
  ret = frame::ProcEntryExit1(main_level_->frame_, ret);
  auto main_frag = new frame::ProcFrag(ret, main_level_->frame_);
  frags->PushBack(main_frag);
}

} // namespace tr

namespace absyn {

tr::ExpAndTy *AbsynTree::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level, temp::Label *label,
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  return root_->Translate(venv, tenv, level, label, errormsg);
}

tr::ExpAndTy *SimpleVar::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level, temp::Label *label,
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  env::EnvEntry *entry = venv->Look(sym_);

  auto var_entry = static_cast<env::VarEntry *>(entry);
  type::Ty *type = var_entry->ty_->ActualTy();

  tr::Level *lv = level;
  tree::Exp *framePtr = tr::staticLink(lv,var_entry->access_->level_);
  tree::Exp * exp = var_entry->access_->access_->ToExp(framePtr);

  return new tr::ExpAndTy(new tr::ExExp(exp),type);
}

tr::ExpAndTy *FieldVar::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level, temp::Label *label,
                                  err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  tr::ExpAndTy* var_trans = var_->Translate(venv,tenv,level,label,errormsg);
  type::Ty* actual_ty = var_trans->ty_->ActualTy();

  auto record = static_cast<type::RecordTy *>(actual_ty);
  int offset = 0;
  std::list<type::Field *> field_list = record->fields_->GetList();

  for (auto& field : field_list) {
    if (field->name_->Name() == sym_->Name()) {

      auto exp =new tree::MemExp(
              new tree::BinopExp(tree::PLUS_OP, var_trans->exp_->UnEx(),
                                 new tree::ConstExp(offset * reg_manager->WordSize()))
                  );
      type::Ty* type = field->ty_->ActualTy();
      return new tr::ExpAndTy(new tr::ExExp(exp), type);

    }
    offset++;
  }
}

tr::ExpAndTy *SubscriptVar::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                      tr::Level *level, temp::Label *label,
                                      err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  tr::ExpAndTy* var_trans = var_->Translate(venv, tenv, level, label, errormsg);
  tr::ExpAndTy* subscript_trans = subscript_->Translate(venv, tenv, level, label, errormsg);

  auto exp = new tree::MemExp(
          new tree::BinopExp(tree::PLUS_OP,var_trans->exp_->UnEx(),
                             new tree::BinopExp(tree::BinOp::MUL_OP,subscript_trans->exp_->UnEx(),
                                                new tree::ConstExp(reg_manager->WordSize()))));

  type::Ty* ty = static_cast<type::ArrayTy *>(var_trans->ty_)->ty_->ActualTy();
  return new tr::ExpAndTy(new tr::ExExp(exp), ty);
}

tr::ExpAndTy *VarExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  return var_->Translate(venv, tenv, level, label, errormsg);
}

tr::ExpAndTy *NilExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  return new tr::ExpAndTy(
    new tr::ExExp(new tree::ConstExp(0)),
      type::NilTy::Instance()
  );
}

tr::ExpAndTy *IntExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  return new tr::ExpAndTy(
    new tr::ExExp(new tree::ConstExp(val_)),
      type::IntTy::Instance()
  );
}

tr::ExpAndTy *StringExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level, temp::Label *label,
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  temp::Label *l = temp::LabelFactory::NewLabel();
  auto str = new frame::StringFrag(l, str_);
  frags->PushBack(str);

  return new tr::ExpAndTy(new tr::ExExp(new tree::NameExp(l)),
                          type::StringTy::Instance());
}

tr::ExpAndTy *CallExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                 tr::Level *level, temp::Label *label,
                                 err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  env::EnvEntry *entry = venv->Look(func_);
  auto *func_entry = static_cast<env::FunEntry *>(entry);
  type::Ty *type  = func_entry->result_->ActualTy();


  auto args_list = new tree::ExpList();
  int i = 0;
  tree::Exp *ret;

  std::string func_name = func_->Name();
  if (func_name == "flush" || func_name == "exit" || func_name == "chr"|| func_name == "getchar" || func_name == "print"||
      func_name == "printi" ||  func_name == "ord" || func_name == "size" || func_name == "concat" || func_name == "substring" ) {

    for (const auto& arg : args_->GetList()) {
      tr::ExpAndTy *arg_trans = arg->Translate(venv, tenv, level, label, errormsg);
      args_list->Append(arg_trans->exp_->UnEx());
      i++;
    }
    ret = frame::ExternalCall(func_name, args_list);
    level->frame_->max_args = std::max(i - 6, level->frame_->max_args);
    return new tr::ExpAndTy(new tr::ExExp(ret), type);

  }
  else {

    tr::Level *lv = level;
    args_list->Append(tr::staticLink(lv, func_entry->level_->parent_));
    i++;
    for (const auto& arg : args_->GetList()) {
      tr::ExpAndTy *arg_trans = arg->Translate(venv, tenv, level, label, errormsg);
      args_list->Append(arg_trans->exp_->UnEx());
      i++;
    }
    ret = new tree::CallExp(new tree::NameExp(func_), args_list);
    level->frame_->max_args = std::max(i - 6, level->frame_->max_args);
    return new tr::ExpAndTy(new tr::ExExp(ret), type);
  }
}

tr::ExpAndTy *OpExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                               tr::Level *level, temp::Label *label,
                               err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  tr::ExpAndTy *left_trans = left_->Translate(venv, tenv, level, label, errormsg);
  tr::ExpAndTy *right_trans = right_->Translate(venv, tenv, level, label, errormsg);
  type::Ty *left_ty = left_trans->ty_;
  type::Ty *right_ty = right_trans->ty_;

  tree::CjumpStm * cj_stm;
  tr::PatchList true_list, false_list;
  tr::Exp *ret;

  if (oper_ == absyn::PLUS_OP || oper_ == absyn::MINUS_OP ||
      oper_ == absyn::TIMES_OP || oper_ == absyn::DIVIDE_OP) {
    tree::BinOp bin_op;

    switch (oper_) {
    case absyn::PLUS_OP:
      bin_op = tree::PLUS_OP;
      break;
    case absyn::MINUS_OP:
      bin_op = tree::MINUS_OP;
      break;
    case absyn::TIMES_OP:
      bin_op = tree::MUL_OP;
      break;
    case absyn::DIVIDE_OP:
      bin_op = tree::DIV_OP;
      break;
    }

    ret = new tr::ExExp(new tree::BinopExp(bin_op, left_trans->exp_->UnEx(),
                                           right_trans->exp_->UnEx()));

    return new tr::ExpAndTy(ret,type::IntTy::Instance());
  }
  else if(oper_ == absyn::AND_OP || oper_ == absyn::OR_OP) {
    temp::Label *condition = temp::LabelFactory::NewLabel();
    tr::Cx left_cx = left_trans->exp_->UnCx(errormsg);
    tr::Cx right_cx = right_trans->exp_->UnCx(errormsg);

    if(oper_ == absyn::AND_OP) {
      left_cx.trues_.DoPatch(condition);
      true_list = tr::PatchList(right_cx.trues_);
      false_list = tr::PatchList::JoinPatch(left_cx.falses_, right_cx.falses_);
    }
    else {
      left_cx.falses_.DoPatch(condition);
      true_list = tr::PatchList::JoinPatch(left_cx.trues_, right_cx.trues_);
      false_list = tr::PatchList(right_cx.falses_);
    }
    ret = new tr::CxExp(true_list,false_list,
                        new tree::SeqStm(left_cx.stm_,
                            new tree::SeqStm(new tree::LabelStm(condition),
                                             right_cx.stm_)));
    return new tr::ExpAndTy(ret,type::IntTy::Instance());
  }
  else {

    switch (oper_) {
    case absyn::LT_OP:
      cj_stm = new tree::CjumpStm(tree::LT_OP, left_trans->exp_->UnEx(),
                                  right_trans->exp_->UnEx(), nullptr, nullptr);
      break;
    case absyn::LE_OP:
      cj_stm = new tree::CjumpStm(tree::LE_OP, left_trans->exp_->UnEx(),
                                  right_trans->exp_->UnEx(), nullptr, nullptr);
      break ;
    case absyn::GE_OP:
      cj_stm = new tree::CjumpStm(tree::GE_OP, left_trans->exp_->UnEx(),
                                  right_trans->exp_->UnEx(), nullptr, nullptr);
      break ;
    case absyn::GT_OP:
      cj_stm = new tree::CjumpStm(tree::GT_OP, left_trans->exp_->UnEx(),
                                  right_trans->exp_->UnEx(), nullptr, nullptr);
      break ;
    case absyn::NEQ_OP:
      cj_stm = new tree::CjumpStm(tree::NE_OP, left_trans->exp_->UnEx(),
                                  right_trans->exp_->UnEx(),nullptr, nullptr);
      break ;
    case absyn::EQ_OP:
      if (!left_ty->IsSameType(type::StringTy::Instance())) {
        cj_stm = new tree::CjumpStm(tree::EQ_OP, left_trans->exp_->UnEx(),
                               right_trans->exp_->UnEx(),nullptr, nullptr);
      }
      else {
        auto *args = new tree::ExpList();
        args->Append(left_trans->exp_->UnEx());
        args->Append(right_trans->exp_->UnEx());
        
        tree::Exp *call_ret = frame::ExternalCall("string_equal", args);
        ret = new tr::ExExp(call_ret);

        return new tr::ExpAndTy(ret, type::IntTy::Instance());
      }
      break ;
    }
    true_list = tr::PatchList({&cj_stm->true_label_});
    false_list = tr::PatchList({&cj_stm->false_label_});
    ret = new tr::CxExp(true_list, false_list, cj_stm);

    return new tr::ExpAndTy(ret,type::IntTy::Instance());
  }
}

tr::ExpAndTy *RecordExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level, temp::Label *label,      
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  temp::Temp *reg = temp::TempFactory::NewTemp();
  tree::Stm *seq_stm = nullptr;
  tree::Stm *init_list_head = nullptr;
  tree::BinopExp *address_exp = nullptr;

  int i = 0;
  bool init = true;

  for (const auto &efield : fields_->GetList()) {

    tr::ExpAndTy *exp_trans = efield->exp_->Translate(venv, tenv, level, label, errormsg);
    if (init) {
      address_exp = new tree::BinopExp(tree::PLUS_OP, new tree::TempExp(reg),
                                       new tree::ConstExp(i *reg_manager->WordSize()));
      init_list_head = new tree::MoveStm(
        new tree::MemExp(address_exp),
          exp_trans->exp_->UnEx()
      );
      init = false;
    }
    else {
      address_exp = new tree::BinopExp(tree::PLUS_OP, new tree::TempExp(reg),
                                       new tree::ConstExp(i *reg_manager->WordSize()));
      init_list_head = new tree::SeqStm(
        init_list_head,
        new tree::MoveStm(
          new tree::MemExp(address_exp),
              exp_trans->exp_->UnEx()
        )
      );
    }

    i++;
  }

  auto *args = new tree::ExpList();
  args->Append(new tree::ConstExp(i * reg_manager->WordSize()));

  auto *left_move_stm = new tree::MoveStm(
      new tree::TempExp(reg), frame::ExternalCall("alloc_record",args));

  seq_stm = new tree::SeqStm(
    left_move_stm, init_list_head
  );

  return new tr::ExpAndTy(
    new tr::ExExp(new tree::EseqExp(seq_stm, new tree::TempExp(reg))),
    tenv->Look(typ_)->ActualTy()
  );
}

tr::ExpAndTy *SeqExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  type::Ty *type = nullptr;
  tree::Stm *current_stm = nullptr;
  tree::Exp *final_exp = nullptr;

  bool init = true;
  for (const auto &exp : seq_->GetList()) {
    tr::ExpAndTy *exp_trans = exp->Translate(venv, tenv, level, label, errormsg);

    if (exp != seq_->GetList().back()) {
      if (init) {
        current_stm = exp_trans->exp_->UnNx();
        init = false;
      }
      else {
        current_stm = new tree::SeqStm(current_stm, exp_trans->exp_->UnNx());
      }
    }
    else {
      type = exp_trans->ty_->ActualTy();
      if (init) {
        final_exp = exp_trans->exp_->UnEx();
        init = false;
      }
      else {
        final_exp = new tree::EseqExp(current_stm, exp_trans->exp_->UnEx());
      }
    }
  }

  return new tr::ExpAndTy(new tr::ExExp(final_exp), type);
}

tr::ExpAndTy *AssignExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level, temp::Label *label,                       
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  tree::Stm * move_stm = nullptr;
  tr::ExpAndTy *var_trans = var_->Translate(venv, tenv, level, label, errormsg);
  tr::ExpAndTy *exp_trans = exp_->Translate(venv, tenv, level, label, errormsg);

  move_stm = new tree::MoveStm(var_trans->exp_->UnEx(), exp_trans->exp_->UnEx());

  return new tr::ExpAndTy(new tr::NxExp(move_stm), type::VoidTy::Instance());
}

tr::ExpAndTy *IfExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                               tr::Level *level, temp::Label *label,
                               err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  tr::ExpAndTy *test_trans = test_->Translate(venv, tenv, level, label, errormsg);
  tr::ExpAndTy *then_trans = then_->Translate(venv, tenv, level, label, errormsg);
  type::Ty *then_ty = then_trans->ty_;

  temp::Label *t = temp::LabelFactory::NewLabel();
  temp::Label *f = temp::LabelFactory::NewLabel();
  temp::Label *joint = temp::LabelFactory::NewLabel();


  tr::Cx test_cx = test_trans->exp_->UnCx(errormsg);
  test_cx.trues_.DoPatch(t);
  test_cx.falses_.DoPatch(f);

  if (!elsee_) {

    auto *stm =
        new tree::SeqStm(test_cx.stm_,
  new tree::SeqStm(new tree::LabelStm(t),
  new tree::SeqStm(then_trans->exp_->UnNx(),new tree::LabelStm(f))));

    return new tr::ExpAndTy(new tr::NxExp(stm), then_ty);
  }
  else {
    temp::Temp *reg = temp::TempFactory::NewTemp();
    tr::ExpAndTy *else_trans = elsee_->Translate(venv, tenv, level, label, errormsg);

    auto *true_stm =
        new tree::SeqStm(new tree::LabelStm(t),
  new tree::SeqStm(
    new tree::MoveStm(new tree::TempExp(reg), then_trans->exp_->UnEx()),
    new tree::JumpStm(new tree::NameExp(joint),
    new std::vector<temp::Label *>{joint})));

    auto *false_stm =
        new tree::SeqStm(new tree::LabelStm(f),
  new tree::SeqStm(
    new tree::MoveStm(new tree::TempExp(reg), else_trans->exp_->UnEx()),
    new tree::JumpStm(new tree::NameExp(joint),
    new std::vector<temp::Label *>{joint})));

    auto *exp =
        new tree::EseqExp(
    new tree::SeqStm(test_cx.stm_,
    new tree::SeqStm(true_stm,
    new tree::SeqStm(false_stm,
    new tree::LabelStm(joint)))),
      new tree::TempExp(reg));

    return new tr::ExpAndTy(new tr::ExExp(exp), then_ty);
  }
}

tr::ExpAndTy *WhileExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level, temp::Label *label,            
                                  err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  temp::Label *test_label = temp::LabelFactory::NewLabel();
  temp::Label *body_label = temp::LabelFactory::NewLabel();
  temp::Label *done_label = temp::LabelFactory::NewLabel();

  tr::ExpAndTy *test_trans = test_->Translate(venv, tenv, level, done_label, errormsg);
  tr::ExpAndTy *body_trans = body_->Translate(venv, tenv, level, done_label, errormsg);

  tr::Cx test_cx = test_trans->exp_->UnCx(errormsg);
  test_cx.trues_.DoPatch(body_label);
  test_cx.falses_.DoPatch(done_label);

  auto *body_stm =
      new tree::SeqStm(new tree::LabelStm(body_label),
new tree::SeqStm(body_trans->exp_->UnNx(),
new tree::SeqStm(new tree::JumpStm(new tree::NameExp(test_label),
                    new std::vector<temp::Label *>({test_label})),
                        new tree::LabelStm(done_label))));

  auto *test_stm =
      new tree::SeqStm(new tree::LabelStm(test_label),
new tree::SeqStm(test_cx.stm_, body_stm));


  return new tr::ExpAndTy(new tr::NxExp(test_stm), type::VoidTy::Instance());
}

tr::ExpAndTy *ForExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  venv->BeginScope();

  venv->Enter(var_, new env::VarEntry(tr::Access::AllocLocal(level, false),
                                      type::IntTy::Instance(), true));
  tr::ExpAndTy *lo_trans = lo_->Translate(venv, tenv, level, label, errormsg);
  tr::ExpAndTy *hi_trans = hi_->Translate(venv, tenv, level, label, errormsg);

  temp::Label *update_label = temp::LabelFactory::NewLabel();
  temp::Label *body_label = temp::LabelFactory::NewLabel();
  temp::Label *done_label = temp::LabelFactory::NewLabel();

  tr::ExpAndTy *body_trans = body_->Translate(venv, tenv, level, done_label, errormsg);

  env::EnvEntry * entry = venv->Look(var_);
  auto *var_entry = static_cast<env::VarEntry *>(entry);
  frame::Access *access = var_entry->access_->access_;

  temp::Temp *loop_reg = (static_cast<frame::InRegAccess *>(access))->reg;
  temp::Temp *limit_reg = temp::TempFactory::NewTemp();

  auto loop_init_stm = new tree::MoveStm(new tree::TempExp(loop_reg), lo_trans->exp_->UnEx());
  auto limit_init_stm = new tree::MoveStm(new tree::TempExp(limit_reg), hi_trans->exp_->UnEx());
  auto loop_gt_stm = new tree::CjumpStm(tree::RelOp::GT_OP,
                                        new tree::TempExp(loop_reg),new tree::TempExp(limit_reg),
                                        done_label, body_label);

  auto loop_eq_stm = new tree::CjumpStm(tree::RelOp::EQ_OP,
                                        new tree::TempExp(loop_reg),new tree::TempExp(limit_reg),
                                        done_label, update_label);

  auto loop_update_stm = new tree::MoveStm(new tree::TempExp(loop_reg),
                                        new tree::BinopExp(tree::BinOp::PLUS_OP, new tree::TempExp(loop_reg),
                                                           new tree::ConstExp(1)));

  auto *seq_stm =
     new tree::SeqStm(loop_init_stm,
new tree::SeqStm(limit_init_stm,
new tree::SeqStm(loop_gt_stm,
new tree::SeqStm(new tree::LabelStm(body_label),
new tree::SeqStm(body_trans->exp_->UnNx(),
new tree::SeqStm(loop_eq_stm,
new tree::SeqStm(new tree::LabelStm(update_label),
new tree::SeqStm(loop_update_stm,
new tree::SeqStm(new tree::JumpStm(new tree::NameExp(body_label),new std::vector<temp::Label *>({body_label})),
new tree::LabelStm(done_label)
    )))))))));

  venv->EndScope();
  return new tr::ExpAndTy(new tr::NxExp(seq_stm), type::VoidTy::Instance());
}

tr::ExpAndTy *BreakExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level, temp::Label *label,
                                  err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */

  auto *jmp_stm = new tree::JumpStm(new tree::NameExp(label),
                                    new std::vector<temp::Label *>({label}));

  return new tr::ExpAndTy(new tr::NxExp(jmp_stm),type::VoidTy::Instance());

}

tr::ExpAndTy *LetExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  tenv->BeginScope();
  venv->BeginScope();

  tree::Stm *dec_stm = nullptr;
  bool init = true;
  for (Dec *dec : decs_->GetList()) {
    tr::Exp *dec_trans = dec->Translate(venv, tenv, level, label, errormsg);

    if (typeid(*dec) == typeid(VarDec)) {
      if (init) {
        dec_stm = dec_trans->UnNx();
        init = false;
      }
      else {
        dec_stm = new tree::SeqStm(dec_stm, dec_trans->UnNx());
      }
    }
  }

  tr::ExpAndTy *body_trans = body_->Translate(venv, tenv, level, label, errormsg);
  type::Ty *type = body_trans->ty_->ActualTy();

  tree::Exp *ret_exp = dec_stm ? new tree::EseqExp(dec_stm, body_trans->exp_->UnEx())
      : body_trans->exp_->UnEx();

  tenv->EndScope();
  venv->EndScope();

  return new tr::ExpAndTy(new tr::ExExp(ret_exp), type);

}

tr::ExpAndTy *ArrayExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level, temp::Label *label,                    
                                  err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  tr::ExpAndTy *size_trans = size_->Translate(venv, tenv, level, label, errormsg);
  tr::ExpAndTy *init_trans = init_->Translate(venv, tenv, level, label, errormsg);
  type::Ty *type = tenv->Look(typ_)->ActualTy();

  auto *args = new tree::ExpList();
  args->Append(size_trans->exp_->UnEx());
  args->Append(init_trans->exp_->UnEx());
  tree::Exp *ret = frame::ExternalCall("init_array", args);

  return new tr::ExpAndTy(new tr::ExExp(ret), type);
}

tr::ExpAndTy *VoidExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                 tr::Level *level, temp::Label *label,
                                 err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  return new tr::ExpAndTy(
    new tr::NxExp(new tree::ExpStm(new tree::ConstExp(0))),
      type::VoidTy::Instance()
  );
}

tr::Exp *FunctionDec::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */

  for (const auto &fun_dec : functions_->GetList()) {
    type::TyList *ty_list = fun_dec->params_->MakeFormalTyList(tenv, errormsg);
    type::Ty *result_ty = fun_dec->result_ ? tenv->Look(fun_dec->result_)
                                           : type::VoidTy::Instance();

    std::list<bool> formal_escapes;
    for (const auto &field : fun_dec->params_->GetList()) {
      formal_escapes.push_back(field->escape_);
    }
    tr::Level *new_level = tr::Level::NewLevel(level, fun_dec->name_, formal_escapes);

    venv->Enter(fun_dec->name_, new env::FunEntry(new_level, fun_dec->name_,
                                                  ty_list, result_ty));
  }


  for (const auto &fun_dec : functions_->GetList()) {
    venv->BeginScope();

    auto *fun_entry = static_cast<env::FunEntry *>(venv->Look(fun_dec->name_));
    auto access_it = fun_entry->level_->frame_->formals_->begin();
    access_it++;
    std::list<type::Field *> field_list = fun_dec->params_->MakeFieldList(tenv, errormsg)->GetList();
    auto field_it = field_list.begin();

    while(access_it != fun_entry->level_->frame_->formals_->end() &&
           field_it != field_list.end()){
      type::Ty *type = (*field_it)->ty_;
      venv->Enter((*field_it)->name_,
                  new env::VarEntry(new tr::Access(fun_entry->level_, (*access_it)),
                                    type));
      access_it++;
      field_it++;
    }

    tr::ExpAndTy *body_trans = fun_dec->body_->Translate(venv, tenv, fun_entry->level_, label, errormsg);

    tree::Stm *ret_stm = fun_dec->result_ ? new tree::MoveStm(new tree::TempExp(reg_manager->ReturnValue()),
                                                        body_trans->exp_->UnEx()) : body_trans->exp_->UnNx();

    frags->PushBack(new frame::ProcFrag(
        frame::ProcEntryExit1(fun_entry->level_->frame_, ret_stm),
        fun_entry->level_->frame_));

    venv->EndScope();
  }

  return new tr::ExExp(new tree::ConstExp(0));
}


tr::Exp *VarDec::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                           tr::Level *level, temp::Label *label,
                           err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  tr::ExpAndTy *init_trans = init_->Translate(venv, tenv, level, label, errormsg);

  tr::Access *tr_access = tr::Access::AllocLocal(level, escape_);
  venv->Enter(var_, new env::VarEntry(tr_access, init_trans->ty_));

  auto *move_stm = new tree::MoveStm(
    tr_access->access_->ToExp(new tree::TempExp(reg_manager->FramePointer())),
      init_trans->exp_->UnEx()
  );
  return new tr::NxExp(move_stm);
}

tr::Exp *TypeDec::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                            tr::Level *level, temp::Label *label,
                            err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  for (NameAndTy *type : types_->GetList()) {
    tenv->Enter(type->name_, new type::NameTy(type->name_, nullptr));
  }
  for (NameAndTy *type : types_->GetList()) {
    auto *name_ty = static_cast<type::NameTy *>(tenv->Look(type->name_));
    name_ty->ty_ = type->ty_->Translate(tenv, errormsg);
  }
  return new tr::ExExp(new tree::ConstExp(0));
}

type::Ty *NameTy::Translate(env::TEnvPtr tenv, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  type::Ty *type = tenv->Look(name_);
  return new type::NameTy(name_, type);
}

type::Ty *RecordTy::Translate(env::TEnvPtr tenv,
                              err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  type::FieldList *field_list = record_->MakeFieldList(tenv, errormsg);
  return new type::RecordTy(field_list);
}

type::Ty *ArrayTy::Translate(env::TEnvPtr tenv, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  type::Ty *type = tenv->Look(array_);
  return new type::ArrayTy(type);
}

} // namespace absyn
