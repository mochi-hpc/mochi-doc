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
        // This custom filter will check if the key size and
        // value size have the same parity
        (void)key;
        (void)val;
        return (vsize % 2) == (ksize % 2);
    }

    size_t keySizeFrom(const void* key, size_t ksize) const override {
        (void)key;
        return ksize;
    }

    size_t valSizeFrom(
        const void* val, size_t vsize) const override {
        (void)val;
        return vsize + m_to_append.size();
    }

    size_t keyCopy(
        void* dst, size_t max_dst_size,
        const void* key, size_t ksize) const override {
        // This custom copy function will reverse the key
        if(max_dst_size < ksize) return YOKAN_SIZE_TOO_SMALL;
        for(size_t i=0; i < ksize; i++) {
            ((char*)dst)[i] = ((const char*)key)[ksize-i-1];
        }
        return ksize;
    }

    size_t valCopy(
        void* dst, size_t max_dst_size,
        const void* val, size_t vsize) const override {
        // This custom copy function will append
        // the user-provided filter argument to the value
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

    bool check(const char* collection, yk_id_t id, const void* doc, size_t docsize) const override {
        // This custom filter will only let through the document with an even id
        (void)collection;
        (void)doc;
        (void)docsize;
        return id % 2 == 0;
    }

    size_t docSizeFrom(const char* collection, const void* val, size_t vsize) const override {
        (void)collection;
        (void)val;
        return vsize;
    }

    /**
     * @brief Copy the document to the target destination. This copy may
     * be implemented differently depending on the mode, and may alter
     * the content of the document.
     * This function should return the size actually copied.
     */
    virtual size_t docCopy(
        const char* collection,
        void* dst, size_t max_dst_size,
        const void* val, size_t vsize) const {
        vsize = std::min(vsize, max_dst_size);
        std::memcpy(dst, val, vsize);
        return vsize;
    }
};
YOKAN_REGISTER_DOC_FILTER(custom_doc, CustomDocFilter);
