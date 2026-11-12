# Перекодировка исходников из UTF-8 в CP1251 для NDK clang
# (NDK не поддерживает -fexec-charset=CP1251)

function(convert_to_cp1251 OUTPUT_VAR INPUT_FILES)
    set(CP1251_SOURCES)
    
    foreach(src ${INPUT_FILES})
        # Определить относительный путь от REPO
        file(RELATIVE_PATH rel_path "${REPO}" "${src}")
        
        # Целевой путь в cp1251/
        set(cp1251_file "${CMAKE_CURRENT_BINARY_DIR}/cp1251/${rel_path}")
        get_filename_component(cp1251_dir "${cp1251_file}" DIRECTORY)
        
        # Создать каталог и перекодировать
        add_custom_command(
            OUTPUT "${cp1251_file}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${cp1251_dir}"
            # TRANSLIT: в комментариях есть →, ×, §, которых нет в CP1251.
            COMMAND iconv -f UTF-8 -t CP1251//TRANSLIT "${src}" -o "${cp1251_file}"
            DEPENDS "${src}"
            COMMENT "Converting ${rel_path} to CP1251"
            VERBATIM
        )
        
        list(APPEND CP1251_SOURCES "${cp1251_file}")
    endforeach()
    
    set(${OUTPUT_VAR} ${CP1251_SOURCES} PARENT_SCOPE)
endfunction()
