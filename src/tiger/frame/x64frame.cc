#include "tiger/frame/x64frame.h"

extern frame::RegManager *reg_manager;

namespace frame {
/* TODO: Put your lab5 code here */
class InFrameAccess : public Access {
public:
  int offset;

  explicit InFrameAccess(int offset) : offset(offset) {}
  /* TODO: Put your lab5 code here */
  tree::Exp *ToExp(tree::Exp *framePtr) const override{
    return nullptr;
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
};
/* TODO: Put your lab5 code here */

temp::TempList *X64RegManager::Registers() {
  auto tmp_list = new temp::TempList();
  for(int i = 0; i < 15; i++){
    tmp_list->Append(GetRegister(i));
  }
  return tmp_list;
//  return nullptr;
}

temp::TempList *X64RegManager::ArgRegs() {
  auto tmp_list = new temp::TempList();
  for(int i = 1; i < 7; i++){
    tmp_list->Append(GetRegister(i));
  }
  return tmp_list;
//  return nullptr;
}

temp::TempList *X64RegManager::CallerSaves() {
  auto tmp_list = new temp::TempList();
  for(int i = 0; i < 9; i++){
    tmp_list->Append(GetRegister(i));
  }
  return tmp_list;
//  return nullptr;
}

temp::TempList *X64RegManager::CalleeSaves() {
  auto tmp_list = new temp::TempList();
  for(int i = 9; i < 15; i++){
    tmp_list->Append(GetRegister(i));
  }
  return tmp_list;
//  return nullptr;
}

temp::TempList *X64RegManager::ReturnSink() {

  temp::TempList *tmp_list = CalleeSaves();

  tmp_list->Append(StackPointer()); //%rsp
  tmp_list->Append(ReturnValue());  //%rax
  
  return tmp_list;
  return nullptr;
}

int X64RegManager::WordSize() {
  return 8;
}

temp::Temp *X64RegManager::FramePointer() {
  return GetRegister(10); //%rbp
  return nullptr;
}

temp::Temp *X64RegManager::StackPointer() {
  return GetRegister(15); //%rsp
  return nullptr;
}

temp::Temp *X64RegManager::ReturnValue() {
  return GetRegister(0); //%rax
  return nullptr;
}
} // namespace frame
