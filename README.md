# UniBackpack

UniBackpack is a lightweight C++ Qt application that installs the software and development tools a university student needs for their studies. You pick your university and department, and UniBackpack detects your Linux distribution's package manager (`pacman` or `apt`) and installs that department's toolchain in one go.

It ships with UniOS, and runs on any Arch- or Debian-based distribution.

## How it works

1. The app lists the universities, then the departments of the one you pick. Departments that have no package list for your package manager yet are greyed out.
2. When you pick a department, UniBackpack checks which of its packages exist in your repositories and reports the ones that don't.
3. It installs the available ones through `pkexec`, so you are asked for your password once, and shows the progress and the package manager's output.

The interface is available in English and Greek, selectable from the status bar.

## Installation

### As a user

**On UniOS**, UniBackpack is preinstalled. If you removed it, install it again from `unios-repo`:

```bash
sudo pacman -S unibackpack
```

**On other Arch-based distributions**, build the package from the `PKGBUILD` in this repository:

```bash
git clone https://github.com/open-source-uom/UniBackpack.git
cd UniBackpack
makepkg -si
```

**On Debian-based distributions**, build and install it from source as described below.

### As a developer

Requirements:

* A C++17 compiler (`gcc` or `clang`)
* CMake 3.16 or newer
* Qt 6 (Core, Gui, Widgets, LinguistTools and Test)
* Polkit (`pkexec`), to run the installs as root

Install them on **Arch Linux / UniOS / Manjaro / EndeavourOS**:

```bash
sudo pacman -S base-devel cmake qt6-base qt6-tools polkit
```

or on **Debian / Ubuntu / Linux Mint**:

```bash
sudo apt install build-essential cmake qt6-base-dev qt6-tools-dev qt6-tools-dev-tools qt6-l10n-tools pkexec
```

Then build, run the tests and start the app:

```bash
cmake -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/unibackpack
```

To install it system-wide, including the desktop entry and icon:

```bash
sudo cmake --install build
```

Every push and pull request is built and tested on both Ubuntu (apt) and Arch Linux (pacman) by GitHub Actions, which also runs `cppcheck`. API documentation can be generated with `doxygen` from the repository root; it is written to `docs/`.

## Project Structure

```
UniBackpack/
├── CMakeLists.txt            build, install and test configuration
├── PKGBUILD                  Arch package recipe
├── Doxyfile                  API documentation settings
├── include/                  headers (.hpp)
│   ├── Downloader.hpp        runs the repository check and the install
│   ├── MainWindow.hpp        the application window
│   ├── PackageManager.hpp    pacman and apt backends
│   └── utils/
│       ├── language.hpp          supported languages and switching
│       ├── loadUniversities.hpp  reads universities.json
│       └── packageList.hpp       reads a department's packages.json
├── src/                      sources (.cpp), same layout as include/
├── ui/
│   └── MainWindow.ui         Qt Designer form
├── resources/
│   ├── universities.json     universities and departments, with Greek names
│   ├── lists/
│   │   └── <University>/<Department>/packages.json
│   ├── icons/                university logos and the app icon
│   ├── resources.qrc         icons and universities.json
│   └── unibackpack.desktop   desktop entry
├── translations/             Qt Linguist files (unibackpack_el.ts, unibackpack_en.ts)
├── tests/
│   └── tst_package_manager.cpp
└── .github/workflows/build.yml
```

All non-UI code is built as the `unibackpack_core` library, so the tests can link against it. The package lists are embedded into the binary automatically at build time; there is no need to list them in `resources.qrc`.

### Data files

`resources/universities.json` lists the universities and their departments. The English `name` is also the directory name under `resources/lists/`, and `translations` holds the names shown in other languages:

```json
[
    {
        "name": "University of Western Macedonia",
        "translations": { "el": "Πανεπιστήμιο Δυτικής Μακεδονίας" },
        "icon": ":/icons/uowm_logo.png",
        "departments": [
            { "name": "Informatics", "translations": { "el": "Τμήμα Πληροφορικής" } }
        ]
    }
]
```

`resources/lists/<University>/<Department>/packages.json` lists the software for one department, with its package name for each package manager. Leave a package manager out when the software isn't packaged for it:

```json
[
    { "name": "Java JDK", "apt": "default-jdk", "pacman": "jdk-openjdk" },
    { "name": "MariaDB", "apt": "mariadb-server", "pacman": "mariadb" },
    { "name": "FreeCAD", "pacman": "freecad" }
]
```

### Files the app writes

UniBackpack only stores the language you picked, through `QSettings`, in `~/.config/UniBackpack/UniBackpack.conf`:

```ini
[General]
language=el_GR
```

Without it, the app follows the system language.

## Adding a Department

1. Add the department to its university in `resources/universities.json`, with an `"el"` translation of its name.
2. Create `resources/lists/<University>/<Department>/packages.json`, using the exact English names from `universities.json` for the directories.
3. Run the tests. They check that every list parses, that every entry has at least one package, and that every list matches a department in `universities.json`.

## Adding a Translation

1. Add the language to `Utils::supportedLanguages()` in `src/utils/language.cpp`.
2. Add `translations/unibackpack_<language>.ts` to `qt_add_translations` in `CMakeLists.txt`.
3. Build the `update_translations` target to fill it with the source strings, translate them with Qt Linguist, and add a `translations` entry for that language to every university and department in `universities.json`.

## Roadmap

Help is welcome on:

* Package lists for the departments that don't have one yet. Only UoWM Informatics has one so far.
* `apt` package names for FreeCAD and Wireshark in the UoWM Informatics list.
* A backend for `dnf` (Fedora).

## Recent Changes

* The install logic moved into separate `pacman` and `apt` backends behind a `PackageManager` interface, with unit tests for their output parsing.
* The two per-department text lists (`pacman_list.txt`, `apt_list.txt`) were merged into one `packages.json`.
* University and department names are translated in `universities.json` instead of the `.ts` files.
* The repository check is now a single non-blocking call, and failures always report a reason instead of leaving the window disabled.
* CI builds and tests on both Ubuntu and Arch Linux.

## Contributors

Maintained by Apostolos Chalis <achalis@csd.auth.gr> and Ioannis Michadasis <cs05415@uowm.gr>, with contributions from:

<a href="https://github.com/open-source-uom/UniBackpack/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=open-source-uom/UniBackpack" />
</a>

UniBackpack is part of the [UniOS project](https://unios-project.eu). See the [UniOS docs](https://docs.unios-project.eu) for the general development workflow.
