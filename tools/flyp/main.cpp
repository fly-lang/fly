#include "cli/Commands.h"

#include <CLI/CLI.hpp>
#include <cstdlib>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    CLI::App app{"flyp — Fly package manager"};
    app.require_subcommand(0, 1);
    app.set_version_flag("-V,--version", "flyp 0.1.0");

    // ── flyp version ─────────────────────────────────────────────────────────
    app.add_subcommand("version", "Print flyp version")
       ->callback([]{ std::exit(flyp::commands::cmd_version()); });

    // ── flyp init ────────────────────────────────────────────────────────────
    {
        auto* sub    = app.add_subcommand("init", "Create a new Fly project");
        auto  name   = std::make_shared<std::string>();
        auto  ver    = std::make_shared<std::string>();
        sub->add_option("--name",    *name, "Package name");
        sub->add_option("--version", *ver,  "Initial version (default 0.1.0)");
        sub->callback([name, ver]{
            std::exit(flyp::commands::cmd_init(*name, *ver));
        });
    }

    // ── flyp build ───────────────────────────────────────────────────────────
    {
        auto* sub     = app.add_subcommand("build", "Build the project");
        auto  release = std::make_shared<bool>(false);
        auto  target  = std::make_shared<std::string>();
        sub->add_flag("--release", *release, "Build with release profile");
        sub->add_option("--target", *target,  "Build only this target");
        sub->callback([release, target]{
            std::exit(flyp::commands::cmd_build(*release, *target));
        });
    }

    // ── flyp run ─────────────────────────────────────────────────────────────
    {
        auto* sub      = app.add_subcommand("run", "Build and run a binary");
        auto  release  = std::make_shared<bool>(false);
        auto  bin_name = std::make_shared<std::string>();
        auto  run_args = std::make_shared<std::vector<std::string>>();
        sub->add_flag("--release",  *release,  "Build with release profile");
        sub->add_option("--bin",    *bin_name, "Binary target to run");
        sub->add_option("args",     *run_args, "Arguments forwarded to the binary")
           ->allow_extra_args();
        sub->callback([release, bin_name, run_args]{
            std::exit(flyp::commands::cmd_run(*release, *bin_name, *run_args));
        });
    }

    // ── flyp test ────────────────────────────────────────────────────────────
    {
        auto* sub     = app.add_subcommand("test", "Build and run tests");
        auto  release = std::make_shared<bool>(false);
        auto  suite   = std::make_shared<std::string>();
        sub->add_flag("--release", *release, "Build with release profile");
        sub->add_option("suite",   *suite,   "Run only this test suite");
        sub->callback([release, suite]{
            std::exit(flyp::commands::cmd_test(*release, *suite));
        });
    }

    // ── flyp add ─────────────────────────────────────────────────────────────
    {
        auto* sub     = app.add_subcommand("add", "Add a dependency");
        auto  name    = std::make_shared<std::string>();
        auto  git_url = std::make_shared<std::string>();
        auto  tag     = std::make_shared<std::string>();
        auto  branch  = std::make_shared<std::string>();
        auto  rev     = std::make_shared<std::string>();
        auto  dev     = std::make_shared<bool>(false);
        sub->add_option("name",     *name,    "Package name")->required();
        sub->add_option("--git",    *git_url, "Git URL")->required();
        sub->add_option("--tag",    *tag,     "Tag to pin");
        sub->add_option("--branch", *branch,  "Branch to track");
        sub->add_option("--rev",    *rev,     "Commit hash to pin");
        sub->add_flag("--dev",      *dev,     "Add as dev-dependency");
        sub->callback([name, git_url, tag, branch, rev, dev]{
            std::exit(flyp::commands::cmd_add(*name, *git_url, *tag, *branch, *rev, *dev));
        });
    }

    // ── flyp remove ──────────────────────────────────────────────────────────
    {
        auto* sub  = app.add_subcommand("remove", "Remove a dependency");
        auto  name = std::make_shared<std::string>();
        sub->add_option("name", *name, "Package name")->required();
        sub->callback([name]{ std::exit(flyp::commands::cmd_remove(*name)); });
    }

    // ── flyp update ──────────────────────────────────────────────────────────
    {
        auto* sub  = app.add_subcommand("update", "Update dependencies");
        auto  name = std::make_shared<std::string>();
        sub->add_option("name", *name, "Update only this package (omit for all)");
        sub->callback([name]{ std::exit(flyp::commands::cmd_update(*name)); });
    }

    // ── flyp lock ────────────────────────────────────────────────────────────
    app.add_subcommand("lock", "Resolve dependencies and write fly.lock")
       ->callback([]{ std::exit(flyp::commands::cmd_lock()); });

    // ── flyp why ─────────────────────────────────────────────────────────────
    {
        auto* sub  = app.add_subcommand("why", "Explain why a package is included");
        auto  name = std::make_shared<std::string>();
        sub->add_option("name", *name, "Package name")->required();
        sub->callback([name]{ std::exit(flyp::commands::cmd_why(*name)); });
    }

    // ── flyp cache ───────────────────────────────────────────────────────────
    {
        auto* cs = app.add_subcommand("cache", "Manage the local package cache");
        cs->require_subcommand(1);
        cs->add_subcommand("clean", "Remove all cached packages")
          ->callback([]{ std::exit(flyp::commands::cmd_cache_clean()); });
        cs->add_subcommand("stats", "Show cache size and entry count")
          ->callback([]{ std::exit(flyp::commands::cmd_cache_stats()); });
    }

    // ── flyp search ──────────────────────────────────────────────────────────
    {
        auto* sub   = app.add_subcommand("search", "Search the package registry");
        auto  query = std::make_shared<std::string>();
        sub->add_option("query", *query, "Search query")->required();
        sub->callback([query]{ std::exit(flyp::commands::cmd_search(*query)); });
    }

    // ── flyp publish ─────────────────────────────────────────────────────────
    app.add_subcommand("publish", "Publish package to registry")
       ->callback([]{ std::exit(flyp::commands::cmd_publish()); });

    // Parse — all subcommand callbacks call std::exit(), so if we fall
    // through here no subcommand was given; print help.
    CLI11_PARSE(app, argc, argv);
    std::cout << app.help();
    return 0;
}
