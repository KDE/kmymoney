KMyMoney README.cmake
Author: Joerg Rodehueser <wampbier@users.sf.net>
    and Holger <yllohy@googlemail.com>
Date  : Feb 7 2009

This README briefly describes how to build KMyMoney with cmake.

-----------------------------------------------------
Quick-start 0: Precondition
-----------------------------------------------------

Check that cmake is installed on your machine and is in your PATH.
To do so, just type

$ cmake --version

on your command line.

For debian/ubuntu user the following command will check for
required packages to build KMyMoney:

  sudo apt-get build-dep kmymoney

-----------------------------------------------------
Quick-start 1: Build KMyMoney
-----------------------------------------------------

cmake is designed so that the build process can be done in a separate
directory. This is highly recommended for users and required for packagers.

Go to the top level of the git working directory.
To build KMyMoney in the subdirectory ./build/ type

$ mkdir build
$ cd build
$ cmake .. -DCMAKE_INSTALL_PREFIX=/usr
    to generate the Makefiles.
$ ccmake .
    to change the configuration of the build process. (optional)

Congratulations, your Makefiles were generated!
Now you could just type

$ make
    to build the project in the build/ directory.

Note that 'make' automatically checks whether any CMakeLists.txt file
has changed and reruns cmake if necessary.

$ make install
    to install the project. You have to install KMyMoney to run it.
Use sudo or switch to a superuser to run this command.

Congratulations, you will never have a chaos of generated files
between the important source files again!

-----------------------------------------------------
Quick-start 2: How to compile Debug-Builds
-----------------------------------------------------

As an example configuration option, you would like to configure a
debug build just as './configure --enable-debug=full' did before.

For this, you can conveniently create a new out-of-source build directory:

$ mkdir Debug
$ cd Debug
$ cmake -D CMAKE_BUILD_TYPE=Debugfull ..

Instead of the last command, you could also call CMake without command
line arguments and use the GUI to switch the build type.

$ cmake ..
$ ccmake .
    and change the option CMAKE_BUILD_TYPE to 'Debugfull'.  Selecting
    an option and pressing 'h' will show you its allowed values.

In any case, your choices are safely stored in the file CMakeCache.txt
which will never be completely overwritten.
If you want to reset your changes, you will have to delete this file.

-----------------------------------------------------
Quick-start 3: More options
-----------------------------------------------------

-DCMAKE_INSTALL_PREFIX=<path_to_install_dir>
    This option tells cmake where to install KMyMoney to.
    During development, this should be a directory in your development
    environment, such that you can debug the program and test the
    installation.
    The default is ${KDE3PREFIX}, which is usually "/opt/kde3/".

-DCMAKE_BUILD_TYPE=<type>
    Choose the type of build. Possible values are:
      'Release' 'RelWithDebInfo' 'Debug' 'Debugfull' 'Profile'
    The default value is: 'RelWithDebInfo'

-DKDE4_BUILD_TESTS=ON
    To also build the unit tests.

-----------------------------------------------------
Quick-start 4: Makefile targets
-----------------------------------------------------

After cmake has finished, you have a set of ordinary Makefiles in your
directory.  You can type

$ make help
    to see all available make targets in the current directory.

$ make
    to reconfigure the Makefiles and build the project.

$ make install
    to install KMyMoney to the directory CMAKE_INSTALL_PREFIX.

$ make DESTDIR=/tmp install
    to install KMyMoney to the directory /tmp/CMAKE_INSTALL_PREFIX.

$ make uninstall
    to uninstall a previous installation.

$ make package
    to create a binary tarball.

$ make package_source
    to create a source tarball.
    (Warning: must have a clean source directory and build out-of-source)

$ make kmymoney-unstable_rpm
$ make kmymoney_rpm
    to create binary rpm packages.
    (they only differ in the package name)

$ make kmymoney_srpm
$ make kmymoney-unstable_srpm
    to create source rpm packages.

$ make messages
    to extract and merge translations.
    (Warning: This will change the source files)

$ make developer-doc
    to create the developer handbook. Will also create HTML-version.

$ make test
    to process all unit tests.

-----------------------------------------------------
Quick-start 5: Run kmymoney from build dir
-----------------------------------------------------

Instead of installing kmymoney in a system location to be able to
run it with any newly compiled plugins, you can use the following
recipe to run kmymoney from a build directory.

After compiling kmymoney (see Quickstart 1:), the following commands
must be executed once to see icons in the application

$ cd build
$ make install DESTDIR=$PWD/tmp

After each build you can start kmymoney with

$ cd build
$ XDG_DATA_DIRS=$PWD/tmp/usr/local/share:$XDG_DATA_DIRS \
  QT_PLUGIN_PATH=$PWD/lib:$QT_PLUGIN_PATH \
  LD_LIBRARY_PATH=$PWD/lib bin/kmymoney
