import SoC.fuzzer;

using namespace ::std::string_view_literals;
/// 用于统计分配的内存块大小的计数器
using block_size_counter_t = ::std::unordered_map<::std::size_t, ::std::size_t>;
/// 用于存储分配的内存块信息的向量
using allocated_memory_t = ::std::vector<::std::pair<void*, ::std::size_t>>;

namespace SoC::test
{
    extern "C++" struct heap : ::SoC::heap
    {
        using ::SoC::heap::heap;
        using ::SoC::heap::heap_full_exception_t;

        /**
         * @brief 检查堆状态是否符合预期
         *
         * @param block_size_counter 用于统计分配的内存块大小的计数器
         * @param allocated_memory 用于存储分配的内存块信息的向量
         */
        void check_heap_status(::block_size_counter_t& block_size_counter, const ::allocated_memory_t& allocated_memory)
        {
            heap_status_counter.clear();
            auto begin{metadata.begin()};
            auto end{metadata.end()};
            // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
            auto data_address{reinterpret_cast<::std::uintptr_t>(data)};
            for(auto ptr{begin}; ptr != end;)
            {
                auto&& [_, free_block_list, used_block, block_size_shift]{*ptr};
                auto max_block_num{1zu << (page_shift - block_size_shift)};
                ::SoC::assert(used_block != max_block_num || free_block_list == nullptr,
                              "页使用计数为max_block_num，但其空闲块链表不为空"sv);

                if(used_block == 0)
                {
                    if(block_size_shift == page_shift)
                    {
                        auto index{ptr - begin};
                        auto expected_free_block_list{reinterpret_cast<void*>(data_address + index * page_size)};
                        ::SoC::assert(free_block_list == expected_free_block_list, "未分块页的空闲块链表指针与预期不符"sv);
                        ::SoC::assert(free_block_list->next == nullptr, "未分块页的空闲块链表下一个指针不为空"sv);
                    }
                    ++ptr;
                    continue;
                }
                if(block_size_shift != page_shift)
                {
                    // 此时block_size_shift对应的就是真实分配的内存块大小，直接累加即可
                    heap_status_counter[1zu << block_size_shift] += used_block;
                    ++ptr;
                }
                else
                {
                    void* start_address{reinterpret_cast<void*>(data_address + (ptr - metadata.begin()) * page_size)};
                    auto result{::std::ranges::find_if(allocated_memory,
                                                       [start_address](const auto& pair) noexcept
                                                       { return pair.first == start_address; })};
                    ::SoC::assert(result != allocated_memory.end(), "未找到对应的内存块"sv);
                    auto actual_size{get_actual_allocate_size(result->second)};
                    auto continuous_pages{static_cast<::std::ptrdiff_t>(actual_size / page_size)};
                    for(auto i{0z}; i != continuous_pages; ++i)
                    {
                        auto&& [_, free_block_list, used_block, block_size_shift]{*(ptr + i)};
                        ::SoC::assert(used_block == 1, "已按页分配分配的页面中使用计数不为1"sv);
                        ::SoC::assert(block_size_shift == page_shift, "已按页分配分配的页面中块大小不为页大小"sv);
                    }
                    ptr += continuous_pages;
                    ++heap_status_counter[actual_size];
                }
            }
            // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)

            ::std::erase_if(block_size_counter, [](const auto& pair) static noexcept { return pair.second == 0; });
            ::SoC::assert(heap_status_counter == block_size_counter, "堆状态计数器与实际分配的内存块大小计数器不一致"sv);
        }

    private:
        ::block_size_counter_t heap_status_counter{};
    };
}  // namespace SoC::test

struct heap_fuzzer_param
{
    /// 堆操作类型
    enum class heap_operation : ::std::uint8_t
    {
        /// 分配内存
        alloc,
        /// 释放内存
        free
    };

    /// 最大分配大小
    constexpr inline static auto max_allocate_size{2048zu};
    /// 每次操作实际需要的字节数
    constexpr inline static auto param_size{3zu};

    /// 操作类型
    heap_operation operation{};
    /// 释放内存索引或分配大小
    ::std::uint16_t value{};

    heap_fuzzer_param(const ::std::uint8_t*& data, ::std::size_t& size) noexcept :
        operation{static_cast<heap_operation>(data[0] % 2)}
    {
        ::std::memcpy(&value, data + 1, sizeof(value));
        size -= param_size;
        data += param_size;
    }

    /**
     * @brief 获取分配大小
     *
     * @return 分配大小，范围为[1, max_allocate_size]
     * @note 只能在alloc操作时调用
     */
    [[nodiscard]] ::std::uint16_t get_alloc_size() const noexcept { return value % max_allocate_size + 1; }

    /**
     * @brief 从已分配的内存块中获取要释放的内存块对应的迭代器
     *
     * @param allocated_memory 已分配的内存块向量
     * @return 要释放的内存块对应的迭代器
     * @note 只能在free操作时调用
     */
    [[nodiscard]] allocated_memory_t::iterator get_free_iter(::allocated_memory_t& allocated_memory) const noexcept
    {
        return allocated_memory.begin() + static_cast<::std::ptrdiff_t>(value % allocated_memory.size());
    }
};

constexpr auto buffer_size{2zu * 1024 * 1024};
constexpr auto buffer_elements{buffer_size / sizeof(::std::uintptr_t)};
// NOLINTNEXTLINE(*-avoid-c-arrays, cert-err58-cpp)
const auto buffer{::std::make_unique<::std::uintptr_t[]>(buffer_elements)};

extern "C" int LLVMFuzzerTestOneInput(const ::std::uint8_t* data, ::std::size_t size)
{
    // 每次测试用例前清零buffer，确保测试用例之间无状态污染
    ::std::ranges::subrange buffer_range{buffer.get(), buffer.get() + buffer_elements};
    ::std::ranges::fill(buffer_range, 0);
    ::SoC::test::heap heap{buffer_range.begin(), buffer_range.end()};
    ::allocated_memory_t allocated_memory{};
    ::block_size_counter_t block_size_counter{};

    while(size >= ::heap_fuzzer_param::param_size)
    {
        ::heap_fuzzer_param param{data, size};
        switch(param.operation)
        {
            case ::heap_fuzzer_param::heap_operation::alloc:
            {
                auto alloc_size{param.get_alloc_size()};
                try
                {
                    void* ptr{heap.allocate(alloc_size)};
                    ::SoC::assert(ptr != nullptr, "分配内存失败但未通过异常路径退出"sv);
                    allocated_memory.emplace_back(ptr, alloc_size);
                    ++block_size_counter[heap.get_actual_allocate_size(alloc_size)];
                }
                catch(const ::SoC::test::heap::heap_full_exception_t&)
                {
                    // 堆空间不足，在正常模式下已经快速失败
                    // 在fuzzer模式下退出该测试用例，避免堆结构被破坏
                    return 0;
                }
                break;
            }
            case ::heap_fuzzer_param::heap_operation::free:
            {
                // 没有分配内存，继续下一个操作
                if(allocated_memory.empty()) { continue; }

                auto iter{param.get_free_iter(allocated_memory)};
                auto&& [ptr, allocated_size]{*iter};
                heap.deallocate(ptr, allocated_size);
                --block_size_counter[heap.get_actual_allocate_size(allocated_size)];
                allocated_memory.erase(iter);
                break;
            }
        }
        heap.check_heap_status(block_size_counter, allocated_memory);
    }
    return 0;
}
