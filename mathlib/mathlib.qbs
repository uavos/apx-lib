import qbs.FileInfo

ApxSharedLibModule {

    files: base
    .concat([
        "**/**",
    ])

    cpp.includePaths: base.concat([
        FileInfo.joinPaths(libsPath, "mathlib"),
    ])

}
