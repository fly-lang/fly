#pragma once

#include <string>
#include <vector>

namespace flyp::commands {

// flyp init [--name NAME] [--version VER]
int cmd_init(const std::string& name, const std::string& version);

// flyp workspace init --name NAME [--members A,B,...]
int cmd_workspace_init(const std::string& name,
                       const std::vector<std::string>& members);

// flyp build [--profile NAME | --release] [--targets KEY,KEY,...] [--jobs N] [--offline] [--cross TRIPLE]
int cmd_build(const std::string& profile, const std::vector<std::string>& targets,
              int jobs, bool offline, const std::string& triple);

// flyp run [--profile NAME | --release] [--bin KEY] [--offline] [-- ARGS...]
int cmd_run(const std::string& profile, const std::string& bin_key,
            const std::vector<std::string>& run_args, bool offline);

// flyp test [--profile NAME | --release] [--suite SUITE] [--offline]
int cmd_test(const std::string& profile, const std::string& suite, bool offline);

// flyp add <name> --git URL (--tag TAG | --branch BRANCH | --rev REV) [--dev]
int cmd_add(const std::string& name,
            const std::string& git_url,
            const std::string& tag,
            const std::string& branch,
            const std::string& rev,
            bool dev);

// flyp remove <name>
int cmd_remove(const std::string& name);

// flyp update [<name>]
int cmd_update(const std::string& name);

// flyp lock
int cmd_lock();

// flyp why <name>
int cmd_why(const std::string& name);

// flyp clean [--profile NAME | --release]
// profile empty → remove entire target/; otherwise remove target/<profile>/
int cmd_clean(const std::string& profile);

// flyp cache clean | stats
int cmd_cache_clean();
int cmd_cache_stats();

// flyp search <query>  (stub — requires registry)
int cmd_search(const std::string& query);

// flyp publish  (stub — requires registry)
int cmd_publish();

// flyp version
int cmd_version();

} // namespace flyp::commands
