#!/bin/sh
echo "Starting compilation..."
echo "mainrepl_enum.hpp"
g++ mainrepl_enum.hpp
echo "mainrepl_struct.hpp"
g++ mainrepl_struct.hpp
echo "b+tree.hpp"
g++ b+tree.hpp
echo "repl_helpers.hpp"
g++ repl_helpers.hpp
echo "mainrepl.cpp"
g++ mainrepl.cpp
