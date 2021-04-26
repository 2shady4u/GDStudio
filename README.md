# GDStudio (WIP)
Code editor made in Godot.

Latest Version: 0.2.1

## Compiling
I have only tested this on Windows, but in theory it should work on other systems
- Install Python and then Install scons
- Clone this repo
```shell
git clone --recursive https://github.com/sdtv9507/GDStudio
```
- Open the cmd/shell at "addons/GDStudio/Scripts"
- Type "scons platform=windows" or "scons platform=x11" without the quotes.

Projects tab has the features I want to implement for the next version and
the bugs I have found while testing.

Please, open an issue if you encounter any bugs.

## Changelog
# Version 0.2.1
- Add autocomplete for Keywords and identifiers for both Rust and C++
- Add a button to Reset Global Settings to Default
- Add an option to use custom resolutions
- Add an "About" popup panel
- Fix bugs for autocomplete, tree sitter parsing, and others

## Screenshots
![Alt text](/screenshots/build.png?raw=true "Screenshot1")
