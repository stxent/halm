# Copyright (C) 2017 xent
# Project is distributed under the terms of the MIT License

function(parse_config _config_file _config_output)
    get_cmake_property(_global_config_data VARIABLES)
    file(STRINGS "${_config_file}" _config_data)
    set(_output "")

    foreach(_entry ${_config_data})
        string(FIND "${_entry}" "CONFIG_" _entry_is_config)
        if(${_entry_is_config} EQUAL 0)
            string(REGEX REPLACE "(CONFIG_[^=]+)=\"?([^\"]+)\"?" "\\1;\\2" _splitted_entry ${_entry})
            list(GET _splitted_entry 0 _entry_name)
            list(GET _splitted_entry 1 _entry_value)

            if(DEFINED ${_entry_name})
                continue()
            endif()

            if("${_entry_value}" STREQUAL "y")
                set(${_entry_name} ON PARENT_SCOPE)
                list(APPEND _output -D${_entry_name})
            elseif("${_entry_value}" STREQUAL "n")
                set(${_entry_name} OFF PARENT_SCOPE)
            else()
                set(${_entry_name} "${_entry_value}" PARENT_SCOPE)
                # Entry values should not contain any special characters, symbols or spaces
                list(APPEND _output -D${_entry_name}=${_entry_value})
            endif()
        endif()
    endforeach()

    foreach(_entry_name ${_global_config_data})
        string(FIND "${_entry_name}" "CONFIG_" _entry_is_config)
        if(${_entry_is_config} EQUAL 0)
            string(TOUPPER ${${_entry_name}} _entry_value)

            # Entry values should not contain any special characters, symbols or spaces
            if("${_entry_value}" MATCHES "ON|YES|TRUE|Y")
                list(APPEND _output -D${_entry_name})
            elseif(NOT "${_entry_value}" MATCHES "OFF|NO|FALSE|N")
                list(APPEND _output -D${_entry_name}=${${_entry_name}})
            endif()

            continue()
        endif()
    endforeach()

    set(${_config_output} "${_output}" PARENT_SCOPE)
endfunction()
