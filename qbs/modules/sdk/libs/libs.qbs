import qbs
import qbs.File
import qbs.FileInfo

Module {

    additionalProductTypes: ["sdk.prepare"]

    Rule {
        inputs: ["dynamiclibrary", "dynamiclibrary_symlink"]
        multiplex: false
        Artifact {
            filePath: {
                var targetFileName = input.fileName;
                if(input.qbs.targetOS.contains("macos"))
                    targetFileName = "lib" + targetFileName + ".dylib"
                return "sdk/lib/" + targetFileName
            }
            fileTags: ["sdk.prepare"]
            qbs.install: false
        }

        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.description = "preparing for sdk " + input.fileName
            cmd.sourceCode = function(inputs, outputs) {
                File.copy(input.filePath, output.filePath)
            }
            return [cmd];
        }
    }
}
