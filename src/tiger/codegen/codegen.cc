#include "tiger/codegen/codegen.h"

#include <cassert>
#include <sstream>

extern frame::RegManager *reg_manager;

namespace {

constexpr int maxlen = 1024;


} // namespace

namespace cg {

void CodeGen::Codegen() {
  /* TODO: Put your lab5 code here */
  auto *list = new assem::InstrList();
  for (tree::Stm *stm : traces_->GetStmList()->GetList()) {
    stm->Munch(*list, frame_->GetLabel()+"_framesize");
  }
  assem_instr_ = std::make_unique<AssemInstr>(frame::ProcEntryExit2(list));
}

void AssemInstr::Print(FILE *out, temp::Map *map) const {
  for (auto instr : instr_list_->GetList())
    instr->Print(out, map);
  fprintf(out, "\n");
}
} // namespace cg

namespace tree {
/* TODO: Put your lab5 code here */

void SeqStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  left_->Munch(instr_list, fs);
  right_->Munch(instr_list, fs);
}

void LabelStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  instr_list.Append(
      new assem::LabelInstr(temp::LabelFactory::LabelString(label_),
                            label_));
}

void JumpStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  auto dst_lab = temp::LabelFactory::LabelString(exp_->name_);
  auto jmp_instr = new assem::OperInstr("jmp "+ dst_lab, new temp::TempList(), new temp::TempList(),
                            new assem::Targets(jumps_));
  instr_list.Append(jmp_instr);
}

void CjumpStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */

  temp::Temp *left_reg = left_->Munch(instr_list, fs);
  temp::Temp *right_reg = right_->Munch(instr_list, fs);

  instr_list.Append(new assem::OperInstr(
    "cmpq `s1,`s0", new temp::TempList(),
    new temp::TempList({left_reg, right_reg}),
    nullptr
  ));

  std::string cj_instr;
  switch (op_){
    case EQ_OP:
      cj_instr = "je ";
      break;
    case NE_OP:
      cj_instr = "jne ";
      break;
    case LT_OP:
      cj_instr = "jl ";
      break;
    case GT_OP:
      cj_instr = "jg ";
      break;
    case LE_OP:
      cj_instr = "jle ";
      break;
    case GE_OP:
      cj_instr = "jge ";
      break;
    default:
      break;
  }
  instr_list.Append(new assem::OperInstr(
      cj_instr+temp::LabelFactory::LabelString(true_label_),
      new temp::TempList(), new temp::TempList(),
      new assem::Targets(new std::vector<temp::Label *>({
          true_label_, false_label_}))));
}

void MoveStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  temp::Temp *src_reg = src_->Munch(instr_list, fs);
  if (typeid(*dst_) == typeid(tree::MemExp)) {

    auto *dst_mem = static_cast<MemExp *>(dst_);
    temp::Temp *dst_reg = dst_mem->exp_->Munch(instr_list, fs);
    auto mov_instr = new assem::MoveInstr("movq `s0, (`s1)",new temp::TempList(),
                                          new temp::TempList({src_reg, dst_reg}));
    instr_list.Append(mov_instr);
  }
  else {
    temp::Temp *dst_reg = dst_->Munch(instr_list, fs);
    auto mov_instr = new assem::MoveInstr("movq `s0, `d0",
                                          new temp::TempList({dst_reg}),
                                          new temp::TempList({src_reg}));
    instr_list.Append(mov_instr);
  }
}

void ExpStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  exp_->Munch(instr_list, fs);
}

temp::Temp *BinopExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  std::string binop_instr;
  switch (op_) {
    case PLUS_OP:
      binop_instr = "addq";
      break;
    case MINUS_OP:
      binop_instr = "subq";
      break;
    case MUL_OP:
      binop_instr = "imulq";
      break;
    case DIV_OP:
      binop_instr = "idivq";
      break;
    default:
      break;
  }
  temp::Temp *left_reg = left_->Munch(instr_list, fs);
  temp::Temp *right_reg = right_->Munch(instr_list, fs);
  temp::Temp *reg = temp::TempFactory::NewTemp();

  if (op_ == BinOp::MUL_OP) {

    auto move_to_rax_instr = new assem::MoveInstr("movq `s0, `d0",
                                              new temp::TempList(reg_manager->GetRegister(frame::RAX)),
                                              new temp::TempList(left_reg));

    auto mul_instr = new assem::OperInstr("imulq `s0",
                                          new temp::TempList({reg_manager->GetRegister(frame::RAX), reg_manager->GetRegister(frame::RDX)}),
                                          new temp::TempList(right_reg), nullptr);

    auto move_result_instr = new assem::MoveInstr("movq `s0, `d0",
                                                  new temp::TempList(reg),
                                                  new temp::TempList(reg_manager->GetRegister(frame::RAX)));
    instr_list.Append(move_to_rax_instr);
    instr_list.Append(mul_instr);
    instr_list.Append(move_result_instr);

  }
  else if (op_ == BinOp::DIV_OP) {

    auto move_to_rax_instr = new assem::MoveInstr("movq `s0, `d0",
                                            new temp::TempList(reg_manager->GetRegister(frame::RAX)),
                                            new temp::TempList(left_reg));
    auto cqto_instr = new assem::OperInstr("cqto",
                                           new temp::TempList(reg_manager->GetRegister(frame::RDX)),
                                           new temp::TempList(reg_manager->GetRegister(frame::RAX)), nullptr);
    auto div_instr = new assem::OperInstr("idivq `s0",
                                          new temp::TempList({reg_manager->GetRegister(frame::RAX), reg_manager->GetRegister(frame::RDX)}),
                                          new temp::TempList(right_reg), nullptr);
    auto move_result_instr = new assem::MoveInstr("movq `s0, `d0",
                                                  new temp::TempList(reg),
                                                  new temp::TempList(reg_manager->GetRegister(frame::RAX)));

    instr_list.Append(move_to_rax_instr);
    instr_list.Append(cqto_instr);
    instr_list.Append(div_instr);
    instr_list.Append(move_result_instr);

  }
  else {
    auto move_to_reg_instr = new assem::MoveInstr("movq `s0, `d0",
                                                  new temp::TempList(reg),
                                                  new temp::TempList(left_reg));
    auto caculate_instr = new assem::OperInstr(binop_instr + " `s0, `d0",
                                               new temp::TempList(reg),
                                               new temp::TempList(right_reg), nullptr);

    instr_list.Append(move_to_reg_instr);
    instr_list.Append(caculate_instr);

  }
  return reg;

}

