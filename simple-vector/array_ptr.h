#pragma once

#include <cassert>
#include <cstdlib>

template <typename Type>
class ArrayPtr {
public:
    // �������������� ArrayPtr ������� ����������
    ArrayPtr() = default;

    // ������ � ���� ������ �� size ��������� ���� Type.
    // ���� size == 0, ���� raw_ptr_ ������ ���� ����� nullptr
    explicit ArrayPtr(size_t size) {
        if (size != 0) {
            raw_ptr_ = new Type[size]{};
        }
        else {
            raw_ptr_ = nullptr;
        }
    }

    // ����������� �� ������ ���������, ��������� ����� ������� � ���� ���� nullptr
    explicit ArrayPtr(Type* raw_ptr) noexcept : raw_ptr_(raw_ptr) { }


    // ��������� �����������
    ArrayPtr(const ArrayPtr&) = delete;

    ArrayPtr(ArrayPtr&& other) noexcept : ArrayPtr() {
        swap(other);
    }

    ~ArrayPtr() {
        delete[] raw_ptr_;
    }

    // ��������� ������������
    ArrayPtr& operator=(const ArrayPtr&) = delete;

    ArrayPtr& operator=(ArrayPtr&& rhs) noexcept {
        if (this != &rhs)
        {
            delete raw_ptr_;
            ArrayPtr tmp(std::move(rhs));
            swap(tmp);
        }
        return *this;
    }

    // ���������� ��������� �������� � ������, ���������� �������� ������ �������
    // ����� ������ ������ ��������� �� ������ ������ ����������
    [[nodiscard]] Type* Release() noexcept {
        Type* ptr = raw_ptr_;
        raw_ptr_ = nullptr;
        return ptr;
    }

    // ���������� ������ �� ������� ������� � �������� index
    Type& operator[](size_t index) noexcept {
        return *(raw_ptr_ + index);
    }

    // ���������� ����������� ������ �� ������� ������� � �������� index
    const Type& operator[](size_t index) const noexcept {
        return *(raw_ptr_ + index);
    }

    // ���������� true, ���� ��������� ���������, � false � ��������� ������
    explicit operator bool() const {
        return raw_ptr_ != nullptr;
    }

    // ���������� �������� ������ ���������, ��������� ����� ������ �������
    Type* Get() const noexcept {
        return raw_ptr_;
    }

    // ������������ ��������� ��������� �� ������ � �������� other
    void swap(ArrayPtr& other) noexcept {
        Type* ptr;
        std::swap(ptr, raw_ptr_);
        std::swap(raw_ptr_, other.raw_ptr_);
        std::swap(other.raw_ptr_, ptr);
    }

private:
    Type* raw_ptr_ = nullptr;
};
