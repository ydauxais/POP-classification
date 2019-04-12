#include "relevant_constraint.hh"

RelevantConstraint::RelevantConstraint(const TidMap &tid_map,
                                       const std::vector<unsigned> &tid_list,
                                       const std::vector<unsigned> &negative_tid_list) {
    std::vector<unsigned> tid1 = tid_map[1];
    std::vector<unsigned> tid0 = tid_map[0];

    tid_map_[0] = std::vector<unsigned>();
    tid_map_[1] = std::vector<unsigned>();

    for (unsigned tid : tid1)
        tid_map_[1].push_back(tid_list[tid]);
    for (unsigned tid : tid0)
        tid_map_[0].push_back(negative_tid_list[tid]);
}

bool
RelevantConstraint::generate_constraint(const std::vector<unsigned> &tid_list, const std::vector<unsigned> &negative_tid_list,
                                        std::vector<unsigned> &ptid_list, std::vector<unsigned> &ntid_list) const {
    const auto& tid1 = tid_map_[1];
    const auto& tid0 = tid_map_[0];

    if (tid0.size() > negative_tid_list.size())
        return false;

    unsigned cnid = 0;
    for (unsigned int tid : tid0) {
        while (negative_tid_list[cnid] < tid) {
            cnid++;
            if (cnid >= negative_tid_list.size())
                return false;

        }
        if (negative_tid_list[cnid] > tid)
            return false;
        ntid_list.push_back(cnid);
    }

    unsigned cpid = 0;
    for (unsigned pid = 0; pid < tid_list.size(); pid++) {
        unsigned tid = tid_list[pid];
        while (tid > tid1[cpid] && cpid < tid1.size())
            cpid++;
        if (tid != tid1[cpid])
            ptid_list.push_back(pid);
    }

    return true;
}

void
RelevantConstraint::join_tid_list(const std::vector<unsigned>& tids, std::ostream &os) const {
    bool first = true;
    for (const auto& tid : tids) {
        if (!first) os << ", ";
        os << tid;
        first = false;
    }
}

void
RelevantConstraint::print_tid_list(std::ostream &os) const {
    const auto& tid1 = tid_map_[1];
    const auto& tid0 = tid_map_[0];

    os << "T+: [";
    join_tid_list(tid1, os);
    os << "]" << std::endl;

    os << "T-: [";
    join_tid_list(tid0, os);
    os << "]" << std::endl;
}

