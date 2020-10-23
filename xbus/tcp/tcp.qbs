ApxSharedLibModule {

    Depends { 
        name: "apx_libs"
        submodules: [ 
            "common",
            "crc",
        ]
    }
}
