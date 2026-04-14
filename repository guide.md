# Explanations of Files

## clangd

<https://clangd.llvm.org/config>  
configues the clangd language server, lets you customize intellisense in non-visualstudio editor such as VScode

## clang-format

<https://clang.llvm.org/docs/ClangFormatStyleOptions.html>  
code formatting and enforcement

## clang-tidy

<https://clang.llvm.org/extra/clang-tidy/checks/list.html>  
gives additional C and C++ warnings

## cmake-format.yaml

<https://cmake-format.readthedocs.io/en/latest/configuration.html>  
configures formatting rules for CMake files.

## editorconfig

<https://editorconfig.org/>  
mostly used to trim trailing whitespace and extra newline at end of files

## gitattributes

<https://git-scm.com/docs/gitattributes>  
configures how git handles files.
eol defines end of line character
diff defines what diff tool is used when diff-ing on files

## markdownlint.yaml

<https://github.com/DavidAnson/markdownlint/blob/main/schema/.markdownlint.yaml>  
configures warnings when using markdown files.
markdownlint is too pendantic and annoying so markdownlint.yaml makes it less so.

## gitignore

<https://git-scm.com/docs/gitignore>  
prevents files from being tracked in the repository.

## CMakeLists.txt

<https://cmake.org/cmake/help/latest/manual/cmake-language.7.html>  
Defines the build process for the project, specifying how to compile and link the code.

## CMakePresets.json

<https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html>  
Defines a set of presets for CMake, simplifying the configuration and build process by providing predefined settings.

## utils.py

have dev utilities available as a unified script, runs cmake-format and clang-format

```ps1
# syntax highlighting:
# sh (shell), bash (bashscript), ps1 (powershell), cmd (command prompt)
python.exe ./utils.py run-cmake-format
python.exe ./utils.py run-clang-format

# ways to get help text
python.exe ./utils.py --help
python.exe ./utils.py -h
python.exe ./utils.py run-cmake-format -h
python.exe ./utils.py run-clang-format -h
```
