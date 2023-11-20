//
// Created by wzl on 2021/10/12.
//

#ifndef TIGER_COMPILER_X64FRAME_H
#define TIGER_COMPILER_X64FRAME_H

#include "tiger/frame/frame.h"

namespace frame {
class X64RegManager : public RegManager {
  /* TODO: Put your lab5 code here */
public:
    X64RegManager() : frame::RegManager() {

      std::vector<std::string *> reg_name;
      reg_name.push_back(new std::string("%rsp")); //0

      reg_name.push_back(new std::string("%rax")); //1
      reg_name.push_back(new std::string("%rdi")); //2
      reg_name.push_back(new std::string("%rsi")); //3
      reg_name.push_back(new std::string("%rdx")); //4
      reg_name.push_back(new std::string("%rcx")); //5
      reg_name.push_back(new std::string("%r8")); //6
      reg_name.push_back(new std::string("%r9")); //7
      reg_name.push_back(new std::string("%r10")); //8
      reg_name.push_back(new std::string("%r11")); //9

      reg_name.push_back(new std::string("%rbx")); //10
      reg_name.push_back(new std::string("%rbp")); //11
      reg_name.push_back(new std::string("%r12")); //12
      reg_name.push_back(new std::string("%r13")); //13
      reg_name.push_back(new std::string("%r14")); //14
      reg_name.push_back(new std::string("%r15")); //15

      for (auto name : reg_name) {
        auto tmp_reg = temp::TempFactory::NewTemp();
        regs_.push_back(tmp_reg);
        temp_map_->Enter(tmp_reg, name);
      }
    }
  
  [[nodiscard]] temp::TempList *Registers() override;

  [[nodiscard]] temp::TempList *ArgRegs() override;

  [[nodiscard]] temp::TempList *CallerSaves() override;

  [[nodiscard]] temp::TempList *CalleeSaves() override;

  [[nodiscard]] temp::TempList *ReturnSink() override;

  [[nodiscard]] int WordSize() override;

  [[nodiscard]] temp::Temp *FramePointer() override;

  [[nodiscard]] temp::Temp *StackPointer() override;

  [[nodiscard]] temp::Temp *ReturnValue() override;
};

} // namespace frame
#endif // TIGER_COMPILER_X64FRAME_H
