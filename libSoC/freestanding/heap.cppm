/**
 * @file heap.cppm
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief 独立的堆分配器接口定义
 */

module;
#ifdef SOC_IN_UNIT_TEST
    #define USE_VIRTUAL virtual
#else
    #define USE_VIRTUAL
#endif
export module SoC.freestanding:heap;
import :allocator;

export namespace SoC
{
    namespace detail
    {
        /**
         * @brief 空闲块链表
         *
         */
        struct free_block_list_t
        {
            // 下一个空闲块，页操作时不使用该变量，块操作时使用
            ::SoC::detail::free_block_list_t* next;
        };

        /**
         * @brief 堆页元数据
         *
         */
        struct heap_page_metadata
        {
            // 下一个空闲页的元数据指针
            ::SoC::detail::heap_page_metadata* next_page;
            // 页内空闲块链表的头指针
            ::SoC::detail::free_block_list_t* free_block_list;
            // 已使用块的数量
            ::std::uint16_t used_block;
            // 块大小的左移量
            ::std::uint16_t block_size_shift;
        };
    }  // namespace detail

    namespace test
    {
        /// @see ::SoC::heap
        extern "C++" struct heap;
    }  // namespace test

    /**
     * @brief 基于空闲链表和slab的堆
     *
     */
    struct heap
    {
    private:
        /// 测试接口
        friend struct ::SoC::test::heap;

        /// 元数据区
        ::std::span<::SoC::detail::heap_page_metadata> metadata;

        /// 最小块大小的左移量
        constexpr inline static auto min_block_shift{4zu};

        /// 页大小的左移量
        constexpr inline static auto page_shift{9zu};

        /// 块大小总数
        constexpr inline static auto block_size_cnt{page_shift - min_block_shift + 1};

        using free_list_t = ::std::array<::SoC::detail::heap_page_metadata*, block_size_cnt>;

        /// 空闲链表
        free_list_t free_page_list{};

        /// 数据区
        ::SoC::detail::free_block_list_t* data;

        /**
         * @brief 寻找一个空闲页并在其中划分出内存块，将其移除空闲页链表
         *
         * @param free_list_index 空闲链表索引
         * @return 页起始地址
         */
        USE_VIRTUAL ::SoC::detail::free_block_list_t*
            make_block_in_page(::std::size_t free_list_index) noexcept(::SoC::optional_noexcept);

        /**
         * @brief 将已分块的页从块空闲链表头部删除，插入页空闲链表头部
         *
         * @param page 页元数据指针
         * @return 下一个元数据的指针
         */
        USE_VIRTUAL ::SoC::detail::heap_page_metadata*
            insert_block_into_page_list(::SoC::detail::heap_page_metadata* page_metadata) noexcept;

        /**
         * @brief 从空闲的已分块页中回收页到空闲页链表
         *
         * @param assert 是否断言空闲页链表非空
         * @return 空闲页链表首指针
         */
        [[using gnu: noinline, cold]] USE_VIRTUAL ::SoC::detail::heap_page_metadata*
            page_gc(bool assert) noexcept(::SoC::optional_noexcept);

        /**
         * @brief 获取页内指针所在页对应的元数据数组索引
         *
         * @param page_ptr 页内指针
         * @return 元数据数组索引
         */
        [[using gnu: always_inline, hot]] USE_VIRTUAL inline ::std::ptrdiff_t
            get_metadata_index(::SoC::detail::free_block_list_t* page_ptr) const noexcept(::SoC::optional_noexcept)
        {
            constexpr ::std::string_view message{"页指针超出当前堆范围"};
            if constexpr(::SoC::use_full_assert) { ::SoC::assert(page_ptr >= data, message); }
            auto page_index{static_cast<::std::ptrdiff_t>((page_ptr - data) * ptr_size / page_size)};
            auto max_page_index{static_cast<::std::ptrdiff_t>(metadata.size())};
            if constexpr(::SoC::use_full_assert) { ::SoC::assert(page_index < max_page_index, message); }
            return page_index;
        }

