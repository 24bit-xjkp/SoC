export module SoC.unit_test.heap;
export import SoC.unit_test;
import "test_framework.hpp";

export namespace SoC::test
{
    /**
     * @brief 导出SoC::heap中的符号用于测试
     *
     */
    extern "C++" struct heap : ::SoC::heap
    {
        using ::SoC::heap::allocate_cold_path;
        using ::SoC::heap::allocate_pages;
        using ::SoC::heap::block_size_cnt;
        using ::SoC::heap::data;
        using ::SoC::heap::deallocate_pages;
        using ::SoC::heap::free_list_t;
        using ::SoC::heap::free_page_list;
        using ::SoC::heap::get_metadata_index;
        using ::SoC::heap::heap;
        using ::SoC::heap::insert_block_into_page_list;
        using ::SoC::heap::make_block_in_page;
        using ::SoC::heap::metadata;
        using ::SoC::heap::min_block_shift;
        using ::SoC::heap::page_gc;
        using ::SoC::heap::page_shift;
        using ::SoC::heap::ptr_size;
        using ::SoC::heap::remove_pages;
    };
}  // namespace SoC::test

export namespace SoC::unit_test::heap
{
    /// 测试使用堆空间的大小
    constexpr auto heap_size{128 * 1024zu};

    /**
     * @brief 堆测试用例的夹具，用于提供共用内存
     *
     */
    struct test_fixture
    {
    private:
        // NOLINTNEXTLINE(*-avoid-c-arrays)
        inline static constinit ::std::unique_ptr<::std::uintptr_t[]> origin_ptr{};
        /// 内存区域[begin, end)，已对齐到页大小
        inline static constinit ::std::uintptr_t* begin{};
        inline static constinit ::std::uintptr_t* end{};

        /**
         * @brief 分配共用内存，避免测试用例间重复分配
         *
         */
        static void allocate_once()
        {
            if(begin == nullptr) [[unlikely]]
            {
                auto size{256 * 1024zu};
                // NOLINTNEXTLINE(*-avoid-c-arrays)
                origin_ptr = ::std::make_unique<::std::uintptr_t[]>(size / sizeof(::std::uintptr_t));
                void* ptr{origin_ptr.get()};
                REQUIRE_NE(ptr, nullptr);
                REQUIRE_NE(::std::align(::SoC::heap::page_size, ::SoC::unit_test::heap::heap_size, ptr, size), nullptr);
                begin = static_cast<::std::uintptr_t*>(ptr);
                end = begin + (::SoC::unit_test::heap::heap_size / sizeof(::std::uintptr_t));
            }
        }

    public:
        /**
         * @brief 获取内存区域
         *
         * @return 内存区域[begin, end)
         */
        static ::std::pair<::std::uintptr_t*, ::std::uintptr_t*> get_memory()
        {
            allocate_once();
            return ::std::pair{begin, end};
        }

        /**
         * @brief 获取堆对象
         *
         * @return 堆对象
         */
        static ::SoC::test::heap get_heap()
        {
            allocate_once();
            return ::SoC::test::heap{begin, end};
        }
    };

    using metadata_t = ::std::remove_reference_t<decltype(::SoC::test::heap::metadata.front())>;
    using free_block_list_t = ::std::remove_pointer_t<decltype(::SoC::unit_test::heap::metadata_t::free_block_list)>;
}  // namespace SoC::unit_test::heap
