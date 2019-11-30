import qbs.FileInfo
import qbs.File
import qbs.Process

Module {
    id: _module

    //constants and template parameters
    property string moduleName: "@moduleName@"
    property string submoduleName: "@submoduleName@"
    property path configFile: "@configFile@"
    property string depends: "@depends@"
    property string libs: "@libs@"
    property string groupNamePrefix: "@groupNamePrefix@"


    //internal
    property path sourceDirectory: FileInfo.path(configFile)
    property var configTimestamp: File.lastModified(configFile)
    property string fullName: moduleName+"."+submoduleName.replace(/\./g, '__')


    Depends { name: "cpp" }

    // collect parameters in main module
    Depends { id: _module_all; name: _module.moduleName+".all" }
    _module_all.apps: content.app
    _module_all.libs: content.libs
    _module_all.configs: content.config
    _module_all.names: content.name

    //get content from main module
    property var content_all: _module_all.content
    property var content: {
        for(var i in content_all){
            var m=content_all[i]
            if(m.name != _module.submoduleName)
                continue
            //cpp.defines="TEST"
            return m
        }
        return {}
    }

    // depends on other modules
    Depends {
        condition: _module.depends
        name: _module.moduleName
        submodules: _module.depends.replace(/\./g, '__').split(',')
    }

    Group {
        property bool include_files: product.moduleProperty(_module.moduleName+".all", "include_files")
        name: _module.groupNamePrefix+_module.submoduleName
        prefix: FileInfo.path(_module.configFile)+"/"
        property stringList mfiles: product.moduleProperty(_module.fullName, "content").files
        files: {
            if(!include_files)
                return []
            var list = []
            if(mfiles) list.push.apply(list, mfiles)
            list.push(FileInfo.fileName(_module.configFile))
            return list
        }

@extra_content_group@

    }


@extra_content@

}
