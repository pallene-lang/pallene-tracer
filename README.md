<h1 align="center">The Pallene Tracer</h1>

Pallene Tracer is a project providing protocols and underlying mechanism aiming to enable Lua C modules to have proper function call-traceback, without touching a single code in Lua internals!

## Installing Pallene Tracer

### Prerequisites

You need to have Lua installed in your system in any prefix.

### Procedure

To install Pallene Tracer in `/usr/local` prefix, against system Lua with `/usr` prefix: 
```
sudo make install
```

To install Pallene Tracer against local Lua with `/usr/local` prefix (or any prefix):
```
sudo make install LUA_PREFIX=/usr/local
```

Pallene Tracer sometimes fails to build if Lua is built with address sanitizer (ASan) enabled. To get around the issue use: 
```
sudo make install LDFLAGS=-lasan
```

### How to use Pallene Tracer

Notes on how Pallene Tracer is designed and how to use it can be found in the [docs](https://github.com/pallene-lang/pallene-tracer/blob/main/docs/MANUAL.md). Also feel free to look at the `examples` directory for further intuition.
