# 食物选择轮盘 (Food Selection Wheel)

## 项目简介
食物选择轮盘是一个基于Arduino的趣味项目，旨在帮助用户随机选择下一顿吃什么。通过一个圆形屏幕，用户可以看到一个旋转的轮盘，轮盘上标有各种食物选项。用户可以通过触摸屏幕启动轮盘旋转，最终随机选择一个食物选项。

## 功能特点
- **随机选择食物**：通过旋转轮盘随机选择食物。
- **动态动画**：包括启动动画、旋转动画和结果展示动画。
- **触摸交互**：通过触摸屏幕启动轮盘旋转。
- **可定制选项**：支持自定义食物选项和轮盘颜色。

## 硬件需求
- **Arduino XIAO S3**：主控板。
- **圆形TFT屏幕**：用于显示轮盘和动画。
- **触摸传感器**：用于检测用户输入。
- **其他**：电源、连接线等。
- https://www.seeedstudio.com/XIAO-ESP32S3-p-5627.html
- https://www.seeedstudio.com/Seeed-Studio-Round-Display-for-XIAO-p-5638.html

## 软件依赖
您需要在以下链接下载TFT_eSPI、LVGL和Round Screen库。
- **TFT_eSPI库**：用于控制TFT屏幕。https://github.com/Seeed-Projects/SeeedStudio_TFT_eSPI
- **lv_xiao_round_screen库**：用于屏幕初始化和显示。https://github.com/Seeed-Projects/SeeedStudio_lvgl
- **Arduino IDE**：用于代码编译和上传。https://github.com/Seeed-Studio/Seeed_Arduino_RoundDisplay
- **ESP32**：注意是3.0.x版本，在library中下载
- 在File > Preferences，然后在“Additional Boards Manager URLs”中填写以下 URL：https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json

您需要将该lv_conf.h文件剪切到 Arduino 库的根目录中。lv_conf.h这里的文件来自Seeed_Arduino_RoundDisplay

在 Windows 上，Arduino 库的根目录是：
C:\Users\${UserName}\Documents\Arduino\libraries

## 文件结构
- **en.cpp**：主程序文件，包含所有功能的实现。
- **README.md**：项目说明文档。

## 代码功能说明
以下是`en.cpp`文件中主要功能的详细说明：

### 1. 初始化部分
- 定义了屏幕的半径、中心坐标等常量。
- 初始化了TFT屏幕和离屏缓冲区(`TFT_eSprite`)。
- 定义了食物选项数组和轮盘颜色数组。

### 2. 主要功能
#### 2.1 轮盘绘制
- **`drawWheel()`**：绘制轮盘，包括外圈、扇区和中心按钮。
- **`fillSector()`**：优化的扇区绘制函数，使用`fillTriangle`提高绘制效率。

#### 2.2 旋转逻辑
- **`startSpin()`**：启动轮盘旋转，随机生成旋转参数并打乱食物选项。
- **`updateSpin()`**：更新轮盘旋转状态，包括加速、匀速和减速三个阶段。

#### 2.3 动画效果
- **`showStartupAnimation()`**：启动动画，显示项目名称和功能介绍。
- **`showResultAnimation()`**：结果展示动画，突出显示选中的食物选项。

#### 2.4 主循环
- **`loop()`**：主循环函数，控制刷新率、更新旋转状态、绘制轮盘并检测触摸输入。

### 3. 辅助功能
- **`shuffleOptions()`**：随机打乱食物选项的顺序。

## 如何运行
1. **硬件连接**：
   - 将TFT屏幕连接到Arduino XIAO S3。
   - 连接触摸传感器。
   - 确保所有连接正确无误。

2. **软件配置**：
   - 在Arduino IDE中安装对应的库。

3. **代码上传**：
   - 打开`en.cpp`文件。
   - 将代码上传到Arduino XIAO S3。

4. **运行项目**：
   - 通电后，屏幕会显示启动动画。
   - 触摸屏幕启动轮盘旋转，等待结果显示。

5. **Demo**:
   - http://xhslink.com/a/fTYfcxjUTdh7

## 未来改进
### 更多动画
### 更多语言字符
### wifi传输信号
### 多次转盘组合结果
