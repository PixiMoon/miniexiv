#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <filesystem>
#include "miniexiv/miniexiv.h"


static void expect_ok(int result)
{
    if (result != MINIEXIV_OK)
    {
        std::cerr
            << "ERROR: "
            << miniexiv_get_last_error()
            << "\n";

        assert(false);
    }
}


static std::vector<uint8_t> load_file(const char* name)
{
    std::cout << "Current dir: "
              << std::filesystem::current_path()
              << "\n";

    std::cout << "Trying file: "
              << name
              << "\n";

    std::ifstream file(name, std::ios::binary);

    assert(file);

    return {
        std::istreambuf_iterator<char>(file),
        {}
    };
}


static void test_version()
{
    std::cout << "[Version]\n";

    assert(miniexiv_version());

    assert(miniexiv_version_major() >= 0);
    assert(miniexiv_version_minor() >= 0);
    assert(miniexiv_version_patch() >= 0);
}


static miniexiv_image* open_image()
{
    auto img =
        miniexiv_image_open_file("test.jpg");

    assert(img);

    return img;
}



static void test_exif(miniexiv_image* img)
{
    std::cout << "[EXIF]\n";


    expect_ok(
        miniexiv_image_set_exif_string(
            img,
            "Exif.Image.Artist",
            "MiniExiv"
        )
    );


    assert(
        miniexiv_image_has_exif_key(
            img,
            "Exif.Image.Artist"
        )
        == MINIEXIV_TRUE
    );


    char* value=nullptr;


    expect_ok(
        miniexiv_image_get_exif_string(
            img,
            "Exif.Image.Artist",
            &value
        )
    );


    assert(value);

    assert(
        strcmp(value,"MiniExiv")==0
    );


    miniexiv_free_string(value);



    expect_ok(
        miniexiv_image_remove_exif(
            img,
            "Exif.Image.Artist"
        )
    );


    assert(
        miniexiv_image_has_exif_key(
            img,
            "Exif.Image.Artist"
        )
        == MINIEXIV_FALSE
    );
}



static void test_xmp(miniexiv_image* img)
{
    std::cout << "[XMP]\n";


    expect_ok(
        miniexiv_image_set_xmp_string(
            img,
            "Xmp.dc.creator",
            "MiniExiv"
        )
    );


    assert(
        miniexiv_image_has_xmp_key(
            img,
            "Xmp.dc.creator"
        )
        == MINIEXIV_TRUE
    );


    char* value=nullptr;


    expect_ok(
        miniexiv_image_get_xmp_string(
            img,
            "Xmp.dc.creator",
            &value
        )
    );


    assert(value);


    miniexiv_free_string(value);



    expect_ok(
        miniexiv_image_remove_xmp(
            img,
            "Xmp.dc.creator"
        )
    );
}



static void test_iptc(miniexiv_image* img)
{
    std::cout << "[IPTC]\n";


    expect_ok(
        miniexiv_image_set_iptc_string(
            img,
            "Iptc.Application2.Caption",
            "MiniExiv"
        )
    );


    assert(
        miniexiv_image_has_iptc_key(
            img,
            "Iptc.Application2.Caption"
        )
        == MINIEXIV_TRUE
    );


    char* value=nullptr;


    expect_ok(
        miniexiv_image_get_iptc_string(
            img,
            "Iptc.Application2.Caption",
            &value
        )
    );


    assert(value);


    miniexiv_free_string(value);



    expect_ok(
        miniexiv_image_remove_iptc(
            img,
            "Iptc.Application2.Caption"
        )
    );
}



static void test_dimensions(miniexiv_image* img)
{
    std::cout<<"[Dimensions]\n";


    uint32_t w=0;
    uint32_t h=0;


    expect_ok(
        miniexiv_image_get_dimensions(
            img,
            &w,
            &h
        )
    );


    assert(w>0);
    assert(h>0);
}



static void test_iterators(miniexiv_image* img)
{
    std::cout<<"[Iterators]\n";


    auto exif =
        miniexiv_exif_iterator_create(img);

    assert(exif);


    while(
        miniexiv_exif_iterator_next(exif)
        ==
        MINIEXIV_ITERATOR_HAS_NEXT
    )
    {
        assert(
            miniexiv_exif_iterator_get_key(exif)
        );

        assert(
            miniexiv_exif_iterator_get_value(exif)
        );
    }


    miniexiv_exif_iterator_free(exif);



    auto xmp =
        miniexiv_xmp_iterator_create(img);

    assert(xmp);


    while(
        miniexiv_xmp_iterator_next(xmp)
        ==
        MINIEXIV_ITERATOR_HAS_NEXT
    )
    {
        assert(
            miniexiv_xmp_iterator_get_key(xmp)
        );
    }


    miniexiv_xmp_iterator_free(xmp);



    auto iptc =
        miniexiv_iptc_iterator_create(img);

    assert(iptc);


    while(
        miniexiv_iptc_iterator_next(iptc)
        ==
        MINIEXIV_ITERATOR_HAS_NEXT
    )
    {
        assert(
            miniexiv_iptc_iterator_get_key(iptc)
        );
    }


    miniexiv_iptc_iterator_free(iptc);
}



static void test_buffer()
{
    std::cout<<"[Buffer]\n";


    auto data =
        load_file("test.jpg");


    auto img =
        miniexiv_image_open_buf(
            data.data(),
            data.size()
        );


    assert(img);


    uint8_t* out=nullptr;
    size_t size=0;


    expect_ok(
        miniexiv_image_export_buf(
            img,
            &out,
            &size
        )
    );


    assert(out);
    assert(size>0);


    miniexiv_free_buffer(out);

    miniexiv_image_free(img);
}



static void test_errors()
{
    std::cout<<"[Errors]\n";


    assert(
        miniexiv_image_open_file(nullptr)
        ==
        nullptr
    );


    assert(
        strlen(
            miniexiv_get_last_error()
        )>0
    );


    miniexiv_clear_last_error();


    assert(
        strlen(
            miniexiv_get_last_error()
        )>0
    );
}



int main()
{
    std::cout
        <<"MiniExiv full test\n";


    expect_ok(
        miniexiv_initialize()
    );


    test_version();


    auto img=open_image();


    test_exif(img);

    test_xmp(img);

    test_iptc(img);

    test_dimensions(img);

    test_iterators(img);


    expect_ok(
        miniexiv_image_metadata_write(img)
    );


    miniexiv_image_free(img);



    test_buffer();

    test_errors();


    miniexiv_free_buffer(nullptr);
    miniexiv_free_string(nullptr);


    miniexiv_shutdown();


    std::cout
        <<"ALL TESTS PASSED\n";


    return 0;
}