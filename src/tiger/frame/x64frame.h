//
// Created by wzl on 2021/10/12.
//

#ifndef TIGER_COMPILER_X64FRAME_H
#define TIGER_COMPILER_X64FRAME_H

#include "tiger/frame/frame.h"

namespace frame {
enum X86Reg {
  RAX = 0,RBX,RCX,RDX,RSI,RDI,RBP,RSP,R8,R9,R10,R11,R12,R13,R14,R15
};

class X64RegManager : public RegManager {

private:

public:
  X64RegManager() {

    const std::string X64RegStr[16] = {"%rax","%rbx","%rcx","%rdx",
                                       "%rsi","%rdi","%rbp","%rsp",
                                       "%r8","%r9","%r10","%r11",
                                       "%r12","%r13","%r14","%r15"};

    for (const auto& name : X64RegStr) {
      temp::Temp *reg = temp::TempFactory::NewTemp();
      temp_map_->Enter(reg, new std::string(name));
      regs_.push_back(reg);
    }

  }

  temp::TempList *Registers() override {

    auto *tmp_list = new temp::TempList;
    for (temp::Temp *reg : regs_) {
      tmp_list->Append(reg);
    }
    return tmp_list;
  }

  temp::TempList *ArgRegs() override{
    return new temp::TempList({regs_[RDI], regs_[RSI], regs_[RDX],
                               regs_[RCX], regs_[R8], regs_[R9]});
  }

  temp::TempList *CallerSaves() override{
    return new temp::TempList({
      regs_[R10],regs_[R11],regs_[RDI],
      regs_[RSI],regs_[RDX],regs_[RCX],
      regs_[R8],regs_[R9],regs_[RAX]
    });
  }

  temp::TempList *CalleeSaves() override{
    return new temp::TempList({
      regs_[RBX],regs_[R12],regs_[R13],
      regs_[R14],regs_[R15],regs_[RBP]
    });
  }

  temp::TempList *ReturnSink()  override{
    temp::TempList *tmp_list = CalleeSaves();
    tmp_list->Append(ReturnValue());
    tmp_list->Append(StackPointer());

    return tmp_list;
  }

  int WordSize() override {return 8;}

  temp::Temp *FramePointer() override {return regs_[RBP];}

  temp::Temp *StackPointer() override  {return regs_[RSP];}

  temp::Temp *ReturnValue() override {return regs_[RAX];}

};

class InFrameAccess : public Access {
public:
  int offset;

  explicit InFrameAccess(int offset) : offset(offset) {}
  /* TODO: Put your lab5 code here */
  tree::Exp *ToExp(tree::Exp *framePtr) const override{
    return new tree::MemExp(new tree::BinopExp(tree::BinOp::PLUS_OP,
                                               new tree::ConstExp(offset),framePtr));
  }
};
class InRegAccess : public Access {
public:
  temp::Temp *reg;

  explicit InRegAccess(temp::Temp *reg) : reg(reg) {}
  /* TODO: Put your lab5 code here */
  tree::Exp *ToExp(tree::Exp *framePtr) const override{
    return new tree::TempExp(reg);
  }
};
class X64Frame : public Frame {
  /* TODO: Put your lab5 code here */
public:

  X64Frame(){}
  Access* AllocLocal(bool escape) override;
  std::string GetLabel() override {return name_->Name();}
};
} // namespace frame
#endif // TIGER_COMPILER_X64FRAME_H
