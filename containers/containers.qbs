import qbs.FileInfo

ApxSharedLibModule {

    Depends { 
        name: "apx_libs"
        submodules: [
            "common",
        ]
    }

}
