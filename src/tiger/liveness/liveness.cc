#include "tiger/liveness/liveness.h"

extern frame::RegManager *reg_manager;

namespace live {

bool MoveList::Contain(INodePtr src, INodePtr dst) {
  return std::any_of(move_list_.cbegin(), move_list_.cend(),
                     [src, dst](std::pair<INodePtr, INodePtr> move) {
                       return move.first == src && move.second == dst;
                     });
}

void MoveList::Delete(INodePtr src, INodePtr dst) {
  assert(src && dst);
  auto move_it = move_list_.begin();
  for (; move_it != move_list_.end(); move_it++) {
    if (move_it->first == src && move_it->second == dst) {
      break;
    }
  }
  move_list_.erase(move_it);
}

MoveList *MoveList::Union(MoveList *list) {
  auto *res = new MoveList();
  for (auto move : move_list_) {
    res->move_list_.push_back(move);
  }
  for (auto move : list->GetList()) {
    if (!res->Contain(move.first, move.second))
      res->move_list_.push_back(move);
  }
  return res;
}

MoveList *MoveList::Intersect(MoveList *list) {
  auto *res = new MoveList();
  for (auto move : list->GetList()) {
    if (Contain(move.first, move.second))
      res->move_list_.push_back(move);
  }
  return res;
}

std::set<temp::Temp *> List2Set(const std::list<temp::Temp *> &origin) {
  std::set<temp::Temp *> res;
  for (auto &it : origin)
    res.insert(it);
  return res;
};
void LiveGraphFactory::LiveMap() {
  /* TODO: Put your lab6 code here */
  auto flow_graph_nodes = flowgraph_->Nodes();
  //init
  for (const auto &s : flow_graph_nodes->GetList()) {
    in_->Enter(s, new temp::TempList());
    out_->Enter(s, new temp::TempList());
  }
  bool change = true;

  while (change) {
    change = false;
    for (const auto s : flow_graph_nodes->GetList()) {
      assem::Instr *instr = s->NodeInfo();
      auto use_s = List2Set(instr->Use()->GetList());
      auto def_s = List2Set(instr->Def()->GetList());

      std::set<temp::Temp *> out_s;
      std::set<temp::Temp *> in_s;
      auto out_s_old = List2Set(out_->Look(s)->GetList());
      auto in_s_old = List2Set(in_->Look(s)->GetList());

      // out[s] = U  (n ∈ succ[s]) in [n]
      auto succ_s = s->Succ();
      for (const auto &n : succ_s->GetList()) {
        auto in_n = List2Set(in_->Look(n)->GetList());
        out_s.merge(in_n);
      }

      // in[s] = use[s] U (out[s] – def[s])
      std::list<temp::Temp *> difference;
      std::set_difference(out_s_old.begin(), out_s_old.end(),def_s.begin(), def_s.end(),
                          back_inserter(difference));
      in_s = use_s;
      in_s.merge(List2Set(difference));

      //if set change
      if (!(in_s == in_s_old && out_s == out_s_old)) {
        change = true;
        auto in_tl = new temp::TempList();
        auto out_tl = new temp::TempList();
        for (const auto &it : in_s) in_tl->Append(it);
        for (const auto &it : out_s) out_tl->Append(it);

        in_->Set(s, in_tl);
        out_->Set(s, out_tl);
      }
    }

  }
}

void LiveGraphFactory::InterfGraph() {
  /* TODO: Put your lab6 code here */
  //precolored regs
  auto precolored = reg_manager->Registers();
  auto flow_graph_nodes = flowgraph_->Nodes();

  for (const auto &temp : precolored->GetList()) {
    auto new_node = live_graph_.interf_graph->NewNode(temp);
    temp_node_map_->Enter(temp, new_node);
  }
  //add edge
  for (const auto &reg_1 : precolored->GetList()) {
    for (const auto &reg_2 : precolored->GetList()) {
      if (reg_1 == reg_2) {
        continue ;
      }
      auto node_1 = temp_node_map_->Look(reg_1);
      auto node_2 = temp_node_map_->Look(reg_2);
      live_graph_.interf_graph->AddEdge(node_1, node_2);
    }
  }


  // get nodes for temps
  for (const auto &node : flow_graph_nodes->GetList()) {

    auto def_list = node->NodeInfo()->Def();
    auto use_list = node->NodeInfo()->Use();

    //regs Def
    for (const auto &def_reg : def_list->GetList()) {
      if (!temp_node_map_->Look(def_reg)){
        auto new_node = live_graph_.interf_graph->NewNode(def_reg);
        temp_node_map_->Enter(def_reg, new_node);
      }
    }
    //regs Use
    for (const auto &use_reg : use_list->GetList()) {
      if (!temp_node_map_->Look(use_reg)){
        auto new_node = live_graph_.interf_graph->NewNode(use_reg);
        temp_node_map_->Enter(use_reg, new_node);
      }
    }
  }

  // add edges
  for (auto node : flow_graph_nodes->GetList()) {
    auto def_list = node->NodeInfo()->Def();
    auto use_list = node->NodeInfo()->Use();

    auto out_list = out_->Look(node)->GetList();
    auto out_set = List2Set(out_list);
    auto use_set = List2Set(use_list->GetList());

    //if isMoveInstruction(I)
    if (typeid(*node->NodeInfo()) == typeid(assem::MoveInstr)) {

      INodePtr def_node;
      INodePtr use_node;
      for (const auto &def : def_list->GetList()) {
        def_node = temp_node_map_->Look(def);
        //workListMoves ← workListMoves ∪ {I}
        for (const auto &use : use_list->GetList()) {
          use_node = temp_node_map_->Look(use);
          live_graph_.moves->Append(use_node, def_node);
        }
      }
    }

    //forall d ∈ def(I)
    for (const auto &def : def_list->GetList()) {
      INodePtr def_node;
      INodePtr out_node;
      def_node = temp_node_map_->Look(def);

      if(typeid(*node->NodeInfo()) == typeid(assem::MoveInstr)){
        //live ← live \ use(I)
        std::set_difference(out_set.begin(), out_set.end(),use_set.begin(), use_set.end(),
                            back_inserter(out_list));
      }
      // forall l ∈ live
      for (const auto &out : out_list) {
        out_node = temp_node_map_->Look(out);
        if(def != out){
          //AddEdge(l, d)
          live_graph_.interf_graph->AddEdge(def_node, out_node);
          live_graph_.interf_graph->AddEdge(out_node, def_node);
        }
      }
    }

  }
}

void LiveGraphFactory::Liveness() {
  LiveMap();
  InterfGraph();
}

} // namespace live
