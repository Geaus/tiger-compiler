#include "tiger/frame/x64frame.h"

extern frame::RegManager *reg_manager;

namespace frame {
Access* X64Frame::AllocLocal(bool escape) {
  Access* local = nullptr;
  if (escape) {
    this->offset_ -= reg_manager->WordSize();
    local_num++;
    local = new InFrameAccess(this->offset_);
  } else {
    local = new InRegAccess(temp::TempFactory::NewTemp());
  }
  return local;
}

Frame *Frame::NewFrame(temp::Label *name, std::list<bool> formals) {

  Frame *new_frame = new X64Frame;
  new_frame->name_ = name;
  new_frame->formals_ = new std::list<Access *>;

  int i = 0;
  bool init = true;
  temp::TempList *arg_regs = reg_manager->ArgRegs();
  for (const auto &escape: formals) {
    if (i >= 6) {
      new_frame->formals_->push_back(
          new InFrameAccess((i - 5) * reg_manager->WordSize())
      );
    }
    else {
      Access *new_access = new_frame->AllocLocal(escape);
      new_frame->formals_->push_back(new_access);

      auto append_move_stm = new tree::MoveStm(
          new_access->ToExp(new tree::TempExp(reg_manager->FramePointer())),
          new tree::TempExp(arg_regs->NthTemp(i)));

      if (init) {
        new_frame->view_shift_ = append_move_stm;
        init = false;
      }
      else {
        new_frame->view_shift_ = new tree::SeqStm(
            new_frame->view_shift_,
            append_move_stm
        );
      }
    }
    i++;
  }

  return new_frame;
}

tree::Exp *ExternalCall(std::string s, tree::ExpList *args)
{
  return new tree::CallExp(
    new tree::NameExp(temp::LabelFactory::NamedLabel(s)), args
  );
}

tree::Stm *ProcEntryExit1(Frame *frame, tree::Stm *stm)
{

  temp::TempList *callee_regs = reg_manager->CalleeSaves();

  for (temp::Temp *callee_reg : callee_regs->GetList()) {
    Access *tmp = frame->AllocLocal(false);

    auto save_callee_stm = new tree::MoveStm(
            tmp->ToExp(new tree::TempExp(reg_manager->FramePointer())),
            new tree::TempExp(callee_reg));
    auto restore_callee_stm = new tree::MoveStm(
            new tree::TempExp(callee_reg),
            tmp->ToExp(new tree::TempExp(reg_manager->FramePointer())));

    stm = new tree::SeqStm(
        new tree::SeqStm(save_callee_stm,stm),restore_callee_stm
    );
  }

  if (frame->view_shift_) {
    stm = new tree::SeqStm(frame->view_shift_, stm);
  }

  return stm;
}

assem::InstrList *ProcEntryExit2(assem::InstrList *body)
{
  body->Append(new assem::OperInstr("", new temp::TempList(), reg_manager->ReturnSink(), nullptr));
  return body;
}

assem::Proc *ProcEntryExit3(frame::Frame *frame, assem::InstrList *body)
{
  int frame_size = (frame->local_num + frame->max_args) * reg_manager->WordSize();

  std::stringstream prologue;
  std::stringstream epilogue;

  prologue<<".set "<<temp::LabelFactory::LabelString(frame->name_)<<"_framesize, "<<frame_size<<std::endl;
  prologue<<temp::LabelFactory::LabelString(frame->name_)<<":" << std::endl;
  prologue<< "subq $" << frame_size << ", %rsp" << std::endl;

  epilogue<< "addq $" << frame_size << ", %rsp" << std::endl;
  epilogue<< "retq" << std::endl;

  return new assem::Proc(prologue.str(), body, epilogue.str());
}
} // namespace frame
