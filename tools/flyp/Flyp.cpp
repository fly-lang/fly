#include "cli/Commands.h"

#include <CLI/CLI.hpp>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

// Respect FLYP_OFFLINE=1 environment variable as a global offline flag.
static bool env_offline() {
    const char* v = std::getenv("FLYP_OFFLINE");
    return v && (std::strcmp(v, "1") == 0 || std::strcmp(v, "true") == 0);
}

int main(int argc, char** argv) {
    CLI::App app{"flyp — Fly package manager"};
    app.require_subcommand(0, 1);
    app.set_version_flag("-V,--version", "flyp " FLYP_VERSION);

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
        auto* sub      = app.add_subcommand("build", "Compile all targets (or a subset with --targets)");
        auto  profile  = std::make_shared<std::string>("debug");
        auto  release  = std::make_shared<bool>(false);
        auto  targets  = std::make_shared<std::string>();
        auto  jobs     = std::make_shared<int>(0);
        auto  offline  = std::make_shared<bool>(false);
        auto  cross    = std::make_shared<std::string>();
        sub->add_option("--profile", *profile,
            "Build profile to use. Must be declared in [profiles] in fly.toml.\n"
            "Built-in profiles: debug (default), release.\n"
            "Example: --profile ci");
        sub->add_flag("--release", *release,
            "Shorthand for --profile release.");
        sub->add_option("--targets", *targets,
            "Comma-separated list of [targets] keys to build.\n"
            "Omit to build all declared targets.\n"
            "Example: --targets my-app,my-lib");
        sub->add_option("--jobs,-j", *jobs,
            "Number of threads for parallel compilation.\n"
            "Forwarded as --jobs to the fly compiler for LLVM internal parallelism.\n"
            "With a single target: all threads go to the compiler.\n"
            "With multiple targets: flyp runs them concurrently, each compiler gets 1 thread.\n"
            "0 (default) = auto-detect CPU cores. 1 = sequential.");
        sub->add_flag("--offline", *offline,
            "Do not access the network. Use cached dependencies only.\n"
            "Fails with an error if any dependency is missing from the local cache.\n"
            "Can also be set via the FLYP_OFFLINE=1 environment variable.");
        sub->add_option("--cross", *cross,
            "Cross-compile for the given target triple.\n"
            "The triple is passed as --target to the fly compiler.\n"
            "Output goes to target/<profile>/<triple>/.\n"
            "Examples: aarch64-linux-musl, x86_64-w64-mingw32, wasm32-wasi");
        sub->footer(
            "Output directory: target/<profile>/  (native) or target/<profile>/<triple>/  (cross)\n"
            "Custom profiles are declared in [profiles] in fly.toml.\n"
            "fly.lock is regenerated automatically if it is stale.");
        sub->callback([profile, release, targets, jobs, offline, cross]{
            if (*release) *profile = "release";
            // Split comma-separated target list.
            std::vector<std::string> keys;
            if (!targets->empty()) {
                std::istringstream ss(*targets);
                std::string tok;
                while (std::getline(ss, tok, ','))
                    if (!tok.empty()) keys.push_back(tok);
            }
            std::exit(flyp::commands::cmd_build(*profile, keys, *jobs,
                                                *offline || env_offline(), *cross));
        });
    }

    // ── flyp run ─────────────────────────────────────────────────────────────
    {
        auto* sub      = app.add_subcommand("run", "Build and immediately run a binary target");
        auto  profile  = std::make_shared<std::string>("debug");
        auto  release  = std::make_shared<bool>(false);
        auto  offline  = std::make_shared<bool>(false);
        auto  bin_name = std::make_shared<std::string>();
        auto  run_args = std::make_shared<std::vector<std::string>>();
        sub->add_option("--profile", *profile,
            "Build profile to use (default: debug).\n"
            "Example: --profile release");
        sub->add_flag("--release", *release,
            "Shorthand for --profile release.");
        sub->add_flag("--offline", *offline,
            "Do not access the network. Use cached dependencies only.");
        sub->add_option("--bin", *bin_name,
            "Key of the bin target to run (from [targets]).\n"
            "Defaults to the first bin target. Libs cannot be run.");
        sub->add_option("args", *run_args,
            "Arguments forwarded to the binary after it is launched.")
           ->allow_extra_args();
        sub->footer(
            "Example: flyp run --profile release --bin server -- --port 8080");
        sub->callback([profile, release, offline, bin_name, run_args]{
            if (*release) *profile = "release";
            std::exit(flyp::commands::cmd_run(*profile, *bin_name, *run_args,
                                              *offline || env_offline()));
        });
    }

    // ── flyp test ────────────────────────────────────────────────────────────
    {
        auto* sub     = app.add_subcommand("test", "Build and run test suites");
        auto  profile = std::make_shared<std::string>("debug");
        auto  release = std::make_shared<bool>(false);
        auto  offline = std::make_shared<bool>(false);
        auto  suite   = std::make_shared<std::string>();
        sub->add_option("--profile", *profile,
            "Build profile to use (default: debug).\n"
            "Example: --profile ci");
        sub->add_flag("--release", *release,
            "Shorthand for --profile release.");
        sub->add_flag("--offline", *offline,
            "Do not access the network. Use cached dependencies only.");
        sub->add_option("--suite,-s", *suite,
            "Run only the matching suite (or a specific method or case).\n"
            "Format: Suite[::method[::\"label\"]]\n"
            "Example: --suite MathSuite::classifyTest::\"positive\"");
        sub->footer(
            "Suite files are discovered via [test] suites = [...] in fly.toml.\n"
            "Each suite is compiled with --test and run as a standalone binary.\n"
            "The filter is passed via the FLY_TEST_FILTER environment variable.");
        sub->callback([profile, release, offline, suite]{
            if (*release) *profile = "release";
            std::exit(flyp::commands::cmd_test(*profile, *suite,
                                               *offline || env_offline()));
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

    // ── flyp clean ───────────────────────────────────────────────────────────
    {
        auto* sub     = app.add_subcommand("clean", "Remove build artifacts");
        auto  profile = std::make_shared<std::string>();
        auto  release = std::make_shared<bool>(false);
        sub->add_option("--profile", *profile,
            "Remove only the artifacts for this profile (e.g. debug, release, ci).\n"
            "Omit to remove the entire target/ directory.");
        sub->add_flag("--release", *release,
            "Shorthand for --profile release.");
        sub->footer(
            "Examples:\n"
            "  flyp clean              # removes target/\n"
            "  flyp clean --release    # removes target/release/\n"
            "  flyp clean --profile ci # removes target/ci/");
        sub->callback([profile, release]{
            if (*release) *profile = "release";
            std::exit(flyp::commands::cmd_clean(*profile));
        });
    }

    // ── flyp workspace ────────────────────────────────────────────────────────
    {
        auto* ws = app.add_subcommand("workspace", "Manage Fly workspaces");
        ws->require_subcommand(1);

        auto* init    = ws->add_subcommand("init", "Create a new workspace");
        auto  ws_name = std::make_shared<std::string>();
        auto  ws_mems = std::make_shared<std::string>();
        init->add_option("--name",    *ws_name, "Workspace directory name");
        init->add_option("--members", *ws_mems,
            "Comma-separated list of member package names to scaffold.\n"
            "Example: --members app,core,utils");
        init->callback([ws_name, ws_mems]{
            std::vector<std::string> members;
            if (!ws_mems->empty()) {
                std::istringstream ss(*ws_mems);
                std::string tok;
                while (std::getline(ss, tok, ','))
                    if (!tok.empty()) members.push_back(tok);
            }
            std::exit(flyp::commands::cmd_workspace_init(*ws_name, members));
        });
    }

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
