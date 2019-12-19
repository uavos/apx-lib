ApxSharedLibModule {

    Depends { 
        name: "apx_libs" 
        submodules: [
            "common",
            "containers",
        ]
    }


    files: base.concat([
        "facts/*.*"
    ])
}
