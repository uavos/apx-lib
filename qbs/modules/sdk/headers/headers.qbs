import qbs
import qbs.File
import qbs.FileInfo

Module {

    additionalProductTypes: ["sdk.prepare", "sdk.headers"]

    Rule {
        inputs: ["hpp"]
        multiplex: false
        Artifact {
            filePath: {
                var dest = "sdk/include"
                var inp = FileInfo.cleanPath(input.filePath)
                var rpath =
                        [
                            FileInfo.cleanPath(product.sourceDirectory),
                            FileInfo.cleanPath(FileInfo.joinPaths(project.sourceDirectory, "../lib")),
                            FileInfo.cleanPath(FileInfo.joinPaths(product.buildDirectory, "gensrc")),
                        ]
                for(var i in rpath){
                    if(inp.indexOf(rpath[i]) == 0)
                    {
                        var tail = FileInfo.relativePath(rpath[i], inp)
                        return FileInfo.cleanPath(FileInfo.joinPaths(dest, product.name, tail))
                    }
                }
                //console.info(input.filePath)
                return FileInfo.joinPaths(dest, product.name, input.fileName)
            }
            fileTags: ["sdk.prepare", "sdk.headers"]
            qbs.install: false
        }

        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.highlight = "filegen"
            cmd.description = "preparing for sdk " + input.fileName
            cmd.sourceCode = function() {
                //console.info(output.filePath)
                File.copy(input.filePath, output.filePath)
            }
            return [cmd];
        }
    }
}
