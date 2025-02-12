/*
 * Copyright 2023-2024 Playlab/ACAL
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SRC_APP_SOC_INCLUDE_REGISTER_HH_
#define SRC_APP_SOC_INCLUDE_REGISTER_HH_

#include <memory>

template <typename T>
class Register {
public:
	Register();
	Register(const std::shared_ptr<T>& val);

	std::shared_ptr<T> get() const;

	void set(const std::shared_ptr<T>& val);

	void setStall();
	void setFlush();

	void update();

	using TraceCallback = std::function<void(void)>;
	void update(TraceCallback cb);

private:
	enum class OutEntry { PING, PONG };

	OutEntry           state_;
	bool               is_stall_;
	bool               is_flush_;
	std::shared_ptr<T> ping_entry_;
	std::shared_ptr<T> pong_entry_;
};

#include "Register.inl"

#endif  // SRC_APP_SOC_INCLUDE_REGISTER_HH_
