#include "cp_rule_learner.hh"


template <class T>
std::vector<std::pair<T, T>>
CPRuleLearner<T>::compute_bounds(const std::vector<std::vector<std::vector<T>>> &occurrences) {
    std::vector<std::pair<T, T>> bounds(occurrences[0][0].size());
    for (const auto& bag: occurrences) {
        for (const auto& instance: bag) {
            for (unsigned i = 0; i < instance.size(); i++) {
                bounds[i].first = std::min(bounds[i].first, instance[i]);
                bounds[i].second = std::max(bounds[i].second, instance[i]);
            }
        }
    }
    return bounds;
}

template <class T>
std::set<T>
CPRuleLearner<T>::compute_bound_set(const std::vector<std::vector<std::vector<T>>> &occurrences, unsigned index) const {
    std::set<T> bounds;
    for (const auto& bag: occurrences)
        for (const auto& instance: bag)
            bounds.insert(instance[index]);
    return bounds;
}

template <class T>
void
CPRuleLearner<T>::constrain(const Space &) {
    if (!unsupervised_ && relevant_) {
        for (; constraint_level_ < relevant_constraints_->size(); constraint_level_++) {
            BoolVarArgs not_covered_positives;
            BoolVarArgs covered_negatives;

            const auto& constraint = (*relevant_constraints_)[constraint_level_];
            for (unsigned i: constraint.first)
                not_covered_positives << positive_bags_[i];
            for (unsigned i: constraint.second)
                covered_negatives << negative_bags_[i];

            rel(*this, (sum(not_covered_positives) > 0) || (sum(covered_negatives) < covered_negatives.size()));
        }
    }
}

template<class T>
unsigned CPRuleLearner<T>::count_instances(const std::vector<std::vector<std::vector<T>>> &bags) const {
    unsigned total = 0;
    for (const auto& bag : bags)
        total += bag.size();
    return total;
}

template <class T>
void
CPRuleLearner<T>::generate_bag_constraints(const std::vector<std::vector<std::vector<T>>> &bags,
                                             BoolVarArray &bag_vars, BoolVarArray &instance_vars) {
    unsigned instance = 0;
    for (unsigned b = 0; b < bags.size(); b++) {
        auto bsize = static_cast<unsigned>(bags[b].size());
        BoolVarArgs bag(static_cast<int>(bsize));
        for (unsigned i = 0; i < bsize; i++)
            bag[i] = instance_vars[instance+i];
        instance += bsize;
        rel(*this, (sum(bag) > 0) == bag_vars[b]);
    }
}

template <class T>
std::pair<T, T>
CPRuleLearner<T>::generate_bound_domains(const std::vector<std::vector<std::vector<T>>> &occurrences) {
    auto n = static_cast<unsigned int>(occurrences[0][0].size());
    std::pair<T, T> extremum;
    bool defined_extremum = false;

    lower_bounds_ = var_array_t (*this, n);
    upper_bounds_ = var_array_t (*this, n);

    /*
    std::vector<std::pair<T, T>> bounds = compute_bounds(occurrences);

    for (unsigned i = 0; i < bounds.size(); i++) {
        const auto& bound = bounds[i];
        lower_bounds_[i] = var_t (*this, bound.first, bound.second);
        upper_bounds_[i] = var_t (*this, bound.first, bound.second);
        if (!defined_extremum) {
            extremum.first = bound.first;
            extremum.second = bound.second;
            defined_extremum = true;
        }
        else {
            extremum.first = std::min(extremum.first, bound.first);
            extremum.second = std::max(extremum.second, bound.second);
        }
    }

    return extremum;
     */

    for (unsigned i = 0; i < n; i++) {
        std::set<T> bound_set = compute_bound_set(occurrences, i);
        T bound_min = *std::min_element(bound_set.begin(), bound_set.end());
        T bound_max = *std::max_element(bound_set.begin(), bound_set.end());

        /*
        for (T val: bound_set)
            std::cout << val << " ";
        std::cout << std::endl;

        std::cout << "Min: " << bound_min << std::endl;
        std::cout << "Max: " << bound_max << std::endl;
        */

        lower_bounds_[i] = var_t (*this, bound_min, bound_max);
        upper_bounds_[i] = var_t (*this, bound_min, bound_max);

        std::vector<T> bound_vector(bound_set.begin(), bound_set.end());
        // std::cout << "Set domain" << std::endl;

        dom(*this, lower_bounds_[i], IntSet(IntArgs(bound_vector)));
        dom(*this, upper_bounds_[i], IntSet(IntArgs(bound_vector)));

        if (!defined_extremum) {
            extremum.first = bound_min;
            extremum.second = bound_max;
            defined_extremum = true;
        }
        else {
            extremum.first = std::min(extremum.first, bound_min);
            extremum.second = std::max(extremum.second, bound_max);
        }
    }

    return extremum;
}

