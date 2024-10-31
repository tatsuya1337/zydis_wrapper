#pragma once

namespace zydis {

    class instructions_iterator {
    public:
        explicit instructions_iterator(uintptr_t address = 0);

        const single_instruction& operator*() const noexcept;
        const single_instruction* operator->() const noexcept;

        instructions_iterator& operator++();
        instructions_iterator operator++(int);

        bool operator==(const instructions_iterator& other) const noexcept;
        bool operator!=(const instructions_iterator& other) const noexcept;

    private:
        void decode();

        uintptr_t address_ = 0;
        ZyanStatus status_ = ZYAN_STATUS_SUCCESS;
        single_instruction instruction_{};
        size_t count_ = 0;
        size_t skip_until_ = 0;
    };

    class instructions {
    public:
        template <typename T>
        explicit instructions(T addr) requires ((std::is_integral_v<T> && sizeof(T) == sizeof(uintptr_t)) || std::is_pointer_v<T>)
        {
            using CastType = std::conditional_t<std::is_pointer_v<T>, uintptr_t, T>;

            if constexpr (std::is_pointer_v<T>) {
                start_address_ = reinterpret_cast<uintptr_t>(addr);
            }
            else {
                start_address_ = static_cast<uintptr_t>(addr);
            }
        }

        instructions_iterator begin() const;
        instructions_iterator end() const;

    private:
        uintptr_t start_address_ = 0;
    };

} 
