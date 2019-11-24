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
    // flat contents of all collected modules
    property var contents: {}

    property var modified: [] //for dep update tracking

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
        var values = {}

        function moduleFiles(m)
        {
            //check files exist
            var list=[]
            var p=FileInfo.path(m.config)
            for(var i in m.files){
                var f=FileInfo.joinPaths(p,m.files[i])
                if(File.exists(f)){
                    list.push(FileInfo.cleanPath(f))
                }else{
                    throw new Error("Node file not found ("+m.name+"): " + f)
                }
            }
            return list
        }

        /*function removeStringDups(names)
        {
          unique = {}
          names.forEach(function(i) {
            if(!unique[i]) {
              unique[i] = true
            }
          })
          return Object.keys(unique)
        }*/

        for(var i in array){
            var m=array[i]

            modified.push(m.timestamp) //for dep update tracking

            m.files = moduleFiles(m)
            m.files.push(m.config)

            //includes are relative to module path
            for(var j in m.include){
                m.include[j] = FileInfo.cleanPath(FileInfo.joinPaths(mpath,m.include[j]))
            }

            //update values object
            for(var key in m){
                if(!m[key]) continue
                if(!values[key])
                    values[key]=[]
                var p=m[key]
                if(p instanceof Array){
                    for(var k in m[key]){
                        var v=m[key][k]
                        if(values[key].contains(v))
                            continue
                        values[key].push(v)
                    }
                }else{
                    if(values[key].contains(m[key]))
                        continue
                    values[key].push(m[key])
                }
            }
        }
        //ensure required fields present
        var rkeys =[ "files", "libs", "app" ]
        for(var i in rkeys){
            var key=rkeys[i]
            if(!values[key])
                values[key]=[]
        }

        contents = values
        console.info(JSON.stringify(contents,0,2))
    }

}
