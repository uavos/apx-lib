import qbs.FileInfo

Project {

    name: "ApxSharedLibs"
    property stringList names: []

    references: names.map(function(s){
        console.info("Library: "+s)
        s=FileInfo.cleanPath(FileInfo.joinPaths(project.sourceDirectory, "../lib", s, s.split("/").slice(-1)[0]+".qbs"))
        //console.info(s)
        return s
    })
}
