#include "cp_engine.hh"
#include "options.hh"

int main(int argc, char* argv[]) {
    General::Options options;
    if (!options.parse(argc, argv))
        return 1;

    Script::run<CPEngine,DFS,CPEngineOptions>(options.cp_options);

	return 0;
}
