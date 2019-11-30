import qbs.FileInfo

Module {
    id: _module

    property stringList files: [
        "*.*",
    ]

    property string libName: name.split(".").pop()

    property string libsPath: FileInfo.cleanPath(FileInfo.joinPaths(path, "../.."))
    property string libPath: FileInfo.joinPaths(libsPath, libName)

    Group {
        name: _module.name
        files: _module.files
        prefix: _module.libPath+"/"
        excludeFiles: ["*.qbs"]
    }

    Depends { name: "cpp" }
    cpp.includePaths: libsPath

}
