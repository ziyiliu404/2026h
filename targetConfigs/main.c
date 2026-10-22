/**
 * @file    main.c
 * @brief   2026H 循迹竞速小车主程序
 *
 * 多任务 (Task2/4/5&6) 状态机，K1/K2/K3 启动任务，K4 急停。
 * 编码器: A1=PB10/B1=PB11(左), A2=PB12/B2=PB19(右)，右轮镜像取反。
 * 所有控制逻辑在 10ms 定时器中断中执行，主循环仅刷新 OLED。
 */

#include "ti_msp_dl_config.h"
#include <stdint.h>
#include "oled.h"
#include "Light.h"
#include "Key.h"
#include "Motor.h"
#include "Encoder.h"
#include "Buzzer.h"

#define OLED_REFRESH_DIV                 5U
#define MIN_PWM_FORWARD                  12
#define MAX_PWM_VALUE                   100

/* 各任务巡航/减速速度 */
#define TASK2_CRUISE_SPEED               15
#define TASK2_FINISH_SPEED               11
#define TASK4_CRUISE_SPEED               14
#define TASK4_FINISH_SPEED               10
#define TASK56_CRUISE_SPEED              12
#define TASK56_FINISH_SPEED               9

/* 距离阈值 (cm) */
#define MARK_ARM_DISTANCE_CM         550.0f
#define TASK2_SLOWDOWN_DISTANCE_CM   520.0f
#define TASK4_TIMER_DISTANCE_CM      150.0f
#define TASK4_SLOWDOWN_DISTANCE_CM   120.0f
#define TASK4_EXTRA_DISTANCE_CM       35.0f
#define TASK56_EXTRA_DISTANCE_CM      35.0f

#define STEER_GAIN                     0.0f
#define MARK_DEBOUNCE_COUNT              2U
#define BEEP_TICKS                       20U

typedef enum
{
    MISSION_NONE = 0,
    MISSION_TASK2,
    MISSION_TASK4,
    MISSION_TASK56
} MissionId_t;

typedef enum
{
    CAR_STATE_IDLE = 0,
    CAR_STATE_RUN,
    CAR_STATE_POST_RUN
} CarState_t;

typedef struct
{
    MissionId_t id;
    int16_t cruise_speed;
    int16_t finish_speed;
    float timer_stop_distance_cm;
    float mark_arm_distance_cm;
    float post_run_distance_cm;
    uint8_t stop_on_mark;
    uint8_t stop_on_distance;
} MissionConfig_t;

extern float err;
extern uint8_t line_detected_flag;
extern uint8_t mark_flag;

static volatile uint32_t g_tick_10ms = 0U;
static volatile uint8_t g_oled_refresh_pending = 0U;
static volatile uint8_t g_buzzer_ticks = 0U;
static volatile uint32_t g_time_ticks_10ms = 0U;
static volatile uint8_t g_timer_running = 0U;
static volatile uint8_t g_mark_valid = 0U;

static volatile MissionConfig_t g_mission_cfg = {0};
static volatile CarState_t g_car_state = CAR_STATE_IDLE;

static volatile int16_t g_left_target_speed = 0;
static volatile int16_t g_right_target_speed = 0;

static float g_post_run_start_distance_cm = 0.0f;

static volatile float g_total_distance_cm = 0.0f;
static volatile int16_t g_left_speed_actual = 0;
static volatile int16_t g_right_speed_actual = 0;
static volatile int16_t g_left_pwm = 0;
static volatile int16_t g_right_pwm = 0;

static const MissionConfig_t mission_task2 = {
    .id = MISSION_TASK2,
    .cruise_speed = TASK2_CRUISE_SPEED,
    .finish_speed = TASK2_FINISH_SPEED,
    .timer_stop_distance_cm = 0.0f,
    .mark_arm_distance_cm = MARK_ARM_DISTANCE_CM,
    .post_run_distance_cm = 0.0f,
    .stop_on_mark = 1U,
    .stop_on_distance = 0U
};

static const MissionConfig_t mission_task4 = {
    .id = MISSION_TASK4,
    .cruise_speed = TASK4_CRUISE_SPEED,
    .finish_speed = TASK4_FINISH_SPEED,
    .timer_stop_distance_cm = TASK4_TIMER_DISTANCE_CM,
    .mark_arm_distance_cm = 0.0f,
    .post_run_distance_cm = TASK4_EXTRA_DISTANCE_CM,
    .stop_on_mark = 0U,
    .stop_on_distance = 1U
};

