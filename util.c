#include "util.h"

void jo_core_suspend(void);

char __sgc_last_error[JO_PRINTF_BUF_SIZE] = {0};

// equivalent to __jo_core_error  but works in release mode
// prints an error message to the screen and waits for further
// input
void __sgc_core_error(char *message, const char *function)
{
    jo_vdp2_clear_bitmap_nbg1(JO_COLOR_Blue);
    jo_set_printf_color_index(0);
    jo_printf(0, 13, "   >>> Jo Engine error handler <<<");

    jo_printf_with_color(0, 21, JO_COLOR_INDEX_Red, "In %s():", function);
    jo_printf_with_color(0, 23, JO_COLOR_INDEX_Red, message);
    jo_set_printf_color_index(0);
    jo_printf(2, 27, "Press [START] to continue...");
    jo_core_suspend();
    jo_clear_screen();

    jo_vdp2_clear_bitmap_nbg1(JO_COLOR_Black);
}

void jo_core_suspend(void)
{
    int         wait_cursor;
    int         frame;

    for (JO_ZERO(frame), JO_ZERO(wait_cursor);; ++frame)
    {
        #ifdef JO_COMPILE_WITH_PRINTF_SUPPORT
        if (frame > 4)
        {
            switch (wait_cursor)
            {
                case 0:
                    jo_printf(0, 27, "-");
                    break;
                case 1:
                    jo_printf(0, 27, "/");
                    break;
                case 2:
                    jo_printf(0, 27, "I");
                    break;
            }
            if (wait_cursor == 2)
                JO_ZERO(wait_cursor);
            else
                ++wait_cursor;
            JO_ZERO(frame);
        }
        #endif // JO_COMPILE_WITH_PRINTF_SUPPORT
        #if JO_COMPILE_USING_SGL
        slSynch();
        #else
        jo_input_update();
        jo_wait_vblank_out();
        jo_wait_vblank_in();
        #endif
        if (jo_is_pad1_available() && (jo_is_pad1_key_down(JO_KEY_START)
            #ifdef JO_COMPILE_WITH_KEYBOARD_SUPPORT
            || (jo_keyboard_get_special_key() == JO_KEYBOARD_ENTER)
            #endif // JO_COMPILE_WITH_KEYBOARD_SUPPORT
        ))
        {
            #ifdef JO_COMPILE_WITH_PRINTF_SUPPORT
            jo_printf(0, 27, " ");
            #endif // JO_COMPILE_WITH_PRINTF_SUPPORT
            return ;
        }
    }
}

// hackish way to to clear the screen
// Need to find API way
void clearScreen(void)
{
    for(int i = 0; i < 32; i++)
    {
        jo_printf(0, i, "                                          ");
    }
}

// calculates the MD5 hash of buffer
// md5Hash is an out parameter that must be at least MD5_HASH_SIZE (16) long
// returns 0 on success
int calculateMD5Hash(unsigned char* buffer, unsigned int bufferSize, unsigned char* md5Hash)
{
    MD5_CTX ctx = {0};

    if(buffer == NULL || md5Hash == NULL)
    {
        sgc_core_error("Invalid parameters to calculateMD5Hash!!");
        return -1;
    }

    if(bufferSize == 0)
    {
        jo_memset(md5Hash, 0, bufferSize);
        return 0;
    }

    MD5_Init(&ctx);
    MD5_Update(&ctx, buffer, bufferSize);
    MD5_Final(md5Hash, &ctx);

    return 0;
}
