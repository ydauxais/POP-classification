#ifndef GDE_CP_INT_RULE_LEARNER_HH
#define GDE_CP_INT_RULE_LEARNER_HH

#include <type_traits>

#include <algorithm>
#include <map>
#include <set>

#include <gecode/float.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>
#include <gecode/search.hh>

#include "cp_rule_learner_options.hh"
#include "multiset.hh"
#include "episode.hh"
#include "relevant_constraint.hh"

using namespace Gecode;


template <class T>
class CPRuleLearner: public Script {
    typedef typename std::conditional<std::is_same<T, int>::value, IntVarArray, FloatVarArray>::type var_array_t;
    typedef typename std::conditional<std::is_same<T, int>::value, IntVarArgs, FloatVarArgs>::type var_args_t;
    typedef typename std::conditional<std::is_same<T, int>::value, IntVar, FloatVar>::type var_t;

public:
    int minimal_support_;
    float minimal_growth_rate_;

private:
    var_array_t lower_bounds_;
    var_array_t upper_bounds_;

    BoolVarArray positive_instances_;
    BoolVarArray negative_instances_;
    BoolVarArray positive_bags_;
    BoolVarArray negative_bags_;

    std::vector<Episode>* extracted_episodes_;
    bool check_relevant_bfs_;
    std::shared_ptr<std::map<std::vector<unsigned>, std::vector<unsigned>>> closed_candidates_;
    bool relevant_;
    bool numeric_intervals_;
    bool unsupervised_;
    std::shared_ptr<std::vector<std::pair<std::vector<unsigned>, std::vector<unsigned>>>> relevant_constraints_;
    unsigned constraint_level_;
    const Multiset& m_;

public:
    CPRuleLearner(CPRuleLearner& s) : Script(s), minimal_support_(s.minimal_support_),
    minimal_growth_rate_(s.minimal_growth_rate_), extracted_episodes_(s.extracted_episodes_),
    check_relevant_bfs_(s.check_relevant_bfs_),
    closed_candidates_(s.closed_candidates_),
    relevant_(s.relevant_), numeric_intervals_(s.numeric_intervals_), unsupervised_(s.unsupervised_),
    relevant_constraints_(s.relevant_constraints_), constraint_level_(s.constraint_level_), m_(s.m_)
    {
        lower_bounds_.update(*this, s.lower_bounds_);
        upper_bounds_.update(*this, s.upper_bounds_);

        positive_instances_.update(*this, s.positive_instances_);
        negative_instances_.update(*this, s.negative_instances_);
        positive_bags_.update(*this, s.positive_bags_);
        negative_bags_.update(*this, s.negative_bags_);
    }

    explicit CPRuleLearner(const CPRuleLearnerOptions& opt): Script(opt), minimal_support_(opt.minimal_support_),
                                                             minimal_growth_rate_(opt.minimal_growth_rate_),
                                                             extracted_episodes_(opt.extracted_episodes_),
                                                             check_relevant_bfs_(true),
                                                             closed_candidates_(new std::map<std::vector<unsigned>, std::vector<unsigned>>()),
                                                             relevant_(opt.relevant_),
                                                             numeric_intervals_(opt.numeric_intervals_),
                                                             unsupervised_(opt.unsupervised_),
                                                             relevant_constraints_(new std::vector<std::pair<std::vector<unsigned>, std::vector<unsigned>>>()),
                                                             constraint_level_(0),
                                                             m_(opt.multiset){
        init(opt);
        run();
    }

    Space* copy() override {
        return new CPRuleLearner(*this);
    }

    void print(std::ostream&) const override;
    void constrain(const Space& _b) override;

private:
    std::vector<std::pair<T, T>> compute_bounds(const std::vector<std::vector<std::vector<T>>> &occurrences);
    std::set<T> compute_bound_set(const std::vector<std::vector<std::vector<T>>> &occurrences, unsigned index) const;
    unsigned count_instances(const std::vector<std::vector<std::vector<T>>> &bags) const;
    void generate_bag_constraints(
            const std::vector<std::vector<std::vector<T>>>& bags,
            BoolVarArray& bag_vars,
            BoolVarArray& instance_vars
    );
    std::pair<T, T> generate_bound_domains(const std::vector<std::vector<std::vector<T>>> &occurrences);
    void generate_instance_constraints(const std::vector<std::vector<std::vector<T>>> &bags,
            BoolVarArray &instance_vars);

    void set_additional_constraints();
    void set_close_constraint(const std::vector<std::vector<std::vector<T>>> &occurrences,
            const std::pair<T, T>& extremum);

    void set_previous_relevant_constraint(const CPRuleLearnerOptions& options);

    void init(const CPRuleLearnerOptions& options);
    void run();
};

/*
 * Valid template specializations
 * - Only int is valid for now
 */
template class CPRuleLearner<int>;

#endif //GDE_CP_INT_RULE_LEARNER_HH