template <class T>
void
CPRuleLearner<T>::generate_instance_constraints(
        const std::vector<std::vector<std::vector<T>>> &bags, BoolVarArray &instance_vars
        ) {
    BoolVarArgs none(0);
    unsigned i = 0;
    for (const auto& bag : bags) {
        for (const auto& instance : bag) {
            BoolVarArgs row(static_cast<int>(2*instance.size()));
            for (unsigned j = 0; j < instance.size(); j++) {
                row[2 * j] = expr(*this, lower_bounds_[j] <= instance[j]);
                row[2 * j + 1] = expr(*this, instance[j] <= upper_bounds_[j]);
            }
            clause(*this, BOT_AND, row, none, instance_vars[i]);
            i++;
        }
    }
}

template <class T>
void
CPRuleLearner<T>::init(const CPRuleLearnerOptions& opt) {
    const auto& occurrences = opt.occurrences_;
    const auto& occ1 = occurrences.find(1)->second;
    const auto& occ0 = occurrences.find(0)->second;

    auto extremum = generate_bound_domains(occ1);
    positive_instances_ = BoolVarArray(*this, count_instances(occ1), 0, 1);
    if (!unsupervised_)
        negative_instances_ = BoolVarArray(*this, count_instances(occ0), 0, 1);

    positive_bags_ = BoolVarArray(*this, static_cast<int>(occ1.size()), 0, 1);
    if (!unsupervised_)
        negative_bags_ = BoolVarArray(*this, static_cast<int>(occ0.size()), 0, 1);

    generate_bag_constraints(occ1, positive_bags_, positive_instances_);
    if (!unsupervised_)
        generate_bag_constraints(occ0, negative_bags_, negative_instances_);

    generate_instance_constraints(occ1, positive_instances_);
    if (!unsupervised_)
        generate_instance_constraints(occ0, negative_instances_);

    set_close_constraint(occ1, extremum);
    set_additional_constraints();

    if (!unsupervised_ && relevant_)
        set_previous_relevant_constraint(opt);
}

template <class T>
void
CPRuleLearner<T>::set_additional_constraints() {
    int minsup = 1;
    if (minimal_support_ > 0)
        minsup = minimal_support_;
    rel(*this, sum(positive_bags_) >= minsup);
    if (!unsupervised_ && minimal_growth_rate_ > 0) {
        rel(*this, sum(positive_bags_) >= (sum(negative_bags_) * minimal_growth_rate_));
    }
}

template <class T>
void
CPRuleLearner<T>::set_close_constraint(const std::vector<std::vector<std::vector<T>>> &occurrences,
        const std::pair<T, T>& extremum) {
    std::vector<var_args_t> bag_lower_closed_constraint(occurrences[0][0].size());
    std::vector<var_args_t> bag_upper_closed_constraint(occurrences[0][0].size());

    int ins = 0;
    std::vector<var_args_t> weak_lower_closed_constraint(occurrences[0][0].size());
    std::vector<var_args_t> weak_upper_closed_constraint(occurrences[0][0].size());

    for (unsigned i = 0; i < occurrences.size(); i++) {
        const auto& bag = occurrences[i];

        std::vector<var_args_t> instance_lower_closed_constraint(occurrences[0][0].size());
        std::vector<var_args_t> instance_upper_closed_constraint(occurrences[0][0].size());

        for (const auto& instance : bag) {
            for (unsigned j = 0; j < instance.size(); j++) {
                instance_lower_closed_constraint[j] << expr(*this, ite(positive_instances_[ins], instance[j], extremum.first));
                instance_upper_closed_constraint[j] << expr(*this, ite(positive_instances_[ins], instance[j], extremum.second));

                weak_lower_closed_constraint[j] << expr(*this, ite(positive_instances_[ins], instance[j], extremum.second));
                weak_upper_closed_constraint[j] << expr(*this, ite(positive_instances_[ins], instance[j], extremum.first));
            }
            ins++;
        }

        for (unsigned j = 0; j < occurrences[0][0].size(); j++) {
            bag_lower_closed_constraint[j] << expr(*this, ite(positive_bags_[i], max(instance_lower_closed_constraint[j]), extremum.second));
            bag_upper_closed_constraint[j] << expr(*this, ite(positive_bags_[i], min(instance_upper_closed_constraint[j]), extremum.first));
        }
    }

    for (unsigned i = 0; i < bag_lower_closed_constraint.size(); i++) {
        rel(*this, lower_bounds_[i] == min(bag_lower_closed_constraint[i]));
        rel(*this, upper_bounds_[i] == max(bag_upper_closed_constraint[i]));

        rel(*this, lower_bounds_[i] == min(weak_lower_closed_constraint[i]));
        rel(*this, upper_bounds_[i] == max(weak_upper_closed_constraint[i]));
    }
}

