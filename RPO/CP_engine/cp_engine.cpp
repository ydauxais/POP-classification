#include "cp_engine.hh"


void CPEngine::generate_maximal_occurrence_constraint_vector() {
    std::vector<std::vector<unsigned>> transactions = base_->get_positives_event_cardinalities();
    for (unsigned item = 0; item < transactions.size(); item++) {
        for (unsigned transaction = 0; transaction < transactions[item].size(); transaction++) {
                std::vector<unsigned> entry;
                entry.push_back(item); entry.push_back(transaction); entry.push_back(transactions[item][transaction]);
                maximal_occurrence_constraint_.push_back(entry);
                if (closed_) max_card_ = (transactions[item][transaction] >= max_card_)?
                                transactions[item][transaction] : max_card_;
        }
    }
}

void CPEngine::ground_base_variables() {
    std::vector<std::vector<unsigned>> transactions = base_->get_positives_event_cardinalities();
    transactions_ = BoolVarArray(*this, static_cast<int>(transactions[0].size()), 0, 1);

    if (!no_duplicates_) {
        items_ = IntVarArray(*this, static_cast<int>(transactions.size()));
        for (unsigned item = 0; item < transactions.size(); item++) {
            std::vector<unsigned> cardinalities = transactions[item];
            unsigned max = *std::max_element(std::begin(cardinalities), std::end(cardinalities));
            items_[item] = IntVar(*this, 0, max);
        }
    }
    else {
        items_ = IntVarArray(*this, static_cast<int>(transactions.size()), 0, 1);
    }
}

void CPEngine::ground_closed_multiset_constraint() {
    if (!no_duplicates_) {
        for (unsigned item = 0; item < items_to_map_bool_.size(); item++)
            ground_item_constraint_for_closure(items_to_map_bool_[item], item);
        items_to_map_bool_.clear();
        maximal_occurrence_constraint_.clear();
    }
    else {
        BoolVarArgs none(0);
        for (int item = 0; item < items_.size(); item++) {
            BoolVarArgs row;
            for (int transaction = 0; transaction < transactions_.size(); transaction++) {
                if (!base_->is_item_in_transaction(static_cast<unsigned int>(item), 1,
                                                   static_cast<unsigned int>(transaction)))
                    row << transactions_[transaction];
            };
            rel(*this, (sum(row) == 0) == (items_[item] == 1));
        }
    }
}

void CPEngine::ground_item_constraint_for_closure(std::vector<unsigned> item, unsigned eid) {
    IntVarArgs row(static_cast<int>(item.size()));
    for (unsigned i = 0; i < item.size(); i++)
        row[i] = weight_domains_for_closure_[item[i]];
    rel(*this, items_[eid] == min(row));
}

void CPEngine::ground_items_in_transactions_constraints() {
    for (unsigned constraint = 0; constraint < maximal_occurrence_constraint_.size(); constraint++) {
        unsigned item = maximal_occurrence_constraint_[constraint][0];
        unsigned transaction = maximal_occurrence_constraint_[constraint][1];
        unsigned ceil = maximal_occurrence_constraint_[constraint][2];
        rel(*this, (items_[item] <= ceil) == items_in_transactions_[constraint]);

        for (unsigned long i = transactions_to_map_bool_.size(); i<=transaction; i++)
            transactions_to_map_bool_.emplace_back();
        transactions_to_map_bool_[transaction].push_back(constraint);
    }
    if (!closed_)
        maximal_occurrence_constraint_.clear();
}

void CPEngine::ground_items_in_transactions_variables() {
    generate_maximal_occurrence_constraint_vector();
    items_in_transactions_ = BoolVarArray(*this, static_cast<int>(maximal_occurrence_constraint_.size()), 0, 1);
}

void CPEngine::ground_transaction_constraint(std::vector<unsigned> transaction, unsigned tid) {
    BoolVarArgs none(0);
    BoolVarArgs row(static_cast<int>(transaction.size()));
    for (unsigned i = 0; i < transaction.size(); i++)
        row[i] = items_in_transactions_[transaction[i]];
    clause(*this, BOT_AND, row, none, transactions_[tid]);
}

void CPEngine::ground_transactions_constraints() {
    if (!no_duplicates_) {
        for (unsigned transaction = 0; transaction < transactions_to_map_bool_.size(); transaction++)
            ground_transaction_constraint(transactions_to_map_bool_[transaction], transaction);
        transactions_to_map_bool_.clear();
    }
    else {
        BoolVarArgs none(0);
        for (int transaction = 0; transaction < transactions_.size(); transaction++) {
            BoolVarArgs row;
            for (int item = 0; item < items_.size(); item++) {
                if (!base_->is_item_in_transaction(static_cast<unsigned int>(item), 1,
                                                   static_cast<unsigned int>(transaction)))
                    row << expr(*this, items_[item] == 0);
            }

            clause(*this, BOT_AND, row, none, transactions_[transaction]);
        }
    }
}

