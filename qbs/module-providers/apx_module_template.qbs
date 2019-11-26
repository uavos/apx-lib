import qbs.FileInfo
import qbs.File
import qbs.Process

Module {
    id: _module
    property string moduleName: "@moduleName@"
    property string submoduleName: "@submoduleName@"
    property string configFile: "@configFile@"
    property string depends: "@depends@"

    //internal
    property var configTimestamp: File.lastModified(configFile)
    property string fullName: moduleName+"."+submoduleName.replace(/\./g, '__')
    property var content: _probe.content


    Depends { name: "apx_module" }
    apx_module.apps: content.app
    apx_module.libs: content.libs

    Depends {
        condition: _module.depends
        name: _module.moduleName
        submodules: _module.depends.replace(/\./g, '__').split(',')
    }

    Probe {
        id: _probe
        // Inputs
        property string configFile: _module.configFile
        property path scan_modules: FileInfo.joinPaths(project.sourceDirectory, "../lib/qbs/scan_modules.py")

        // Output
        // JSON config content
        property var content

        configure: {
            var p = new Process();
            var args = []
            args.push(scan_modules)
            args.push("--config")
            args.push(configFile)

            if(p.exec("python", args, true)===0){
                json=p.readStdOut()
                p.close()
                //console.info(json)
                content=JSON.parse(json)
            }else{
                throw("Module scan failed: "+p.readStdErr())
            }
        }
    }

    Group {
        name: _module.submoduleName
        prefix: FileInfo.path(_module.configFile)+"/"
        files: product.moduleProperty(_module.fullName, "content").files
        .concat([
            FileInfo.fileName(_module.configFile),
        ])
    }

}
