#ifndef GDE_CP_RULE_LEARNER_OPTIONS_HH
#define GDE_CP_RULE_LEARNER_OPTIONS_HH

#include <map>
#include <vector>

#include <gecode/driver.hh>
#include "database.hh"
#include "multiset.hh"
#include "episode.hh"
#include "relevant_constraint.hh"

class CPRuleLearnerOptions: public Gecode::Options {

public:
    int minimal_support_;
    float minimal_growth_rate_;
    float coverage_mult_weight_;
    float coverage_add_weight_;
    bool relevant_;
    bool numeric_intervals_;
    bool unsupervised_;

    std::vector<Episode>* extracted_episodes_;
    std::vector<RelevantConstraint> relevant_constraints_;
    std::vector<unsigned> tid_list_;
    std::vector<unsigned> negative_tid_list_;
    std::map<unsigned, std::vector<std::vector<std::vector<int>>>> occurrences_;


    Multiset multiset;

public:
    explicit CPRuleLearnerOptions(const char* s): Gecode::Options(s),
    minimal_support_(0), minimal_growth_rate_(0), coverage_mult_weight_(1), coverage_add_weight_(0),
    relevant_(false), numeric_intervals_(false), unsupervised_(false), extracted_episodes_(nullptr){};
};


#endif //GDE_CP_RULE_LEARNER_OPTIONS_HH
