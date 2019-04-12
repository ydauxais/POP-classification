#ifndef GENERALIZED_DISCRIMINANT_EPISODES_CP_ENGINE_HH
#define GENERALIZED_DISCRIMINANT_EPISODES_CP_ENGINE_HH

#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <vector>

#include <gecode/int.hh>
#include <gecode/minimodel.hh>
#include <gecode/search.hh>
#include <cstdlib>
#include <wait.h>

#include "cp_engine_options.hh"
#include "cp_rule_learner_options.hh"
#include "cp_rule_learner.hh"
#include "database.hh"
#include "episode.hh"
#include "multiset.hh"
#include "relevant_constraint.hh"

using namespace Gecode;

class CPEngine: public Script {

protected:
    std::shared_ptr<Database> base_;
    IntVarArray items_;
    BoolVarArray transactions_;
    // Sparse matrix of maximal size items (rows) x transactions (columns)
    BoolVarArray items_in_transactions_;
    // Only if closed multiset
    IntVarArray weight_domains_for_closure_;

    std::vector<std::vector<unsigned>> items_to_map_bool_;
    std::vector<std::vector<unsigned>> maximal_occurrence_constraint_;
    std::vector<std::vector<unsigned>> transactions_to_map_bool_;

    unsigned minimal_support_;
    float minimal_growth_rate_;
    unsigned minimal_length_;
    unsigned maximal_length_;
    unsigned max_card_;
    bool branch_on_min_;
    bool closed_;
    bool relevant_;
    bool numeric_intervals_;
    bool output_tid_lists_;
    bool no_output_;
    bool unsupervised_;
    bool no_duplicates_{false};

    std::string ml_algorithm_;

    std::shared_ptr<std::vector<RelevantConstraint>> relevant_constraints_;
    std::shared_ptr<std::vector<Episode>> episodes_;

public:

    CPEngine(CPEngine& s) : Script(s), base_(s.base_), minimal_support_(s.minimal_support_),
        minimal_growth_rate_(s.minimal_growth_rate_),
        minimal_length_(s.minimal_length_), maximal_length_(s.maximal_length_),
        max_card_(1), branch_on_min_(s.branch_on_min_),
        closed_(s.closed_), relevant_(s.relevant_), numeric_intervals_(s.numeric_intervals_),
        output_tid_lists_(s.output_tid_lists_), no_output_(s.no_output_), unsupervised_(s.unsupervised_),
        ml_algorithm_(s.ml_algorithm_), relevant_constraints_(s.relevant_constraints_),
        episodes_(s.episodes_)
        {
        items_.update(*this, s.items_);
        transactions_.update(*this, s.transactions_);
        items_in_transactions_.update(*this, s.items_in_transactions_);
    }

    explicit CPEngine(const CPEngineOptions& opt): Script(opt), base_(opt.base_), minimal_support_(opt.minimal_support_),
            minimal_growth_rate_(opt.minimal_growth_rate_),
            minimal_length_(opt.minimal_length_), maximal_length_(opt.maximal_length_),
            max_card_(1), branch_on_min_(opt.branch_on_min_),
            closed_(opt.closed_), relevant_(opt.relevant_), numeric_intervals_(opt.numeric_intervals_),
            output_tid_lists_(opt.output_tid_lists_), no_output_(opt.no_output_),
            unsupervised_(opt.unsupervised_), ml_algorithm_("cp"),
            relevant_constraints_(std::make_shared<std::vector<RelevantConstraint>>()),
            episodes_(opt.episodes_){
        init();
        run();
    }

    Space* copy() override {
        return new CPEngine(*this);
    }

    void init();

    void print(std::ostream&) const override;

    void run();

private:
    void print_discriminant_episodes(const Multiset &m, std::ostream &os) const;
    void print_cp_discriminant_episodes(const Multiset &m,
                                            const std::map<unsigned, std::vector<std::vector<std::vector<int>>>> &occurrences,
                                            const std::vector<unsigned> &tid_list,
                                            const std::vector<unsigned> &negative_tid_list, std::ostream &os) const;
    void print_singleton_discriminant_episodes(const Multiset &m,
                                        const std::vector<unsigned> &tid_list,
                                        const std::vector<unsigned> &negative_tid_list, std::ostream &os) const;

    void generate_maximal_occurrence_constraint_vector();

    void ground_item_constraint_for_closure(std::vector<unsigned> item, unsigned eid);
    void ground_items_in_transactions_constraints();
    void ground_base_variables();
    void ground_items_in_transactions_variables();
    void ground_transaction_constraint(std::vector<unsigned> transaction, unsigned tid);
    void ground_transactions_constraints();

    void ground_closed_multiset_constraint();

    bool is_relevant_singleton(const std::vector<unsigned> &tid_list, const std::vector<unsigned> &negative_tid_list) const;

    void set_length_constraints();
    void set_minimal_support_constraint();
    void set_weight_variable_for_closure();

    std::vector<std::vector<std::vector<int>>> remove_redundant_occurrences(
            const std::vector<std::vector<std::vector<int>>> &bags) const;
};

#endif //GENERALIZED_DISCRIMINANT_EPISODES_CP_ENGINE_HH
