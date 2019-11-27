import qbs.FileInfo

StaticLibrary {

    Depends { name: project.arch; condition: project.arch }
    Depends { name: "cpp" }

    files: [
        "*.h",
        "*.c*",
    ]

    /*Group {
        fileTagsFilter: ["staticlibrary"]
        fileTags: ["apx_lib", "apx_lib_Calc"]
        overrideTags: false
        filesAreTargets: true
    }*/

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