void CPEngine::init() {
    ground_base_variables();
    if (!no_duplicates_) {
        ground_items_in_transactions_variables();
        ground_items_in_transactions_constraints();
    }
    ground_transactions_constraints();
    set_length_constraints();
    set_minimal_support_constraint();

    if (closed_) {
        if (!no_duplicates_)
            set_weight_variable_for_closure();
        ground_closed_multiset_constraint();
    }
}

bool
CPEngine::is_relevant_singleton(const std::vector<unsigned> &tid_list, const std::vector<unsigned> &negative_tid_list) const{
    for (const RelevantConstraint& constraint : *relevant_constraints_) {
        const std::vector<unsigned>& ctid_list = constraint.get_tid_list()[1];
        if (std::includes(ctid_list.cbegin(), ctid_list.cend(),
                 tid_list.cbegin(), tid_list.cend())) {
            const std::vector<unsigned> &cnegative_tid_list = constraint.get_tid_list()[0];
            if (std::includes(negative_tid_list.cbegin(), negative_tid_list.cend(),
                              cnegative_tid_list.cbegin(), cnegative_tid_list.cend())) {
                return false;
            }
        }
    }
    return true;
}

void CPEngine::print(std::ostream & os) const {
    std::vector<unsigned> multiset;
    for (unsigned item = 0; item < static_cast<unsigned>(items_.size()); item++)
        multiset.push_back(static_cast<unsigned>(items_[item].val()));
    Multiset m(base_, multiset);
    print_discriminant_episodes(m, os);
}

