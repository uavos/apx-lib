import qbs.FileInfo
import qbs.File

ModuleProvider {


    property path sourceDirectory: FileInfo.cleanPath(FileInfo.joinPaths(path, "../../.."))
    property string groupNamePrefix: ""

    relativeSearchPaths: {
        console.info("Scanning for submodules: "+name)
        var list=[]
        var paths = File.directoryEntries(sourceDirectory, File.Dirs|File.NoDotAndDotDot)
        for(var i in paths){
            var libName = paths[i]
            var p = FileInfo.joinPaths(sourceDirectory,libName)
            var inputFilePath = FileInfo.joinPaths(p, libName+".qbs")
            if(!File.exists(inputFilePath))
                continue
            outputFilePath = FileInfo.joinPaths(outputBaseDir, "modules", name, libName, libName+".qbs")
            File.makePath(FileInfo.path(outputFilePath))
            File.copy(inputFilePath, outputFilePath)
        }
        return [ "" ]
    }
}