        /**
         * @brief 从空闲页链表中删除范围[range_begin, range_end]内的页
         *
         * @param range_begin 页范围首指针
         * @param range_end 页范围尾指针
         * @return void* 页范围首指针
         */
        USE_VIRTUAL void* remove_pages(::SoC::detail::free_block_list_t* range_begin,
                                       ::SoC::detail::free_block_list_t* range_end) noexcept;

        /**
         * @brief 分配一个或多个，慢速路径
         *
         * @param page_cnt 要操作的页数量
         */
        USE_VIRTUAL void* allocate_pages(::std::size_t page_cnt) noexcept(::SoC::optional_noexcept);

        /**
         * @brief 释放一个或多个，慢速路径
         *
         * @param ptr 块指针
         * @param actual_size 要释放的大小
         */
        [[using gnu: noinline, cold]] USE_VIRTUAL void
            deallocate_pages(void* ptr, ::std::size_t actual_size) noexcept(::SoC::optional_noexcept);

        /**
         * @brief 分配冷路径
         *
         * @param actual_size 要分配的大小
         */
        [[using gnu: noinline, cold]] USE_VIRTUAL void*
            allocate_cold_path(::std::size_t actual_size) noexcept(::SoC::optional_noexcept);

        /// 指针大小
        constexpr inline static auto ptr_size{sizeof(void*)};

#ifdef SOC_BUILD_MODE_FUZZER
        /**
         * @brief 堆已满异常
         *
         */
        struct heap_full_exception_t : ::std::runtime_error
        {
            inline heap_full_exception_t() : ::std::runtime_error{"堆已满"} {}
        };

        /**
         * @brief 抛出堆已满异常
         *
         * @note 只在fuzzer模式下起作用
         */
        [[noreturn]] inline static void throw_heap_full_exception() { throw heap_full_exception_t{}; }
#else
        /**
         * @brief 抛出堆已满异常
         *
         * @note 只在fuzzer模式下起作用
         */
        constexpr inline static void throw_heap_full_exception() noexcept {}
#endif

    public:
        /// 堆页大小
        constexpr inline static auto page_size{1zu << page_shift};

        /// 最小块大小
        constexpr inline static auto min_block_size{1zu << min_block_shift};

        /**
         * @brief 初始化堆
         *
         * @param begin 堆起始地址
         * @param end 堆结束地址
         * @note end必须对齐到页边界
         */
        explicit heap(::std::uintptr_t* begin, ::std::uintptr_t* end) noexcept(::SoC::optional_noexcept);

        USE_VIRTUAL inline ~heap() noexcept
        {
            // 防止覆盖率计算时，析构函数被优化掉
            if constexpr(::SoC::is_build_mode(::SoC::build_mode::coverage))
            {
                ::std::atomic_signal_fence(::std::memory_order_relaxed);
            }
        }

        inline heap(const heap&) noexcept = delete;
        inline heap& operator= (const heap&) = delete;
        inline heap(heap&&) noexcept = delete;
        inline heap& operator= (heap&&) noexcept = delete;

        /**
         * @brief 获取实际分配的大小
         *
         * @param size 需要分配的大小
         * @return 实际分配的大小
         */
        [[using gnu: always_inline, artificial, hot]] constexpr inline static ::std::size_t
            get_actual_allocate_size(::std::size_t size) noexcept
        {
            auto ceiled_size{::std::max(::std::bit_ceil(size), min_block_size)};
            // 不超过页大小时对齐到下一个2^N块边界
            // 超过页大小时对齐到下一个页边界
            return size > page_size ? (size + page_size - 1) / page_size * page_size : ceiled_size;
        }

        /**
         * @brief 获取当前堆的总页数
         *
         * @return 总页数
         */
        [[nodiscard]] USE_VIRTUAL inline ::std::size_t get_total_pages() const noexcept { return metadata.size(); }

        /**
         * @brief 获取当前堆中空闲页数，不论是否分块
         *
         * @return 空闲页数
         */
        [[nodiscard]] USE_VIRTUAL ::std::size_t get_free_pages() const noexcept;

