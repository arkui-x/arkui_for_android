# ArkUI Android 平台适配层

## 项目概述

ArkUI Android 平台适配层是 ArkUI-X 跨平台框架的 Android 平台实现，通过 JNI 桥接机制实现 ArkTS 应用在 Android 设备上的原生渲染和交互。

**代码位置**: `foundation/arkui/ace_engine/adapter/android/`

## 目录结构

```
adapter/android/
├── entrance/java/                    # 入口层
│   ├── jni/                          # JNI 桥接实现 (C++)
│   │   ├── jni_environment.h         # JNI 环境/类型转换核心
│   │   ├── jni_registry.cpp          # JNI 方法注册
│   │   ├── window_view_jni.cpp       # WindowView JNI 绑定
│   │   ├── mmi_event_convertor.cpp   # 事件转换 (MotionEvent → TouchEvent)
│   │   ├── display_info_jni.cpp      # 显示信息 JNI
│   │   ├── subwindow_manager_jni.cpp # 子窗口管理 JNI
│   │   ├── web_adapter_jni.cpp       # Web 组件 JNI
│   │   ├── ace_env_jni.cpp           # ACE 环境 JNI
│   │   ├── virtual_rs_window.cpp     # 虚拟渲染窗口 (1576 行)
│   │   ├── interaction/              # 交互能力
│   │   ├── html/                     # HTML 转换
│   │   ├── picker/                   # 选择器
│   │   ├── report/                   # 上报能力
│   │   ├── udmf/                     # 统一数据管理框架
│   │   ├── ui_session/               # UI 会话管理
│   │   └── xcollie/                  # 看门狗
│   └── src/ohos/ace/adapter/         # Java 公共类
│       ├── AceEnv.java               # ACE 环境初始化
│       ├── WindowViewInterface.java  # 窗口视图接口
│       ├── WindowViewBuilder.java    # 窗口视图构建器
│       ├── WindowViewAospSurface.java # Surface 渲染实现
│       ├── WindowViewAospTexture.java # Texture 渲染实现
│       ├── ArkUIXPluginRegistry.java # 插件注册中心
│       └── ...
├── stage/                            # Stage 模型适配
│   ├── ability/java/src/             # Activity/Application
│   │   ├── StageActivity.java        # 主 Activity
│   │   ├── StageApplication.java     # 应用入口
│   │   ├── StageActivityDelegate.java
│   │   ├── StageFragment.java
│   │   ├── SubWindow.java
│   │   └── SubWindowManager.java
│   ├── ability/java/jni/             # Stage JNI
│   │   ├── stage_asset_provider.cpp
│   │   ├── ability_context_adapter.cpp
│   │   └── stage_application_delegate_jni.cpp
│   └── uicontent/                    # UI 内容
│       ├── ace_container_sg.cpp      # ACE 容器 (1386 行)
│       ├── ui_content_impl.cpp       # UI 内容实现 (851 行)
│       └── ace_view_sg.cpp           # ACE 视图
├── osal/                             # 平台抽象层 (C++)
│   ├── js_accessibility_manager.cpp  # 无障碍服务 (3354 行, 核心)
│   ├── subwindow_android.cpp         # 子窗口管理 (888 行)
│   ├── display_manager_android.cpp   # 显示器管理
│   ├── resource_adapter_impl.cpp     # 资源适配 (730 行)
│   ├── resource_adapter_impl_v2.cpp  # 资源适配 V2 (589 行)
│   ├── resource_convertor.cpp        # 资源转换
│   ├── image_source_android.cpp      # 图片解码
│   ├── pixel_map_android.cpp         # 位图操作
│   ├── input_method_manager_android.cpp # 输入法管理
│   ├── system_properties.cpp         # 系统属性 (452 行)
│   ├── mouse_style_android.cpp       # 鼠标样式
│   ├── navigation_route.cpp          # 导航路由
│   ├── advance/                      # 高级功能适配
│   │   ├── ai_write_adapter.cpp      # AI 写作
│   │   ├── data_detector_adapter.cpp # 数据检测
│   │   ├── image_analyzer_adapter_impl.cpp # 图片分析
│   │   ├── text_share_adapter.cpp    # 文本分享
│   │   └── text_translation_adapter.cpp # 文本翻译
│   ├── mock/                         # 无障碍模拟实现
│   │   ├── accessibility_element_info.cpp (1047 行)
│   │   ├── accessibility_event_info.cpp
│   │   ├── accessibility_constants.cpp
│   │   └── ...
│   ├── fonts/                        # 字体资源
│   │   └── hwsymbol.xml
│   └── ...
├── capability/java/                  # 平台能力扩展
│   ├── jni/                          # 能力 JNI (C++)
│   │   ├── bridge/                   # JS Bridge
│   │   ├── vibrator/                 # 振动
│   │   ├── storage/                  # 存储
│   │   ├── editing/                  # 编辑
│   │   ├── environment/              # 环境
│   │   ├── font/                     # 字体
│   │   ├── clipboard/                # 剪贴板
│   │   └── ...
│   └── src/ohos/ace/adapter/capability/ # 能力实现 (Java)
│       ├── web/                      # WebView 组件
│       ├── video/                    # 视频播放
│       ├── bridge/                   # JS Bridge
│       ├── platformview/             # 原生视图嵌入
│       ├── surface/                  # Surface 管理
│       ├── texture/                  # 纹理渲染
│       ├── vibrator/                 # 振动反馈
│       ├── storage/                  # 存储能力
│       ├── editing/                  # 文本编辑
│       ├── environment/              # 环境变量
│       └── font/                     # 字体管理
└── build/                            # 构建配置
```

