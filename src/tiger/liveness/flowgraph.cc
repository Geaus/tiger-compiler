#include "tiger/liveness/flowgraph.h"

namespace fg {

void FlowGraphFactory::AssemFlowGraph() {
  /* TODO: Put your lab6 code here */

  for (const auto &instr : instr_list_->GetList()) {

    FNode *new_node = flowgraph_->NewNode(instr);

    if (typeid(*instr) == typeid(assem::LabelInstr)) {
      auto label = (static_cast<assem::LabelInstr *>(instr))->label_;
      label_map_->Enter(label, new_node);
    }

  }

  bool prev_jump = false;
  FNode *prev_node = nullptr;
  for (const auto &cur_node : flowgraph_->Nodes()->GetList()) {

    if (prev_jump == false && prev_node) {
      flowgraph_->AddEdge(prev_node, cur_node);
    }

    assem::Instr *instr = cur_node->NodeInfo();
    if (typeid(*instr) == typeid(assem::OperInstr)){
      auto jumps = static_cast<assem::OperInstr *>(instr)->jumps_;

      if(jumps){
        for (temp::Label *jump : *(jumps->labels_)) {
          auto target = label_map_->Look(jump);
          flowgraph_->AddEdge(cur_node, target);
        }
        prev_jump = true;
        continue;
      }
    }

    prev_jump = false;
    prev_node = cur_node;
  }
}

} // namespace fg

namespace assem {

temp::TempList *LabelInstr::Def() const {
  /* TODO: Put your lab6 code here */
  return new temp::TempList();
}

temp::TempList *MoveInstr::Def() const {
  /* TODO: Put your lab6 code here */
  return dst_;
}

temp::TempList *OperInstr::Def() const {
  /* TODO: Put your lab6 code here */
  return dst_;
}

temp::TempList *LabelInstr::Use() const {
  /* TODO: Put your lab6 code here */
  return new temp::TempList();
}

temp::TempList *MoveInstr::Use() const {
  /* TODO: Put your lab6 code here */
  return src_;
}

temp::TempList *OperInstr::Use() const {
  /* TODO: Put your lab6 code here */
  return src_;
}
} // namespace assem
