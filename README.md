# ArkUI Android平台适配层 <a name="ZH-CN_TOPIC_0000001076213364"></a>

-   [简介](#section15701932113019)
-   [目录介绍](#section1791423143211)
-   [使用说明](#section171384529150)

## 简介<a name="section15701932113019"></a>

ArkUI框架是OpenHarmony UI开发框架，提供基础类、容器类、画布类等UI组件，当前支持类Web编程范式和声明式编程范式。


**图 1**  ArkUI框架架构<a name="fig2606133765017"></a>  
![](https://gitee.com/openharmony/arkui_ace_engine/raw/master/figures/JS-UI%E6%A1%86%E6%9E%B6%E6%9E%B6%E6%9E%84.png "JS-UI框架架构")

ArkUI框架包括应用层（Application）、前端框架层（Framework）、引擎层（Engine）和平台适配层（Porting Layer）。

-   **Application**

    应用层表示开发者使用JS UI框架开发的FA应用，这里的FA应用特指JS FA应用。

-   **Framework**

    前端框架层主要完成前端页面解析，以及提供MVVM（Model-View-ViewModel）开发模式、页面路由机制和自定义组件等能力。

-   **Engine**

    引擎层主要提供动画解析、DOM（Document Object Model）树构建、布局计算、渲染命令构建与绘制、事件管理等能力。

-   **Porting Layer**

    适配层主要完成对平台层进行抽象，提供抽象接口，可以对接到系统平台。比如：事件对接、渲染管线对接和系统生命周期对接等。

本项目基于上述**平台适配层**的接口，实现对接Android平台的适配，可以帮助开发者将基于ArkUI开发的应用运行在标准的Android设备上。

## 目录介绍<a name="section1791423143211"></a>

ArkUI开发框架的源代码结构参见[代码工程结构及构建说明](https://gitee.com/arkui-x/docs/blob/master/zh-cn/framework-dev/quick-start/project-structure-guide.md)，Android平台的适配代码在/foundation/arkui/ace\_engine/adapter/android下，目录结构如下图所示：

```
/foundation/arkui/ace_engine/adapter/android
├── build                         # 编译配置
├── capability                    # 系统平台能力适配
├── entrance                      # 启动入口相关适配
├── osal                          # 操作系统抽象层
└── test                          # 测试相关
```

## 使用说明<a name="section171384529150"></a>

参考[应用开发者文档](https://gitee.com/arkui-x/docs/blob/master/zh-cn/application-dev/README.md)可以创建出跨平台应用工程，在Android平台集成开发时，Android应用的入口Application和Activity需要继承自ArkUI-X提供的两个基类AceActivity和AceApplication，并调用相应接口传入开发范式类型以及ArkUI模块实例名称，即可构建ArkUI跨Android平台应用，具体如下：

Android应用的入口Application和Activity需要继承自ArkUI提供的基类，具体如下：

```java
import ohos.ace.adapter.AceApplication;

public class MyApplication extends AceApplication {
    @Override
    public void onCreate() {
        super.onCreate();
    }
}
```

```java
import ohos.ace.adapter.AceActivity;

public class MainActivity extends AceActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        setVersion(VERSION_ETS);            // ArkUI开发范式类型，VERSION_JS:兼容JS的类Web开发范式，VERSION_ETS:基于ArkTS的声明式开发范式。
        setInstanceName("MainAbility");   // ArkUI JSBundle在应用工程assets/js中存放的目录名（即模块实例名）。
        super.onCreate(savedInstanceState);
    }
}
```
其中AceActivity提供两个方法，分别用于设置当前Activity对应的ArkUI模块实例名称和使用的开发范式：
```java
/**
* set the instance name, should called before super.onCreate()
* 
* @param name the instance name to set
*/
public void setInstanceName(String name);

/**
* set arkui app type version
* 
* @param version the version of app type, can be one of this:
*                VERSION_JS / VERSION_ETS, should called before super.onCreate()
*/
public void setVersion(int version);
```

经过上述配置，该应用启动时会自动加载对应的ArkUI模块源码执行并渲染显示。