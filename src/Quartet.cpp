#include <sstream>
#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include "Quartet.hpp"
#include "util/Options.hpp"
#include "util/Logger.hpp"

using namespace std;
#ifdef _WIN32
#define strtok_r strtok_s
#endif

Quartet::Quartet(TaxonSet& ts, Taxon a, Taxon b, Taxon c, Taxon d) :
  ts(ts)
{
  taxa[0] = a;
  taxa[1] = b;
  taxa[2] = c;
  taxa[3] = d;
}

double Quartet::parse(char* str){
  char* p = str;
  while(*p && *p != ':' && *p != ';'){ p++; }
  assert(*p);
  double weight = 1;
  if (*p == ';') {
    weight = atof(p+2);
  }
  if (*p == ':') {
    weight = atof(p+1);
  }
  *p = '\0';
  
  if (str[0] == '(') {
    parse_newick(str);
  } else {
    parse_wqmc(str);
  }

  //  cout << this->str() << '\t' << weight << endl;
  
  return weight;
}

void Quartet::parse_newick(char* c) {
  char* saveptr;
  int i = 0;
  while(char* token=strtok_r(c, "(),", &saveptr)) {
    c = NULL;
    taxa[i] = ts[token];
    i++;
  }
}


void Quartet::parse_wqmc(char* c) {
  char* saveptr;
  int i = 0;
  while(char* token=strtok_r(c, "|,", &saveptr)) {
    c = NULL;
    taxa[i] = ts[token];
    i++;
  }
}

string Quartet::str() {
  stringstream ss;
  ss << "((" << ts[taxa[0]] << ", " << ts[taxa[1]] << "),(" << ts[taxa[2]] << ", " << ts[taxa[3]] << "))";
  return ss.str();
}


QuartetDict::QuartetDict(TaxonSet& ts, string quartetfile) :
  ts(ts)
{

  array_type::extent_gen extents;
  DEBUG << "Making quartet dict with size " << ts.size() << endl;
  array.resize(extents[ts.size()][ts.size()][ts.size()][ts.size()]);
  size_t i,j,k,l;

  for (i = 0; i < ts.size(); i++){
      for (j = 0; j < ts.size(); j++)
	  for (k = 0; k < ts.size(); k++)
	    for (l = 0; l < ts.size(); l++){
	      array[i][j][k][l] = 0;
	    }
  }

  if (quartetfile.size()) {
    read_file(quartetfile);
  }
};

void QuartetDict::read_file(string quartetfile) {
  
  string s;
  double w;
  
  Quartet q(ts);
  ifstream infile(quartetfile);
  
  while(!infile.eof()) {
    getline(infile, s);
    if (s.size() == 0)
      continue;
    w = q.parse(&(s[0]));

    set(q.a(),q.b(),q.c(),q.d(),w);
    
  }  
}


double QuartetDict::operator()(Taxon a, Taxon b, Taxon c, Taxon d) {
  return array[a][b][c][d];
}


void QuartetDict::set(Taxon a, Taxon b, Taxon c, Taxon d, double val) {
  array[a][b][c][d] = val;
  array[b][a][c][d] = val;  
  array[a][b][d][c] = val;
  array[b][a][d][c] = val;  
  array[c][d][a][b] = val;
  array[c][d][b][a] = val;
  array[d][c][a][b] = val;
  array[d][c][b][a] = val;

}


double QuartetDict::operator()(Quartet& q) {
  return array[q.a()][q.b()][q.c()][q.d()];
}

string QuartetDict::str() {
  stringstream ss;
  size_t i,j,k,l;
  for (i = 0; i < ts.size(); i++) {
     for (j = 0; j < i; j++) {
       for (k = 0; k < j; k++) {
	 for (l = 0; l < k; l++) {
	   Quartet q1(ts,i,j,k,l);
	   Quartet q2(ts,i,k,l,j);
	   Quartet q3(ts,l,i,j,k);
	   ss << q1.str() << ":" << (*this)(q1) << endl;
	   ss << q2.str() << ":" << (*this)(q2) << endl;
	   ss << q3.str() << ":" << (*this)(q3) << endl;
	 }
       }
     } 
  }
  return ss.str() ;
}

void QuartetDict::test() {
  cout << "QuartetDict::test()" << endl;
  TaxonSet ts("{1,2,3,4}");
  QuartetDict qd(ts, "quartetdict_test");
  cout << qd.str();
}


void Quartet::test() {
  // //  TaxonSet ts;
  // Quartet q(ts);
  // string c("((1,2),(3,4)):3.5");
  // q.parse(&c[0]);
  // cout << q.str() << endl;
  // assert(q.taxa[0] == ts["1"]);
  // assert(q.taxa[1] == ts["2"]);
  // assert(q.taxa[2] == ts["3"]);
  // assert(q.taxa[3] == ts["4"]);
}

QuartetDict* QuartetDict::cl_qd = 0;

QuartetDict* QuartetDict::cl(TaxonSet& ts) {
  if (cl_qd) {
    DEBUG << "Returning existing quartet dict" << endl;
    return cl_qd;
  }
  string quartetFile;
  Options::get("q quartets", &quartetFile);
  DEBUG << "Making quartet dict from " << quartetFile << endl;
  cl_qd = new QuartetDict(ts, quartetFile);
  return cl_qd;
}
