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

#include "Register.hh"

template <typename T>
Register<T>::Register()
    : state_(OutEntry::PING), is_stall_(false), is_flush_(false), ping_entry_(nullptr), pong_entry_(nullptr) {}

template <typename T>
Register<T>::Register(const std::shared_ptr<T>& val)
    : state_(OutEntry::PING), is_stall_(false), is_flush_(false), ping_entry_(val), pong_entry_(val) {}

template <typename T>
std::shared_ptr<T> Register<T>::get() const {
	return (this->state_ == OutEntry::PING) ? this->ping_entry_ : this->pong_entry_;
}

template <typename T>
void Register<T>::set(const std::shared_ptr<T>& val) {
	if (this->state_ == OutEntry::PING) {
		this->pong_entry_ = val;
	} else {
		this->ping_entry_ = val;
	}
}

template <typename T>
void Register<T>::setStall() {
	this->is_stall_ = true;
}

template <typename T>
void Register<T>::setFlush() {
	this->is_flush_ = true;
}

template <typename T>
void Register<T>::clearStall() {
	this->is_stall_ = false;
}

template <typename T>
void Register<T>::clearFlush() {
	this->is_flush_ = false;
}

template <typename T>
void Register<T>::update() {
	if (!this->is_stall_) { this->state_ = (this->state_ == OutEntry::PING) ? OutEntry::PONG : OutEntry::PING; }
	if (this->is_flush_) {
		this->state_ = (this->state_ == OutEntry::PING) ? OutEntry::PONG : OutEntry::PING;
		if (this->state_ == OutEntry::PING) {
			this->ping_entry_.reset();
		} else {
			this->pong_entry_.reset();
		}
	}

	this->clearStall();
	this->clearFlush();
}

template <typename T>
void Register<T>::update(TraceCallback cb) {
	if (!this->is_stall_ || this->is_flush_) {
		this->state_ = (this->state_ == OutEntry::PING) ? OutEntry::PONG : OutEntry::PING;
	}
	if (this->is_flush_) {
		if (this->state_ == OutEntry::PING) {
			this->ping_entry_.reset();
		} else {
			this->pong_entry_.reset();
		}
	}
	cb();

	this->clearStall();
	this->clearFlush();
}