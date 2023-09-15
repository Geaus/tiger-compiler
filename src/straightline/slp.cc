#include "straightline/slp.h"

#include <iostream>

namespace A {
int A::CompoundStm::MaxArgs() const {
  // TODO: put your code here (lab1).
}

Table *A::CompoundStm::Interp(Table *t) const {
  // TODO: put your code here (lab1).
}

int A::AssignStm::MaxArgs() const {
  // TODO: put your code here (lab1).
}

Table *A::AssignStm::Interp(Table *t) const {
  // TODO: put your code here (lab1).
}

int A::PrintStm::MaxArgs() const {
  // TODO: put your code here (lab1).
}

Table *A::PrintStm::Interp(Table *t) const {
  // TODO: put your code here (lab1).
}

//Exp function
//IdExp
int A::IdExp::MaxArgs() const {

}

IntAndTable A::IdExp::Interp(Table *t) const {

}
//NumExp
int A::NumExp::MaxArgs() const {

}

IntAndTable A::NumExp::Interp(Table *t) const {

}
//OpExp
int A::OpExp::MaxArgs() const {

}

IntAndTable A::OpExp::Interp(Table *t) const {

}
//EseqExp
int A::EseqExp::MaxArgs() const {

}

IntAndTable A::EseqExp::Interp(Table *t) const {

}

//ExpList function
//PairExpList
int A::PairExpList::MaxArgs() const {

}

int A::PairExpList::NumExps() const {

}

IntAndTable A::PairExpList::Interp(Table *t) const {

}
//LastExpList
int A::LastExpList::MaxArgs() const {

}

int A::LastExpList::NumExps() const {

}

IntAndTable A::LastExpList::Interp(Table *t) const {

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
}  // namespace A
