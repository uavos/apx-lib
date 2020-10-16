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


    /*Probe {
        id: _probe

        //input
        property stringList dictPath: FileInfo.joinPaths(_module.libPath, "dict", "mandala.yml")
        property stringList tool: FileInfo.joinPaths(_module.libPath, "parse_mandala_dict.py")

        property pathList files: {
            var list = []
            var files = File.directoryEntries(FileInfo.path(dictPath), File.Files)
            for(var i in files){
                var f=FileInfo.joinPaths(FileInfo.path(dictPath), files[i])
                list.push(f)
            }
            //console.info(list)
            return list
        }

        //save config files timestamps to refresh probe on changes
        property varList timestamps: {
            var list = []
            var files = File.directoryEntries(FileInfo.path(dictPath), File.Files)
            for(var i in files){
                var f=FileInfo.joinPaths(FileInfo.path(dictPath), files[i])
                list.push(File.lastModified(f))
            }
            list.push(File.lastModified(tool))
            console.info("Mandala dict parsed")
            return list
        }

        //output
        property var data

        configure: {
            var p = new Process();
            p.throwOnError=true
            var args = []
            args.push(tool)
            args.push("--dict")
            args.push(dictPath)

            if(p.exec("python", args)===0){
                data = JSON.parse(p.readStdOut());
                found = true

                //console.info(JSON.stringify(data,'',' '))
            }
        }
    }*/
}
