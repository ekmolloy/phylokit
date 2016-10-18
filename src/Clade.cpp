#include "Clade.hpp"
#include "util/Options.hpp"
#include "util/Logger.hpp"
#include <sstream>
#include <unordered_set>
#include <unordered_map>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <cmath>

Clade::Clade(TaxonSet& ts_, string& str) :
  taxa(ts_.size()),
  ts(ts_),
  sz(0)
{
  char* cladestr = &(str[1]);
  char* token;
  char* saveptr;

  while((token = strtok_r(cladestr, ",} ", &saveptr))) {
    cladestr = NULL;
    add(ts[string(&(token[0]))]);
  }
}

Clade::Clade(TaxonSet& ts_) :
  taxa(ts_.size()),
  ts(ts_),
  sz(0)
{}

Clade::Clade(TaxonSet& ts_, clade_bitset& taxa) :
  taxa(taxa),
  ts(ts_),
  sz(taxa.popcount())
{
}

Clade::Clade(TaxonSet& ts_, unordered_set<Taxon>& taxa) :
  taxa(ts_.size()),
  ts(ts_),
  sz(taxa.size())
{
  for (Taxon t : taxa) {
    add(t);
  }
}

Clade::Clade(const Clade& other) :
  taxa(other.taxa),
  ts(other.ts),
  sz(other.sz)
{
}



Clade& Clade::operator=(const Clade& other) {
   taxa = other.taxa;
   ts = other.ts;
   sz = other.sz;
   return *this;
}

bool Clade::operator==(const Clade& other) const {
   return taxa == other.taxa;
}





string Clade::str() const {
  stringstream ss;
  vector<string> strings;

  for (Taxon i : *this) {
    strings.push_back(ts[i]);
  }

  sort(strings.begin(), strings.end());
  
  ss << '{';
  for (string s : strings) {
    ss << s << ", ";
  }
  ss << '}';
  return ss.str();
}




string Clade::newick_str(TripartitionScorer& scorer, vector<Clade>& clades) {
  //  BOOST_LOG_TRIVIAL(info) << str() << endl;
  if (size() == 0) {
    return "";
  }
  if (size() == 1) {
    return ts[*begin()];
  }
  if (size() == 2) {
    stringstream ss;

    vector<Taxon> tv;
    for (Taxon t : *this) {
      tv.push_back(t);
    }
    
    ss << "("<<ts[tv[0]] << "," << ts[tv[1]] << ")" ;    
    
    return ss.str();

  }


  stringstream ss;
  pair<clade_bitset, clade_bitset>& subclades = scorer.get_subclades(taxa, clades);

  Clade c1(ts, subclades.first);
  Clade c2(ts, subclades.second);
  
  //  BOOST_LOG_TRIVIAL(debug) << str() << c1.str() << c2.str() << (int)scorer.get_score(taxa) <<endl;

  Tripartition tp(ts, *this, c1);
  
  ss << "(" << c1.newick_str(scorer, clades) << "," << c2.newick_str(scorer, clades) << ")";
  return ss.str();
}

void Clade::test() {
  string str = string("{tx1, tx8, tx3, tx2, tx4}");
  TaxonSet ts(str);
  cout << Clade(ts, str).str() << endl;
  cout << ts.str() << endl;
}

Clade Clade::overlap(const Clade& other) const {
  clade_bitset cb = other.taxa & taxa;
  return Clade(ts, cb);
}

bool Clade::contains(const Clade& other) const {
  bool status = 1;
  for (size_t i = 0; i < taxa.cap; i++) {
    status &= ((other.taxa.data[i] & taxa.data[i]) == other.taxa.data[i]);
  }
  
  return status;
}
bool Clade::contains(const Taxon taxon) const {
  return taxa.get(taxon);
}

void Clade::add(const Taxon taxon) {
  taxa.set(taxon);
  sz++;
}

Clade Clade::complement() const {
  BitVectorFixed comp = ts.taxa_bs & (~taxa);
  Clade c(ts, comp);
  return c;
}

Clade Clade::minus(const Clade& other) const {
  BitVectorFixed m(taxa & (~other.taxa));
  Clade c(ts, m);
  return c;
}

int Clade::size() const {
  return sz;
}

double Clade::score(TripartitionScorer& scorer, vector<Clade>& clades, unordered_set<clade_bitset>& cladetaxa) {
  double value;

  
  if (size() == 1) {
    value = 0;
    Clade eclade (ts);
    scorer.set_score(taxa, value, taxa, eclade.taxa);
    return value;
  }

  value = scorer.get_score(taxa);
  if (!std::isnan(value)) {
    return value;
  }
  clade_bitset sub1(ts.size()), sub2(ts.size());

  int invert = 1;

  if (Options::get("maximize")) {
    invert = -1;
  }
  
  if (size() == 2) {
    Clade c1(ts);
    c1.add(*begin());
    Tripartition tp(ts, *this, c1);
    value = invert * scorer.score(tp);

    scorer.set_score(taxa, value, tp.a1.taxa, tp.a2.taxa);
  }
  else {
    for (size_t i = 0; i < clades.size(); i++) {
      Clade& subclade = clades[i];
      if (subclade.size() >= size() || !contains(subclade) || subclade.size() == 0 )
	continue;

      Tripartition tp(ts, *this, subclade);

      if (cladetaxa.count(tp.a1.taxa) == 0 || cladetaxa.count(tp.a2.taxa) == 0)
	continue;

      
      double score = invert * scorer.score(tp) + tp.a1.score(scorer, clades, cladetaxa) + tp.a2.score(scorer, clades, cladetaxa);
      if (std::isnan(value) || (score < value) ) {
	value = score;
	sub1 = tp.a1.taxa;
	sub2 = tp.a2.taxa;	
      }
    }
    scorer.set_score(taxa, value, sub1, sub2);    
  }
  
  
  return value;
  
}

Tripartition::Tripartition(TaxonSet& ts, Clade& clade, Clade& subclade) :
  a1(clade.minus(subclade)),
  a2(subclade),
  rest(clade.complement())
{
  // assert(clade.contains(subclade));

  // assert(a1.overlap(rest).size() == 0);
  // assert(a2.overlap(rest).size() == 0);
  // assert(a2.overlap(a1).size() == 0);
}

void Clade::do_swap(Clade& other) {
  std::swap(taxa, other.taxa);
  std::swap(sz, other.sz);
}


string Tripartition::str()  const {
  
  assert(a1.overlap(rest).size() == 0);
  assert(a2.overlap(rest).size() == 0);
  assert(a2.overlap(a1).size() == 0);
  return "{" + a1.str() + "/" + a2.str() + "/" + rest.str() + "}";
}


string Bipartition::str() const {
  return "{" + a1.str() + " " + a2.str() + "}";
}

namespace std
{
    template<>
    void swap<Clade>(Clade& lhs, Clade& rhs)
    {
      lhs.do_swap(rhs);
    }
}
