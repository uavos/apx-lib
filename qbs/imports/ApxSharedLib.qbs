import qbs.FileInfo

StaticLibrary {

    Depends { name: project.arch; condition: project.arch }
    Depends { name: "cpp" }

    files: [
        "*.h*",
        "*.c*",
    ]

    cpp.cLanguageVersion: "c11"
    cpp.cxxLanguageVersion: "c++11"

    //support multiplex build
    qbs.architectures: project.qbs.architectures
    multiplexByQbsProperties: ["architectures"]
    aggregate: false

    Depends { name: "sdk"; submodules: [ "headers" ]; condition: project.sdk }
    //cpp.visibility: "default"
    //cpp.defines: project.cpp_defines

    Export {
        Depends { name: "cpp" }
        cpp.includePaths: FileInfo.joinPaths(project.sourceDirectory, "../lib")
        cpp.defines: product.cpp.defines

        Parameters {
            cpp.linkWholeArchive: true
        }
    }
}
