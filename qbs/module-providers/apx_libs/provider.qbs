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
                var libName = paths[i]
                if(libName=="qbs")
                    continue

                var libModuleName = libName
                if(module_prefix) libModuleName = module_prefix + "__" + libModuleName

                var p = FileInfo.joinPaths(srcDir,libName)

                //recursive
                scanDirRecursively(p, libModuleName)

                //find lib module
                var inputFilePath = FileInfo.joinPaths(p, libName+".qbs")
                if(!File.exists(inputFilePath))
                    continue

                outputFilePath = FileInfo.joinPaths(outputBaseDir, "modules", name, libModuleName, libName+".qbs")
                File.makePath(FileInfo.path(outputFilePath))
                File.copy(inputFilePath, outputFilePath)
            }
        }

        scanDirRecursively(sourceDirectory)
        return [ "" ]
    }
}

