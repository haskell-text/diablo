#!/bin/sh

# This configures an Alpine container, which is used for testing.
apk add clang gcc meson git binutils luajit 

# Pulls down relevant code.
git clone https://github.com/haskell-text/diablo
