#ifndef _filters_h
#define _filters_h
#include <cmath>
#include <cstdint>
//=============================================================================
#define FLT_CHECK_NAN
//=============================================================================
#ifdef FLT_CHECK_NAN
#define FLT_CHECK_NAN_IMPL(avalue) \
    if (std::isnan(avalue) || std::isinf(avalue)) { \
        avalue = 0; \
        reset(); \
    }
#else
#define FLT_CHECK_NAN_IMPL(...)
#endif
//=============================================================================
class Derivative
{
public:
    Derivative()
        : index_0(0)
        , index_1(1)
        , index_3(3)
        , index_4(4)
        , init_cnt(0)
    {
        reset();
    }
    float step_angle(float value, const float &dt, const float &span)
    {
        f_dv[index_4] = value;
        float diff = (4.0 / 3.0 / 2.0) * bound(f_dv[index_3] - f_dv[index_1], span) / dt - (1.0 / 3.0 / 4.0) * bound(value - f_dv[index_0], span) / dt;
        return step_next(diff);
    }
    float step(float value, const float &dt)
    {
        f_dv[index_4] = value;
        float diff = (4.0 / 3.0 / 2.0) * (f_dv[index_3] - f_dv[index_1]) / dt - (1.0 / 3.0 / 4.0) * (value - f_dv[index_0]) / dt;
        return step_next(diff);
    }
    void reset(const float &v = 0)
    {
        for (uint8_t i = 0; i < 5; i++)
            f_dv[i] = v;
        init_cnt = 0;
    }

private:
    float f_dv[5];
    uint8_t index_0;
    uint8_t index_1;
    uint8_t index_3;
    uint8_t index_4;
    uint8_t init_cnt;

    float step_next(float diff)
    {
        index_0 = (index_0 + 1) % 5;
        index_1 = (index_1 + 1) % 5;
        index_3 = (index_3 + 1) % 5;
        index_4 = (index_4 + 1) % 5;
        if (init_cnt < 5) {
            init_cnt++;
            return 0;
        }
        FLT_CHECK_NAN_IMPL(diff)
        return diff;
    }
    float bound(float v, float span) const
    {
        const float dspan = span * 2.0;
        return v - std::floor(v / dspan + 0.5) * dspan;
    }
};
//=============================================================================
template<uint8_t window_size = 8>
class Average
{
public:
    Average()
        : inc_index(0)
    //average_inc(0),
    //init_done(false)
    {
        reset();
    }
    float moving_average(const float &value)
    {
        /*if(!init_done){
      init_done=true;
      reset(value);
    }*/
        //average_inc += (value - incs[inc_index]) / (float)window_size;
        incs[inc_index] = value;
        inc_index = (inc_index + 1) % window_size;
        float average_inc = 0;
        for (uint8_t i = 0; i < window_size; i++)
            average_inc += incs[i];
        float v = average_inc / (float) window_size;
        FLT_CHECK_NAN_IMPL(v)
        return v;
    }
    float exp_moving_average(const float &value)
    {
        static float ema = 0;
        ema += (value - ema) * ((float) 2.0 / (float) (window_size + 1));
        FLT_CHECK_NAN_IMPL(ema)
        return ema;
    }
    void reset(const float &v = 0)
    {
        for (uint8_t i = 0; i < window_size; i++)
            incs[i] = v;
        //average_inc=v;
    }

private:
    float incs[window_size];
    uint8_t inc_index;
    //float average_inc;
    //bool init_done;
};
//=============================================================================
template<uint32_t window_size = 1>
class DynamicWindow
{
public:
    DynamicWindow()
        : index(0)
        , middle((window_size + 1) / 2)
    {
        reset();
    }
    void reset(const float &v = 0)
    {
        for (uint32_t i = 0; i < window_size; i++)
            data[i] = v;
    }
    float dyn_window_avg(float val, float K)
    {
        int interval[2], interval_new[2];
        float res = 0;
        int k;
        data[index] = val;
        k = 0;
        dk_calc(K, k, interval);
        while (interval[0] < interval[1]) {
            dk_calc(K, k, interval_new);
            intersect(interval, interval_new, interval);
            k++;
        }
        res = avg(k);
        index = (index + 1) % window_size;
        middle = (middle + 1) % window_size;
        FLT_CHECK_NAN_IMPL(res)
        return res;
    }

private:
    float data[window_size];
    int index;
    int middle;

    float avg(int k)
    {
        float s = data[middle];
        int index_l, index_r;
        for (int i = 1; i <= k; i++) {
            index_r = (middle + i) % window_size;
            index_l = (middle - i >= 0) ? (middle - i) : (middle - i + window_size);
            s += data[index_l] + data[index_r];
        }
        return s / (2 * k + 1);
    }
    void dk_calc(float K, int k, int *interval)
    {
        float s, r;
        s = avg(k);
        r = (2.0f * K) / std::sqrt(2.0f * k + 1.0f);
        interval[0] = s - r;
        interval[1] = s + r;
    }
    static void intersect(int *a, int *b, int *res)
    {
        res[0] = (a[0] > b[0]) ? a[0] : b[0];
        res[1] = (a[1] < b[1]) ? a[1] : b[1];
    }
};
//=============================================================================
template<uint32_t window_size>
class DerivativeX
{
public:
    DerivativeX()
        : index_l(0)
        , index_h(window_size / 2)
        , index(0)
        , init_cnt(0)
        , diff(0)
    {
        reset();
    }
    float step(float value, const float &dt) { return step(value) / dt; }
    float step(const float &value)
    {
        diff += ((value - 2.0 * f_dv[index_h] + f_dv[index_l]) / (float) window_size);
        f_dv[index] = value;
        index = (index + 1) % window_size;
        index_h = (index_h + 1) % window_size;
        index_l = (index_l + 1) % window_size;
        if (init_cnt < window_size) {
            init_cnt++;
            return 0;
        }
        FLT_CHECK_NAN_IMPL(diff)
        return diff;
    }
    void reset(const float &v = 0)
    {
        for (uint32_t i = 0; i < window_size; i++)
            f_dv[i] = v;
        diff = 0;
        init_cnt = 0;
    }

private:
    float f_dv[window_size];
    uint8_t index_l;
    uint8_t index_h;
    uint8_t index;
    uint8_t init_cnt;
    float diff;
};
//=============================================================================
class HPF //Must be reset due to static error
{
public:
    HPF()
        : f_x(0)
        , f_y(0)
    {
        reset();
    }
    float step(const float &value, const float &dt, const float &RC)
    {
        float v = RC / (RC + dt) * (f_y + value - f_x);
        f_x = value;
        f_y = v;
        return v;
    }
    void reset(const float &v = 0) { f_x = f_y = v; }

private:
    float f_x, f_y;
};
//=============================================================================
#endif
