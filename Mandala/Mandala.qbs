import qbs.File
import qbs.FileInfo
import qbs.Process

ApxSharedLibModule {
    id: _module

    Depends {
        name: "gensrc"
    }
    gensrc.data: _probe.data
    gensrc.prefix: "Mandala"

    files: base
    .concat([
        "dict/**/*.yml",
    ])

    Probe {
        id: _probe

        //input
        property stringList dictPath: FileInfo.joinPaths(_module.libPath, "dict", "mandala.yml")
        property stringList tool: FileInfo.joinPaths(_module.libPath, "parse_mandala_dict.py")

        //save config files timestamps to refresh probe on changes
        property varList timestamps: {
            var list = []
            var files = File.directoryEntries(FileInfo.path(dictPath), File.Files)
            for(var i in files){
                list.push(File.lastModified(FileInfo.joinPaths(FileInfo.path(dictPath), files[i])))
            }
            list.push(File.lastModified(tool))
            //console.info(list)
            return list
        }

        //output
        property var data: {}

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
    }
}
