# Function which takes a file and pastes in all #include-ed files
# Outputs in a variable called COMPLETE_FILE
function(parse_includes resource_file_name)
    get_filename_component(abs_path "${resource_file_name}" ABSOLUTE)
    get_filename_component(abs_dir "${abs_path}" DIRECTORY)

    file(READ "${abs_path}" og_file)
    message("Compiling: ${resource_file_name}")

    # Look for includes
    string(REGEX MATCHALL "#include ((<[^>]+>)|(\"[^\"]+\"))" files_to_include "${og_file}")

    foreach(x IN LISTS files_to_include)
        # Step 1: Get the required filename
        string(REGEX MATCH "((<[^>]+>)|(\"[^\"]+\"))" fname "${x}")
        string(REPLACE "<" "" fname "${fname}")
        string(REPLACE ">" "" fname "${fname}")
        string(REPLACE "\"" "" fname "${fname}")

        # Step 2: Parse includes on the required filename
        get_filename_component(combined_path "${abs_dir}/${fname}" ABSOLUTE)
        parse_includes("${combined_path}")

        # Step 3: Insert into original file
        string(REPLACE "${x}" "\n// Included file: ${fname}\n${COMPLETE_FILE}\n" og_file "${og_file}")
    endforeach()

    set(COMPLETE_FILE "${og_file}" PARENT_SCOPE)
endfunction()

function(parse_includes_file input_fname output_fname)
    parse_includes("${input_fname}")
    file(WRITE "${output_fname}" "${COMPLETE_FILE}")
endfunction()

function(glsl_validate input_fname)
    # Make sure to call the correct executable
    set(has_glslang OFF)
    set(glslang_path "")
    if (UNIX AND NOT APPLE)
        set(has_glslang ON)
        set(glslang_path "Tools/glslang-master-linux-Release/bin/glslangValidator")
    elseif(WIN32)
        set(has_glslang ON)
        set(glslang_path "Tools/glslang-master-windows-x64-Release/bin/glslangValidator")
    else()
        message("-- GLSL Validation is not supported on this platform")
    endif()

    if(has_glslang)
        get_filename_component(glslang_path "${glslang_path}" ABSOLUTE)
        execute_process(COMMAND "${glslang_path}" "${input_fname}"
                RESULT_VARIABLE result
                OUTPUT_VARIABLE output)

        if (NOT result EQUAL "0")
            message("${output}")
        endif()
    endif()
endfunction()