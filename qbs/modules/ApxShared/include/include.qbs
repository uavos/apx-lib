import qbs.FileInfo

Module {
    id: _module

    Depends { name: "cpp" }

    property string libDir: FileInfo.cleanPath(FileInfo.joinPaths(path, "../../../.."))

    cpp.includePaths: [
        libDir,
        FileInfo.joinPaths(libDir, "include"),
    ]

    Group {
        name: _module.name
        prefix: FileInfo.joinPaths(_module.libDir, "include", "/")
        files: [
            "**/*",
        ]
    }
}