static const MissionConfig_t mission_task56 = {
    .id = MISSION_TASK56,
    .cruise_speed = TASK56_CRUISE_SPEED,
    .finish_speed = TASK56_FINISH_SPEED,
    .timer_stop_distance_cm = 0.0f,
    .mark_arm_distance_cm = MARK_ARM_DISTANCE_CM,
    .post_run_distance_cm = TASK56_EXTRA_DISTANCE_CM,
    .stop_on_mark = 1U,
    .stop_on_distance = 0U
};

static float clamp_float(float value, float min_value, float max_value)
{
    if (value > max_value)
    {
        return max_value;
    }

    if (value < min_value)
    {
        return min_value;
    }

    return value;
}

static int16_t clamp_pwm(int16_t value)
{
    if (value > MAX_PWM_VALUE)
    {
        return MAX_PWM_VALUE;
    }

    if (value < -MAX_PWM_VALUE)
    {
        return -MAX_PWM_VALUE;
    }

    return value;
}

/* 目标速度非零但 PWM 过小时强制最小 PWM，防止电机堵转无法启动 */
static int16_t apply_min_pwm(int16_t pwm, int16_t target_speed)
{
    if ((target_speed > 0) && (pwm < MIN_PWM_FORWARD))
    {
        pwm = MIN_PWM_FORWARD;
    }
    else if ((target_speed < 0) && (pwm > -MIN_PWM_FORWARD))
    {
        pwm = -MIN_PWM_FORWARD;
    }

    return pwm;
}

static float get_total_distance_cm(void)
{
    float dl = get_distance_left_cm();
    float dr = get_distance_right_cm();
    return (dl + dr) * 0.5f;
}

static void beep_once(void)
{
    g_buzzer_ticks = BEEP_TICKS;
    Buzzer_Start();
}

static void stop_car_now(void)
{
    g_left_target_speed = 0;
    g_right_target_speed = 0;
    Motor_Set(0, 0);
    g_left_speed_actual = 0;
    g_right_speed_actual = 0;
    g_left_pwm = 0;
    g_right_pwm = 0;
}

static void mission_abort_to_idle(void)
{
    g_mission_cfg.id = MISSION_NONE;
    g_car_state = CAR_STATE_IDLE;
    g_timer_running = 0U;
    g_time_ticks_10ms = 0U;
    g_post_run_start_distance_cm = 0.0f;
    g_mark_valid = 0U;
    stop_car_now();
    Buzzer_Stop();
}

static void mission_finish_to_idle(void)
{
    g_mission_cfg.id = MISSION_NONE;
    g_car_state = CAR_STATE_IDLE;
    g_timer_running = 0U;
    g_post_run_start_distance_cm = 0.0f;
    g_mark_valid = 0U;
    stop_car_now();
}

static void mission_start(const MissionConfig_t *cfg)
{
    g_mission_cfg = *cfg;
    g_car_state = CAR_STATE_RUN;
    g_timer_running = 1U;
    g_time_ticks_10ms = 0U;
    g_mark_valid = 0U;
    g_post_run_start_distance_cm = 0.0f;

    reset_encoder_count();
    reset_encoder_distance();
    stop_car_now();
    beep_once();
}

static void enter_post_run(void)
{
    g_timer_running = 0U;

    if (g_mission_cfg.post_run_distance_cm > 0.0f)
    {
        g_post_run_start_distance_cm = g_total_distance_cm;
        g_car_state = CAR_STATE_POST_RUN;
    }
    else
    {
        mission_finish_to_idle();
        beep_once();
    }
}

/* K1=Task2, K2=Task4, K3=Task5/6, K4=急停 */
static void process_key_event(void)
{
    if (Key_Get(KEY1) == KEY_SHORT_PRESS)
    {
        mission_start(&mission_task2);
    }

    if (Key_Get(KEY2) == KEY_SHORT_PRESS)
    {
        mission_start(&mission_task4);
    }

    if (Key_Get(KEY3) == KEY_SHORT_PRESS)
    {
        mission_start(&mission_task56);
    }

    if (Key_Get(KEY4) == KEY_SHORT_PRESS)
    {
        mission_abort_to_idle();
    }
}