## 核心模块一览

| 模块 | 职责 | 代码位置 | 关键文件 |
|-----|------|---------|---------|
| **Stage 模型** | Activity/Application 生命周期 | `stage/ability/` | StageActivity.java, StageApplication.java |
| **UI 内容** | ACE 容器和 UI 内容管理 | `stage/uicontent/` | ace_container_sg.cpp (1386行) |
| **JNI 桥接** | Java ↔ C++ 双向通信 | `entrance/java/jni/` | jni_environment.h, window_view_jni.cpp |
| **窗口视图** | Surface/Texture 渲染 | `entrance/java/src/` | WindowViewAospSurface.java |
| **平台抽象层** | 系统 API 封装 | `osal/` | js_accessibility_manager.cpp (3354行) |
| **平台能力** | Web/Video/键盘等 | `capability/java/` | web/, video/, bridge/ |

## 分层架构

```
┌─────────────────────────────────────────────────────────────────────┐
│                        ArkTS Application                            │
└─────────────────────────────────────────────────────────────────────┘
                                ↓
┌─────────────────────────────────────────────────────────────────────┐
│              Stage 模型层 (Java)                                     │
│   stage/ability/java/src/                                           │
│   ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐    │
│   │ StageApplication │→│  StageActivity   │→│ StageFragment   │    │
│   └─────────────────┘  └─────────────────┘  └─────────────────┘    │
└─────────────────────────────────────────────────────────────────────┘
                                ↓
┌─────────────────────────────────────────────────────────────────────┐
│              UI 内容层 (C++)                                         │
│   stage/uicontent/                                                   │
│   ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐    │
│   │ AceContainerSg  │←│ UIContentImpl    │←│ AceViewSg        │    │
│   └─────────────────┘  └─────────────────┘  └─────────────────┘    │
└─────────────────────────────────────────────────────────────────────┘
                                ↓
┌─────────────────────────────────────────────────────────────────────┐
│              窗口视图层 (Java)                                       │
│   entrance/java/src/                                                 │
│   ┌─────────────────┐  ┌─────────────────┐                          │
│   │WindowViewAosp   │  │WindowViewTexture │  ← Surface/Texture 渲染 │
│   │    Surface      │  │                  │                          │
│   └─────────────────┘  └─────────────────┘                          │
└─────────────────────────────────────────────────────────────────────┘
                                ↓
┌─────────────────────────────────────────────────────────────────────┐
│              JNI 桥接层 (C++)                                        │
│   entrance/java/jni/                                                 │
│   ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐    │
│   │ JniEnvironment  │  │ WindowViewJni   │  │ MmiEventConvert │    │
│   │ (类型转换/引用)  │  │ (窗口绑定)      │  │ (事件转换)      │    │
│   └─────────────────┘  └─────────────────┘  └─────────────────┘    │
└─────────────────────────────────────────────────────────────────────┘
                                ↓
┌─────────────────────────────────────────────────────────────────────┐
│              平台抽象层 OSAL (C++)                                   │
│   osal/                                                              │
│   ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐    │
│   │DisplayManager   │  │ResourceAdapter  │  │Accessibility    │    │
│   │(显示器管理)     │  │(资源加载)       │  │(无障碍,3354行)  │    │
│   └─────────────────┘  └─────────────────┘  └─────────────────┘    │
│   ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐    │
│   │SubWindow        │  │ImageSource      │  │InputMethod      │    │
│   │(子窗口,888行)   │  │(图片解码)       │  │(输入法)         │    │
│   └─────────────────┘  └─────────────────┘  └─────────────────┘    │
└─────────────────────────────────────────────────────────────────────┘
                                ↓
┌─────────────────────────────────────────────────────────────────────┐
│              平台能力层 Capability (Java + C++)                      │
│   capability/java/                                                   │
│   ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐    │
│   │Web (WebView)    │  │Video (播放器)   │  │Bridge (JS桥)   │    │
│   └─────────────────┘  └─────────────────┘  └─────────────────┘    │
│   ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐    │
│   │PlatformView     │  │Vibrator         │  │Storage          │    │
│   └─────────────────┘  └─────────────────┘  └─────────────────┘    │
└─────────────────────────────────────────────────────────────────────┘
                                ↓
┌─────────────────────────────────────────────────────────────────────┐
│              Android Framework                                       │
│   Activity | Surface | WebView | MediaPlayer | Vibrator | ...      │
└─────────────────────────────────────────────────────────────────────┘
```

