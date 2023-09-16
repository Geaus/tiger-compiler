#include "straightline/slp.h"

#include <iostream>

namespace A {
int A::CompoundStm::MaxArgs() const {
  // TODO: put your code here (lab1).
  int a = this->stm1->MaxArgs();
  int b = this->stm2->MaxArgs();
  return a > b ? a : b;
}

Table *A::CompoundStm::Interp(Table *t) const {
  // TODO: put your code here (lab1).
  Table * tmp= this->stm1->Interp(t);
  return this->stm2->Interp(tmp);
}

int A::AssignStm::MaxArgs() const {
  // TODO: put your code here (lab1).
  return this->exp->MaxArgs();
}

Table *A::AssignStm::Interp(Table *t) const {
  // TODO: put your code here (lab1).
  IntAndTable * tmp=this->exp->Interp(t);
  Table * table=tmp->t;
  table = table->Update(this->id,tmp->i);
  return table;

}

int A::PrintStm::MaxArgs() const {
  // TODO: put your code here (lab1).
    return this->exps->MaxArgs();
}

Table *A::PrintStm::Interp(Table *t) const {
  // TODO: put your code here (lab1).
  IntAndTable * tmp=this->exps->Interp(t);
  return tmp->t;
}

// Exp function
// IdExp
int A::IdExp::MaxArgs() const { return 1; }

IntAndTable *A::IdExp::Interp(Table *t) const {
  int value = t->Lookup(this->id);
  IntAndTable * tmp=new IntAndTable(value,t);
  return tmp;
}
// NumExp
int A::NumExp::MaxArgs() const { return 1; }

IntAndTable *A::NumExp::Interp(Table *t) const {
  IntAndTable * tmp=new IntAndTable(this->num,t);
  return tmp;
}
// OpExp
int A::OpExp::MaxArgs() const {
  return 1;
}

IntAndTable *A::OpExp::Interp(Table *t) const {
  IntAndTable * l=left->Interp(t);
  IntAndTable * r=right->Interp(l->t);
  
  int a=l->i;
  int b=r->i;

  switch(oper){

    case TIMES:
    return new IntAndTable(a*b,r->t);
    break;
    case DIV:
    return new IntAndTable(a/b,r->t);
    break;
    case PLUS:
    return new IntAndTable(a+b,r->t);
    break;
    case MINUS:
    return new IntAndTable(a-b,r->t);
    break;

  }

}
// EseqExp
int A::EseqExp::MaxArgs() const {
  int a = this->stm->MaxArgs();
  int b = this->exp->MaxArgs();
  return a > b ? a : b;
}

IntAndTable *A::EseqExp::Interp(Table *t) const {
  Table * tmp=this->stm->Interp(t); 
  return this->exp->Interp(tmp);
}

// ExpList function
// PairExpList
int A::PairExpList::MaxArgs() const {
  int a = this->exp->MaxArgs();
  int b = this->tail->MaxArgs();
  // 
  return a+b;
}

int A::PairExpList::NumExps() const {
  int sum =this->tail->NumExps()+1;
  return sum;
}

IntAndTable *A::PairExpList::Interp(Table *t) const {
  IntAndTable *tmp =this->exp->Interp(t);
  return this->tail->Interp(tmp->t);
}
// LastExpList
int A::LastExpList::MaxArgs() const { return 1; }

int A::LastExpList::NumExps() const {
  return 1;
}

IntAndTable *A::LastExpList::Interp(Table *t) const {
  IntAndTable *tmp=this->exp->Interp(t);
  return tmp;
}

int Table::Lookup(const std::string &key) const {
  if (id == key) {
    return value;
  } else if (tail != nullptr) {
    return tail->Lookup(key);
  } else {
    assert(false);
  }
}

Table *Table::Update(const std::string &key, int val) const {
  return new Table(key, val, this);
}
} // namespace A
