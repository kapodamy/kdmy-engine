#ifndef _cloneutils_h
#define _cloneutils_h


#define CLONE_ARRAY(STRUCT_TYPE, STRUCT_TYPEDEF, PTR_ARRAY, ELEMENTS_COUNT)                \
    ({                                                                                     \
        STRUCT_TYPEDEF SRC_ARRAY##_temp_copy;                                              \
        if (PTR_ARRAY != NULL && ELEMENTS_COUNT > 0) {                                     \
            size_t SRC_ARRAY##_temp_length = sizeof(STRUCT_TYPE) * (size_t)ELEMENTS_COUNT; \
            SRC_ARRAY##_temp_copy = malloc_chk(SRC_ARRAY##_temp_length);                   \
            malloc_assert(SRC_ARRAY##_temp_copy, STRUCT_TYPEDEF[]);                        \
            memcpy(SRC_ARRAY##_temp_copy, PTR_ARRAY, SRC_ARRAY##_temp_length);             \
        } else {                                                                           \
            SRC_ARRAY##_temp_copy = NULL;                                                  \
        }                                                                                  \
                                                                                           \
        SRC_ARRAY##_temp_copy;                                                             \
    })


#define CLONE_STRUCT_ARRAY(STRUCT_TYPE, PTR_ARRAY, ELEMENTS_COUNT)                         \
    ({                                                                                     \
        STRUCT_TYPE* SRC_ARRAY##_temp_copy;                                                \
        if (PTR_ARRAY != NULL && ELEMENTS_COUNT > 0) {                                     \
            size_t SRC_ARRAY##_temp_length = sizeof(STRUCT_TYPE) * (size_t)ELEMENTS_COUNT; \
            SRC_ARRAY##_temp_copy = malloc_chk(SRC_ARRAY##_temp_length);                   \
            malloc_assert(SRC_ARRAY##_temp_copy, STRUCT_TYPE[]);                           \
            memcpy(SRC_ARRAY##_temp_copy, PTR_ARRAY, SRC_ARRAY##_temp_length);             \
        } else {                                                                           \
            SRC_ARRAY##_temp_copy = NULL;                                                  \
        }                                                                                  \
                                                                                           \
        SRC_ARRAY##_temp_copy;                                                             \
    })


#define CLONE_STRUCT(STRUCT_TYPE, STRUCT_TYPEDEF, PTR)         \
    ({                                                         \
        STRUCT_TYPEDEF PTR##_temp_copy;                        \
        if (PTR) {                                             \
            PTR##_temp_copy = malloc_chk(sizeof(STRUCT_TYPE)); \
            malloc_assert(PTR##_temp_copy, STRUCT_TYPEDEF);    \
            memcpy(PTR##_temp_copy, PTR, sizeof(STRUCT_TYPE)); \
        } else {                                               \
            PTR##_temp_copy = NULL;                            \
        }                                                      \
                                                               \
        PTR##_temp_copy;                                       \
    })


#endif
