import qbs.FileInfo

Project {

    //name: "ApxSharedLibs"
    property stringList names: []
    property string prefix: name+"."

    property bool sdk: false

    property stringList defines: []

    references: names.map(function(s){
        //console.info("Library: "+s)
        s=FileInfo.cleanPath(FileInfo.joinPaths(project.sourceDirectory, "../lib", s, s.split("/").slice(-1)[0]+".qbs"))
        //console.info(s)
        return s
    })
}
