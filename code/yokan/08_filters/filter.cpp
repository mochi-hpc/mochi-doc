#include <yokan/filters.hpp>
#include <cstring>

struct CustomKeyValueFilter : public yokan::KeyValueFilter {

    std::string m_to_append;

    CustomKeyValueFilter(margo_instance_id mid,
                         int32_t mode,
                         const yokan::UserMem& data)
    : m_to_append(data.data, data.size) {
        (void)mid;
        (void)mode;
    }

    bool requiresValue() const override {
        return true;
    }

    bool check(const void* key, size_t ksize,
               const void* val, size_t vsize) const override {
        (void)key;
        (void)val;
        return vsize % 2 == ((ksize % 2 == 0) ? 1 : 0);
    }

    size_t keyCopy(
        void* dst, size_t max_dst_size,
        const void* key, size_t ksize) const override {
        if(max_dst_size < ksize) return YOKAN_SIZE_TOO_SMALL;
        for(size_t i=0; i < ksize; i++) {
            ((char*)dst)[i] = ((const char*)key)[ksize-i-1];
        }
        return ksize;
    }

    size_t valCopy(
        void* dst, size_t max_dst_size,
        const void* val, size_t vsize) const override {
        if(max_dst_size < vsize + m_to_append.size()) return YOKAN_SIZE_TOO_SMALL;
        std::memcpy(dst, val, vsize);
        std::memcpy((char*)dst+vsize, m_to_append.data(), m_to_append.size());
        return vsize + m_to_append.size();
    }
};
YOKAN_REGISTER_KV_FILTER(custom_kv, CustomKeyValueFilter);

struct CustomDocFilter : public yokan::DocFilter {

    CustomDocFilter(margo_instance_id mid, int32_t mode,
                    const yokan::UserMem& data) {
        (void)mid;
        (void)mode;
        (void)data;
    }

    bool check(yk_id_t id, const void* doc, size_t docsize) const override {
        (void)doc;
        (void)docsize;
        return id % 2 == 0;
    }
};
YOKAN_REGISTER_DOC_FILTER(custom_doc, CustomDocFilter);