## JNI 桥接机制详解

### 1. 类型转换工具 (`jni_environment.h:28-54`)

```cpp
namespace OHOS::Ace::Platform {

// 指针 ↔ jlong 转换 (用于 Native 对象在 Java 层传递)
template<typename T>
inline T* JavaLongToPointer(jlong value) {
    return reinterpret_cast<T*>(static_cast<uintptr_t>(value));
}

template<typename T>
inline jlong PointerToJavaLong(T* ptr) {
    return static_cast<jlong>(reinterpret_cast<uintptr_t>(ptr));
}

// shared_ptr 跨边界传递 (用于需要共享所有权的场景)
template<typename T>
class SharedPtrHolder {
public:
    static jlong Save(const std::shared_ptr<T>& sharedPtr);
    static std::shared_ptr<T> Load(jlong sharedPtrId);
    static void Destroy(jlong sharedPtrId);
};

} // namespace OHOS::Ace::Platform
```

### 2. JNI 引用管理 (`jni_environment.h:93-158`)

```cpp
class JniEnvironment : public NonCopyable {
public:
    // 引用类型定义
    using JavaGlobalRef = std::unique_ptr<JavaObject, JavaGlobalRefDeleter>;
    using JavaLocalRef = std::unique_ptr<JavaObject, JavaLocalRefDeleter>;
    using JavaWeakRef = std::unique_ptr<JavaWeak, JavaWeakRefDeleter>;
    
    // 方法信息结构
    struct JniMethod {
        const char* name;
        const char* signaure;
        void* function;
    };
    
    static JniEnvironment& GetInstance();
    bool Initialize(const std::shared_ptr<JavaVM>& javaVm);
    std::shared_ptr<JNIEnv> GetJniEnv(JNIEnv* jniEnv = nullptr, bool isDetach = true) const;
    
    // 引用管理
    static JavaGlobalRef MakeJavaGlobalRef(const std::shared_ptr<JNIEnv>& jniEnv, jobject object);
    static void DeleteJavaGlobalRef(jobject object);
    static void DeleteJavaLocalRef(jobject object);
};
```

