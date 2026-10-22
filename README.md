# 2026H 循迹竞速小车

2026 电赛 H 题循迹竞速小车，基于 TI MSPM0G3507。采用 CD4051 灰度传感器阵列循迹、
TB6612 双电机差速驱动、编码器里程计，通过按键启动多个比赛任务（Task2 / Task4 / Task5&6）。

## 硬件平台

| 项目 | 说明 |
|------|------|
| 主控 | MSPM0G3507（LQFP-64） |
| 开发板 | LP-MSPM0G3507 LaunchPad |
| SDK | MSPM0 SDK `mspm0_sdk@2.11.00.07` |
| 配置工具 | SysConfig（`targetConfigs/main.syscfg`） |

## 任务说明

| 按键 | 任务 | 模式 |
|------|------|------|
| K1 | Task2 | 巡航 → 检测路口标记后减速停车 |
| K2 | Task4 | 巡航 → 到达指定距离后减速停车（+ 滑行） |
| K3 | Task5/6 | 巡航 → 检测路口标记后停车（+ 滑行） |
| K4 | 急停 | 任意状态回到 IDLE |

运行状态机：`IDLE → RUN → POST_RUN → IDLE`。所有控制逻辑在 10ms 定时器中断中执行，
主循环只负责刷新 OLED 显示。

## 模块结构

| 模块 | 目录 | 功能 |
|------|------|------|
| 循迹 | `Light/` | CD4051 8 路灰度传感器轮询 + 查表计算循迹误差 |
| 电机 | `Motor/` | TB6612 双路电机驱动（PWM + 方向） |
| 运动 | `Move/` | 基本运动指令集（前进/后退/转向/掉头） |
| PWM | `PWM_Car/` | 电机 PWM 定时器初始化与频率/占空比设置 |
| 编码器 | `Encoder/` | MG513X 编码器 4x 正交解码 + 速度/里程计算 |
| PID | `Pid/` | 位置式 PID 控制器（死区 + 积分抗饱和） |
| 按键 | `KEY/` | 4 键扫描（短按 / 长按首次 / 持续长按） |
| 蜂鸣器 | `Buzzer/` | 有源蜂鸣器开关 |
| 显示 | `OLED/` | SSD1306 OLED（I2C） |
| 电磁铁 | `Magnet/` | 3 路电磁铁控制 |
| 舵机 | `Servo/` | 双路舵机角度控制 |
| 延时 | `Delay/` | 毫秒延时 |

## 引脚分配

| 功能 | 引脚 |
|------|------|
| 左轮编码器 A/B | PB10 / PB11 |
| 右轮编码器 A/B | PB12 / PB19 |
| 蜂鸣器 | PB21 |
| 灰度传感器（CD4051 地址线 / 输出） | `GPIO_Follow_AD0/AD1/AD2`、`GPIO_Follow_OUT` |
| 电机 PWM / 方向 | `TIMA0`、`TIMG7` + `GPIO_Motor_*` |
| 按键 K1~K4 | `GPIO_Key_PIN_S1~S4` |
| 电磁铁 | `GPIO_Magnet_PIN_0~2` |
| OLED | `I2C_OLED_INST`（I2C） |

> 完整、准确的引脚定义以 `targetConfigs/main.syscfg` 中的 SysConfig 配置为准。

## 关键硬件约定

- **左轮接线交叉**：左电机 IN1/IN2 与右电机逻辑镜像（`Motor.c` 中 `Car_SetLDirection` 已处理）。
- **右轮编码器镜像安装**：`encoder_update()` 中对右轮增量取反，统一「正数 = 前进」语义。
- **编码器参数**：MG513X（13 线、1:28 减速比、轮径 4.8cm），4x 解码后每圈 1456 PPR。

## 开发环境

- **CCStudio IDE**（Code Composer Studio）
- MSPM0 SDK `2.11.00.07`
- 编译器：TI Arm Clang（tiarmclang）

## 构建与烧录

1. 在 CCS 中导入本工程（打开 `.project` 或 `.ccsproject`）。
2. `Project → Build Project` 编译。
3. 连接 LaunchPad，使用调试配置烧录运行（`targetConfigs/MSPM0G3507.ccxml`）。

## 目录结构

```
2026H/
├── Light/          循迹传感器 + 查表
├── Motor/          电机驱动
├── Move/           运动指令
├── PWM_Car/        PWM 定时器
├── Encoder/        编码器解码
├── Pid/            PID 控制器
├── KEY/            按键扫描
├── Buzzer/         蜂鸣器
├── OLED/           OLED 显示
├── Magnet/         电磁铁
├── Servo/          舵机
├── Delay/          延时
├── Timer/          （空）
├── UART/           （已注释）
├── targetConfigs/  主程序 main.c、SysConfig、调试配置
└── Debug/          构建输出（已 .gitignore 忽略）
```
