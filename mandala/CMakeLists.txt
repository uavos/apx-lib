file(GLOB_RECURSE dict_src "dict/*.yml")

add_custom_command(
    OUTPUT mandala.json
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/parse_mandala_dict.py --dict ${CMAKE_CURRENT_SOURCE_DIR}/dict/mandala.yml --out ${CMAKE_CURRENT_BINARY_DIR}/mandala.json
    DEPENDS parse_mandala_dict.py ${dict_src}
)

apx_gensrc(mandala_headers DATA mandala.json PREFIX mandala)

apx_module(
    DEPENDS
    shared.common
    GENSRC
    mandala_headers
    INCLUDES
    .
    PREFIX
    mandala
)