### 3. JNI 方法注册模式 (`window_view_jni.cpp`)

```cpp
// Android 特有方法
static const JNINativeMethod ANDROID_METHODS[] = {
    {
        .name = "nativeSurfaceCreated",
        .signature = "(JLandroid/view/Surface;)V",
        .fnPtr = reinterpret_cast<void*>(&WindowViewJni::SurfaceCreated),
    },
};

// 通用方法 (Surface/Texture 共用)
static const JNINativeMethod COMMON_METHODS[] = {
    { .name = "nativeSurfaceChanged", .signature = "(JIIF)V",
      .fnPtr = reinterpret_cast<void*>(&WindowViewJni::SurfaceChanged) },
    { .name = "nativeSurfaceDestroyed", .signature = "(J)V",
      .fnPtr = reinterpret_cast<void*>(&WindowViewJni::SurfaceDestroyed) },
    { .name = "nativeKeyboardHeightChanged", .signature = "(JI)V",
      .fnPtr = reinterpret_cast<void*>(&WindowViewJni::KeyboardHeightChanged) },
    { .name = "nativeDispatchPointerDataPacket", .signature = "(JLjava/nio/ByteBuffer;I)Z",
      .fnPtr = reinterpret_cast<void*>(&WindowViewJni::DispatchPointerDataPacket) },
    { .name = "nativeDispatchKeyEvent", .signature = "(JIIIJJIII)Z",
      .fnPtr = reinterpret_cast<void*>(&WindowViewJni::DispatchKeyEvent) },
    // ... 更多方法
};
```

### 4. 事件转换 (`mmi_event_convertor.cpp`)

```cpp
// Android MotionEvent → ArkUI TouchEvent
TouchEvent ConvertTouchEvent(JNIEnv* env, jobject motionEvent);

// Android KeyEvent → ArkUI KeyEvent  
KeyEvent ConvertKeyEvent(JNIEnv* env, jobject keyEvent);
```

## Stage 模型适配

### StageActivity 生命周期

```java
// stage/ability/java/src/StageActivity.java

public class StageActivity extends Activity implements KeyboardHeightObserver {
    protected WindowViewInterface windowView;
    protected String instanceName;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // 1. 创建 WindowView (Surface 或 Texture)
        // 2. 初始化 Native ACE 引擎
        // 3. 注册插件 (Web/Video/PlatformView)
    }
    
    // 生命周期: onStart → onResume → onPause → onStop → onDestroy
    // 事件: onBackPressed, onKeyDown, onTouchEvent, onConfigurationChanged
}
```

### StageApplication 初始化

```java
// stage/ability/java/src/StageApplication.java

public class StageApplication extends Application {
    @Override
    public void onCreate() {
        super.onCreate();
        AceEnv.init(this);                                    // 初始化 ACE 环境
        ArkUIXPluginRegistry.getInstance().registerPlugins(this); // 注册插件
    }
}
```

### UI 内容层 (stage/uicontent/)

| 文件 | 行数 | 职责 |
|-----|------|-----|
| `ace_container_sg.cpp` | 1386 | ACE 容器实现，管理 UI 线程和渲染 |
| `ui_content_impl.cpp` | 851 | UI 内容接口实现 |
| `ace_view_sg.cpp` | 405 | ACE 视图实现，处理输入事件 |

## 平台抽象层 (OSAL)

### 核心模块 (按代码量排序)