        /**
         * @brief 获取当前堆中正在使用的页数，不论是否分块
         *
         * @return 正在使用的页数
         */
        [[nodiscard]] USE_VIRTUAL inline ::std::size_t get_using_pages() const noexcept
        {
            return get_total_pages() - get_free_pages();
        }

        /**
         * @brief 分配指定大小的块
         *
         * @param size 块大小
         * @return void* 块起始地址
         */
        [[nodiscard]] USE_VIRTUAL void* allocate(::std::size_t size) noexcept(::SoC::optional_noexcept);

        /**
         * @brief 释放指定块
         *
         * @param ptr 块起始地址
         * @param size 块大小
         */
        USE_VIRTUAL void deallocate(void* ptr, ::std::size_t size) noexcept(::SoC::optional_noexcept);
    };

    namespace detail
    {
        /**
         * @brief 判断类型type是否可以通过堆进行知类型分配，要求满足:
         * - type不为void
         * - type的对齐不超过堆的页大小
         * @tparam type 要判断的类型
         */
        template <typename type>
        concept is_known_type_allocatable = !::std::is_void_v<type> && alignof(type) <= ::SoC::heap::page_size;

        /**
         * @brief 堆分配器实现，通过CRTP使用，要求满足：
         * - static SoC::heap* wrapper::heap
         * @tparam wrapper 堆分配器类型
         */
        template <typename wrapper>
        struct heap_allocator_impl
        {
        private:
            constexpr inline heap_allocator_impl() noexcept = default;
            friend wrapper;

        public:
            /**
             * @brief 分配至少size个字节
             *
             * @param size 要分配的字节数
             * @return 内存区域首指针
             */
            inline static void* allocate(::std::size_t size) noexcept(::SoC::optional_noexcept)
            {
                return wrapper::heap->allocate(size);
            }

            /**
             * @brief 分配一个type类型对象所需的空间
             *
             * @tparam type 要分配的类型
             * @return 内存区域首指针
             */
            template <::SoC::detail::is_known_type_allocatable type>
            inline static type* allocate() noexcept(::SoC::optional_noexcept)
            {
                constexpr auto size{::std::max(sizeof(type), alignof(type))};
                return static_cast<type*>(wrapper::heap->allocate(size));
            }

            /**
             * @brief 分配至少连续n个type类型对象所需的空间
             *
             * @tparam type 要分配的类型
             * @param n 要分配的对象个数
             * @return SoC::allocation_result<type*> 内存区域首指针和实际可容纳对象数
             */
            template <::SoC::detail::is_known_type_allocatable type>
            inline static ::SoC::allocation_result<type*> allocate(::std::size_t n) noexcept(::SoC::optional_noexcept)
            {
                // sizeof(type) >= alignof(type)，天然保证对齐
                constexpr auto size{sizeof(type)};
                auto total_size{size * n};
                // 获取实际分配的大小
                auto actual_size{::SoC::heap::get_actual_allocate_size(total_size)};
                return ::SoC::allocation_result<type*>{static_cast<type*>(wrapper::heap->allocate(total_size)),
                                                       actual_size / size};
            }

            /**
             * @brief 释放n个type类型对象占用的空间
             *
             * @tparam type 要释放的类型
             * @param ptr 内存区域首指针
             * @param n 要释放的对象个数
             */
            template <::SoC::detail::is_known_type_allocatable type>
            inline static void deallocate(type* ptr, ::std::size_t n = 1) noexcept(::SoC::optional_noexcept)
            {
                wrapper::heap->deallocate(ptr, sizeof(type) * n);
            }

            /**
             * @brief 释放ptr起连续size个字节的内存区域
             *
             * @param ptr 内存区域首指针
             * @param size 要释放的字节数，需要和分配时保持一致
             */
            inline static void deallocate(void* ptr, ::std::size_t size) noexcept(::SoC::optional_noexcept)
            {
                wrapper::heap->deallocate(ptr, size);
            }

            /**
             * @brief 比较两个分配器对象是否相同
             *
             * @param self 当前分配器
             * @param other 其他分配器
             * @return 分配器对象是否相同
             */
            constexpr inline bool operator== (this auto self [[maybe_unused]], wrapper other [[maybe_unused]]) noexcept
            {
                return true;
            }

