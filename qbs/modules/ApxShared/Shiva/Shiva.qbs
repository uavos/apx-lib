import qbs.FileInfo

Module {
    id: _module

    Depends {
        name: "ApxShared"
        submodules: [
            "Mandala",
        ]
    }
    Group {
        name: _module.name
        prefix: FileInfo.joinPaths(ApxShared.include.libDir, "Shiva", "/")
        files: [
            "**/*",
        ]
    }
}
