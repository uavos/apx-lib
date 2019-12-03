import qbs.FileInfo

Module {
    id: _module

    property stringList files: [
        "*.*",
    ]


    property string libName: name.replace(/__/g, ".")

    property string libsPath: FileInfo.cleanPath(FileInfo.joinPaths(path, "../.."))
    property string libPath: FileInfo.joinPaths(libsPath, libName.split(".").slice(1).join("/"))

    Group {
        property bool skip_libs: product.skip_libs
        name: _module.libName
        files: skip_libs?[]:_module.files
        prefix: _module.libPath+"/"
        excludeFiles: ["*.qbs"]
        condition: !skip_libs
        filesAreTargets: skip_libs
    }

    Depends { name: "cpp" }
    cpp.includePaths: [
        libsPath,
    ]

}
