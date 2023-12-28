#ifndef TIGER_REGALLOC_REGALLOC_H_
#define TIGER_REGALLOC_REGALLOC_H_

#include "tiger/codegen/assem.h"
#include "tiger/codegen/codegen.h"
#include "tiger/frame/frame.h"
#include "tiger/frame/temp.h"
#include "tiger/liveness/liveness.h"
#include "tiger/regalloc/color.h"
#include "tiger/util/graph.h"

namespace ra {

class Result {
public:
  temp::Map *coloring_;
  assem::InstrList *il_;

  Result() : coloring_(nullptr), il_(nullptr) {}
  Result(temp::Map *coloring, assem::InstrList *il)
      : coloring_(coloring), il_(il) {}
  Result(const Result &result) = delete;
  Result(Result &&result) = delete;
  Result &operator=(const Result &result) = delete;
  Result &operator=(Result &&result) = delete;
  ~Result();
};

class RegAllocator {
  /* TODO: Put your lab6 code here */
<<<<<<< HEAD
=======
private:

  temp::Map *color;
  std::set<std::string *> okColors;

  int K;

  live::IGraphPtr interf_graph;
  tab::Table<temp::Temp, live::INode> *temp_node_map;
  assem::InstrList *assem_instr_;

  frame::Frame *frame_;
  std::unique_ptr<ra::Result> result_;

  live::INodeListPtr precolored;
  live::INodeListPtr initial;
  live::INodeListPtr selectStack;
  live::INodeListPtr newTemps;

  live::INodeListPtr simplifyWorklist;
  live::INodeListPtr freezeWorklist;
  live::INodeListPtr spillWorklist;

  live::INodeListPtr spilledNodes;
  live::INodeListPtr coalescedNodes;
  live::INodeListPtr coloredNodes;

  live::MoveList *workListMoves;
  live::MoveList *activeMoves;
  live::MoveList *coalescedMoves;
  live::MoveList *constrainedMoves;
  live::MoveList *frozenMoves;

  tab::Table<live::INode, int> *degree;
  tab::Table<live::INode, live::INode> *alias;
  tab::Table<live::INode, live::MoveList> *moveList;



public:
  RegAllocator(frame::Frame *frame, std::unique_ptr<cg::AssemInstr> assem_instr);
  void RegAlloc();
  std::unique_ptr<ra::Result> TransferResult() { return std::move(result_); }

  void LivenessAnalysis();
  void Clear();
  void Init();
  void Build();
  void AddEdge(live::INodePtr u, live::INodePtr v);
  void MakeWorklist();
  live::INodeListPtr Adjacent(live::INodePtr n);
  live::MoveList *NodeMoves(live::INodePtr n);
  bool MoveRelated(live::INodePtr n);
  void Simplify();
  void Decrement(live::INodePtr m);
  void EnableMoves(live::INodeListPtr nodes);
  void Coalesce();
  void AddWorkList(live::INodePtr u);
  bool OK(live::INodePtr t, live::INodePtr r);
  bool Conservative(live::INodeListPtr nodes);
  live::INodePtr GetAlias(live::INodePtr n);
  void Combine(live::INodePtr u, live::INodePtr v);
  void Freeze();
  void FreezeMoves(live::INodePtr u);
  void SelectSpill();
  void AssignColor();
  void RewriteProgram();
  void deleteUselessMoves();



>>>>>>> lab6
};

} // namespace ra

#endif