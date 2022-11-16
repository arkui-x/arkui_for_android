# ArkUI Android Adaptation Layer<a name="EN-US_TOPIC_0000001076213364"></a>

-   [Introduction](#section15701932113019)
-   [Directory Structure](#section1791423143211)
-   [How to Use](#section171384529150)

## Introduction<a name="section15701932113019"></a>

The ArkUI framework empowers OpenHarmony UI development by providing a series of UI components, including basic components, container components, and canvas components. Currently, the ArkUI framework supports web-like programming paradigms and declarative programming paradigms.


**Figure 1** ArkUI framework architecture<a name="fig2606133765017"></a> 
![](https://gitee.com/openharmony/arkui_ace_engine/blob/master/figures/framework-architecture.png "JS-UI framework architecture")

The ArkUI framework consists of the application, framework, engine, and porting layers.

-   **Application**

    This layer contains apps with Feature Abilities (FAs) developed using the JS UI framework. The FA app in this document refers to the app with FAs developed using JavaScript.

-   **Framework**

    This layer parses UI pages and provides the Model-View-ViewModel (MVVM), page routing, custom components and more for front end development.

-   **Engine**

    This layer implements animation parsing, Document Object Model (DOM) building, layout computing, rendering command building and drawing, and event management.

-   **Porting Layer**

    This layer abstracts the platform layer to provide interfaces for the interconnection with the OS. For example, event interconnection, rendering pipeline interconnection, and lifecycle interconnection.

Using the APIs provided by the preceding layers, apps developed with the ArkUI framework will be able to access the Android platform and run on standard Android devices.

## Directory Structure<a name="section1791423143211"></a>

For details about the source code structure of the ArkUI framework, see [ArkUI-X Application Project Structure](https://gitee.com/arkui-x/docs/blob/master/en/framework-dev/quick-start/project-structure-guide.md). The adaptation code for the Android platform is available at **/foundation/arkui/ace\_engine/adapter/android**. The directory structure is as follows:

```
/foundation/arkui/ace_engine/adapter/android
├── build                         # Build configuration
├── capability                    # System capability adaptation
├── entrance                      # Entry-related adaptation
├── osal                          # OS abstraction layer
└── test                          # Test code
```

## How to Use<a name="section171384529150"></a>

You can create an ArkUI cross-platform application project by following instructions in [Application Development](https://gitee.com/arkui-x/docs/blob/master/en/application-dev/README.md). During integrated development on the Android platform, let the **Application** and **Activity** classes of the Android application inherit from the base class of ArkUI, and then call the corresponding APIs to set the development paradigm and ArkUI module instance name for the **Activity** instance. The details are as follows:

Let the **Application** and **Activity** classes of the Android application inherit from the base class of ArkUI. Sample code:

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
        setVersion(VERSION_ETS);            // ArkUI development paradigm: VERSION_JS (JS-compatible web-like development paradigm) or VERSION_ETS (ArkTS-based declarative development paradigm).
        setInstanceName("MainAbility");   // Name of the directory (module instance name) where the ArkUI JSBundle is stored in assets/js of the application project
        super.onCreate(savedInstanceState);
    }
}
```
Use the two APIs provided by **AceActivity** to set the ArkUI module instance name and development paradigm for the current **Activity** instance. Sample code:
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

Up to now, when the app is started, the corresponding ArkUI module source code is automatically loaded and executed for UI rendering and display.