temp::Temp *MemExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  temp::Temp *reg = temp::TempFactory::NewTemp();

  temp::Temp *exp = exp_->Munch(instr_list, fs);
  auto mem_move_instr = new assem::OperInstr("movq (`s0), `d0",
                                         new temp::TempList(reg),
                                         new temp::TempList(exp), nullptr);
  instr_list.Append(mem_move_instr);
  return reg;
}

temp::Temp *TempExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  if(temp_ == reg_manager->FramePointer()){
    temp::Temp *reg = temp::TempFactory::NewTemp();

    auto fp_to_sp_instr = new assem::OperInstr("leaq "+std::string(fs.data())+"(`s0),`d0",
        new temp::TempList(reg),
        new temp::TempList(reg_manager->StackPointer()),nullptr);
    instr_list.Append(fp_to_sp_instr);
    return reg;
  }
  else{
    return temp_;
  }
}

temp::Temp *EseqExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  stm_->Munch(instr_list, fs);
  return exp_->Munch(instr_list, fs);
}

temp::Temp *NameExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  temp::Temp *reg = temp::TempFactory::NewTemp();
  auto leaq_instr = new assem::OperInstr("leaq "+temp::LabelFactory::LabelString(name_)+"(%rip),`d0",
                        new temp::TempList({reg}),new temp::TempList(), nullptr);

  instr_list.Append(leaq_instr);
  return reg;
}

temp::Temp *ConstExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  temp::Temp *reg = temp::TempFactory::NewTemp();
  auto mov_instr = new assem::OperInstr("movq $"+std::to_string(consti_)+",`d0",
                       new temp::TempList({reg}),new temp::TempList(), nullptr
                       );
  instr_list.Append(mov_instr);
  return reg;
}

temp::Temp *CallExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  temp::TempList *arg_list = args_->MunchArgs(instr_list, fs);
  auto *fun = static_cast<NameExp *>(fun_);

  auto call_instr = new assem::OperInstr("callq "+temp::LabelFactory::LabelString(fun->name_),
                   reg_manager->CallerSaves(), arg_list,nullptr);
  instr_list.Append(call_instr);

  temp::Temp *reg = temp::TempFactory::NewTemp();
  auto ret_instr = new assem::MoveInstr("movq `s0,`d0",
                       new temp::TempList(reg),
                       new temp::TempList({reg_manager->ReturnValue()}));
  instr_list.Append(ret_instr);
  return reg;
}

temp::TempList *ExpList::MunchArgs(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  int arg_index = 0;
  temp::TempList *arg_temps = reg_manager->ArgRegs();
  auto *arg_reg_list = new temp::TempList;

  for (const auto &exp : exp_list_) {
    temp::Temp *reg = exp->Munch(instr_list, fs);
    if (arg_index < 6) {
      temp::Temp *tmp = arg_temps->NthTemp(arg_index);
      auto move_to_reg_instr = new assem::MoveInstr("movq `s0,`d0",
                       new temp::TempList({tmp}),
                       new temp::TempList({reg}));
      instr_list.Append(move_to_reg_instr);
      arg_reg_list->Append(tmp);
    }
    else {
      auto move_to_stack_instr = new assem::OperInstr("movq `s0," + std::to_string((arg_index -6) * reg_manager->WordSize()) + "(`d0)",
                       new temp::TempList({reg_manager->StackPointer()}),
                       new temp::TempList({reg}),nullptr);
      instr_list.Append(move_to_stack_instr);
    }
    arg_index++;
  }
  return arg_reg_list;
}

} // namespace tree
