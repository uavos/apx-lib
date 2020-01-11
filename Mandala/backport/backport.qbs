import qbs.File
import qbs.FileInfo
import qbs.Process

ApxSharedLibModule {
    Depends { 
        name: "apx_libs"
        submodules: [
            "Mandala",
        ]
    }
}
