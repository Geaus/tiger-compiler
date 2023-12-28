#include "tiger/regalloc/regalloc.h"

#include "tiger/output/logger.h"

extern frame::RegManager *reg_manager;

namespace ra {
/* TODO: Put your lab6 code here */
<<<<<<< HEAD
=======

Result::~Result() {
}
RegAllocator::RegAllocator(frame::Frame *frame, std::unique_ptr<cg::AssemInstr> assem_instr)
    : frame_(frame), assem_instr_(assem_instr->GetInstrList()) {
  K = reg_manager->Registers()->GetList().size();

  precolored = new live::INodeList();
  initial = new live::INodeList();
  selectStack = new live::INodeList();
  newTemps = new live::INodeList();

  simplifyWorklist = new live::INodeList();
  freezeWorklist = new live::INodeList();
  spillWorklist = new live::INodeList();

  spilledNodes = new live::INodeList();
  coalescedNodes = new live::INodeList();
  coloredNodes = new live::INodeList();

  workListMoves = new live::MoveList();
  activeMoves = new live::MoveList();
  coalescedMoves = new live::MoveList();
  constrainedMoves = new live::MoveList();
  frozenMoves = new live::MoveList();

  moveList = new tab::Table<live::INode, live::MoveList>();
  degree = new tab::Table<live::INode, int>();
  alias = new tab::Table<live::INode, live::INode>();

  color = temp::Map::Empty();
  auto reg_list = reg_manager->Registers()->GetList();
  for (const auto &reg : reg_list) {
    temp::Map *temp_map = reg_manager->temp_map_;
    auto reg_str = temp_map->Look(reg);
    color->Enter(reg, reg_str);
  }
}
void RegAllocator::Clear() {

  delete precolored;
  delete initial;
  delete selectStack;
  delete newTemps;

  delete simplifyWorklist;
  delete freezeWorklist;
  delete spillWorklist;

  delete spilledNodes;
  delete coalescedNodes;
  delete coloredNodes;

  delete activeMoves;
  delete coalescedMoves;
  delete constrainedMoves;
  delete frozenMoves;

  delete moveList;
  delete degree;
  delete alias;

  precolored = new live::INodeList();
  initial = new live::INodeList();
  selectStack = new live::INodeList();
  newTemps = new live::INodeList();

  simplifyWorklist = new live::INodeList();
  freezeWorklist = new live::INodeList();
  spillWorklist = new live::INodeList();

  spilledNodes = new live::INodeList();
  coalescedNodes = new live::INodeList();
  coloredNodes = new live::INodeList();

  activeMoves = new live::MoveList();
  coalescedMoves = new live::MoveList();
  constrainedMoves = new live::MoveList();
  frozenMoves = new live::MoveList();

  degree = new tab::Table<live::INode, int>();
  alias = new tab::Table<live::INode, live::INode>();
  moveList = new tab::Table<live::INode, live::MoveList>();
}

void RegAllocator::RegAlloc() {
  LivenessAnalysis();
  Init();
  Build();
  MakeWorklist();
  bool flag = true;
  do {
    if (simplifyWorklist->GetList().empty() == false){
      Simplify();
    }
    else if (workListMoves->GetList().empty() == false){
      Coalesce();
    }
    else if (freezeWorklist->GetList().empty() == false){
      Freeze();
    }
    else if (spillWorklist->GetList().empty() == false){
      SelectSpill();
    }
    else{
      flag = false;
    }
  } while (flag);

  AssignColor();

  if (spilledNodes->GetList().empty() == false) {
    RewriteProgram();
    RegAlloc();
  }
  else {
    deleteUselessMoves();
    result_ = std::make_unique<Result>(color,assem_instr_);
  }
}


void RegAllocator::LivenessAnalysis() {

  fg::FlowGraphFactory flow_graph_factory(assem_instr_);
  flow_graph_factory.AssemFlowGraph();

  auto flow_graph = flow_graph_factory.GetFlowGraph();
  live::LiveGraphFactory live_graph_factory(flow_graph);
  live_graph_factory.Liveness();

  auto live_graph = live_graph_factory.GetLiveGraph();
  interf_graph = live_graph.interf_graph;
  workListMoves = live_graph.moves;
  temp_node_map = live_graph_factory.GetTempNodeMap();

}
void RegAllocator::Init(){

  Clear();
  //init degree  alias  precolored  initial
  temp::Map *temp_map = reg_manager->temp_map_;
  for(const auto &node : interf_graph->Nodes()->GetList()){
    auto out_degree = new int(node->OutDegree());
    degree->Enter(node, out_degree);
    alias->Enter(node, node);
    auto temp = temp_map->Look(node->NodeInfo());
    if(temp){
      precolored->Append(node);
    }
    else{
      initial->Append(node);
    }
  }

}
//procedure Build()
void RegAllocator::Build() {

  auto interf_graph_nodes =  interf_graph->Nodes();
  for (live::INodePtr node : interf_graph_nodes->GetList()) {

    live::MoveList *associated_moves = new live::MoveList();
    //forall n ∈ def(I) ∪ use(I)
    for (const auto &move : workListMoves->GetList()) {
      if (move.first == node || move.second == node) {
        // moveList[n] ← moveList[n] ∪ {I}
        associated_moves->Append(move.first, move.second);
      }
    }
    moveList->Enter(node, associated_moves);

  }
}
//procedure AddEdge(u,v)
void RegAllocator::AddEdge(live::INodePtr u, live::INodePtr v) {

  //if ((u,v) ∉ adjSet ∧ (u≠v)
  if ((u->Adj(v) == false) && (u != v)) {

    //if u ∉ precolored
    if (precolored->Contain(u) == false) {
      // adjList[u] ← adjList[u] ∪ {v}
      interf_graph->AddEdge(u, v);
      //degree[u]+=1
      auto degree_u = degree->Look(u);
      *degree_u += 1;

    }
    //if v ∉ precolored
    if (precolored->Contain(v) == false) {
      // adjList[v] ← adjList[v] ∪ {u}
      interf_graph->AddEdge(v, u);
      //degree[v]+=1
      auto degree_v = degree->Look(v);
      *degree_v += 1;

    }
  }
}
// procedure MakeWorkList()
void RegAllocator::MakeWorklist() {

  //forall n ∈ intial
  for (auto n : initial->GetList()) {
    
    //if degree[n] ≥ K
    auto degree_n = degree->Look(n);
    if (*degree_n >= K) {
      //spillWorkList ← spillWorkList ∪ {n}
      spillWorklist->Append(n);
    }
    //else if MoveRelated(n)
    else if (MoveRelated(n)) {
      // freezeWorkList ← freezeWorkList ∪ {n}
      freezeWorklist->Append(n);
    }
    else {
      // simplifyWorkList ← simplifyWorkList ∪ {n}
      simplifyWorklist->Append(n);
    }
  }
  //initial ← initial\{n}
  initial->Clear();

}
// function  Adjacent(n)
live::INodeListPtr RegAllocator::Adjacent(live::INodePtr n) {
  // adjList[n] \ (selectStack ∪ coalescedNodes)
  auto adjList_n = n->Succ();
  auto tmp_list = selectStack->Union(coalescedNodes);
  auto res_list = adjList_n->Diff(tmp_list);
  return res_list;
}
// function  NodeMoves(n)
live::MoveList *RegAllocator::NodeMoves(live::INodePtr n) {
  // moveList[n] ∩ (activeMoves ∪ worklistMoves)
  auto tmp_list = activeMoves->Union(workListMoves);
  auto moveList_n = moveList->Look(n);
  auto res_list = moveList_n->Intersect(tmp_list);
  return res_list;
}
//function  MoveRelated(n)
bool RegAllocator::MoveRelated(live::INodePtr n) {
  // NodeMoves(n) ≠ {}
  auto res = (NodeMoves(n)->GetList().empty() == false);
  return res;
}
//procedure Simplify()
void RegAllocator::Simplify() {

  // let n ∈ simplifyWorkList
  auto n = simplifyWorklist->GetList().front();
  // simplifyWorkList ← simplifyWorkList\{n}
  simplifyWorklist->DeleteNode(n);
  // push(n, selectStack)
  selectStack->Prepend(n);
  // forall m ∈ Adjacent(n)
  for (const auto &m : Adjacent(n)->GetList()) {
    // DecrementDegree(m)
      Decrement(m);
  }
}
// procedure Decrement(m)
void RegAllocator::Decrement(live::INodePtr m) {

  if(precolored->Contain(m) == false){

      //let d = degree[m]
      auto degree_m = degree->Look(m);
      //degree[m] ← d - 1
      *degree_m -= 1;
      //if d = K then
      if (*degree_m == K) {
          // EnableMoves(m ∪ Adjcent(m))
          auto m_list = new live::INodeList ();
          m_list->Append(m);
          auto adjacent_m = Adjacent(m);
          auto tmp_list = m_list->Union(adjacent_m);
          EnableMoves(tmp_list);

          // spillWorkList ← spillWorkList \ {m}
          spillWorklist->DeleteNode(m);

          //if MoveRelated(m)
          if (MoveRelated(m)) {
            //freezeWorkList ← freezeWorkList ∪ {n}
            freezeWorklist->Append(m);
          }
          else {
            //simplifyWorkList ← simplifyWorkList ∪ {n}
            simplifyWorklist->Append(m);
          }
      }
  }

}
//procedure EnableMoves(nodes)
void RegAllocator::EnableMoves(live::INodeListPtr nodes) {
  //forall n ∈ nodes
  for (const auto &n : nodes->GetList()) {
    //forall I ∈ NodeMoves(n)
    for (const auto &i : NodeMoves(n)->GetList()) {
      //if I ∈ activeMoves then
      if (activeMoves->Contain(i.first, i.second)) {
        //activeMoves ← activeMoves \ {I}
        activeMoves->Delete(i.first, i.second);
        //workListMoves ← workListMoves ∪ {I}
        workListMoves->Append(i.first, i.second);
      }
    }
  }
}
//procedure Coalesce()
void RegAllocator::Coalesce() {

  //let m(=copy(x,y)) ∈ workListMoves
  auto m = workListMoves->GetList().front();
  auto x = m.first;
  auto y = m.second;
  //x ← GetAlias(x)
  x = GetAlias(x);
  //y ← GetAlias(y)
  y = GetAlias(y);
  live::INodePtr u;
  live::INodePtr v;
  //if y ∈ precolored
  if (precolored->Contain(y)) {
    //let (u,v) = (y, x)
    u = y;v = x;
  }
  else {
    //let (u,v) = (x, y)
    u = x;v = y;
  }
  //workListMoves ← workListMoves \ {m}
  workListMoves->Delete(m.first, m.second);
  if (u == v) {
    //coalescedMoves ← coalescedMoves ∪ {m}
    coalescedMoves->Append(m.first, m.second);
    //AddWorkList(u)
    AddWorkList(u);
  }
  //else if v ∈ precolored ∨ (u,v) ∈ adjSet
  else if (precolored->Contain(v) || u->Adj(v)) {
    //constrainedMoves ← contrainedMoves ∪ {m}
    constrainedMoves->Append(m.first, m.second);

    // AddWorkList(u)
    // AddWorkList(v)
    AddWorkList(u);
    AddWorkList(v);
  }
  else {

    bool condition = true;
    //if u ∈ precolored
    if (precolored->Contain(u)) {
      //Adjacent(v)
      auto Adjacent_v = Adjacent(v);
      // t ∈ Adjacent(v),OK(t,u)
      if (Adjacent_v != nullptr) {
        for (const auto &t : Adjacent_v->GetList()) {
          if(OK(t,u) == false){
            condition = false;
            break ;
          }
        }
      }
    }
    //if u !∈ precolored
    else {
      //Conservative(Adjacent(v) ∪ Adjacent(v))
      auto Adjacent_u = Adjacent(u);
      auto Adjacent_v = Adjacent(v);
      auto tmp_list = Adjacent_u->Union(Adjacent_v);
      condition = Conservative(tmp_list);
    }

    if (condition) {
      //coalescedMoves ← coalescedMoves ∪ {m}
      coalescedMoves->Append(m.first, m.second);
      Combine(u, v);
      AddWorkList(u);
    }
    else {
      activeMoves->Append(m.first, m.second);
      //activeMoves ← activeMoves ∪ {m}
    }
  }
}
//procedure addWorkList()
void RegAllocator::AddWorkList(live::INodePtr u) {
  bool condition = true;
  //if u !∈ precolored not MoveRelated(u) degree[u] < K
  if(precolored->Contain(u)) condition = false;
  if(MoveRelated(u)) condition = false;
  auto degree_u = degree->Look(u);
  if(*degree_u >= K) condition = false;

  if (condition) {
    //freezeWorklist ← freezeWorklist \ {u}
    freezeWorklist->DeleteNode(u);
    //simplifyWorklist ← simplifyWorklist ∪ {u}
    simplifyWorklist->Append(u);
  }
}

bool RegAllocator::OK(live::INodePtr t, live::INodePtr r) {
  bool res = false;
  // degree[t] < K  if t ∈ precolored (t,r) ∈ adjSet
  auto degree_t = degree->Look(t);
  if(*degree_t < K) res = true;
  if(precolored->Contain(t)) res = true;
  if(t->Adj(r)) res = true;
  
  return res;
}

bool RegAllocator::Conservative(live::INodeListPtr nodes) {
  //let k = 0;
  int k = 0;
  //forall n ∈ nodes
  for (const auto &n : nodes->GetList()) {
    auto degree_n = degree->Look(n);
    //if degree[n]>= K
    if (*degree_n >= K) {
      k += 1;
    }
  }
  return (k < K);
}
live::INodePtr RegAllocator::GetAlias(live::INodePtr n) {
  live::INodePtr res = n;
  //if n ∈ coalescedNodes then
  if (coalescedNodes->Contain(n)){
    //GetAlias(alias[n])
    auto alias_n =  alias->Look(n);
    res = GetAlias(alias_n);
    return  res;
  }
  //else n
  else{
    return res;
  }

}
//procedure Combine(u,v)
void RegAllocator::Combine(live::INodePtr u, live::INodePtr v) {

  //if v ∈ freezeWorkList
  if (freezeWorklist->Contain(v)) {
    //freezeWorkList ← freezeWorkList \ {v}
    freezeWorklist->DeleteNode(v);
  }
  else {
    //spillWorkList ← spillWorkList \ {v}
    spillWorklist->DeleteNode(v);
  }
  //coalescedNodes ← coalescedNodes ∪ {v}
  coalescedNodes->Append(v);
  //alias[v] ← u
  alias->Set(v, u);
  //moveList[u] ← moveList[u] ∪ moveList[v]
  auto moveList_u = moveList->Look(u);
  auto moveList_v = moveList->Look(v);
  auto tmp_list = moveList_u->Union(moveList_v);
  moveList->Set(u,tmp_list);

  //EnableMoves(v)
  auto v_inode_list = new live::INodeList();
  v_inode_list->Append(v);
  EnableMoves(v_inode_list);

  //forall t ∈ Adjacent(v)
  auto Adjacent_v = Adjacent(v);
  for (const auto &t : Adjacent_v->GetList()) {
    AddEdge(t, u);
    Decrement(t);
  }
  //if degree[u]≥K ∧  u ∈ freezeWorkList
  auto degree_u = degree->Look(u);
  if (*degree_u >= K && freezeWorklist->Contain(u)) {
    //freezeWorkList ← freezeWorkList \ {u}
    freezeWorklist->DeleteNode(u);
    // spillWorkList ← spillWorkList ∪ {u}
    spillWorklist->Append(u);
  }
}
//procedure Freeze()
void RegAllocator::Freeze() {

  //let u ∈ freezeWorkList
  auto u = freezeWorklist->GetList().front();
  //freezeWorkList ← freezeWorkList \ {u}
  freezeWorklist->DeleteNode(u);
  //simplifyWorkList ← simplifyWorkList ∪ {u}
  simplifyWorklist->Append(u);
  FreezeMoves(u);
}
// procedure FreezeMoves(u)
void RegAllocator::FreezeMoves(live::INodePtr u) {
  //forall m(=copy(x,y)) ∈ NodeMoves(u)
  for (auto m : NodeMoves(u)->GetList()) {
    auto x = m.first;
    auto y = m.second;
    live::INodePtr v;

    // if GetAlias(y) = GetAlias(u)
    auto alias_y = GetAlias(y);
    auto alias_u = GetAlias(u);
    if (alias_y == alias_u) {
      //v ← GetAlias(x)
      v = GetAlias(x);
    }
    else {
      // v ← GetAlias(y)
      v = GetAlias(y);
    }
    //activeMoves ← activeMoves \ {m}
    activeMoves->Delete(m.first, m.second);
    //frozenMoves ← frozenMoves ∪ {m}
    frozenMoves->Append(m.first, m.second);

    //if NodeMoves(v) = {} ∧ degree[v] < K
    auto degree_v = degree->Look(v);
    if (NodeMoves(v)->GetList().empty() && *degree_v < K) {
      //freezeWorkList ← freezeWorkList \ {v}
      freezeWorklist->DeleteNode(v);
      //simplifyWorkList ← simplifyWorkList ∪ {v}
      simplifyWorklist->Append(v);
    }
  }
}
// procedure SelectSpill()
void RegAllocator::SelectSpill() {

  //let u ∈ spillWorkList
  auto m = spillWorklist->GetList().front();
  //heuristic algorithm
  for(const auto &t : spillWorklist->GetList()){
    if(t->Degree() <= m->Degree()) {
      continue ;
    }
    m = t;
  }

  //spillWorkList ← spillWorkList \ {m}
  spillWorklist->DeleteNode(m);
  //simplifyWorkList ← simplifyWorkList ∪ {m}
  simplifyWorklist->Append(m);
  //FreezeMoves(m)
  FreezeMoves(m);
}
//procedure AssignColor()
void RegAllocator::AssignColor() {

  //while SelectStack not empty
  while (selectStack->GetList().empty() == false) {
    //let n = pop(SelectStack)
    auto n = selectStack->GetList().front();
    selectStack->DeleteNode(n);

    //okColors ← [0, … , K-1]
    okColors.clear();
    for (const auto &temp : reg_manager->Registers()->GetList()) {
      temp::Map *temp_map = reg_manager->temp_map_;
      auto *tmp_color = temp_map->Look(temp);
      okColors.insert(tmp_color);
    }

    //forall w ∈ adjList[n]
    auto adjList_n = n->Succ();
    for (const auto &w : adjList_n->GetList()) {
      //if GetAlias[w] ∈ (ColoreNodes ∪ precolored) then
      auto alias_w = GetAlias(w);
      auto tmp_list = coloredNodes->Union(precolored);
      if (tmp_list->Contain(alias_w)) {
        // okColors ← okColors \ {color[GetAlias(w)]}
        auto color_alias_w = color->Look(alias_w->NodeInfo());
        okColors.erase(color_alias_w);
      }
    }
    // if okColor = {} then
    if (okColors.empty()) {
      //spillWorkList ← spillWorkList ∪ {n}
      spilledNodes->Append(n);
    }
    else {
      //coloredNodes ← coloredNodes ∪ {n}
      coloredNodes->Append(n);
      //color[n] ← c
      auto c = okColors.begin();
      color->Enter(n->NodeInfo(), *c);
    }
  }
  // assign color for coalesced_nodes
  for (const auto &n : coalescedNodes->GetList()) {
    auto alias_n = GetAlias(n);
    auto c = color->Look(alias_n->NodeInfo());
    color->Enter(n->NodeInfo(), c);
  }

}
// procedure RewriteProgram()
void RegAllocator::RewriteProgram() {

  auto instrs = assem_instr_->GetList();
  std::string frame_size = frame_->GetLabel() + "_framesize";

  for(const auto &spill_node : spilledNodes->GetList()){
    //Allocate memory locations for each v ∈ spilledNodes
    auto tmp_access = frame_->AllocLocal(true);
    auto spill_access = static_cast<frame::InFrameAccess *>(tmp_access);
    auto new_instr_list = new assem::InstrList();

    auto old_temp = spill_node->NodeInfo();
    //Create a new temporary vi
    auto v_i = temp::TempFactory::NewTemp();
    for(const auto &instr : instrs){
      instr->Temp2Temp(old_temp,v_i);
      if(instr->Use()->Contain(v_i)){
        // a fetch before each use of a vi
        auto fetch_instr = new assem::OperInstr(
            "movq (" + frame_size +
                std::to_string(spill_access->offset) + ")(`s0), `d0",
            new temp::TempList(v_i),
            new temp::TempList(reg_manager->StackPointer()), nullptr);
        new_instr_list->Append(fetch_instr);
      }
      new_instr_list->Append(instr);
      if(instr->Def()->Contain(v_i)){
        // insert a store after each definition of a vi
        auto store_instr = new assem::OperInstr(
            "movq `s0, (" + frame_size +
                std::to_string(spill_access->offset) + ")(`d0)",
            new temp::TempList(reg_manager->StackPointer()),
            new temp::TempList(v_i), nullptr);
        new_instr_list->Append(store_instr);
      }
    }

    assem_instr_ = new_instr_list;
    auto new_node = interf_graph->NewNode(v_i);
    //Put All the vi into a set newTemps
    newTemps->Append(new_node);
  }

  // spilledNode ← {}
  delete spilledNodes;
  spilledNodes = new live::INodeList();
  //initial ← coloredNodes ∪ coalescedNodes ∪ newTemp
  delete initial;
  initial = new live::INodeList();
  auto tmp_list = coalescedNodes->Union(newTemps);
  initial = coloredNodes->Union(tmp_list);
  //coloredNodes ← {}
  delete coloredNodes;
  coloredNodes = new live::INodeList();
  //coalescedNodes ←{}
  delete coalescedNodes;
  coalescedNodes = new live::INodeList();
}
void RegAllocator::deleteUselessMoves(){
  auto new_instr_list = new assem::InstrList();
  for(const auto &instr : assem_instr_->GetList()){
    if(typeid(*instr) == typeid(assem::MoveInstr)){
      auto move_instr = static_cast<assem::MoveInstr *>(instr);
      if(move_instr->src_ != nullptr && move_instr->dst_ != nullptr){
        if(move_instr->src_->GetList().size() ==1 && move_instr->dst_->GetList().size() ==1){
          if(color->Look(move_instr->src_->GetList().front()) ==
              color->Look(move_instr->dst_->GetList().front())){
            continue ;
          }
        }
      }
    }
    new_instr_list->Append(instr);
  }
  assem_instr_ = new_instr_list;
}

>>>>>>> lab6
} // namespace ra