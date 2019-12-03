import qbs.FileInfo

ApxSharedLibModule {

    cpp.includePaths: base.concat([
        FileInfo.joinPaths(libsPath, "common"),
    ])

}