/* 根据状态和距离计算基准目标速度 */
static int16_t get_base_speed_target(void)
{
    int16_t target_speed = 0;
    float remain_cm = 0.0f;

    if (g_car_state == CAR_STATE_IDLE)
    {
        return 0;
    }

    target_speed = g_mission_cfg.cruise_speed;

    if (g_car_state == CAR_STATE_POST_RUN)
    {
        target_speed = g_mission_cfg.finish_speed;
    }
    else if (g_mission_cfg.id == MISSION_TASK2)
    {
        if (g_total_distance_cm >= TASK2_SLOWDOWN_DISTANCE_CM)
        {
            target_speed = g_mission_cfg.finish_speed;
        }
    }
    else if (g_mission_cfg.id == MISSION_TASK4)
    {
        remain_cm = g_mission_cfg.timer_stop_distance_cm - g_total_distance_cm;
        if (remain_cm <= (g_mission_cfg.timer_stop_distance_cm - TASK4_SLOWDOWN_DISTANCE_CM))
        {
            target_speed = g_mission_cfg.finish_speed;
        }
    }
    else if (g_mission_cfg.id == MISSION_TASK56)
    {
        if (g_total_distance_cm >= TASK2_SLOWDOWN_DISTANCE_CM)
        {
            target_speed = g_mission_cfg.finish_speed;
        }
    }

    if (target_speed < 6)
    {
        target_speed = 6;
    }

    return target_speed;
}

/* 开环直驱: 左=base+err, 右=base-err，最小 PWM + 限幅后输出 */
static void motor_output_update(void)
{
    int16_t base_speed_target;
    float steer;
    int16_t left_pwm;
    int16_t right_pwm;

    if (g_car_state == CAR_STATE_IDLE)
    {
        stop_car_now();
        return;
    }

    base_speed_target = get_base_speed_target();

    steer = err;

    g_left_target_speed  = (int16_t) ((float)base_speed_target + steer + 0.5f);
    g_right_target_speed = (int16_t) ((float)base_speed_target - steer + 0.5f);

    if (g_left_target_speed  > MAX_PWM_VALUE) g_left_target_speed  = MAX_PWM_VALUE;
    if (g_left_target_speed  < 0)             g_left_target_speed  = 0;
    if (g_right_target_speed > MAX_PWM_VALUE) g_right_target_speed = MAX_PWM_VALUE;
    if (g_right_target_speed < 0)             g_right_target_speed = 0;

    g_left_speed_actual  = get_normalized_speed_left();
    g_right_speed_actual = get_normalized_speed_right();

    left_pwm  = g_left_target_speed;
    right_pwm = g_right_target_speed;

    left_pwm  = apply_min_pwm(left_pwm,  g_left_target_speed);
    right_pwm = apply_min_pwm(right_pwm, g_right_target_speed);

    left_pwm  = clamp_pwm(left_pwm);
    right_pwm = clamp_pwm(right_pwm);

    g_left_pwm  = left_pwm;
    g_right_pwm = right_pwm;
    Motor_Set(left_pwm, right_pwm);
}

/* 停车/标记/结束段判断 */
static void mission_logic_update(void)
{
    float post_run_distance_cm;

    g_total_distance_cm = get_total_distance_cm();

    if (g_car_state == CAR_STATE_RUN)
    {
        if (g_mission_cfg.stop_on_distance != 0U)
        {
            if (g_total_distance_cm >= g_mission_cfg.timer_stop_distance_cm)
            {
                enter_post_run();
                beep_once();
            }
        }
        else if (g_mission_cfg.stop_on_mark != 0U)
        {
            /* 距离 > 使能距离 AND 标记消抖有效 */
            if ((g_total_distance_cm >= g_mission_cfg.mark_arm_distance_cm) &&
                (g_mark_valid != 0U))
            {
                enter_post_run();
                beep_once();
            }
        }
    }
    else if (g_car_state == CAR_STATE_POST_RUN)
    {
        post_run_distance_cm = g_total_distance_cm - g_post_run_start_distance_cm;

        if (post_run_distance_cm >= g_mission_cfg.post_run_distance_cm)
        {
            mission_finish_to_idle();
            beep_once();
        }
    }
}

static void oled_show_time(uint8_t x, uint8_t y, uint32_t ticks_10ms)
{
    uint32_t sec = ticks_10ms / 100U;
    uint32_t tenth = (ticks_10ms % 100U) / 10U;

    OLED_ShowString(x, y, "T:", 16);
    OLED_ShowNum(x + 16U, y, sec, 2, 16);
    OLED_ShowChar(x + 32U, y, '.', 16);
    OLED_ShowNum(x + 40U, y, tenth, 1, 16);
    OLED_ShowChar(x + 48U, y, 's', 16);
}

