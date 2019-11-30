var File = require("qbs.File");
var TextFile = require("qbs.TextFile");
var FileInfo = require("qbs.FileInfo");

function installTemplate(templateFilePath, outputFilePath, dict)
{
    File.makePath(FileInfo.path(outputFilePath))

    var sourceFile = new TextFile(templateFilePath, TextFile.ReadOnly)
    var newContent = sourceFile.readAll()
    sourceFile.close()

    //fix template content
    for(var key in dict) {
        var re = new RegExp("@"+key+"@", "g")
        newContent = newContent.replace(re, dict[key])
    }

    var targetFile = new TextFile(outputFilePath, TextFile.WriteOnly)
    targetFile.write(newContent)
    targetFile.close()
}


function setPropValues(dict, prefix)
{
    if(!prefix)prefix=""
    var result=""
    for(var key in dict){
        var v=dict[key]
        if(v instanceof Array){
            result=result+prefix+key+": product."+prefix+key+".concat("+JSON.stringify(v)+");\n"
            continue;
        }
        if(v instanceof Object && !(v instanceof Array) ){
            result=result+setPropValues(v, prefix+key+".")
            continue;
        }
        result=result+prefix+key+": "+JSON.stringify(v)+";\n"
    }
    return result
}
