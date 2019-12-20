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

Module {
    id: _module

    //input
    property varList data


    //internal
    Depends { name: "cpp" }
    cpp.includePaths: [FileInfo.joinPaths(product.buildDirectory,"gensrc")]

    property string tool: FileInfo.joinPaths(path, "gensrc.py")

    FileTagger {
        patterns: ["*.h.j2", "*.cpp.j2", "*.c.j2"]
        fileTags: ["gensrc.input"]
    }
    Rule {
        inputs: ["gensrc.input"]
        outputFileTags: ["h", "cpp", "c"]
        outputArtifacts: {
            var a, list = []
            var data=product.gensrc.data
            var tname = input.completeBaseName
            /*if(!(data instanceof Array)){
                data=[data]
            }
            for(var i in data){
                var d=data[i]
                a = {}
                a.fileTags = [ FileInfo.completeSuffix(tname) ]
                a.filePath = FileInfo.joinPaths(product.buildDirectory, "gensrc", d.name.replace(/\./g,"_")+"_"+tname)
                list.push(a)
            }*/
            a = {}
            a.fileTags = [ FileInfo.completeSuffix(tname) ]
            a.filePath = FileInfo.joinPaths(product.buildDirectory, "gensrc", tname)
            list.push(a)
            return list
        }
        prepare: {
            var cmd, commands = []

            var args = []
            args.push(product.gensrc.tool)
            args.push("--template")
            args.push(input.filePath)
            args.push("--dest")
            args.push(FileInfo.joinPaths(product.buildDirectory, "gensrc"))
            args.push("--data")
            args.push(JSON.stringify(product.gensrc.data))

            cmd = new Command("python", args);
            cmd.description = "Parsing "+input.fileName
            cmd.highlight = "codegen"
            cmd.stdoutFilterFunction=function(s){
                return ""
            }
            commands.push(cmd)

            return commands
        }
    }
}