static void oled_update(void)
{
    /* 第0行: 任务名 + 状态 */
    if (g_mission_cfg.id == MISSION_TASK2)
    {
        OLED_ShowString(0, 0, "K1 T2", 16);
    }
    else if (g_mission_cfg.id == MISSION_TASK4)
    {
        OLED_ShowString(0, 0, "K2 T4", 16);
    }
    else if (g_mission_cfg.id == MISSION_TASK56)
    {
        OLED_ShowString(0, 0, "K3 T56", 16);
    }
    else
    {
        OLED_ShowString(0, 0, "IDLE ", 16);
    }

    if (g_car_state == CAR_STATE_RUN)
    {
        OLED_ShowString(72, 0, "RUN ", 16);
    }
    else if (g_car_state == CAR_STATE_POST_RUN)
    {
        OLED_ShowString(72, 0, "POST", 16);
    }
    else
    {
        OLED_ShowString(72, 0, "STOP", 16);
    }

    /* 第2行: 计时器 */
    oled_show_time(0, 2, g_time_ticks_10ms);

    /* 第4行: 距离 + 标记 */
    OLED_ShowString(0, 4, "D:", 16);
    OLED_ShowNum(16, 4, (uint32_t) g_total_distance_cm, 3, 16);
    OLED_ShowString(48, 4, " M:", 16);
    OLED_ShowNum(80, 4, mark_flag, 1, 16);

    /* 第6/7行: 左右轮遥测 (目标/实际/PWM) */
    OLED_ShowString(0, 6, "L", 8);
    OLED_SignedShowNum(6, 6, g_left_target_speed, 2, 8);
    OLED_ShowChar(24, 6, '/', 8);
    OLED_SignedShowNum(30, 6, g_left_speed_actual, 3, 8);
    OLED_ShowString(54, 6, " P", 8);
    OLED_SignedShowNum(66, 6, g_left_pwm, 3, 8);

    OLED_ShowString(0, 7, "R", 8);
    OLED_SignedShowNum(6, 7, g_right_target_speed, 2, 8);
    OLED_ShowChar(24, 7, '/', 8);
    OLED_SignedShowNum(30, 7, g_right_speed_actual, 3, 8);
    OLED_ShowString(54, 7, " P", 8);
    OLED_SignedShowNum(66, 7, g_right_pwm, 3, 8);
}

int main(void)
{
    SYSCFG_DL_init();

    Key_Init();
    OLED_Init();
    OLED_Clear();
    encoder_init();

    mission_abort_to_idle();
    OLED_ShowString(0, 0, "K1:T2 K2:T4", 8);
    OLED_ShowString(0, 2, "K3:T56 K4:STP", 8);

    DL_Timer_startCounter(PWM_Car_INST);
    DL_Timer_startCounter(PWM_Car_Left_INST);

    NVIC_ClearPendingIRQ(TIMER_10ms_INST_INT_IRQN);
    NVIC_EnableIRQ(TIMER_10ms_INST_INT_IRQN);
    DL_TimerG_startCounter(TIMER_10ms_INST);

    while (1)
    {
        if (g_oled_refresh_pending != 0U)
        {
            g_oled_refresh_pending = 0U;
            oled_update();
        }
    }
}

/* 10ms 主调度节拍: 蜂鸣器/计时/按键/编码器/循迹/标记/任务/电机/OLED */
void TIMER_10ms_INST_IRQHandler(void)
{
    static uint8_t mark_stable_count = 0U;
    static uint8_t oled_refresh_div = 0U;

    switch (DL_TimerG_getPendingInterrupt(TIMER_10ms_INST))
    {
        case DL_TIMER_IIDX_ZERO:
        {
            g_tick_10ms++;

            if (g_buzzer_ticks > 0U)
            {
                g_buzzer_ticks--;
                if (g_buzzer_ticks == 0U)
                {
                    Buzzer_Stop();
                }
            }

            if (g_timer_running != 0U)
            {
                g_time_ticks_10ms++;
            }

            Key_Scan();

            encoder_update();

            Track_Process();

            /* 路口标记消抖 */
            if (mark_flag != 0U)
            {
                if (mark_stable_count < 255U)
                {
                    mark_stable_count++;
                }
            }
            else
            {
                mark_stable_count = 0U;
            }

            g_mark_valid = (mark_stable_count >= MARK_DEBOUNCE_COUNT) ? 1U : 0U;

            process_key_event();

            mission_logic_update();

            motor_output_update();

            oled_refresh_div++;
            if (oled_refresh_div >= OLED_REFRESH_DIV)
            {
                oled_refresh_div = 0U;
                g_oled_refresh_pending = 1U;
            }
            break;
        }

        default:
            break;
    }
}
