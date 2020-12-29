#include "bup_header.h"

/*
 *  Format          void BUP_GetDate(unsigned int date, jo_backup_date *date)
 *  Input           pdate : data and time data of directory information
 *                  date : date and time table
 *  Output          none
 *  Function value  none
 *  Function        Expands the date and time data in the directory information table.
 */
void bup_getdate(unsigned int date, jo_backup_date *tb)
{
    unsigned long div;

    /* Set minute count. */
    tb->min = (unsigned char)(date % 60);

    /* Set hour. */
    tb->time = (unsigned char)((date % (60*24)) / 60);

    /* Compute days count. */
    div = date / (60*24);

    /* Set of week. */
    if (div > 0xAB71)
    {
        tb->week = (unsigned char)((div + 1) % 7);
    }
    else
    {
        tb->week = (unsigned char)((div + 2) % 7);
    }

    /* To process leap year, simply consider a pack of 4 years whose first one
     * is a multiple of 4, and loop up to 48 months until getting remaining
     * days count fitting into one month.
     *
     * It's a bit kind of naive (not to say dumb) implementation, but it works,
     * and as it's not the kind of routine that require heavy optimization
     * there's no plan to make this algorithm smarter.
     */
    unsigned long year_base   = div / ((365*4) + 1);
    year_base = year_base * 4;
    unsigned long days_remain = div % ((365*4) + 1);
    const char days_count[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    unsigned char month = 0;
    int i;
    for(i=0; i<(4*12); i++)
    {
        unsigned char days_per_month = days_count[i % 12];
        if(i == 1)
        {
            days_per_month++;
        }

        if(days_remain < days_per_month)
        {
            break;
        }

        days_remain -= days_per_month;
        month++;

        if((i % 12) == 11)
        {
            month = 0;
            year_base++;
        }
    }

    tb->month = month + 1;
    tb->day   = days_remain + 1;
    tb->year  = year_base;


    /* Code below is here to reproduce the same bug as in Saturn BIOS,
     * so please disable it if you prefer a valid date rather than a
     * completely equivalent re-implementation.
     *
     * Description : 1st January following a leap year is incorrectly
     *               returned as December 32nd of this leap year.
     *
     * Examples :
     *  >Error2 #  1, BIO:1980/12/32 00:42 4, B:00080AEA
     *  >           , VMM:1981/01/01 00:42 4
     *  >           , ORG:1981/01/01 00:42
     *  >Error2 #  2, BIO:1980/12/32 07:42 4, B:00080C8E
     *  >           , VMM:1981/01/01 07:42 4
     *  >           , ORG:1981/01/01 07:42
     *  >Error2 #  5, BIO:1984/12/32 00:42 2, B:0028250A
     *  >           , VMM:1985/01/01 00:42 2
     *  >           , ORG:1985/01/01 00:42
     *
     * Note : it is preferable that the implementation here (vmem) should
     *        return a valid date rather than pushing mimic of BIOS behavior
     *        including its bugs.
     */
#if 0
    if((tb->month      == 1)
    && (tb->day        == 1)
    && ((tb->year % 4) == 1))
    {
        tb->month = 12;
        tb->day   = 32;
        tb->year--;
    }
#endif
}

/*
 *  Format          Uint32 BUP_SetDate(BupDate *date)
 *  Input           date : date and time table
 *  Output          none
 *  Function value  Data in date and time data form in the directory information table.
 *  Function        Compresses the date and time data in the directory information table.
 */
unsigned int bup_setdate(jo_backup_date *tb)
{
    /* Table of elapsed days per month. */
    const unsigned short monthtbl[11] =
    {
        31,
        31+28,
        31+28+31,
        31+28+31+30,
        31+28+31+30+31,
        31+28+31+30+31+30,
        31+28+31+30+31+30+31,
        31+28+31+30+31+30+31+31,
        31+28+31+30+31+30+31+31+30,
        31+28+31+30+31+30+31+31+30+31,
        31+28+31+30+31+30+31+31+30+31+30
    };
    unsigned long date;
    unsigned char data;
    unsigned long remainder;


    /* Easter Egg when specified time is set to invalid null values
     *
     * Technical details on SegaXtreme forums :
     *  https://segaxtreme.net/threads/backup-memory-structure.16803/
     *
     * antime   : If you take the 1980/0/0 value and go backwards from
     *            1980/1/1 you get 1963/10/8. I wonder if that's an
     *            easter egg - the programmer's birthday or something similar.
     * TascoDLX : The value in question, 0x008246A0, corresponds to
     *            1996/3/26 0:00, which pretty much coincides with the
     *            passing of Comet Hyakutake.
     *            As far as easter eggs go, that'd be my guess.
     * antime   : The comet wasn't discovered until January 1996.
     *            That'd be hell of an easter egg...
     *
     * Note : in this re-implementation, the value before adding this
     *        easter egg was : t:0x026CECE0 -> 2057/05/15 00:00
     *        Probably something caused by random value read outside range,
     *        or indicating the passing of another new comet ?!
     */
    if((tb->year  == 0)
    && (tb->month == 0)
    && (tb->day   == 0)
    && (tb->time  == 0)
    && (tb->min   == 0))
    {
        return 0x008246A0;
    }

    /* Add year to result. */
    data = tb->year;
    date = (data / 4) * 0x5B5; // 0x5B5 = 365.25 * 4

    remainder = data % 4;
    if(remainder)
    {
        date += (remainder * 0x16D) + 1; // 0x16D = 365
    }

    /* Leap year fix.
     * Code from Yabause HLE BIOS seems broken regarding leap years support.
     *
     * Rather than making a "clean" fix, and also because I'm not smart enough
     * to understand the "365.25 * 4" code above, date is adjusted in cases
     * causing problems.
     *
     * Additionally, I don't want to spend days in optimizing that : writing
     * accurate date manipulation routines is some kind of art that may chew
     * a lot of free time that would be better being dedicated in more
     * interesting projects.
     *
     * Fix is verified under test bench implemented in Save Data Manager's
     * extra menu which compares result between BIOS and this custom BUP_SetDate
     * functions, so that it should behave fine in any possible case until 2199.
     */
    unsigned short year = 1980 + tb->year;
    int leap_year;
    if((year % 4) != 0)
    {
        leap_year = 0;
    }
    else if((year % 100) != 0)
    {
        leap_year = 1;
    }
    else if((year % 400) != 0)
    {
        leap_year = 0;
    }
    else
    {
        leap_year = 1;
    }

    date--;
    if((leap_year) && (tb->month == 2))
    {
        date--;
    }
    if((year > 2000) && ((year % 100) == 0) && (tb->month == 2))
    {
        date--;
    }

    /* Add month to result. */
    data = tb->month;
    if(data != 1 && data < 13)
    {
        date += monthtbl[data - 2];
        if(date > 2 && remainder == 0)
        {
            date++;
        }
    }

    /* Add day to result. */
    date += tb->day;
    date *= 0x5A0;

    /* Add hour to result. */
    date += (tb->time * 0x3C);

    /* Add minute to result. */
    date += tb->min;

    return date;
}

