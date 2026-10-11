# UniBackpack

UniBackpack is a lightweight C++ Qt application designed to automate the installation of essential software and development tools for university students. By selecting the target University and Department, the application automatically detects the underlying Linux distribution's package manager (`apt` or `pacman`) and installs the required toolchain.

## Prerequisites

Before building UniBackpack, ensure that the following dependencies are installed on your system:

* A C++17 compatible compiler (`gcc` or `clang`)
* CMake (version 3.16 or higher)
* Qt6 Development Libraries
* Polkit (`pkexec`) for granting installation privileges

### Installing Dependencies

**For Arch Linux / Manjaro / EndeavourOS:**

```bash
sudo pacman -S base-devel cmake qt6-base qt6-tools polkit
```

**For Debian / Ubuntu / Linux Mint:**
```bash
sudo apt install build-essential cmake qt6-base-dev qt6-tools-dev qt6-tools-dev-tools qt6-l10n-tools pkexec
```

## Project Structure

* `src/` - C++ source files (`.cpp`)
* `include/` - C++ header files (`.hpp`)
* `ui/` - Qt Designer forms (`.ui`)
* `resources/lists/<University>/<Department>/packages.json` - The software list for each academic department, with each tool's package name for every supported package manager. Every list in this directory is embedded automatically at build time.
* `resources/universities.json` - The universities and departments shown in the app, with their translated names.
* `resources/icons/` - Icons for each university.
* `translations/` - Qt Linguist translation files for the user interface.
* `tests/` - Unit tests for the package manager backends and the data files.

## Building

```bash
cmake -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/unibackpack
```

To install system-wide, including the desktop entry and icon:

```bash
sudo cmake --install build
```

## Adding a Department

1. Add the department to its university in `resources/universities.json`, with an `"el"` translation of its name.
2. Create `resources/lists/<University>/<Department>/packages.json`, using the exact English names from `universities.json` for the directories:

   ```json
   [
       { "name": "Java JDK", "apt": "default-jdk", "pacman": "jdk-openjdk" },
       { "name": "FreeCAD", "pacman": "freecad" }
   ]
   ```

   Leave out a package manager when the software isn't packaged for it.
3. Run the tests: they check that every list parses and matches a department.

Departments without packages for the user's package manager are shown greyed out.


## Contributors

<a href="https://github.com/open-source-uom/UniBackpack/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=open-source-uom/UniBackpack" />
</a>

---