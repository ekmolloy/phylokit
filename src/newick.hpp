#ifndef __NEWICK_HPP__
#define __NEWICK_HPP__

#include <boost/tokenizer.hpp>
#include <boost/multi_array.hpp>
#include <boost/algorithm/string.hpp>

#include <string>
#include <unordered_set>
#include <iostream>

#include "Clade.hpp"
#include "TreeClade.hpp"
#include "TaxonSet.hpp"

using namespace std;

typedef boost::multi_array<double, 2> dm_type;

int newick_to_ts(const string& s, unordered_set<string>& taxa);


Clade newick_to_taxa(const string& s, TaxonSet& ts);
void newick_to_dm(const string& s, TaxonSet& ts, dm_type& dist_mat, dm_type& mask_mat );
void newick_to_clades(const string& s, TaxonSet& ts, unordered_set<Clade>& clade_set);
Tree newick_to_treeclades(const string& s, TaxonSet& ts);
void newick_to_postorder(const string& s, TaxonSet& ts, vector<Taxon>& order);


string map_newick_names(const string& s, TaxonSet& ts);
string unmap_newick_names(const string& s, TaxonSet& ts);
string unmap_clade_names(const string& s, TaxonSet& ts);
#endif
