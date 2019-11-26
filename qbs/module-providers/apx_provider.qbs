import qbs.FileInfo
import qbs.File
import qbs.Process
import qbs.TextFile

ModuleProvider {


    property stringList paths


    property path topdir: FileInfo.cleanPath(FileInfo.joinPaths(path, "../../.."))

    relativeSearchPaths: {
        console.info("ModuleProvider: "+name)
        console.info("path: "+path)
        console.info("filePath: "+filePath)

        var list=[]
        var p = new Process();

        var args = []
        args.push(FileInfo.joinPaths(path, "../scan_modules.py"))
        args.push("--paths")
        args.push.apply(args, paths.map(function(p){return FileInfo.joinPaths(topdir, p)}))
        args.push("--modules")
        args.push("all")

        console.info(args)

        if(p.exec("python", args, true)===0){
            json=p.readStdOut()
            list=JSON.parse(json)
        }else{
            console.info("Module scan failed: "+p.readStdErr())
            return []
        }

        //ensure probe update on modified config
        for(var i in list){
            list[i].timestamp=File.lastModified(list[i].config)
        }

        var names = []
        for(var i in list){
            var m=list[i]
            //console.info(JSON.stringify(m))
            names.push(m.name)
            //console.info("Module: "+name)
            var inputFilePath = FileInfo.joinPaths(path, "apx_module_template.qbs")
            var outputDir = FileInfo.joinPaths(outputBaseDir, "modules", name, m.name.replace(/\./g, '__'))
            File.makePath(outputDir)
            var moduleQbsName = m.name.split('.').pop()
            var outputFilePath = FileInfo.joinPaths(outputDir, moduleQbsName + ".qbs")
            //File.copy(inputFilePath, outputFilePath)

            var sourceFile = new TextFile(inputFilePath, TextFile.ReadOnly)
            var newContent = sourceFile.readAll()
            sourceFile.close()

            //fix template content
            newContent = newContent
            .replace(/@configFile@/g, m.config)
            .replace(/@moduleName@/g, name)
            .replace(/@submoduleName@/g, m.name)
            .replace(/@depends@/g, (m.depends?m.depends:[]).join(','))

            var targetFile = new TextFile(outputFilePath, TextFile.WriteOnly)
            targetFile.write(newContent)
            targetFile.close()
        }
        //console.info("Modules: "+names.join(','))


        return [ "" ]
    }
}

