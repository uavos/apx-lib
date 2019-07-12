import qbs.FileInfo

Module {
    id: _module

    Depends {
        name: "ApxShared"
        submodules: [
            "include",
        ]
    }
    Group {
        name: _module.name
        prefix: FileInfo.joinPaths(ApxShared.include.libDir, "Mandala", "/")
        files: [
            "Mandala.cpp", "Mandala.h",
            "MandalaCore.cpp", "MandalaCore.h",
            "MatrixMath.cpp", "MatrixMath.h",
            "MandalaTemplate.h",
            "MandalaIndexes.h",
            "MandalaConstants.h",
            "preprocessor.h",
        ]
    }
}
