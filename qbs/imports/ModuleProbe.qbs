/****************************************************************************
**
** Copyright (C) 2019 Aliaksei Stratsilatau
** Contact: http://docs.uavos.com
**
** This file is part of APX.
**
** All rights reserved.
**
****************************************************************************/

import qbs.FileInfo
import qbs.File
import qbs.Process

Probe {
    // Inputs
    property stringList names
    property stringList searchPaths

    // Output
    property stringList files:      []
    property stringList includes:   []
    property stringList defines:    []
    property stringList paths:      []
    property stringList mdefs:      []
    property stringList libs:       []

    // internal
    property var array: {
        var v=[]
        if(!names.length>0)return v
        var p = new Process();

        var args = []
        args.push(FileInfo.joinPaths(path,"scan_modules.py"))
        args.push("--paths")
        args=args.concat(searchPaths)
        args.push("--modules")
        args=args.concat(names)

        if(p.exec("python", args, true)===0){
            json=p.readStdOut()
            //console.info(json)
            v=JSON.parse(json)
        }else{
            throw("Module scan failed: "+p.readStdErr())
        }

        //ensure probe update on modified config
        for(var i in v)
            v[i].timestamp=File.lastModified(v[i].config)

        return v
    }

    configure: {
        var v_files=[]
        var v_inc=[]
        var v_def=[]

        var v_paths=[]
        var v_mdefs=[]

        var v_libs=[]

        function nodeModuleFiles(m)
        {
            //check files exist
            var list=[]
            var p=FileInfo.path(m.config)
            for(var i in m.files){
                var f=FileInfo.joinPaths(p,m.files[i])
                if(File.exists(f)){
                    list.push(f)
                }else{
                    throw new Error("Node file not found ("+m.name+"): " + f)
                }
            }
            return list
        }

        function removeDups(names)
        {
          unique = {}
          names.forEach(function(i) {
            if(!unique[i]) {
              unique[i] = true
            }
          })
          return Object.keys(unique)
        }

        for(var i in array){
            var m=array[i]
            v_files.push(m.config)
            v_files.push.apply(v_files,nodeModuleFiles(m))

            v_libs.push.apply(v_libs, m.libs)

            var mpath=FileInfo.path(m.config)
            for(var j in m.includes){
                var p=m.includes[j]
                v_inc.push(FileInfo.cleanPath(FileInfo.joinPaths(mpath,p)))
            }

            v_def.push.apply(v_def,m.defines)

            v_paths.push(mpath)
            v_mdefs.push("MODULE_"+m.name.replace(/\//g,"_").toUpperCase())
        }

        //fix path for files
        for(var i in v_files){
            //v_files[i]=FileInfo.relativePath(searchPath,v_files[i])
            v_files[i]=FileInfo.cleanPath(v_files[i])
        }

        files=removeDups(v_files)
        includes=removeDups(v_inc)
        defines=removeDups(v_def)
        paths=removeDups(v_paths)
        mdefs=removeDups(v_mdefs)
        libs=removeDups(v_libs)
        //console.info(files)
    }

}
