#define VER_MAJOR     "0"
#define VER_MINOR     "0"
#define VER_REVISION  "2"

#define VER_NUMERIC   000002L



const char* kv_version()
{
    return VER_MAJOR "." VER_MINOR "." VER_REVISION;
}

long kv_version_numeric()
{
    return VER_NUMERIC;
}