| 模块 | 文件 | 行数 | 职责 |
|-----|------|------|-----|
| **Accessibility** | `js_accessibility_manager.cpp` | 3354 | 无障碍服务核心 |
| **SubWindow** | `subwindow_android.cpp` | 888 | 子窗口管理 (弹窗/菜单) |
| **ResourceAdapter** | `resource_adapter_impl.cpp` | 730 | 资源加载适配 |
| **ResourceAdapter V2** | `resource_adapter_impl_v2.cpp` | 589 | 资源加载适配 V2 |
| **SystemProperties** | `system_properties.cpp` | 452 | 系统属性获取 |
| **DisplayManager** | `display_manager_android.cpp` | - | 显示器信息管理 |
| **ImageSource** | `image_source_android.cpp` | - | 图片解码 |
| **PixelMap** | `pixel_map_android.cpp` | - | 位图操作 |
| **InputMethod** | `input_method_manager_android.cpp` | - | 输入法管理 |
| **MouseStyle** | `mouse_style_android.cpp` | - | 鼠标光标样式 |
| **NavigationRoute** | `navigation_route.cpp` | - | 导航路由 |

### 高级功能适配 (`osal/advance/`)

| 模块 | 文件 | 职责 |
|-----|------|-----|
| AI Writing | `ai_write_adapter.cpp` | AI 写作辅助 |
| DataDetector | `data_detector_adapter.cpp` | 数据检测 (电话/链接/地址) |
| ImageAnalyzer | `image_analyzer_adapter_impl.cpp` | 图片智能分析 |
| TextShare | `text_share_adapter.cpp` | 文本分享 |
| TextTranslation | `text_translation_adapter.cpp` | 文本翻译 |

### 无障碍模拟层 (`osal/mock/`)

| 文件 | 行数 | 职责 |
|-----|------|-----|
| `accessibility_element_info.cpp` | 1047 | 无障碍元素信息 |
| `accessibility_event_info.cpp` | - | 无障碍事件信息 |
| `accessibility_constants.cpp` | - | 无障碍常量定义 |

## 平台能力层 (Capability)

### 能力模块概览

| 模块 | Java 位置 | JNI 位置 | 职责 |
|-----|----------|---------|-----|
| **Web** | `capability/.../web/` | `web_adapter_jni.cpp` (1259行) | WebView 封装 |
| **Video** | `capability/.../video/` | - | 视频播放组件 |
| **Bridge** | `capability/.../bridge/` | `bridge_jni.cpp` (892行) | JS Bridge 通信 |
| **PlatformView** | `capability/.../platformview/` | - | 原生视图嵌入 |
| **Vibrator** | `capability/.../vibrator/` | `vibrator_proxy_impl.cpp` | 振动反馈 |
| **Storage** | `capability/.../storage/` | `storage/` | 存储能力 |
| **Editing** | `capability/.../editing/` | `editing/` | 文本编辑 |
| **Environment** | `capability/.../environment/` | `environment/` | 环境变量 |
| **Font** | `capability/.../font/` | `font/` | 字体管理 |
| **Surface** | `capability/.../surface/` | - | Surface 管理 |
| **Texture** | `capability/.../texture/` | - | 纹理渲染 |

### Web 组件关键类

```
capability/java/src/ohos/ace/adapter/capability/web/
├── AceWeb.java                    # WebView 主类
├── AceWebBase.java               # WebView 基类
├── AceWebPluginBase.java         # 插件基类
├── AceWebDownloadHelperObject.java
├── AceWebFileChooserObject.java
├── AceWebFullScreenEnterObject.java
├── AceWebJsDialogObject.java
├── AceGeolocationPermissions.java
└── webdatabase/                  # Web 数据库
```

## 关键代码位置索引

### JNI 层 (`entrance/java/jni/`)

