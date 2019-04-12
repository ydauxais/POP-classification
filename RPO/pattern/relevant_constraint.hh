#ifndef GDE_RELEVANTCONSTRAINT_HH
#define GDE_RELEVANTCONSTRAINT_HH

#include <map>
#include <vector>
#include <utility>

#include <gecode/int.hh>
#include <gecode/minimodel.hh>
#include <gecode/search.hh>

#include "tid_map.hh"

using namespace Gecode;

class RelevantConstraint {
private:
    TidMap tid_map_;

public:
    RelevantConstraint(TidMap tid_map): tid_map_(std::move(tid_map)) {};
    RelevantConstraint(const TidMap &tid_map,
                       const std::vector<unsigned> &tid_list,
                       const std::vector<unsigned> &negative_tid_list);

    bool generate_constraint(const std::vector<unsigned> &tid_list, const std::vector<unsigned> &negative_tid_list,
                                 std::vector<unsigned> &ptid_list, std::vector<unsigned> &ntid_list) const;

    const TidMap& get_tid_list() const {
        return tid_map_;
    }
    void print_tid_list(std::ostream& os) const;
    void join_tid_list(const std::vector<unsigned>& tids, std::ostream& os) const;
};


#endif //GDE_RELEVANTCONSTRAINT_HH
