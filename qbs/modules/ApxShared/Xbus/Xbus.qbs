import qbs.FileInfo

Module {
    id: _module

    Depends {
        name: "ApxShared"
        submodules: [
            "include",
        ]
    }
    Group {
        name: _module.name
        prefix: FileInfo.joinPaths(ApxShared.include.libDir, "Xbus", "/")
        files: [
//            "xbus.h",
//            "xbus_vehicle.h",
//            "xbus_node.h",
//            "xbus_node_conf.h",
//            "xbus_node_file.h",
//            "xbus_node_blackbox.h",
//            "xbus_mission.h",
//            "escaped.h",
            "*.*",
        ]
        excludeFiles: [
            "_*",
        ]
    }
}