| 功能 | 文件 | 说明 |
|-----|------|-----|
| JNI 环境/类型转换 | `jni_environment.h` | 核心类型转换工具 |
| JNI 注册 | `jni_registry.cpp` | 方法注册 |
| WindowView 绑定 | `window_view_jni.cpp` | 窗口 JNI |
| 事件转换 | `mmi_event_convertor.cpp` (386行) | MotionEvent → TouchEvent |
| 触摸事件 | `touch_event_convertor.cpp` | 触摸事件转换 |
| 显示信息 | `display_info_jni.cpp` | 显示器信息 |
| 无障碍 JNI | `js_accessibility_manager_jni.cpp` (460行) | 无障碍 JNI |
| SubWindow JNI | `subwindow_manager_jni.cpp` (861行) | 子窗口 JNI |
| Web JNI | `web_adapter_jni.cpp` (1259行) | Web 组件 JNI |
| 虚拟窗口 | `virtual_rs_window.cpp` (1576行) | 虚拟渲染窗口 |
| 交互能力 | `interaction/interaction_impl.cpp` (402行) | 交互实现 |
| UDMF | `udmf/udmf_impl.cpp` (375行) | 统一数据管理 |

### Stage 模型 (`stage/`)

| 功能 | 文件 | 行数 |
|-----|------|------|
| StageActivity | `ability/java/src/StageActivity.java` | - |
| StageApplication | `ability/java/src/StageApplication.java` | - |
| StageActivityDelegate | `ability/java/src/StageActivityDelegate.java` | - |
| SubWindow | `ability/java/src/SubWindow.java` | - |
| SubWindowManager | `ability/java/src/SubWindowManager.java` | - |
| ACE 容器 | `uicontent/ace_container_sg.cpp` | 1386 |
| UI 内容 | `uicontent/ui_content_impl.cpp` | 851 |
| ACE 视图 | `uicontent/ace_view_sg.cpp` | 405 |
| Stage 资源 | `ability/java/jni/stage_asset_provider.cpp` | 941 |
| 上下文适配 | `ability/java/jni/ability_context_adapter.cpp` | 469 |

### 入口层 Java (`entrance/java/src/ohos/ace/adapter/`)

| 功能 | 文件 |
|-----|------|
| 窗口视图接口 | `WindowViewInterface.java` |
| 窗口视图构建 | `WindowViewBuilder.java` |
| AOSP Surface | `WindowViewAospSurface.java` |
| AOSP Texture | `WindowViewAospTexture.java` |
| AOSP 通用 | `WindowViewAospCommon.java` |
| ACE 环境 | `AceEnv.java` |
| 插件注册 | `ArkUIXPluginRegistry.java` |
| 平台能力 | `AcePlatformCapability.java` |
| 平台插件 | `AcePlatformPlugin.java` |
| 日志 | `ALog.java`, `LoggerAosp.java` |

### OSAL 层 (`osal/`)

| 功能 | 文件 | 行数 |
|-----|------|------|
| 无障碍管理 | `js_accessibility_manager.cpp` | 3354 |
| 子窗口 | `subwindow_android.cpp` | 888 |
| 资源适配 | `resource_adapter_impl.cpp` | 730 |
| 资源适配 V2 | `resource_adapter_impl_v2.cpp` | 589 |
| 系统属性 | `system_properties.cpp` | 452 |
| 显示管理 | `display_manager_android.cpp` | - |
| 图片解码 | `image_source_android.cpp` | - |
| 位图操作 | `pixel_map_android.cpp` | - |
| 输入法 | `input_method_manager_android.cpp` | - |
| 鼠标样式 | `mouse_style_android.cpp` | - |
| 导航路由 | `navigation_route.cpp` | - |

### Capability 层 (`capability/java/`)

| 功能 | 目录 |
|-----|------|
| Web 组件 | `src/ohos/ace/adapter/capability/web/` |
| 视频组件 | `src/ohos/ace/adapter/capability/video/` |
| JS Bridge | `src/ohos/ace/adapter/capability/bridge/` |
| 原生视图 | `src/ohos/ace/adapter/capability/platformview/` |
| 振动 | `src/ohos/ace/adapter/capability/vibrator/` |
| 存储 | `src/ohos/ace/adapter/capability/storage/` |
| 编辑 | `src/ohos/ace/adapter/capability/editing/` |
| 环境 | `src/ohos/ace/adapter/capability/environment/` |
| 字体 | `src/ohos/ace/adapter/capability/font/` |
| Surface | `src/ohos/ace/adapter/capability/surface/` |
| Texture | `src/ohos/ace/adapter/capability/texture/` |

