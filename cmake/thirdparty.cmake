include(FetchContent)
set(FETCHCONTENT_TRY_FIND_PACKAGE_MODE ALWAYS)
include(cmake/CPM.cmake)

# cpmaddpackage(
# arbitrary name based on the authorname.githubname
# looking at 'add_library(iir::iir_static ALIAS iir_static)' it gives us the name of target, and we are using static because yes
# we can copy paste the git link "https://github.com/berndporr/iir1.git" however this git has a release section, copy paste the zip link
# NAME berndporr.iir1
# URL "https://github.com/berndporr/iir1/archive/refs/tags/1.9.5.zip"
# )
cpmaddpackage(
  NAME
  unevens.oversimple
  GIT_REPOSITORY
  "https://github.com/unevens/oversimple.git"
  GIT_TAG
  origin/master
)

if(NOT TARGET Microsoft.GSL::GSL)
  cpmaddpackage(
    NAME
    microsoft.GSL
    URL
    "https://github.com/microsoft/GSL/archive/refs/tags/v4.0.0.tar.gz"
    FIND_PACKAGE_ARGS
    4.0.0...<5.0.0
    NAMES
    Microsoft.GSL
  )
endif()

if(NOT TARGET fmt::fmt)
  cpmaddpackage(
    NAME
    fmtlib.fmt
    URL
    "https://github.com/fmtlib/fmt/archive/refs/tags/9.1.0.tar.gz"
    FIND_PACKAGE_ARGS
    9.1.0...<10.0.0
    NAMES
    fmt
  )
endif()

if(NOT TARGET magic_enum::magic_enum)
  cpmaddpackage(
    NAME
    Neargye.magic_enum
    URL
    "https://github.com/Neargye/magic_enum/archive/refs/tags/v0.8.2.tar.gz"
    FIND_PACKAGE_ARGS
    0.8.2...<1.0.0
    NAMES
    magic_enum
  )
endif()

if(NOT TARGET spdlog::spdlog)
  cpmaddpackage(
    NAME
    gabime.spdlog
    OPTIONS
    "SPDLOG_FMT_EXTERNAL_HO 1"
    URL
    "https://github.com/gabime/spdlog/archive/refs/tags/v1.11.0.tar.gz"
    FIND_PACKAGE_ARGS
    1.11.0...<2.0.0
    NAMES
    spdlog
  )
endif()
