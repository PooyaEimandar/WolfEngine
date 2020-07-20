#include "convey.h"
#include <wolf.h>
#include <chrono/w_timespan.h>
#include <chrono/w_chrono.h>
#include <chrono/w_gametime.h>
#include<time.h>
#include<stdint.h>



Main({
    //initialize wolf
    wolf_initialize();

   //testing chrono
   printf("testing chrono");

Convey("chrono", {

    //testing w_timespan
   printf("testing w_timespan");
    w_timespan* _t_0 = w_timespan_init_from_zero();
    So(_t_0->ticks == 0 && !_t_0->overflowed);

    w_timespan* _t_min = w_timespan_init_from_min_value();
    So(_t_min->ticks == -9223372036854775808 && !_t_min->overflowed);

    w_timespan* _t_max = w_timespan_init_from_max_value();
    So(_t_max->ticks == 9223372036854775807 && !_t_max->overflowed);

    w_timespan* _t_now = w_timespan_init_from_now();
    So(_t_now->ticks != 0);

    w_timespan* _t_days = w_timespan_init_from_days(31);
    So(_t_days->ticks == 26784000000000);

    w_timespan* _t_hours = w_timespan_init_from_hours(1);
    So(_t_hours->ticks == 36000000000);

    w_timespan* _t_minu = w_timespan_init_from_minutes(2);
    So(_t_minu->ticks == 1200000000);

    w_timespan* _t_sec = w_timespan_init_from_seconds(3);
    So(_t_sec->ticks == 30000000);

    w_timespan* _t_millisec = w_timespan_init_from_milliseconds(50);
    So(_t_millisec->ticks == 500000);

    w_timespan* _t_tick = w_timespan_init_from_ticks(5);
    So(_t_tick->ticks == 5);

    w_timespan* _t_string = w_timespan_init_from_string("02:02:01:00:000");
    So(_t_string->ticks == 1800600000000);

    w_timespan* _t_wstring = w_timespan_init_from_wstring(L"02:02:01:00:000");
    So(_t_string->ticks == 1800600000000);

    w_timespan* _t_short = w_timespan_init_from_shorttime(2, 2, 2);
    So(_t_short->ticks == 73220000000);

    w_timespan* _t_long = w_timespan_init_from_longtime(3, 4, 5, 8, 10);
    So(_t_long->ticks == 2739080100000);

    w_timespan* _t_add = w_timespan_add(_t_long, _t_short);
    So(_t_add->ticks == 2812300100000);

    w_timespan_add_by_ref(_t_long, _t_short);
    So(_t_long->ticks == 2812300100000);
     
    const char* _time_to_string = w_timespan_to_string(_t_short);
    const char* _time_origin = "0:2:2:2:000";
    int result = strcmp(_time_to_string, _time_origin);
    So(result == 0);

    const wchar_t* _time_to_wstring = w_timespan_to_wstring(_t_short);
    const wchar_t* _time_Worigin = L"0:2:2:2:000";
    int w_result = wcscmp(_time_to_wstring, _time_Worigin);
    So(w_result == 0);

    int _get_days = w_timespan_get_days(_t_long);
    So(_get_days == 3);

    double _get_total_days = w_timespan_get_total_days(_t_tick);
    So(_get_total_days ==5.7870370370370368e-12);

    int _get_hours = w_timespan_get_hours(_t_short);
    So(_get_hours ==2);

    double _get_total_hours = w_timespan_get_total_hours(_t_short);
    So(_get_total_hours == 2.0338888888888889);

    int _get_minutes = w_timespan_get_minutes(_t_short);
    So(_get_minutes == 2);

    double get_total_minutes = w_timespan_get_total_minutes(_t_short);
    So(get_total_minutes == 122.03333333333333);

    int _get_seconds = w_timespan_get_seconds(_t_short);
    So(_get_seconds == 2);

    double _get_total_seconds = w_timespan_get_total_seconds(_t_short);
    So(_get_total_seconds == 7322.0000000000000);

    int _get_milliseconds = w_timespan_get_milliseconds(_t_short);
    So(_get_milliseconds == 0);

    double _get_total_milliseconds = w_timespan_get_total_milliseconds(_t_short);
    So(_get_total_milliseconds ==7322000.0000000000);

   printf("testing w_chrono");

    struct timespec _c_now = w_chrono_now();
    So(_c_now.tv_sec ==0 && _c_now.tv_nsec==3);


    struct timespec _c_now2 = w_chrono_now();
    So(_c_now2.tv_sec ==5 && _c_now2.tv_nsec == 1007);


    struct timespec _c_duration = w_chrono_duration(&_c_now, &_c_now2);
    So(_c_duration.tv_sec == 5 && _c_duration.tv_nsec == 264);

    struct timespec _c_clock = w_chrono_clock_now(1);
    So(_c_clock.tv_sec == 11 && _c_clock.tv_nsec == 665234);

    double _c_now_in_sec = w_chrono_now_in_sec();
    So(_c_now_in_sec == 10.00032875);

    double _c_timespec_to_sec = w_chrono_timespec_to_sec(&_c_now);
    So(_c_timespec_to_sec == 2.0000000000000001e-9);

    double _c_duration_nanoseconds = w_chrono_duration_nanoseconds(&_c_now, &_c_now2);
    So(_c_duration_nanoseconds == 5000000710.0000000);


    double _c_duration_microseconds = w_chrono_duration_microseconds(&_c_now, &_c_now2);
    So(_c_duration_microseconds == 5000000.8350000000);

    double _c__duration_milliseconds = w_chrono_duration_milliseconds(&_c_now, &_c_now2);
    So(_c__duration_milliseconds == 5000.00007800000000);

    double _c_duration_seconds = w_chrono_duration_seconds(&_c_now, &_c_now2);
    So(_c_duration_seconds == 5.0000008429999996);

    printf("testing w_gametime");
    w_gametime _g_t_init = w_gametime_init();
    So(((w_gametime_imp*)_g_t_init)->last_time == 18.000698928999999 && ((w_gametime_imp*)_g_t_init)->fixed_time_step == false && ((w_gametime_imp*)_g_t_init)->max_delta == 313918.00000000000
        && ((w_gametime_imp*)_g_t_init)->elapsed_ticks == 0 && ((w_gametime_imp*)_g_t_init)->total_ticks == 0 && ((w_gametime_imp*)_g_t_init)->left_over_ticks == 0 && ((w_gametime_imp*)_g_t_init)->frame_count == 0
        && ((w_gametime_imp*)_g_t_init)->fps == 0 && ((w_gametime_imp*)_g_t_init)->frames_this_second == 0 && ((w_gametime_imp*)_g_t_init)->seconds_counter == 0.00000000000000000 && ((w_gametime_imp*)_g_t_init)->target_elapsed_ticks == 166666);

    uint64_t _g_t__get_elapsed_ticks = w_gametime_get_elapsed_ticks(_g_t_init);
    So(_g_t__get_elapsed_ticks == 0);

    double _t_g__get_elapsvoided_seconds = w_gametime_get_elapsvoided_seconds(_g_t_init);
    So(_t_g__get_elapsvoided_seconds == 0);

    uint64_t _t_g__get_total_ticks = w_gametime_get_total_ticks(_g_t_init);
    So(_t_g__get_total_ticks == 0);

    double _T_g__get_total_seconds = w_gametime_get_total_seconds(_g_t_init);
    So(_T_g__get_total_seconds == 0);

   uint32_t _t_g__get_frame_count = w_gametime_get_frame_count(_g_t_init);
    So(_t_g__get_frame_count == 0);

    uint32_t _t_g__get_frames_per_second = w_gametime_get_frames_per_second(_g_t_init);
    So(_t_g__get_frames_per_second == 0);

    bool _t_g__get_fixed_time_step = w_gametime_get_fixed_time_step(_g_t_init);
    So(_t_g__get_fixed_time_step == false);

    w_gametime_disable_fixed_time_step(_g_t_init);
    So(((w_gametime_imp*)_g_t_init)->fixed_time_step == false);

    w_gametime_enable_fixed_time_step(_g_t_init);
    So(((w_gametime_imp*)_g_t_init)->fixed_time_step == true);

    w_gametime_set_target_elapsed_ticks(_g_t_init, 5);
    So(((w_gametime_imp*)_g_t_init)->target_elapsed_ticks == 5 );

    w_gametime_set_target_elapsed_seconds(_g_t_init, 7);
    So(((w_gametime_imp*)_g_t_init)->target_elapsed_ticks == 70000000);
});



//terminate wolf
wolf_terminate();

    })

