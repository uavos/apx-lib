import qbs.FileInfo

Module {
    id: _module

    Depends {
        name: "ApxShared"
        submodules: [
            "include",
        ]
    }
    Group {
        name: _module.name
        prefix: FileInfo.joinPaths(ApxShared.include.libDir, "Math", "/")
        files: [
            "*.h",
            "crc.c",
        ]
    }
}
