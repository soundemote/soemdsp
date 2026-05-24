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
if(NOT TARGET Microsoft.GSL::GSL)
    cpmaddpackage(
        NAME
        microsoft.GSL
        URL
        "https://github.com/microsoft/GSL/archive/refs/tags/v4.2.1.tar.gz"
        FIND_PACKAGE_ARGS
        4.2.1...<5.0.0
        NAMES
        Microsoft.GSL
    )
endif()

if(NOT TARGET fmt::fmt)
    cpmaddpackage(
        NAME
        fmtlib.fmt
        URL
        "https://github.com/fmtlib/fmt/archive/refs/tags/12.1.0.tar.gz"
        FIND_PACKAGE_ARGS
        12.1.0...<13.0.0
        NAMES
        fmt
    )
endif()

if(NOT TARGET magic_enum::magic_enum)
    cpmaddpackage(
        NAME
        Neargye.magic_enum
        URL
        "https://github.com/Neargye/magic_enum/archive/refs/tags/v0.9.8.tar.gz"
        FIND_PACKAGE_ARGS
        0.9.8...<1.0.0
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
        "https://github.com/gabime/spdlog/archive/refs/tags/v1.17.0.tar.gz"
        FIND_PACKAGE_ARGS
        1.17.0...<2.0.0
        NAMES
        spdlog
    )
endif()
