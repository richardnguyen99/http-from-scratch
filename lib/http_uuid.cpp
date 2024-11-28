#include <http_uuid.h>

namespace hfs
{
http_uuid::http_uuid()
{
}

http_uuid::~http_uuid()
{
}

std::string
http_uuid::generate(const void *obj) noexcept
{
    std::string uuid;

#ifdef HAVE_UUID_UUID_H
    (void)obj;
    uuid_t bin;
    uuid_generate(bin);

    // Convert binary UUID to string
    char str[37];
    std::memset(str, 0, sizeof(str));
    snprintf(
        str, sizeof(str),
        "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
        bin[0], bin[1], bin[2], bin[3], bin[4], bin[5], bin[6], bin[7], bin[8],
        bin[9], bin[10], bin[11], bin[12], bin[13], bin[14], bin[15]
    );

    uuid = str;
#else
    // Fallback to use object memory address and save to bin
    std::ostringstream stream;
    stream << obj;
    uuid = stream.str();
#endif

    return uuid;
}
} // namespace hfs
