#ifndef GENERALIZED_DISCRIMINANT_EPISODES_OPTIONS_HH
#define GENERALIZED_DISCRIMINANT_EPISODES_OPTIONS_HH

#include <iostream>
#include <memory>
#include <string>
#include <boost/program_options.hpp>

#include "cp_engine_options.hh"
#include "vertical_database.hh"

using namespace boost;
namespace po = boost::program_options;

namespace General {
    class Options {

    public:
        bool parse(int argc, char **argv);

    private:
        const std::string basename_(const std::string &p);

        bool notify(int argc, char **argv,
                    const po::options_description &hidden,
                    const po::options_description &desc,
                    po::positional_options_description &p,
                    po::variables_map &vm
        );

        bool parse_files();

        bool parse_vm(const po::variables_map &vm);

        void setMinsup(unsigned long nb_seq);

        const std::vector<std::string> get_unlimited_positional_args(const po::positional_options_description &p);

        const std::string make_help_description(const std::string &program_name);

        const std::string make_usage_string(
                const std::string &program_name,
                const po::options_description &hidden,
                const po::options_description &desc,
                const po::positional_options_description &p);

    public:
        CPEngineOptions cp_options;

        Options() : cp_options("GDE"), fmin(0), gmin(0) { };

        explicit Options(const std::string &name) : cp_options(name.c_str()), fmin(0), gmin(0) { };


    private:
        float fmin;
        float gmin;
        std::string positives;
        std::string negatives;
    };
}

#endif //GENERALIZED_DISCRIMINANT_EPISODES_OPTIONS_HH
