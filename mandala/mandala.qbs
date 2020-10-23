import qbs.File
import qbs.FileInfo
import qbs.Process

ApxSharedLibModule {
    id: _module

    Depends {
        name: "gensrc"
    }
    //gensrc.data: _probe.data
    gensrc.prefix: "mandala"

    files: base
    .concat([
        "dict/**/*.yml",
    ])

    FileTagger {
        patterns: ["*.yml"]
        fileTags: ["mandala.dict"]
    }

    Rule {
        inputs: ["mandala.dict"]
        multiplex: true
        Artifact {
            filePath: FileInfo.joinPaths(product.buildDirectory, "gensrc", product.gensrc.prefix, "mandala.json")
            fileTags: ["gensrc.data.mandala"]
        }

        prepare: {
            var cmd, commands = []

            var args = []
            args.push(FileInfo.joinPaths(product.apx_libs.mandala.libPath, "parse_mandala_dict.py"))
            args.push("--dict")
            args.push(FileInfo.joinPaths(product.apx_libs.mandala.libPath, "dict", "mandala.yml"))
            args.push("--out")
            args.push(output.filePath)

            cmd = new Command("python", args);
            cmd.description = "Parsing Mandala Dictionary source YML files..."
            cmd.highlight = "codegen"
            //cmd.stdoutFilterFunction=function(s){ return "" }
            commands.push(cmd)

            return commands
        }
    }
}