template <class T>
void CPRuleLearner<T>::set_previous_relevant_constraint(const CPRuleLearnerOptions &options) {
    for (const RelevantConstraint& constraint : options.relevant_constraints_) {
        std::vector<unsigned> ptid_list;
        std::vector<unsigned> ntid_list;

        if (constraint.generate_constraint(options.tid_list_, options.negative_tid_list_, ptid_list, ntid_list)) {
            BoolVarArgs not_covered_positives;
            BoolVarArgs covered_negatives;

            for (unsigned i : ptid_list)
                not_covered_positives << positive_bags_[i];
            for (unsigned i : ntid_list)
                covered_negatives << negative_bags_[i];

            rel(*this, (sum(not_covered_positives) > 0) || (sum(covered_negatives) < covered_negatives.size()));
        }
    }
}

template <class T>
void
CPRuleLearner<T>::print(std::ostream & os) const {
    std::vector<std::pair<T, T>> constraints;
    constraints.reserve(static_cast<unsigned long>(lower_bounds_.size()));
    for (int rule = 0; rule < lower_bounds_.size(); rule++)
        constraints.emplace_back(lower_bounds_[rule].val(), upper_bounds_[rule].val());

    Episode e(m_, constraints, numeric_intervals_);

    std::vector<unsigned> not_covered_positives;
    std::map<unsigned, std::vector<unsigned>> tid_list({{0, std::vector<unsigned>()}, {1, std::vector<unsigned>()}});
    std::map<unsigned, unsigned> frequencies({{0,0}, {1,0}});
    for (int i = 0; i < positive_bags_.size(); i++) {
        if (positive_bags_[i].val()) {
            frequencies[1] += 1;
            tid_list[1].push_back(static_cast<unsigned int &&>(i));
        }
        else
            not_covered_positives.push_back(static_cast<unsigned int &&>(i));
    }
    if (!unsupervised_)
        for (int i = 0; i < negative_bags_.size(); i++) {
            if (negative_bags_[i].val()) {
                frequencies[0] += 1;
                tid_list[0].push_back(static_cast<unsigned int &&>(i));
            }
        }

    e.set_frequencies(frequencies);
    e.set_tid_list(TidMap(tid_list));
    relevant_constraints_->emplace_back(not_covered_positives, tid_list[0]);

    if (extracted_episodes_) {
        if (relevant_ && check_relevant_bfs_) {
            bool existing_subgroup = false;
            for (auto &it : *closed_candidates_) {
                if (tid_list[1].size() >= it.first.size()) {
                    if (std::includes(tid_list[1].begin(), tid_list[1].end(), it.first.begin(), it.first.end())) {
                        if (tid_list[1].size() == it.first.size())
                            existing_subgroup = true;
                        for (unsigned i: it.second) {
                            if ((*extracted_episodes_)[i].is_valid()) {
                                std::vector<unsigned> old_nsupport = (*extracted_episodes_)[i].get_tid_list()[0];
                                if (old_nsupport.size() >= tid_list[0].size()) {
                                    bool inclusion = std::includes(old_nsupport.begin(),
                                                                   old_nsupport.end(),
                                                                   tid_list[0].begin(),
                                                                   tid_list[0].end());
                                    if (inclusion)
                                        (*extracted_episodes_)[i].set_not_valid();
                                }
                            }
                        }
                    }
                }
            }

            if (!existing_subgroup) {
                std::vector<unsigned> key = tid_list[1];
                (*closed_candidates_)[tid_list[1]] = {static_cast<unsigned>(extracted_episodes_->size())};
            }
            else {
                (*closed_candidates_)[tid_list[1]].push_back(static_cast<unsigned>(extracted_episodes_->size()));
            }
        }

        extracted_episodes_->push_back(e);
    }
    else {
        os << std::string(e);
        os << "Support: " << frequencies[1] << "/" << frequencies[0] << std::endl << std::endl;
    }
}

template <>
void
CPRuleLearner<float>::run() {
    branch(*this, lower_bounds_, FLOAT_VAR_NONE(), FLOAT_VAL_SPLIT_MIN());
    branch(*this, upper_bounds_, FLOAT_VAR_NONE(), FLOAT_VAL_SPLIT_MAX());
}

template <>
void
CPRuleLearner<int>::run() {
    /*int combinations = 0;
    for (auto& lower_bound: lower_bounds_)
        combinations *= lower_bound.width();
    if (positive_bags_.size() > combinations) {*/
        branch(*this, lower_bounds_, INT_VAR_NONE(), INT_VAL_MIN());
        branch(*this, upper_bounds_, INT_VAR_NONE(), INT_VAL_MAX());
    /*}
    else {
        branch(*this, positive_bags_, BOOL_VAR_NONE(), BOOL_VAL_MAX());
    }*/
}