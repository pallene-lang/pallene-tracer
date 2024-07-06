rockspec_format="3.0"
package = "pallene-tracer"
version = "0.5.0a-1"
source = {
    url = "http://github.com/pallene-lang/pallene-tracer"
}
description = {
    summary = "Pallene Tracer for Traceback",
    detailed = [[
        Universal traceback library for Lua C modules
    ]],
    homepage = "http://github.com/pallene-lang/pallene-tracer",
    license = "MIT"
}
dependencies = {
    "lua >= 5.4"
}
external_dependencies = {
   PTRACER = {
      header = "ptracer.h"
   }
}
build = {
    type = "builtin",
    modules = {
        ptinit = "ptinit/ptinit.c", 
    },
    install = {
        bin = {
            ["pallene-debug"] = "src/bin/pallene-debug"
        }
    }
}
