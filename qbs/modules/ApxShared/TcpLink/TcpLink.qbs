import qbs.FileInfo

Module {
    id: _module

    Depends {
        name: "ApxShared"
        submodules: [
            "include",
            "Math",
        ]
    }
    Group {
        name: _module.name
        prefix: FileInfo.joinPaths(ApxShared.include.libDir, "TcpLink", "/")
        files: [
            "tcp_client.cpp", "tcp_client.h",
            "tcp_server.cpp", "tcp_server.h",
        ]
    }
}
