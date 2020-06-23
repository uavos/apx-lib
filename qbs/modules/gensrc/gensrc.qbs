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
import qbs.TextFile
import qbs.File

Module {
    id: _module

    //input
    property var data
    property string prefix


    //internal
    Depends { name: "cpp" }
    cpp.includePaths: [FileInfo.joinPaths(product.buildDirectory, "gensrc")]

    property string tool: FileInfo.joinPaths(path, "gensrc.py")

    FileTagger {
        patterns: ["*.h.j2", "*.cpp.j2", "*.c.j2"]
        fileTags: ["gensrc.input"]
    }
    Rule {
        //inputsFromDependencies: ["gensrc.data."+product.gensrc.prefix]
        explicitlyDependsOn:  ["gensrc.data."+product.gensrc.prefix]
        inputs: ["gensrc.input", "gensrc.data."+product.gensrc.prefix]
        multiplex: true
        outputFileTags: ["hpp", "cpp", "c", "gensrc.output"]
        outputArtifacts: {
            var list = []

            for(var i in inputs["gensrc.input"]){
                var inp = inputs["gensrc.input"][i]
                var tname = inp.completeBaseName
                var ftype = FileInfo.completeSuffix(tname)
                var a = {}
                a.fileTags = [ "gensrc.output" ]
                if(ftype.startsWith("h"))a.fileTags.push("hpp")
                else a.fileTags.push(ftype)

                a.filePath = FileInfo.joinPaths(product.buildDirectory, "gensrc", product.gensrc.prefix, tname)
                //console.info(product.gensrc.prefix)

                list.push(a)
            }
            return list
        }

        prepare: {
            var cmd, commands = []

            for(var i in inputs["gensrc.input"]){
                var inp = inputs["gensrc.input"][i]
                var tname = inp.completeBaseName
                var out = FileInfo.joinPaths(product.buildDirectory, "gensrc", product.gensrc.prefix, tname)

                var args = []
                args.push(product.gensrc.tool)
                args.push("--template")
                args.push(inp.filePath)
                args.push("--dest")
                args.push(FileInfo.path(out))
                args.push("--data")

                var tdata = "gensrc.data."+product.gensrc.prefix
                var fdata = inputs[tdata]
                var cmd_desc
                if(fdata){
                    console.info(tdata+"["+fdata.length+"]: " + fdata[0].filePath)
                    args.push(fdata[0].filePath)
                    cmd_desc=fdata[0].fileName
                }else{
                    var fname = FileInfo.joinPaths(product.buildDirectory, inp.fileName+".json")
                    args.push(fname)
                    File.makePath(FileInfo.path(fname))
                    var f = TextFile(fname, TextFile.WriteOnly)
                    f.write(JSON.stringify(product.gensrc.data,0,2))
                    f.close()
                }

                cmd = new Command("python", args);
                cmd.description = "Parsing "+inp.fileName + (cmd_desc?" with "+cmd_desc:"")
                cmd.highlight = "codegen"
                cmd.stdoutFilterFunction=function(s){
                    return ""
                }
                commands.push(cmd)
            }

            return commands
        }
    }
}
