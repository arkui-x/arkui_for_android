# ArkUI Android平台适配层

-   [简介]
-   [目录介绍]
-   [使用说明]

## 简介

ArkUI是一套构建分布式应用的声明式UI开发框架。它具备简洁自然的UI信息语法、丰富的UI组件、多维的状态管理，以及实时界面预览等相关能力，帮助您提升应用开发效率，并能在多种设备上实现生动而流畅的用户体验。详情可参考[ArkUI框架介绍](https://gitee.com/openharmony/docs/blob/master/zh-cn/application-dev/ui/arkui-overview.md)

ArkUI-X进一步将ArkUI扩展到Android平台，实现对接Android平台的适配，开发者基于一套ArkTS主代码，就可以构建Android平台的精美、高性能应用。

## 目录介绍

ArkUI开发框架的源代码结构参见[代码工程结构及构建说明](https://gitee.com/arkui-x/docs/blob/master/zh-cn/framework-dev/quick-start/project-structure-guide.md)，Android平台的适配代码在/foundation/arkui/ace\_engine/adapter/android下，目录结构如下图所示：

```
/foundation/arkui/ace_engine/adapter/android
├── build                         # 编译配置
├── capability                    # 系统平台能力适配
├── entrance                      # 启动入口相关适配
├── osal                          # 操作系统抽象层
└── stage                         # Stage开发模型适配
```

## 使用说明

### Android 工程创建
通过ACE Tools或DevEco Studio创建一个ArkUI-X应用工程（示例工程名为HelloWorld），其工程目录下的.arkui-x/android文件代表对应的Android工程。Android应用的入口Application和Activity类，这两个类需要继承自ArkUI提供的基类，Activity继承StageActivity类，Application则会继承StageApplication类，Application也可以通过代理类StageApplicationDelegate使用，详情参见[使用说明](https://gitee.com/arkui-x/docs/tree/master/zh-cn/application-dev/reference/arkui-for-android)。
* Activity类
该类名通过通过module名和ability名拼接而得，一个ability对应一个Android工程侧的Activity类。详情参见[Ability使用说明](https://gitee.com/arkui-x/docs/blob/master/zh-cn/application-dev/quick-start/start-with-ability-on-android.md):
    ```java
    package com.example.helloworld;

    import android.os.Bundle;
    import android.util.Log;

    import ohos.stage.ability.adapter.StageActivity;

    public class EntryEntryAbilityActivity extends StageActivity {
        @Override
        protected void onCreate(Bundle savedInstanceState) {
            Log.e("HiHelloWorld", "EntryMainAbilityActivity");
            
            setInstanceName("com.example.helloworld:entry:EntryAbility:");// ArkUI-X应用编译产物在应用工程assets/js中存放的目录名（即模块实例名）。
            super.onCreate(savedInstanceState);
        }
    }
    ```
* Application类
    ```java
    package com.example.helloworld;

    import android.util.Log;

    import ohos.stage.ability.adapter.StageApplication;

    public class MainApplication extends StageApplication {
        private static final String LOG_TAG = "HiHelloWorld";

        private static final String RES_NAME = "res";

        @Override
        public void onCreate() {
            Log.e(LOG_TAG, "MyApplication");
            super.onCreate();
            Log.e(LOG_TAG, "MyApplication onCreate");
        }
    }
    ```


### Android 工程编译
对Android工程编译时，ACE Tools或DevEco Studio会完成两个步骤：

* 集成ArkUI-X SDK \
    Android工程集成ArkUI-X SDK遵循Android应用工程集成Jar和动态库规则，即SDK组成清单中的arkui_android_adapter.jar包拷贝到libs目录，动态库（libarkui_android.so\libhilog_android.so\libhilog.so\libresourcemanager.so）会自动拷贝到libs/arm64-v8a目录。
* 集成ArkUI-X应用编译产物 \
ArkUI-X编译产物生成后，拷贝到Android应用工程assets/arkui-x目录下。这里“arkui-x”目录名称是固定的，不能更改；详情参见[ArkUI-X应用工程结构说明](https://gitee.com/arkui-x/docs/blob/master/zh-cn/application-dev/quick-start/package-structure-guide.md)

    ```
    src/main/assets/arkui-x
        ├── entry
        |   ├── ets
        |   |   ├── modules.abc
        |   |   └── sourceMaps.map
        |   ├── resouces.index
        |   ├── resouces
        |   └── module.json
        └── systemres
    ```



完成上述步骤后即可按照Android应用构建流程，构建ArkUI Android应用，并且可以安装至Android手机后运行。
### 参考

【1】[ArkUI-X Samples仓](https://gitee.com/arkui-x/samples)