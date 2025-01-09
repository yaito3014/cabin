#include "Cabin.hpp"

#include "Cli.hpp"
#include "Cmd.hpp"
#include "Rustify/Result.hpp"

#include <cstdlib>
#include <exception>
#include <span>
#include <string_view>
#include <vector>

namespace cabin {

const Cli&
getCli() noexcept {
  static const Cli cli =  //
      Cli{ "cabin" }
          .setDesc("A package manager and build system for C++")
          .addOpt(Opt{ "--verbose" }
                      .setShort("-v")
                      .setDesc("Use verbose output (-vv very verbose output)")
                      .setGlobal(true))
          .addOpt(Opt{ "-vv" }
                      .setDesc("Use very verbose output")
                      .setGlobal(true)
                      .setHidden(true))
          .addOpt(Opt{ "--quiet" }
                      .setShort("-q")
                      .setDesc("Do not print cabin log messages")
                      .setGlobal(true))
          .addOpt(Opt{ "--color" }
                      .setDesc("Coloring: auto, always, never")
                      .setPlaceholder("<WHEN>")
                      .setGlobal(true))
          .addOpt(Opt{ "--help" }  //
                      .setShort("-h")
                      .setDesc("Print help")
                      .setGlobal(true))
          .addOpt(Opt{ "--version" }
                      .setShort("-V")
                      .setDesc("Print version info and exit")
                      .setGlobal(false))
          .addOpt(Opt{ "--list" }  //
                      .setDesc("List all subcommands")
                      .setGlobal(false)
                      .setHidden(true))
          .addSubcmd(ADD_CMD)
          .addSubcmd(BUILD_CMD)
          .addSubcmd(CLEAN_CMD)
          .addSubcmd(FMT_CMD)
          .addSubcmd(HELP_CMD)
          .addSubcmd(INIT_CMD)
          .addSubcmd(LINT_CMD)
          .addSubcmd(NEW_CMD)
          .addSubcmd(RUN_CMD)
          .addSubcmd(SEARCH_CMD)
          .addSubcmd(TEST_CMD)
          .addSubcmd(TIDY_CMD)
          .addSubcmd(VERSION_CMD);
  return cli;
}

Result<void>
cliMain(const std::span<char* const> args) noexcept {
  // Parse arguments (options should appear before the subcommand, as the help
  // message shows intuitively)
  // cabin --verbose run --release help --color always --verbose
  // ^^^^^^^^^^^^^^ ^^^^^^^^^^^^^ ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  // [global]       [run]         [help (under run)]
  for (auto itr = args.begin(); itr != args.end(); ++itr) {
    // Global options
    const auto control = Try(Cli::handleGlobalOpts(itr, args.end()));
    if (control == Cli::Return) {
      return Ok();
    } else if (control == Cli::Continue) {
      continue;
    }
    // else: Fallthrough: current argument wasn't handled

    // Local options
    else if (*itr == "-V"sv || *itr == "--version"sv) {
      const std::vector<std::string_view> remArgs(itr + 1, args.end());
      return versionMain(remArgs);
    } else if (*itr == "--list"sv) {
      getCli().printAllSubcmds(true);
      return Ok();
    }

    // Subcommands
    else if (getCli().hasSubcmd(*itr)) {
      const std::vector<std::string_view> remArgs(itr + 1, args.end());
      try {
        return getCli().exec(*itr, remArgs);
      } catch (const std::exception& e) {
        Bail(e.what());
      }
    }

    // Unexpected argument
    else {
      return getCli().noSuchArg(*itr);
    }
  }

  return getCli().printHelp({});
}

}  // namespace cabin