## 关键技术点

### 1. 内存管理规则

```cpp
// 规则 1: Native 对象由 Java 持有，通过 jlong 传递
jlong nativeId = PointerToJavaLong(new NativeObject());
// 释放时:
delete JavaLongToPointer<NativeObject>(nativeId);

// 规则 2: shared_ptr 使用 SharedPtrHolder (需要共享所有权时)
jlong id = SharedPtrHolder<Context>::Save(context);
auto ctx = SharedPtrHolder<Context>::Load(id);
SharedPtrHolder<Context>::Destroy(id);  // 不再需要时销毁

// 规则 3: JNI 引用管理 (避免内存泄漏)
auto globalRef = JniEnvironment::MakeJavaGlobalRef(jniEnv, localRef);
// 自动释放，或手动: JniEnvironment::DeleteJavaGlobalRef(globalRef.get());
```

### 2. JNI 异常处理

```cpp
bool CheckAndClearJNIException(JNIEnv* env) {
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();  // 打印异常堆栈
        env->ExceptionClear();     // 清除异常
        return true;
    }
    return false;
}

// 使用示例
env->CallVoidMethod(obj, methodId);
if (CheckAndClearJNIException(env)) {
    LOGE("JNI call failed");
    return;
}
```

### 3. 线程安全

```cpp
// JNI 调用必须在正确的线程
auto jniEnv = JniEnvironment::GetInstance().GetJniEnv(nullptr, true);
// 参数说明:
// - jniEnv = nullptr: 获取当前线程的 JNIEnv
// - isDetach = true: 如果是子线程，用完后自动 Detach
```

### 4. 条件编译

```cpp
#ifdef ANDROID_PLATFORM
    #include "adapter/android/osal/display_manager_android.h"
#elif defined(IOS_PLATFORM)
    #include "adapter/ios/osal/display_manager_ios.h"
#endif

// 跨平台适配示例
#ifdef IS_ARKUI_X_TARGET
    #ifdef ANDROID_PLATFORM
        // Android 特定实现
    #elif defined(IOS_PLATFORM)
        // iOS 特定实现
    #endif
#endif
```

## 开发注意事项

1. **JNI 调用开销**: 避免频繁的 JNI 调用，使用 ByteBuffer 批量传递数据
2. **线程安全**: JNI 调用需确保 JNIEnv 在正确线程，使用 `GetJniEnv()` 获取
3. **内存泄漏**: 
   - GlobalRef 必须手动释放或使用 `JavaGlobalRef` RAII
   - Native 对象生命周期必须与 Java 对象同步
4. **异常处理**: 每次 JNI 调用后检查异常状态
5. **代码为准**: 修改代码前务必阅读实际源码，以源码实现为准
6. **大型文件注意**: 
   - `js_accessibility_manager.cpp` (3354行) - 无障碍核心
   - `virtual_rs_window.cpp` (1576行) - 虚拟窗口
   - `ace_container_sg.cpp` (1386行) - ACE 容器
   修改这些文件需格外谨慎

## 常见问题排查

| 问题 | 可能原因 | 排查方向 |
|-----|---------|---------|
| JNI 调用崩溃 | JNIEnv 线程不匹配 | 检查是否在正确线程调用 |
| 内存泄漏 | GlobalRef 未释放 | 搜索 `NewGlobalRef` 确认配对释放 |
| 事件不响应 | 事件转换错误 | 检查 `mmi_event_convertor.cpp` |
| 资源加载失败 | 路径适配问题 | 检查 `resource_adapter_impl.cpp` |
| 子窗口异常 | SubWindow 状态管理 | 检查 `subwindow_android.cpp` |
| 无障碍失效 | 元素信息未正确传递 | 检查 `js_accessibility_manager.cpp` |
