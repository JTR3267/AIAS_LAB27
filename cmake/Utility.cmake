# Copyright 2023-2025 Playlab/ACAL
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

macro(subdirlist result curdir)
    file(GLOB children RELATIVE ${curdir} ${curdir}/*)
    set(dirlist "")
    foreach(child ${children})
        if(IS_DIRECTORY ${curdir}/${child})
            set(dirlist ${dirlist} ${child})
        endif()
    endforeach()
    set(${result} ${dirlist})
endmacro(subdirlist)

macro(realpath result input)
    get_filename_component(${result} ${input} REALPATH)
endmacro(realpath)

function(add_target_compile_definitions target_name)
    foreach(definition IN LISTS ARGN)
        if(${definition})
            target_compile_definitions(${target_name} PUBLIC ${definition})
        endif()
    endforeach()
endfunction(add_target_compile_definitions)