            /**
             * @brief 将堆对象绑定到分配器
             *
             * @param heap_ref 堆对象引用
             */
            inline static void set_heap(::SoC::heap& heap_ref) noexcept { wrapper::heap = &heap_ref; }
        };
    }  // namespace detail

    /**
     * @brief 适配主内存堆的全局分配器
     *
     */
    struct ram_heap_allocator_t : ::SoC::detail::heap_allocator_impl<::SoC::ram_heap_allocator_t>
    {
    private:
        constinit inline static ::SoC::heap* heap{};
        using base_t = ::SoC::detail::heap_allocator_impl<::SoC::ram_heap_allocator_t>;
        friend base_t;

    public:
        using base_t::base_t;
    } inline constexpr ram_allocator{};

    /**
     * @brief 适配ccmram堆的全局分配器
     *
     */
    struct ccmram_heap_allocator_t : ::SoC::detail::heap_allocator_impl<::SoC::ccmram_heap_allocator_t>
    {
    private:
        constinit inline static ::SoC::heap* heap{};
        using base_t = ::SoC::detail::heap_allocator_impl<::SoC::ccmram_heap_allocator_t>;
        friend base_t;

    public:
        using base_t::base_t;
    } inline constexpr ccmram_allocator{};
}  // namespace SoC

export namespace SoC
{
    /**
     * @brief 智能指针，未支持数组形式
     *
     * @tparam type 指向的类型
     * @tparam allocator_t 分配器类型
     */
    template <typename type, typename allocator_t = ::SoC::ram_heap_allocator_t>
    struct unique_ptr
    {
        using value_type = type;
        using pointer = type*;
        using const_pointer = const type*;
        using reference = type&;
        using const_reference = const type&;
        using allocator = allocator_t;

    private:
        pointer ptr;
        [[no_unique_address]] allocator alloc;

    public:
        /**
         * @brief 默认构造空的智能指针
         *
         */
        constexpr inline explicit unique_ptr(allocator alloc = ::SoC::ram_allocator) noexcept : ptr{nullptr}, alloc{alloc} {}

        constexpr inline explicit unique_ptr(pointer ptr, allocator alloc = ::SoC::ram_allocator) noexcept :
            ptr{ptr}, alloc{alloc}
        {
        }

        inline unique_ptr(const unique_ptr&) noexcept = delete;
        inline unique_ptr& operator= (const unique_ptr&) noexcept = delete;

        constexpr inline unique_ptr(unique_ptr&& other) noexcept : ptr{other.ptr}, alloc{::std::move(other.alloc)}
        {
            other.ptr = nullptr;
        }

        constexpr inline unique_ptr& operator= (unique_ptr&& other) noexcept
        {
            unique_ptr temp{::std::move(other)};
            ::std::ranges::swap(*this, temp);
            return *this;
        }

        constexpr inline unique_ptr& operator= (pointer ptr) noexcept
        {
            release();
            this->ptr = ptr;
            return *this;
        }

        constexpr inline ~unique_ptr() noexcept { release(); }

        /**
         * @brief 释放智能指针指向的对象
         *
         */
        constexpr inline void release() noexcept(::SoC::optional_noexcept)
        {
            if(ptr != nullptr)
            {
                ptr->~type();
                alloc.deallocate(ptr);
                ptr = nullptr;
            }
        }

        constexpr inline auto&& operator* (this auto&& self) noexcept { return *self.ptr; }

        constexpr inline auto operator->(this auto&& self) noexcept
        {
            using self_t = decltype(self);
            if constexpr(::std::is_const_v<::std::remove_reference_t<self_t>>) { return const_pointer{self.ptr}; }
            else
            {
                return pointer{self.ptr};
            }
        }

        constexpr inline operator pointer() noexcept { return ptr; }

        constexpr inline operator const_pointer() const noexcept { return ptr; }

        constexpr inline explicit operator bool() const noexcept { return ptr != nullptr; }
    };
}  // namespace SoC
