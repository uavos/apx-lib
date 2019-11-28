ApxSharedLib {
    name: project.prefix+"Mandala"

    Depends { name: project.prefix+"MandalaCore" }
    cpp.defines: "MANDALA_FULL"
}