void CPEngine::print_cp_discriminant_episodes(const Multiset &m,
                                              const std::map<unsigned, std::vector<std::vector<std::vector<int>>>> &occurrences,
                                              const std::vector<unsigned> &tid_list,
                                              const std::vector<unsigned> &negative_tid_list, std::ostream &os) const {
    std::vector<Episode> episodes;

    CPRuleLearnerOptions opt("Rule learning");
    opt.minimal_support_ = minimal_support_;
    opt.minimal_growth_rate_ = minimal_growth_rate_;
    opt.occurrences_ = occurrences;
    opt.multiset = m;
    opt.extracted_episodes_ = &episodes;
    opt.relevant_ = relevant_;
    opt.numeric_intervals_ = numeric_intervals_;
    opt.relevant_constraints_ = *relevant_constraints_;
    opt.tid_list_ = tid_list;
    opt.negative_tid_list_ = negative_tid_list;
    opt.unsupervised_ = unsupervised_;
    opt.solutions(0);

#ifdef _WIN32
    opt.out_file("nul");
    opt.log_file("nul");
#else
    opt.out_file("/dev/null");
    opt.log_file("/dev/null");
#endif

    Script::run<CPRuleLearner<int>,BAB,CPRuleLearnerOptions>(opt);

    for (const Episode& e : episodes) {
        if (e.is_valid()) {
            if (!no_output_) os << std::string(e) << std::endl;

            RelevantConstraint r = RelevantConstraint(e.get_tid_list(), tid_list, negative_tid_list);
            if (relevant_)
                relevant_constraints_->push_back(r);
            if (!no_output_ && output_tid_lists_) r.print_tid_list(os);
            if (episodes_) {
                Episode new_episode(e);
                new_episode.set_tid_list(r.get_tid_list());

                if (relevant_ && !closed_) {
                    const TidMap& tids = new_episode.get_tid_list();

                    for (Episode& episode: *episodes_) {
                        if (episode.is_valid()) {
                            const TidMap& other_tids = episode.get_tid_list();
                            if (other_tids[1].size() <= tids[1].size()) {
                                if (std::includes(tids[1].begin(), tids[1].end(), other_tids[1].begin(), other_tids[1].end())) {
                                    if (tids[1].size() == other_tids[1].size()) {
                                        if (other_tids[0].size() >= tids[0].size()) {
                                            bool inclusion = std::includes(other_tids[0].begin(),
                                                                           other_tids[0].end(),
                                                                           tids[0].begin(),
                                                                           tids[0].end());
                                            if (inclusion)
                                                episode.set_not_valid();
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                episodes_->push_back(new_episode);
            }

            if (!no_output_) os << std::endl;
        }
    }
}

void CPEngine::print_discriminant_episodes(const Multiset &m, std::ostream &os) const {

    std::vector<unsigned> tid_list;
    for (unsigned transaction = 0; transaction < static_cast<unsigned>(transactions_.size()); transaction++) {
        if (transactions_[transaction].val())
            tid_list.push_back(transaction);
    }

    std::vector<unsigned> negative_tid_list;

    if (m.size() > 0) {
        std::map<unsigned, std::vector<std::vector<std::vector<int>>>> occurrences =
                m.get_occurrences(tid_list, numeric_intervals_, &negative_tid_list, false, -1);

        occurrences[1] = remove_redundant_occurrences(occurrences[1]);
        occurrences[0] = remove_redundant_occurrences(occurrences[0]);

        /* std::cout << std::string(m) << std::endl;
        std::cout << "Bags: " << occurrences[1].size() << std::endl;
        int instances = 0;
        for (const auto& bag: occurrences[1])
            instances += bag.size();
        std::cout << "Instances: " << instances << std::endl; */

        if (m.size() > 1)
            print_cp_discriminant_episodes(m, occurrences, tid_list, negative_tid_list, os);
        else if (tid_list.size() >= negative_tid_list.size() * minimal_growth_rate_) {
                if (is_relevant_singleton(tid_list, negative_tid_list)) {
                    print_singleton_discriminant_episodes(m, tid_list, negative_tid_list, os);
                }
        }
    }
    else {
        for (unsigned i = 0; i < base_->get_negatives_size(); i++)
            negative_tid_list.push_back(i);
        print_singleton_discriminant_episodes(m, tid_list, negative_tid_list, os);
    }
}

void CPEngine::print_singleton_discriminant_episodes(const Multiset &m, const std::vector<unsigned> &tid_list,
                                                     const std::vector<unsigned> &negative_tid_list,
                                                     std::ostream &os) const {
    Episode e(m, {}, false);
    e.set_tid_list(TidMap({{1, tid_list}, {0, negative_tid_list}}));
    e.set_frequencies({{1, static_cast<const unsigned int &>(tid_list.size())},
                       {0, static_cast<const unsigned int &>(negative_tid_list.size())}});
    if (episodes_)
        episodes_->push_back(e);
    RelevantConstraint r = RelevantConstraint(e.get_tid_list());
    relevant_constraints_->push_back(r);

    if (!no_output_) {
        os << std::string(e) << std::endl;
        if (output_tid_lists_) r.print_tid_list(os);
        os << std::endl;
    }
}

std::vector<std::vector<std::vector<int>>> CPEngine::remove_redundant_occurrences(
        const std::vector<std::vector<std::vector<int>>> &bags) const {
    std::vector<std::vector<std::vector<int>>> res;
    for (const std::vector<std::vector<int>>& bag: bags) {
        std::set<std::vector<int>> bag_set(bag.begin(), bag.end());
        res.emplace_back(bag_set.begin(), bag_set.end());
    }
    return res;
}

void CPEngine::run() {
    if (branch_on_min_)
        branch(*this, items_, INT_VAR_NONE(), INT_VAL_MIN());
    else
        branch(*this, items_, INT_VAR_NONE(), INT_VAL_MAX());
}

void CPEngine::set_length_constraints() {
    if (minimal_length_ > 0)
        rel(*this, sum(items_) >= minimal_length_);
    if (maximal_length_ > 0)
        rel(*this, sum(items_) <= maximal_length_);
}

void CPEngine::set_minimal_support_constraint() {
    linear(*this, transactions_, IRT_GQ, minimal_support_);
}

void CPEngine::set_weight_variable_for_closure() {
    weight_domains_for_closure_ = IntVarArray(*this, static_cast<int>(maximal_occurrence_constraint_.size()));
    for (unsigned constraint = 0; constraint < maximal_occurrence_constraint_.size(); constraint++) {
        unsigned item = maximal_occurrence_constraint_[constraint][0];
        unsigned transaction = maximal_occurrence_constraint_[constraint][1];
        unsigned ceil = maximal_occurrence_constraint_[constraint][2];

        weight_domains_for_closure_[constraint] =  IntVar(*this, IntSet(
                IntArgs(std::vector<int>({static_cast<int>(ceil), static_cast<int>(max_card_+1)}))));
        rel(*this, (transactions_[transaction] == 1) == (weight_domains_for_closure_[constraint] == ceil));

        for (unsigned long i = items_to_map_bool_.size(); i<=item; i++)
            items_to_map_bool_.emplace_back();
        items_to_map_bool_[item].push_back(constraint);
    }
}