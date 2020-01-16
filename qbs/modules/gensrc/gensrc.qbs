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
        inputsFromDependencies: ["gensrc.input"]
        inputs: ["gensrc.input"]
        outputFileTags: ["hpp", "cpp", "c", "gensrc.output"]
        outputArtifacts: {
            var a, list = []
            var tname = input.completeBaseName
            var ftype = FileInfo.completeSuffix(tname)
            a = {}
            a.fileTags = [ "gensrc.output" ]
            if(ftype.startsWith("h"))a.fileTags.push("hpp")
            else a.fileTags.push(ftype)

            a.filePath = FileInfo.joinPaths(product.buildDirectory, "gensrc", product.gensrc.prefix, tname)
            console.info(product.gensrc.prefix)

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
            args.push(FileInfo.path(output.filePath))
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
