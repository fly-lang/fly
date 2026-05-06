#pragma once

#include <string>
#include <vector>

namespace flyp::commands {

// flyp init [--name NAME] [--version VER]
int cmd_init(const std::string& name, const std::string& version);

// flyp build [--release] [--target NAME]
int cmd_build(bool release, const std::string& target);

// flyp run [--release] [--bin NAME] [-- ARGS...]
int cmd_run(bool release, const std::string& bin_name,
            const std::vector<std::string>& run_args);

// flyp test [--release] [SUITE]
int cmd_test(bool release, const std::string& suite);

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
