#ifndef __HTTP_UUID_H__
#define __HTTP_UUID_H__ 1

#include <http_core.h>

namespace hfs
{
class http_uuid
{
public:
    typedef unsigned char uuid_t[16];

    http_uuid();
    ~http_uuid();

    /**
     * @brief Generate a new UUID.
     *
     * @return `std::string`
     */
    static std::string
    generate(const void *obj) noexcept;
};
} // namespace hfs

#endif // __HTTP_UUID_H__
