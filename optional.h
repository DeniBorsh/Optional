#pragma once
#include <stdexcept>
#include <utility>

// Исключение этого типа должно генерироватся при обращении к пустому optional
class BadOptionalAccess : public std::exception {
public:
	using exception::exception;

	virtual const char* what() const noexcept override {
		return "Bad optional access";
	}
};

template <typename T>
class Optional {
public:
	Optional() = default;

	Optional(const T& value) {
		new(data_) T{ value };
		is_initialized_ = true;
	}

	Optional(T&& value) {
		new(data_) T{ std::move(value) };
		is_initialized_ = true;
	}

	Optional(const Optional& other) {
		if (other.is_initialized_) {
			new (data_) T{ *reinterpret_cast<const T*>(other.data_) };
			is_initialized_ = true;
		}
	}

	Optional(Optional&& other) {
		if (other.is_initialized_) {
			new (data_) T{ std::move(*reinterpret_cast<T*>(other.data_)) };
			is_initialized_ = true;
		}
	}


	Optional& operator=(const T& rhs) {
		Reset();
		new (data_) T{ rhs };
		is_initialized_ = true;
		return *this;
	}

	Optional& operator=(T&& rhs) {
		Reset();
		new (data_) T{ std::move(rhs) };
		is_initialized_ = true;
		return *this;
	}

	Optional& operator=(const Optional& rhs) {
		if (&rhs != this) {
			if (rhs.is_initialized_) {
				if (is_initialized_) {
					*reinterpret_cast<T*>(data_) = *reinterpret_cast<const T*>(rhs.data_);
				}
				else {
					new (data_) T(*reinterpret_cast<const T*>(rhs.data_));
					is_initialized_ = true;
				}
			}
			else {
				Reset();
			}
		}
		return *this;
	}

	Optional& operator=(Optional&& rhs) {
		if (&rhs != this) {
			if (rhs.is_initialized_) {
				if (is_initialized_) {
					*reinterpret_cast<T*>(data_) = std::move(*reinterpret_cast<T*>(rhs.data_));
				}
				else {
					new (data_) T(std::move(*reinterpret_cast<T*>(rhs.data_)));
					is_initialized_ = true;
				}
			}
			else {
				Reset();
			}
		}
		return *this;
	}

	template<typename... Types>
	void Emplace(Types&&... args) {
		Reset();
		new (data_) T{ std::forward<Types>(args)... };
		is_initialized_ = true;
	}


	~Optional() {
		if (is_initialized_) {
			reinterpret_cast<T*>(data_)->~T();
			is_initialized_ = false;
		}
	}

	bool HasValue() const { return is_initialized_; }
	operator bool() const { return is_initialized_; }

	T& operator*() { return *reinterpret_cast<T*>(data_); }
	const T& operator*() const { return *reinterpret_cast<const T*>(data_); }
	T* operator->() { return reinterpret_cast<T*>(data_); }
	const T* operator->() const { return reinterpret_cast<const T*>(data_); }

	T& Value() {
		if (!is_initialized_) throw BadOptionalAccess();
		return *reinterpret_cast<T*>(data_);
	}
	const T& Value() const {
		if (!is_initialized_) throw BadOptionalAccess();
		return *reinterpret_cast<const T*>(data_);
	}

	void Reset() {
		if (is_initialized_) {
			reinterpret_cast<T*>(data_)->~T();
			is_initialized_ = false;
		}
	}

private:
	alignas(T) char data_[sizeof(T)];
	bool is_initialized_ = false;
};