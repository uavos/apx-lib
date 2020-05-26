import qbs.FileInfo
import qbs.File

ModuleProvider {


    property path sourceDirectory: FileInfo.cleanPath(FileInfo.joinPaths(path, "../../.."))
    property string groupNamePrefix: ""


    relativeSearchPaths: {
        console.info("Scanning for submodules: "+name)

        function scanDirRecursively(srcDir, module_prefix)
        {
            var paths = File.directoryEntries(srcDir, File.Dirs|File.NoDotAndDotDot)
            for(var i in paths){
                var libFolder = paths[i]
                if(libFolder=="qbs")
                    continue

                var libModuleName = libFolder
                if(module_prefix) libModuleName = module_prefix + "__" + libModuleName

                var p = FileInfo.joinPaths(srcDir, libFolder)

                //recursive
                scanDirRecursively(p, libModuleName)

                //find lib module
                var files = File.directoryEntries(p, File.Files)
                for(var f in files){
                    var fname=files[f]
                    if(FileInfo.completeSuffix(fname) != "qbs")
                        continue
                    var inputFilePath = FileInfo.joinPaths(p, fname)
                    var basename = FileInfo.completeBaseName(fname)
                    if(basename == libFolder){
                        outputFilePath = FileInfo.joinPaths(outputBaseDir, "modules", name, libModuleName, fname)
                    }else{
                        outputFilePath = FileInfo.joinPaths(outputBaseDir, "modules", name, libModuleName+"__"+basename, fname)
                    }
                    File.makePath(FileInfo.path(outputFilePath))
                    File.copy(inputFilePath, outputFilePath)
                }

//                var inputFilePath = FileInfo.joinPaths(p, libFolder+".qbs")
//                if(!File.exists(inputFilePath))
//                    continue

//                outputFilePath = FileInfo.joinPaths(outputBaseDir, "modules", name, libModuleName, libFolder+".qbs")
//                File.makePath(FileInfo.path(outputFilePath))
//                File.copy(inputFilePath, outputFilePath)
            }
        }

        scanDirRecursively(sourceDirectory)
        return [ "" ]
    }
}

