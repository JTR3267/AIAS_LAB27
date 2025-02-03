#!/usr/bin/env python3

# Copyright 2023-2024 Playlab/ACAL
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

from typing import Any, Dict, List

# Constants
"""
PROJ_TEMPLATE = {
    "name": "",
    "log-name": "",
    "src-subdir": "",
    "exec-args": [],            # element-type: str
    "compile-mode": "Debug",    # options: Debug, Release, GTest
    "pre-steps": [],            # element-type: List[str]
    "post-steps": []            # element-type: List[str]
    "total-tick": 0				# (optional) element-type: int
    "total-tick-range": [0, 10]	# (optional) expected [min, max] ticks, element-type: List[int]
}
"""
PROJ_ARR: List[Dict[str, Any]] = []
