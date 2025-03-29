#!/bin/bash
git submodule update --init
./vcpkg/bootstrap-vcpkg.sh
./vcpkg/vcpkg install