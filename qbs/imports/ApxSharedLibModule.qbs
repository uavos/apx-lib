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
        property bool skip_libs: product.skip_libs
        name: _module.name
        files: skip_libs?[]:_module.files
        prefix: _module.libPath+"/"
        excludeFiles: ["*.qbs"]
        condition: !skip_libs
        filesAreTargets: skip_libs
    }

    Depends { name: "cpp" }
    cpp.includePaths: libsPath

}
