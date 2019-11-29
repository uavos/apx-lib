import qbs.FileInfo

StaticLibrary {

    property path libSourceDirectory: FileInfo.joinPaths(project.sourceDirectory, "../lib")

    Depends { name: project.arch; condition: project.arch }
    Depends { name: "cpp" }

    files: [
        "*.h*",
        "*.c*",
    ]

    cpp.cLanguageVersion: "c11"
    cpp.cxxLanguageVersion: "c++11"

    cpp.defines: project.defines

    cpp.includePaths: libSourceDirectory


    //support multiplex build
    qbs.architectures: project.qbs.architectures
    multiplexByQbsProperties: ["architectures"]
    aggregate: false

    Depends { name: "sdk"; submodules: [ "headers" ]; condition: project.sdk }

    Export {
        Depends { name: "cpp" }
        cpp.includePaths: product.libSourceDirectory

        Parameters {
            cpp.linkWholeArchive: true
        }
    }
}
