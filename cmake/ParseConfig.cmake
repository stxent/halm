# Copyright (C) 2017 xent
# Project is distributed under the terms of the GNU General Public License v3.0

function(parse_config _config_variables _config_file)
    file(STRINGS ${_config_file} _config_data)
    set(_config_flags "")

    foreach(_entry ${_config_data})
        string(REGEX REPLACE "(CONFIG_[^=]+)=\"?([^\"]+)\"?" "\\1;\\2" _splitted_entry ${_entry})
        list(LENGTH _splitted_entry _list_length)
        if(${_list_length} EQUAL 2)
            list(GET _splitted_entry 0 _entry_name)
            list(GET _splitted_entry 1 _entry_value)
            if(${_entry_value} STREQUAL "y")
                set(${_entry_name} ON PARENT_SCOPE)
                set(_config_flags "${_config_flags} -D${_entry_name}")
            elseif(${_entry_value} STREQUAL "n")
                set(${_entry_name} OFF PARENT_SCOPE)
            else()
                set(${_entry_name} ${_entry_value} PARENT_SCOPE)
                set(_config_flags "${_config_flags} -D${_entry_name}=${_entry_value}")
            endif()
        endif()
    endforeach()
    set(${_config_variables} "${_config_flags}" PARENT_SCOPE)
endfunction()
