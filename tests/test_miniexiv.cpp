#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

#include "miniexiv/miniexiv.h"


static void print_error()
{
    std::cerr 
        << "Error: "
        << miniexiv_get_last_error()
        << "\n";
}


static miniexiv_image* open_test()
{
    auto img = miniexiv_image_open_file("test.jpg");

    if (!img)
    {
        print_error();
        assert(false);
    }

    return img;
}



void test_init()
{
    assert(miniexiv_initialize()==MINIEXIV_OK);

    assert(miniexiv_version()!=nullptr);

    miniexiv_clear_last_error();

    assert(
        strcmp(
            miniexiv_get_last_error(),
            "No Error"
        )==0
    );
}



void test_exif()
{
    auto img=open_test();


    assert(
        miniexiv_image_set_exif_string(
            img,
            "Exif.Image.Artist",
            "MiniExiv"
        )
        ==
        MINIEXIV_OK
    );


    assert(
        miniexiv_image_has_exif_key(
            img,
            "Exif.Image.Artist"
        )
        ==
        MINIEXIV_TRUE
    );


    char* value=nullptr;


    assert(
        miniexiv_image_get_exif_string(
            img,
            "Exif.Image.Artist",
            &value
        )
        ==
        MINIEXIV_OK
    );


    assert(value);

    assert(
        strcmp(
            value,
            "MiniExiv"
        )==0
    );


    miniexiv_free_string(value);



    assert(
        miniexiv_image_remove_exif(
            img,
            "Exif.Image.Artist"
        )
        ==
        MINIEXIV_OK
    );


    assert(
        miniexiv_image_has_exif_key(
            img,
            "Exif.Image.Artist"
        )
        ==
        MINIEXIV_FALSE
    );


    miniexiv_image_free(img);
}




void test_xmp()
{
    auto img=open_test();


    assert(
        miniexiv_image_set_xmp_string(
            img,
            "Xmp.dc.creator",
            "MiniExiv"
        )
        ==
        MINIEXIV_OK
    );


    assert(
        miniexiv_image_has_xmp_key(
            img,
            "Xmp.dc.creator"
        )
        ==
        MINIEXIV_TRUE
    );


    char* value=nullptr;


    assert(
        miniexiv_image_get_xmp_string(
            img,
            "Xmp.dc.creator",
            &value
        )
        ==
        MINIEXIV_OK
    );


    assert(value);


    miniexiv_free_string(value);



    assert(
        miniexiv_image_remove_xmp(
            img,
            "Xmp.dc.creator"
        )
        ==
        MINIEXIV_OK
    );


    miniexiv_image_free(img);
}





void test_iptc()
{
    auto img=open_test();


    assert(
        miniexiv_image_set_iptc_string(
            img,
            "Iptc.Application2.Caption",
            "Test Caption"
        )
        ==
        MINIEXIV_OK
    );


    assert(
        miniexiv_image_has_iptc_key(
            img,
            "Iptc.Application2.Caption"
        )
        ==
        MINIEXIV_TRUE
    );



    char* value=nullptr;


    assert(
        miniexiv_image_get_iptc_string(
            img,
            "Iptc.Application2.Caption",
            &value
        )
        ==
        MINIEXIV_OK
    );


    assert(value);


    miniexiv_free_string(value);



    assert(
        miniexiv_image_remove_iptc(
            img,
            "Iptc.Application2.Caption"
        )
        ==
        MINIEXIV_OK
    );


    miniexiv_image_free(img);
}





void test_exif_iterator()
{
    auto img=open_test();


    auto it=
        miniexiv_exif_iterator_create(img);


    assert(it);


    int count=0;


    while(
        miniexiv_exif_iterator_next(it)
        ==
        MINIEXIV_ITERATOR_HAS_NEXT
    )
    {

        auto key=
            miniexiv_exif_iterator_get_key(it);


        auto value=
            miniexiv_exif_iterator_get_value(it);


        assert(key);
        assert(value);


        count++;
    }


    std::cout
        <<"EXIF count "
        <<count
        <<"\n";


    miniexiv_exif_iterator_free(it);


    miniexiv_image_free(img);
}




void test_export()
{
    auto img=open_test();


    uint8_t* buf=nullptr;
    size_t size=0;


    assert(
        miniexiv_image_export_buf(
            img,
            &buf,
            &size
        )
        ==
        MINIEXIV_OK
    );


    assert(buf);
    assert(size>0);


    miniexiv_free_buffer(buf);


    miniexiv_image_free(img);
}




void test_dimensions()
{
    auto img=open_test();


    uint32_t w=0;
    uint32_t h=0;


    assert(
        miniexiv_image_get_dimensions(
            img,
            &w,
            &h
        )
        ==
        MINIEXIV_OK
    );


    assert(w>0);
    assert(h>0);


    miniexiv_image_free(img);
}





void test_clear()
{
    auto img=open_test();


    assert(
        miniexiv_image_clear(
            img
        )
        ==
        MINIEXIV_OK
    );


    assert(
        miniexiv_image_clear_exif(img)
        ==
        MINIEXIV_OK
    );


    assert(
        miniexiv_image_clear_xmp(img)
        ==
        MINIEXIV_OK
    );


    miniexiv_image_free(img);
}





void test_errors()
{

    assert(
        miniexiv_image_clear(nullptr)
        ==
        MINIEXIV_ERROR
    );


    assert(
        miniexiv_image_set_exif_string(
            nullptr,
            "a",
            "b"
        )
        ==
        MINIEXIV_ERROR
    );


    char* out=nullptr;


    assert(
        miniexiv_image_get_exif_string(
            nullptr,
            "a",
            &out
        )
        ==
        MINIEXIV_ERROR
    );


    std::cout
        <<"Errors OK\n";
}





int main()
{

    test_init();

    test_exif();

    test_xmp();

    test_iptc();

    test_exif_iterator();

    test_export();

    test_dimensions();

    test_errors();


    assert(
        miniexiv_shutdown()
        ==
        MINIEXIV_OK
    );


    std::cout
        <<"ALL TESTS PASSED\n";

    return 0;
}