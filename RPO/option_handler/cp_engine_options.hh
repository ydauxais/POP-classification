#ifndef GDE_CP_ENGINE_OPTIONS_HH
#define GDE_CP_ENGINE_OPTIONS_HH

#include <gecode/driver.hh>
#include <memory>
#include "database.hh"
#include "episode.hh"

class CPEngineOptions: public Gecode::Options {

public:
    std::shared_ptr<Database> base_;
    unsigned minimal_support_;
    float minimal_growth_rate_;
    unsigned minimal_length_;
    unsigned maximal_length_;
    bool closed_;
    bool branch_on_min_;
    bool numeric_intervals_;
    bool output_tid_lists_;
    bool no_output_;
    bool relevant_;
    bool unsupervised_;
    std::shared_ptr<std::vector<Episode>> episodes_;

public:
    explicit CPEngineOptions(const char* s): Gecode::Options(s),
        base_(nullptr), minimal_support_(0), minimal_growth_rate_(0),
        minimal_length_(0), maximal_length_(0), closed_(false), branch_on_min_(true), numeric_intervals_(false),
        output_tid_lists_(false), no_output_(false), relevant_(false), unsupervised_(false), episodes_(nullptr) {}

    std::vector<Episode> get_episodes() {
        if (episodes_ == nullptr)
            return std::vector<Episode>();
        return *episodes_;
    }

    void set_export_episodes() {
        episodes_ = std::make_shared<std::vector<Episode>>();
    }

    // Only for SWIG
    // TODO: use proper ifdef
    void set_solutions(unsigned n) {
        this->solutions(n);
    }

    void no_output() {
        #ifdef _WIN32
                this->out_file("nul");
                this->log_file("nul");
        #else
                this->out_file("/dev/null");
                this->log_file("/dev/null");
        #endif
        no_output_ = true;
    }
};


#endif //GDE_CPENGINEOPTIONS_HH
