%module rpo
%{
#include "tid_map.hh"
#include "item.hh"
#include "multiset.hh"
#include "episode.hh"
#include "vertical_database.hh"
#include "cp_engine_options.hh"
#include "python_functions.hh"
#include "relationship_map.hh"
%}


%include "typemaps.i"
%include "std_map.i"
%include "std_pair.i"
%include "std_string.i"
%include "std_vector.i"

namespace std {
%template(UVector) vector<unsigned>;
%template(UMatrix) vector<vector<unsigned>>;
%template(StringVector) vector<string>;
%template(EpisodeVector) vector<Episode>;
%template(ItemPair) pair<Item, Item>;
%template(EventVector) vector<pair<unsigned, unsigned>>;
%template(EventStrVector) vector<pair<string, unsigned>>;
%template(ClassVerticalDatabase) vector<vector<vector<unsigned>>>;
}

%ignore *::operator std::string;
%rename(__str__) *::operator std::string;

%ignore *::operator[];
%rename(__getitem__) *::operator[];

%rename(__getitem__) RelationshipMap::get_relationship;

%include "tid_map.hh"
%include "item.hh"
%include "multiset.hh"
%include "episode.hh"
%include "vertical_database.hh"
%include "cp_engine_options.hh"
%include "python_functions.hh"
%include "relationship_map.hh"
